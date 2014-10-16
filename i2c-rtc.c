/*
 * i2c-rtc.c
 *
 *	Funciones de bajo nivel para comunicación I2C con el RTC BQ32000.
 *	Utiliza los pines del P4 especificados en el header (P4.0, P4.1)
 *	como las lineas SDA y SCL.
 *  El estado de los pines cuando estan configurados como salidas es bajo.
 *  El estado por defecto de los pines cuando estan configurados como
 *  entradas es alto por la accion de los pullups presentes en el modulo
 *  del BQ32000.
 *
 *  Created on: Aug 1, 2014
 *  
 */

#include "i2c-rtc.h"

void init_I2C() {
    PxDIR &= ~(SDA | SCL); //SDA y SCL como entradas.
    PxOUT &= ~(SDA | SCL); //Si estan como salidas a 0.
}

static void start() {
    PxDIR &= ~SCL; //reloj en alto
    PxDIR &= ~SDA; //SDA en alto
    __delay_cycles(3);
    PxDIR |= SDA; //SDA en bajo
    __delay_cycles(3);
    PxDIR |= SCL; //reloj en bajo
    __delay_cycles(3); //wait
}

static void stop() {
    PxDIR |= SDA; //SDA en bajo
    __delay_cycles(3); //wait
    PxDIR &= ~SCL; //reloj en alto
    __delay_cycles(3);
    PxDIR &= ~SDA; //SDA en alto
    __delay_cycles(3); //wait
}

static void ack() {
    __delay_cycles(3);
    PxDIR |= SDA; //SDA en bajo
    __delay_cycles(3); //wait
    PxDIR &= ~SCL; //reloj en alto
    __delay_cycles(3); //wait
    PxDIR |= SCL;
    __delay_cycles(3); //wait
    PxDIR &= ~SDA;
    __delay_cycles(3); //wait
}

static void nack() {
    __delay_cycles(3);
    PxDIR &= ~SDA;
    __delay_cycles(3); //wait
    PxDIR &= ~SCL; //reloj en alto
    __delay_cycles(3); //wait
    PxDIR |= SCL;
    __delay_cycles(3); //wait
}

static uint8_t is_ack() {
    if (!(PxIN & SDA)) {
        //"a low on the SDA I/O during the high of the acknowledge-related clock pulse."
        //ACK Recibido
        return 0;
    } else {
        return 1;
    }
}

static uint8_t tx_byte(uint8_t byte) {
    // al terminar la función SDA tiene que estar como entrada
    // y SCL como salida en bajo.
    unsigned int mask = 0x80; //1000 0000
    unsigned int i;

    for (i = 0; i < 8; i++) {
        //send
        if ((byte & mask)) {
            //es un 1
            __delay_cycles(3);
            PxDIR &= ~SDA; //SDA en alto
            __delay_cycles(3); //wait
            PxDIR &= ~SCL; //reloj en alto
            __delay_cycles(3); //wait
            PxDIR |= SCL;
            __delay_cycles(3); //wait
            //PxDIR |= SDA; //SDA ya esta liberado
            __delay_cycles(3); //wait
        } else {
            //es un 0
            __delay_cycles(3);
            PxDIR |= SDA; //SDA en bajo (como salida)
            __delay_cycles(3); //wait
            PxDIR &= ~SCL; //reloj en alto
            __delay_cycles(3); //wait
            PxDIR |= SCL;
            __delay_cycles(3); //wait
            PxDIR &= ~SDA; //libera SDA
            __delay_cycles(3); //wait
        }
        byte <<= 1;
    }
    //Byte transmitido.
    PxDIR &= ~SCL; //reloj en alto
    __delay_cycles(3);
    int res = is_ack();
    PxDIR |= SCL; //reloj en bajo
    __delay_cycles(3);
    return res;
}

static uint8_t rx_byte() {
    unsigned int mask = 0x80;
    char byte = 0x00;
    unsigned int i;
    //wait
    __delay_cycles(3);
    for (i = 0; i < 8; i++) {
        PxDIR &= ~SCL; //reloj en alto
        __delay_cycles(3);
        if ((PxIN & SDA)) {
            byte |= mask; //pin en 1
        } else {
            //pin en 0
        }
        PxDIR |= SCL; //reloj abajo
        mask >>= 1;
        __delay_cycles(3); //wait
    }
    return byte; //ack(); //enviar ack
}

