/*
 * bl.c
 *
 *  Created on: 2017年10月10日
 *      Author: houxd
 */

#include "bl.h"
#include <stdint.h>
#include "S32K144.h" /* include peripheral declarations S32K144 */
#include <stdio.h>
#include "SPI_MSD0_Driver.h"
#include "clocks_and_modes.h"
#include "FlexCAN.h"

void WDOG_disable(void) {
	WDOG->CNT = 0xD928C520; /*Unlock watchdog*/
	WDOG->TOVAL = 0x0000FFFF; /*Maximum timeout value*/
	WDOG->CS = 0x00002100; /*Disable watchdog*/
}

void NVIC_init_IRQs (void) {
	S32_NVIC->ICPR[1] = 1 << (LPUART0_RxTx_IRQn % 32); /* IRQ48-LPIT0 ch0: clr any pending IRQ*/
	S32_NVIC->ISER[1] = 1 << (LPUART0_RxTx_IRQn % 32); /* IRQ48-LPIT0 ch0: enable IRQ */
	S32_NVIC->IP[LPUART0_RxTx_IRQn] =0x0f; /* IRQ48-LPIT0 ch0: priority 10 of 0-15*/

	S32_NVIC->ICPR[1] = 1 << (LPUART1_RxTx_IRQn % 32); /* IRQ48-LPIT0 ch0: clr any pending IRQ*/
	S32_NVIC->ISER[1] = 1 << (LPUART1_RxTx_IRQn % 32); /* IRQ48-LPIT0 ch0: enable IRQ */
	S32_NVIC->IP[LPUART1_RxTx_IRQn] =0x0f; /* IRQ48-LPIT0 ch0: priority 10 of 0-15*/

	S32_NVIC->ICPR[1] = 1 << (LPUART2_RxTx_IRQn % 32); /* IRQ48-LPIT0 ch0: clr any pending IRQ*/
	S32_NVIC->ISER[1] = 1 << (LPUART2_RxTx_IRQn % 32); /* IRQ48-LPIT0 ch0: enable IRQ */
	S32_NVIC->IP[LPUART2_RxTx_IRQn] =0x0f; /* IRQ48-LPIT0 ch0: priority 10 of 0-15*/

//	  S32_NVIC->ICPR[1] = 1 << (48 % 32);  /* IRQ48-LPIT0 ch0: clr any pending IRQ*/
//	  S32_NVIC->ISER[1] = 1 << (48 % 32);  /* IRQ48-LPIT0 ch0: enable IRQ */
//	  S32_NVIC->IP[48] = 0xA;              /* IRQ48-LPIT0 ch0: priority 10 of 0-15*/
}

void LPIT0_init (void) {
  PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(6);    /* Clock Src = 6 (SPLL2_DIV2_CLK)*/
  PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clk to LPIT0 regs */
  LPIT0->MCR = 0x00000001;    /* DBG_EN-0: Timer chans stop in Debug mode */
                              /* DOZE_EN=0: Timer chans are stopped in DOZE mode */
                              /* SW_RST=0: SW reset does not reset timer chans, regs */
                           /* M_CEN=1: enable module clk (allows writing other LPIT0 regs)*/
  LPIT0->MIER = 0x00000001;   /* TIE0=1: Timer Interrupt Enabled fot Chan 0 */
  LPIT0->TMR[0].TVAL = 40000;//1m tick 000;    /* Chan 0 Timeout period: 40M clocks */
  LPIT0->TMR[0].TCTRL = 0x00000001; /* T_EN=1: Timer channel is enabled */
                              /* CHAIN=0: channel chaining is disabled */
                              /* MODE=0: 32 periodic counter mode */
                              /* TSOT=0: Timer decrements immediately based on restart */
                              /* TSOI=0: Timer does not stop after timeout */
                              /* TROT=0 Timer will not reload on trigger */
                              /* TRG_SRC=0: External trigger soruce */
                              /* TRG_SEL=0: Timer chan 0 trigger source is selected*/
}

