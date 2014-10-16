/*
 * i2c-rtc.h
 *
 *  Funciones de bajo nivel para comunicación I2C con el RTC BQ32000.
 *  Utiliza los pines especificados como las lineas SDA y SCL.
 *  El estado de los pines cuando estan configurados como salidas es bajo.
 *  El estado por defecto de los pines cuando estan configurados como
 *  entradas es alto por la accion de los pullups presentes en el modulo
 *  del BQ32000.
 *
 *  Created on: Aug 1, 2014
 * 
 */

#ifndef I2C_RTC_H_
#define I2C_RTC_H_

#include <msp430.h>
#include <stdint.h>

/*Puerto y pines correspondientes al I2C*/
#define PxDIR P1DIR
#define PxOUT P1OUT
#define PxIN P1IN
#define SDA BIT7
#define SCL BIT6
/****************************************/

#define READADDRESS 0xd1
#define WRITEADDRESS 0xd0

void init_I2C();        //Inicializacion

static void ack();      //Funciones internas
static void nack();
static uint8_t is_ack();
static void start();
static void stop();
static uint8_t rx_byte();
static uint8_t tx_byte(uint8_t byte);
static uint8_t readI2C(uint8_t subaddress, uint8_t * buff, uint8_t size);
static uint8_t writeI2C(uint8_t subaddress, uint8_t * buff, uint8_t size);

uint8_t leer_registro_I2C(uint8_t reg);
uint8_t escribir_registro_I2C(uint8_t reg, uint8_t valor);

uint8_t fecha_hora(uint8_t * ts);
uint8_t falla_oscilador();

#endif /* I2C_RTC_H_ */