uint8_t leer_registro_I2C(uint8_t reg) {
    unsigned int res = 1;
    start();
    if (tx_byte(WRITEADDRESS)) {
        return 1;
    } //ack no recibido
    if (tx_byte(reg)) {
        return 1;
    }
    start();
    if (tx_byte(READADDRESS)) {
        return 1;
    }
    res = rx_byte();
    nack();
    stop();
    return res;

}

uint8_t escribir_registro_I2C(uint8_t reg, uint8_t valor) {
    start();
    if (tx_byte(WRITEADDRESS)) {
        return 1;
    }
    if (tx_byte(reg)) {
        return 1;
    }
    if (tx_byte(valor)) {
        return 1;
    }
    stop();
    return 0;
}

/*readI2C y writeI2C: Funciones utiles para leer y escribir varios
 *  registros hacia o desde un buffer */
uint8_t readI2C(uint8_t subaddress, uint8_t * buff, uint8_t size) {
    //secuencia de Start
    start();
    if (tx_byte(WRITEADDRESS)) {
        return 1;
    }
    if (tx_byte(subaddress)) {
        return 1;
    }
    start();
    if (tx_byte(READADDRESS)) {
        return 1;
    }
    unsigned int i;
    for (i = 0; i < size - 1; i++) {
        buff[i] = rx_byte();
        ack();
    }
    buff[i] = rx_byte();
    nack();
    stop();
    return 0;
}

uint8_t writeI2C(uint8_t subaddress, uint8_t * buff, uint8_t size) {
    start();
    if (tx_byte(WRITEADDRESS)) {
        return 1;
    }
    if (tx_byte(subaddress)) {
        return 1;
    }
    unsigned int i;
    for (i = 0; i < size; i++) {
        if (tx_byte(buff[i])) {
            return 1;
        }
    }
    stop();
    return 0;
}

/*fecha_hora: Escribe en timestamp la fecha y hora en formato AAMMDDThhmmss*/
uint8_t fecha_hora(uint8_t * timestamp) {
    //YYMMDDThhmmss
    char datos[7];
    //char timestamp[14];
    if (!(readI2C(0x00, datos, 7))) {
        int j = 0;
        int i = 13;
        //segs
        char u_seg, d_seg;
        u_seg = datos[j] & 0x0f;
        u_seg += 0x30; //conversion a ascii
        d_seg = datos[j++] & 0x70;
        d_seg >>= 4;
        d_seg += 0x30;
        timestamp[i--] = '\0';
        timestamp[i--] = u_seg;
        timestamp[i--] = d_seg;

        //min
        char u_min, d_min;
        u_min = datos[j] & 0x0f;
        u_min += 0x30; //conversion a ascii
        d_min = datos[j++] & 0x70;
        d_min >>= 4;
        d_min += 0x30;
        timestamp[i--] = u_min;
        timestamp[i--] = d_min;

        //hora
        char u_hora, d_hora;
        u_hora = datos[j] & 0x0f;
        u_hora += 0x30; //conversion a ascii
        d_hora = datos[j++] & 0x30;
        d_hora >>= 4;
        d_hora += 0x30;
        timestamp[i--] = u_hora;
        timestamp[i--] = d_hora;

        //descartar dia
        j++;

        //fecha
        char u_fecha, d_fecha;
        u_fecha = datos[j] & 0x0f;
        u_fecha += 0x30; //conversion a ascii
        d_fecha = datos[j++] & 0x30;
        d_fecha >>= 4;
        d_fecha += 0x30;
        timestamp[i--] = 'T';
        timestamp[i--] = u_fecha;
        timestamp[i--] = d_fecha;

        //mes
        char u_mes, d_mes;
        u_mes = datos[j] & 0x0f;
        u_mes += 0x30; //conversion a ascii
        d_mes = datos[j++] & 0x10;
        d_mes >>= 4;
        d_mes += 0x30;
        timestamp[i--] = u_mes;
        timestamp[i--] = d_mes;

        //año
        char u_year, d_year;
        u_year = datos[j] & 0x0f;
        u_year += 0x30; //conversion a ascii
        d_year = datos[j] & 0xf0;
        d_year >>= 4;
        d_year += 0x30;
        timestamp[i--] = u_year;
        timestamp[i--] = d_year;

        return 0;
    } else {
        return 1;
    }
}

uint8_t falla_oscilador(){
    uint8_t registro;
    registro = leer_registro_I2C(0x01);
    if(registro & 0x80){
        return 1;   //Falla de oscilador detectada.
    }else{
        return 0;   //No hay falla.
    }
}
