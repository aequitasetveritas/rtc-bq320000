//***************************************************************************************
//  Poner en hora rtc BQ32000: Programa pensado para ser utilizado junto 
//  con poner_en_hora_rtc.py
//  Para iniciar, el programa espera el envio del valor 0xc1, al cual respondera con 0xc2
//  Luego espera recibir año,mes,fecha,diadelasemana,hora,minuto,segundo. Con cada dato 
//  recibido respondera con 0xc3, 0xc4, 0xc5, 0xc6...etc.
//  El comando 0xbb retornara el resultado de fecha_hora().
//  Configuraciones de los pines I2C, SDA y SCL en el header i2c-rtc.h  
//***************************************************************************************

#include <msp430.h>				
#include <msp430g2955.h>
#include <ctes_calibracion.h>
#include <i2c-rtc.h>
#include <uart.h>
#include <stdint.h>

int r=0; //Contador para el buffer de recepcion

int main(void) {
	WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    grabar_cts_DCO_955();
    init_UART();


    P4DIR |= BIT4 | BIT3;
    P4OUT &= ~(BIT4 | BIT3);
    init_I2C();

    P2DIR &= ~BIT5;
    P2REN |= BIT5;
    P2OUT &= ~BIT5; //P2.5 como entrada con pullup


    IE2 |= UCA0RXIE;                        // Enable USCI_A0 RX interrupt

  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0, interrupts enabled
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)
#else
#error Compiler not supported!
#endif
{
  while (!(IFG2 & UCA0TXIFG));                // USCI_A0 TX buffer ready?
  P4DIR |= BIT3 | BIT4;
  
  uint8_t tiempo[14];
  switch(UCA0RXBUF){
      case 0xc1:
          UCA0TXBUF = 0xc2;
          r = 0; 
          break;
      case 0xbb:
          fecha_hora(tiempo);
          send_uart(tiempo);
          break;
      default:
          if(escribir_registro_I2C(0x06 - r, UCA0RXBUF)){
            P4OUT|=BIT3;
          }
          UCA0TXBUF = 0xc3 + r;
          if(r == 0x06){
            r=0;
          }else{
            r++;
          }
          
  }
}