extern void nic_io_init();
void PORT_init (void) {
	//PD8 for buzzer
	PCC->PCCn[PCC_PORTD_INDEX] = PCC_PCCn_CGC_MASK; /* Enable clock for PORT D */
	PTD->PDDR |= 1 << 8; 					/* Port D8:  Data Direction= output */
	PORTD->PCR[8] |= PORT_PCR_MUX(1);		 /* Port D8:  MUX = ALT1, GPIO (to blue LED on EVB) */
	//Buzzer init low for disable
	PTD-> PCOR |= 1<<8;

	//can0
	PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clock for PORTE */
	PORTC->PCR[2] |= PORT_PCR_MUX(3); /* Port E4: MUX = ALT5, CAN0_RX */
	PORTC->PCR[3] |= PORT_PCR_MUX(3); /* Port E5: MUX = ALT5, CAN0_TX */
	//can1
	PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clock for PORTE */
	PORTC->PCR[6] |= PORT_PCR_MUX(3); /* Port E4: MUX = ALT5, CAN0_RX */
	PORTC->PCR[7] |= PORT_PCR_MUX(3); /* Port E5: MUX = ALT5, CAN0_TX */
	//can2
	PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clock for PORTE */
	PORTC->PCR[16] |= PORT_PCR_MUX(3); /* Port E4: MUX = ALT5, CAN0_RX */
	PORTC->PCR[17] |= PORT_PCR_MUX(3); /* Port E5: MUX = ALT5, CAN0_TX */

	//uart0
	PCC->PCCn[PCC_PORTB_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clock for PORTB */
	PORTB->PCR[0] |= PORT_PCR_MUX(3); /* Port C6: MUX = ALT2,UART0 TX */
	PORTB->PCR[1] |= PORT_PCR_MUX(3); /* Port C7: MUX = ALT2,UART0 RX */
	//uart1
	PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clock for PORTC */
	PORTD->PCR[13] |= PORT_PCR_MUX(3); /* Port C6: MUX = ALT2,UART1 TX */
	PORTD->PCR[14] |= PORT_PCR_MUX(3); /* Port C7: MUX = ALT2,UART1 RX */
	//uart2
	PCC->PCCn[PCC_PORTA_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clock for PORTC */
	PORTA->PCR[8] |= PORT_PCR_MUX(2); /* Port C6: MUX = ALT2,UART1 TX */
	PORTA->PCR[9] |= PORT_PCR_MUX(2); /* Port C7: MUX = ALT2,UART1 RX */

	//4g
	nic_io_init();

	///DS3231SN
	//PTA0 INT  PTA1 RST
	PCC->PCCn[PCC_PORTA_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clock for PORTC */
	PORTA->PCR[0] |= PORT_PCR_MUX(1); /* Port C6: MUX = ALT2,UART1 TX */
	PORTA->PCR[1] |= PORT_PCR_MUX(1); /* Port C7: MUX = ALT2,UART1 RX */
	PTA->PDDR |= 1 << 0; 					/* Port D8:  Data Direction= output */
	PTA->PDDR |= 1 << 1; 					/* Port D8:  Data Direction= output */

	//PB9 I2C0 SCL, PB10 SDA
	PCC->PCCn[PCC_PORTB_INDEX] = PCC_PCCn_CGC_MASK; /* Enable clock for PORT D */
	PORTB->PCR[9] |= PORT_PCR_MUX(1);		 /* Port D8:  MUX = ALT1, GPIO (to blue LED on EVB) */
	PORTB->PCR[10] |= PORT_PCR_MUX(1);		 /* Port D8:  MUX = ALT1, GPIO (to blue LED on EVB) */
	PTB->PDDR |= 1 << 9; 					/* Port D8:  Data Direction= output */
	PTB->PDDR |= 1 << 10; 					/* Port D8:  Data Direction= output */
	PTB-> PSOR |= 1<<9;
	PTB-> PSOR |= 1<<10;

	//PTA16  DS2411R
	PCC->PCCn[PCC_PORTA_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clock for PORTC */
	PORTA->PCR[16] |= PORT_PCR_MUX(1); /* Port C6: MUX = ALT2,UART1 TX */
	PTA->PDDR |= 1 << 16; 					/* Port D8:  Data Direction= output */

}
inline void ds2411_dir(int d)
{
	if(d) PTA->PDDR |= 1 << 16;
	else PTA->PDDR &= ~(1 << 16);
}
inline void ds2411_line(int v)
{
	if(v) PTA-> PSOR |= 1<<16;
	else PTA-> PCOR |= 1<<16;
}






static volatile uint32_t tickCountVal;
uint32_t get_tick_count()
{
	return tickCountVal;
}
extern inline void vApplicationTickHook(void)
{
	tickCountVal++;
}

void delay_ms(int ms)
{
	uint32_t t = get_tick_count() + ms;
	while(t>get_tick_count());
}

void buzzer_ctrl(int en)
{
	if(en)
		PTD-> PSOR |= 1<<8;
	else
		PTD-> PCOR |= 1<<8;
}

