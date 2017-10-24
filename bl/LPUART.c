/*
 * LPUART.c              Copyright NXP 2016
 * Description: LPUART functions
 * 2015 Sept 28 Kushal Shaw - original version AN5213;
 * 2016 Mar 17  O Romero - ported to S32K144;
 *
 */

#include "S32K144.h" /* include peripheral declarations S32K144 */
#include "LPUART.h"

/*
int calc_opt_div(int bps, float *errpto){
	float min_val=1.f;
	int min_index=0;
	for(int i=4;i<=32;i++){
		printf("8000000/115200/%d = %d \ttrue_bps=%d \tdiff=%d \terrpto=%f\n",
				(int)i,
				8000000/115200/i,
				8000000/(8000000/115200/i)/i,
				8000000/(8000000/115200/i)/i-115200,
				(8000000/(8000000/115200/i)/i-115200)/(115200.f)
		);
		if((8000000/(8000000/115200/i)/i-115200)/(115200.f)<min_val){
			min_val = (8000000/(8000000/115200/i)/i-115200)/(115200.f);
			min_index = i;
		}
	}
	*errpto = min_val;
	return min_index;
}

int main(void) {
	int bpstbl[] = {2400,4800,9600,19200,38400,57600,115200};
	for(int i=0;i<sizeof(bpstbl)/sizeof(bpstbl[0]);i++){
		float errpto;
		int v = calc_opt_div(bpstbl[i],&errpto);
		printf("bps=%d div=%d, err=%f\n",bpstbl[i],v,errpto);
	}
	return EXIT_SUCCESS;
}

output:
bps=2400 div=23, err=0.006441
bps=4800 div=23, err=0.006441
bps=9600 div=23, err=0.006441
bps=19200 div=23, err=0.006441
bps=38400 div=23, err=0.006441
bps=57600 div=23, err=0.006441
bps=115200 div=23, err=0.006441

*/

static LPUART_Type* uartTbl[3] = { LPUART0, LPUART1, LPUART2 };

