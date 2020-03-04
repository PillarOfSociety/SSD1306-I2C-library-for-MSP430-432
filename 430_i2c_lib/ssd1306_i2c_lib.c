/*
 * AJS 2-21-20
 * Origonal code from SSD1306-I2C-library-for-MSP430-432 github
 *
 * Handles low level I2C stuff for interfacing with Screen
 */

//******************************************************************************************************************************************
//                 MSP430FR5994
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |                 |
//            |                 |
//            |         SDA/P7.0|------->  //was P1.2
//            |         SCK/P7.1|------->   //was P1.3
//******************************************************************************************************************************************

#include <msp430.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "ssd1306_i2c_lib.h"

#define SLAVE_ADDRESS   0x3C    //uint8_t SlaveAddress = 0x3C;

uint8_t TXByteCtr;
unsigned char *TI_transmit_field;

//******************************************************************************************************************************************
void i2c_init () {
    // Configure GPIO
    P7SEL0 |= BIT0 | BIT1;  //Change pins from origonal github
    P7SEL1 &= ~(BIT0 | BIT1);

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Configure USCI_B2 for I2C mode
    UCB2CTLW0 = UCSWRST;                    // put eUSCI_B in reset state
    UCB2CTLW0 |= UCMODE_3 | UCMST | UCSSEL__SMCLK; // I2C master mode, SMCLK
    UCB2BRW = 0x8;                          // baudrate = SMCLK / 8
    UCB2CTLW0 &= ~UCSWRST;                  // clear reset register
    UCB2IE |= UCTXIE0 | UCNACKIE;           // transmit and NACK interrupt enable
    __enable_interrupt();
}
//******************************************************************************************************************************************
void i2c_transmit (unsigned char *params, unsigned char flag) {

    __delay_cycles(1000);               // Delay between transmissions  //TODO move this elsewhere
    UCB2I2CSA = SLAVE_ADDRESS;// configure slave address
    TXByteCtr = flag;                      // Load TX byte counter
    while (UCB2CTLW0 & UCTXSTP);        // Ensure stop condition got sent

    UCB2CTLW0 |= UCTR | UCTXSTT;        // I2C TX, start condition

    __bis_SR_register(LPM0_bits | GIE); // Enter LPM0 w/ interrupts

}
//******************************************************************************************************************************************
// I2C interrupt service routine

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = EUSCI_B2_VECTOR
__interrupt void USCI_B2_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(EUSCI_B2_VECTOR))) USCI_B2_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(UCB2IV, USCI_I2C_UCBIT9IFG))
       {
           case USCI_NONE:          break;     // Vector 0: No interrupts
           case USCI_I2C_UCALIFG:   break;     // Vector 2: ALIFG
           case USCI_I2C_UCNACKIFG:            // Vector 4: NACKIFG
               //UCB2CTL1 |= UCTXSTT;            // I2C start condition
               UCB2CTLW0 |= UCTXSTT;           // resend start if NACK
               break;
           case USCI_I2C_UCSTTIFG:  break;     // Vector 6: STTIFG
           case USCI_I2C_UCSTPIFG:  break;     // Vector 8: STPIFG
           case USCI_I2C_UCRXIFG3:  break;     // Vector 10: RXIFG3
           case USCI_I2C_UCTXIFG3:  break;     // Vector 12: TXIFG3
           case USCI_I2C_UCRXIFG2:  break;     // Vector 14: RXIFG2
           case USCI_I2C_UCTXIFG2:  break;     // Vector 16: TXIFG2
           case USCI_I2C_UCRXIFG1:  break;     // Vector 18: RXIFG1
           case USCI_I2C_UCTXIFG1:  break;     // Vector 20: TXIFG1
           case USCI_I2C_UCRXIFG0:             // Vector 22: RXIFG0
               //RXData = UCB2RXBUF;             // Get RX data
               //__bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
               break;
           case USCI_I2C_UCTXIFG0:      // Vector 24: TXIFG0
               if (TXByteCtr)                  // Check TX byte counter
               {
                   UCB2TXBUF = *TI_transmit_field;//TXData[SlaveFlag];  // Load TX buffer
                   TI_transmit_field++;
                   TXByteCtr--;                // Decrement TX byte counter
               }
               else
               {
                   UCB2CTLW0 |= UCTXSTP;       // I2C stop condition
                   UCB2IFG &= ~UCTXIFG;        // Clear USCI_B2 TX int flag
                   __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
               }
               break;
           case USCI_I2C_UCBCNTIFG:            // Vector 26: BCNTIFG
               P1OUT ^= BIT0;                  // Toggle LED on P1.0
               break;
           case USCI_I2C_UCCLTOIFG: break;     // Vector 28: clock low timeout
           case USCI_I2C_UCBIT9IFG: break;     // Vector 30: 9th bit
           default: break;
       }

    /* origonal to github
  switch(__even_in_range(UCB2IV,USCI_I2C_UCBIT9IFG)) {
  case USCI_I2C_UCNACKIFG:
      UCB2CTL1 |= UCTXSTT;                      //resend start if NACK
      break;                                      // Vector 4: NACKIFG break;
  case USCI_I2C_UCTXIFG0:
      if (TXByteCtr)  {                              // Check TX byte counter
          UCB2TXBUF = *TI_transmit_field;
          TI_transmit_field++;
          TXByteCtr--;                              // Decrement TX byte counter
      } else {
          UCB2CTLW0 |= UCTXSTP;                     // I2C stop condition
          UCB2IFG &= ~UCTXIFG;                      // Clear USCI_B0 TX int flag
      }
      break;                                      // Vector 26: TXIFG0 break;

  }*/
}