void LPUART_init(int port, uint32_t bps)  /* Init. summary: 9600 baud, 1 stop bit, 8 bit format, no parity */
{
	LPUART_Type *LPUART = uartTbl[port];
	if(LPUART==LPUART0){
	  PCC->PCCn[PCC_LPUART0_INDEX] &= ~PCC_PCCn_CGC_MASK;    /* Ensure clk disabled for config */
	  PCC->PCCn[PCC_LPUART0_INDEX] |= PCC_PCCn_PCS(0b001)    /* Clock Src= 1 (SOSCDIV2_CLK) */
								   |  PCC_PCCn_CGC_MASK;     /* Enable clock for LPUART regs */
	}else if(LPUART==LPUART1){
	  PCC->PCCn[PCC_LPUART1_INDEX] &= ~PCC_PCCn_CGC_MASK;    /* Ensure clk disabled for config */
	  PCC->PCCn[PCC_LPUART1_INDEX] |= PCC_PCCn_PCS(0b001)    /* Clock Src= 1 (SOSCDIV2_CLK) */
								   |  PCC_PCCn_CGC_MASK;     /* Enable clock for LPUART regs */
	}else if(LPUART==LPUART2){
	  PCC->PCCn[PCC_LPUART2_INDEX] &= ~PCC_PCCn_CGC_MASK;    /* Ensure clk disabled for config */
	  PCC->PCCn[PCC_LPUART2_INDEX] |= PCC_PCCn_PCS(0b001)    /* Clock Src= 1 (SOSCDIV2_CLK) */
								   |  PCC_PCCn_CGC_MASK;     /* Enable clock for LPUART regs */
	}
	/*
	 * fuck sysclk
	 */
	LPUART->BAUD = ((23-1)<<24)+(8000000/23/bps);// 0x03000000+(8000000/4/bps);

//  LPUART->BAUD = 0x0F000034;  /* Initialize for 9600 baud, 1 stop: */
//                               /* SBR=52 (0x34): baud divisor = 8M/9600/16 = ~52 */
//                               /* OSR=15: Over sampling ratio = 15+1=16 */
//                               /* SBNS=0: One stop bit */
//                               /* BOTHEDGE=0: receiver samples only on rising edge */
//                               /* M10=0: Rx and Tx use 7 to 9 bit data characters */
//                               /* RESYNCDIS=0: Resync during rec'd data word supported */
//                               /* LBKDIE, RXEDGIE=0: interrupts disable */
//                               /* TDMAE, RDMAE, TDMAE=0: DMA requests disabled */
//                               /* MAEN1, MAEN2,  MATCFG=0: Match disabled */
	LPUART->STAT = 0xC01FC000;
	LPUART->MATCH = 0x0;
	LPUART->CTRL = 0x0;
	LPUART->CTRL &= ~(LPUART_CTRL_R9T8_MASK | LPUART_BAUD_M10_MASK);
	LPUART->CTRL |= LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK | LPUART_CTRL_RIE_MASK;

	 //LPUART->CTRL=0x002C0000;    /* Enable transmitter & receiver, no parity, 8 bit char: */
                               /* RE=1: Receiver enabled */
                               /* TE=1: Transmitter enabled */
                               /* PE,PT=0: No hw parity generation or checking */
                               /* M7,M,R8T9,R9T8=0: 8-bit data characters*/
                               /* DOZEEN=0: LPUART enabled in Doze mode */
                               /* ORIE,NEIE,FEIE,PEIE,TIE,TCIE,RIE,ILIE,MA1IE,MA2IE=0: no IRQ*/
                               /* TxDIR=0: TxD pin is input if in single-wire mode */
                               /* TXINV=0: TRansmit data not inverted */
                               /* RWU,WAKE=0: normal operation; rcvr not in statndby */
                               /* IDLCFG=0: one idle character */
                               /* ILT=0: Idle char bit count starts after start bit */
                               /* SBK=0: Normal transmitter operation - no break char */
                               /* LOOPS,RSRC=0: no loop back */
	//LPUART1->GLOBAL |= 1<<1;
}

void LPUART_transmit_char(int port,char send) {    /* Function to Transmit single Char */
  LPUART_Type *LPUART = uartTbl[port];
  //while((LPUART->STAT & LPUART_STAT_TDRE_MASK)>>LPUART_STAT_TDRE_SHIFT==0);
                                   /* Wait for transmit buffer to be empty */
  LPUART->DATA=send;              /* Send data */
  while((LPUART->STAT & LPUART_STAT_TC_MASK)==0);
}

int LPUART_receive_char(int port) {    /* Function to Receive single Char */
	LPUART_Type *LPUART = uartTbl[port];
  char recieve;
  if((LPUART->STAT & LPUART_STAT_RDRF_MASK)>>LPUART_STAT_RDRF_SHIFT==0)
	  return -1;
                                     /* Wait for received buffer to be full */
  recieve= LPUART->DATA;            /* Read received data*/

  return recieve & 0xFFU;
}
extern void UartRxIRQHandler(int port,uint8_t rxbyte);
void LPUART0_RxTx_IRQHandler(void)
{
	if((LPUART0->STAT&LPUART_STAT_RDRF_MASK)>>LPUART_STAT_RDRF_SHIFT){
		uint8_t rxbyte = LPUART0->DATA;
		UartRxIRQHandler(0,rxbyte);
	}
}
void LPUART1_RxTx_IRQHandler(void)
{
	if((LPUART1->STAT&LPUART_STAT_RDRF_MASK)>>LPUART_STAT_RDRF_SHIFT){
		uint8_t rxbyte = LPUART1->DATA;
		UartRxIRQHandler(1,rxbyte);
	}
}
void LPUART2_RxTx_IRQHandler(void)
{
	if((LPUART2->STAT&LPUART_STAT_RDRF_MASK)>>LPUART_STAT_RDRF_SHIFT){
		uint8_t rxbyte = LPUART2->DATA;
		UartRxIRQHandler(2,rxbyte);
	}
}
