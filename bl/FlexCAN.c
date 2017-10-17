/* FlexCAN.c              (c) 2016 NXP
 * Descriptions: S32K144 CAN 2.0 A/B example..
 * 2016 Jul 16 S. Mihalik: Initial version
 * 2016 Sep 12 SM: Updated with SBC init, Node A - B communication
 * 2016 Oct 31 SM: Updated for new header symbols for PCCn
 */

#include "S32K144.h" /* include peripheral declarations S32K144 */
#include "FlexCAN.h"
#include "convert.h"


CAN_Type *CAN[3] = {CAN0,CAN1,CAN2};
int PCC_FlexCAN_INDEX[3] = {PCC_FlexCAN0_INDEX,PCC_FlexCAN1_INDEX,PCC_FlexCAN2_INDEX};


void FLEXCAN_init(int port,uint32_t ID, uint32_t bps)
{
#define MSG_BUF_SIZE  4    /* Msg Buffer Size. (CAN 2.0AB: 2 hdr +  2 data= 4 words) */
	uint32_t i = 0;

	PCC->PCCn[PCC_FlexCAN_INDEX[port]] |= PCC_PCCn_CGC_MASK; /* CGC=1: enable clock to FlexCAN0 */
	CAN[port]->MCR |= CAN_MCR_MDIS_MASK; /* MDIS=1: Disable module before selecting clock */
	CAN[port]->CTRL1 &= ~CAN_CTRL1_CLKSRC_MASK; /* CLKSRC=0: Clock Source = oscillator (8 MHz) */
	CAN[port]->MCR &= ~CAN_MCR_MDIS_MASK; /* MDIS=0; Enable module config. (Sets FRZ, HALT)*/
	while (!((CAN[port]->MCR & CAN_MCR_FRZACK_MASK) >> CAN_MCR_FRZACK_SHIFT)) {
	}
	uint32_t PRESDIV = 8000000 / (16*bps) -1;
	/* Good practice: wait for FRZACK=1 on freeze mode entry/exit */
	CAN[port]->CTRL1 = 0x00DB0006 | (PRESDIV<<24); /* Configure for 500 KHz bit time */
	/* Time quanta freq = 16 time quanta x 500 KHz bit time= 8MHz */
	/* PRESDIV+1 = Fclksrc/Ftq = 8 MHz/8 MHz = 1 */
	/*    so PRESDIV = 0 */
	/* PSEG2 = Phase_Seg2 - 1 = 4 - 1 = 3 */
	/* PSEG1 = PSEG2 = 3 */
	/* PROPSEG= Prop_Seg - 1 = 7 - 1 = 6 */
	/* RJW: since Phase_Seg2 >=4, RJW+1=4 so RJW=3. */
	/* SMP = 1: use 3 bits per CAN sample */
	/* CLKSRC=0 (unchanged): Fcanclk= Fosc= 8 MHz */
//	for (i = 0; i < 128; i++) { /* CAN[port]: clear 32 msg bufs x 4 words/msg buf = 128 words*/
//		CAN[port]->RAMn[i] = 0; /* Clear msg buf word */
//	}
	for (i = 0; i < 16; i++) { /* In FRZ mode, init CAN[port] 16 msg buf filters */
		CAN[port]->RXIMR[i] = 0xFFFFFFFF; /* Check all ID bits for incoming messages */
	}
	CAN[port]->RXMGMASK = 0x1FFFFFFF; /* Global acceptance mask: check all ID bits */

	CAN[port]->RAMn[4 * MSG_BUF_SIZE + 0] = 0x04000000; /* Msg Buf 4, word 0: Enable for reception */
	/* EDL,BRS,ESI=0: CANFD not used */
	/* CODE=4: MB set to RX inactive */
	/* IDE=0: Standard ID */
	/* SRR, RTR, TIME STAMP = 0: not applicable */
	CAN[port]->RAMn[4 * MSG_BUF_SIZE + 1] = ID<<18; /* Msg Buf 4, word 1: Standard ID = 0x111 */

	/* PRIO = 0: CANFD not used */
	CAN[port]->MCR = 0x0000001F; /* Negate FlexCAN 1 halt state for 32 MBs */
	while ((CAN[port]->MCR && CAN_MCR_FRZACK_MASK) >> CAN_MCR_FRZACK_SHIFT) {
	}
	/* Good practice: wait for FRZACK to clear (not in freeze mode) */
	while ((CAN[port]->MCR && CAN_MCR_NOTRDY_MASK) >> CAN_MCR_NOTRDY_SHIFT) {
	}
	/* Good practice: wait for NOTRDY to clear (module ready)  */
}

void FLEXCAN_transmit_msg(
		int port,
		uint32_t TxID,
		uint32_t TxLENGTH,
		const uint8_t TxDATA[8]) { /* Assumption:  Message buffer CODE is INACTIVE */
	CAN[port]->IFLAG1 = 0x00000001; /* Clear CAN 0 MB 0 flag without clearing others*/
	CAN[port]->RAMn[0 * MSG_BUF_SIZE + 2] = HX_MSB_B2DW(&TxDATA[0]); /* MB0 word 2: data word 0 */
	CAN[port]->RAMn[0 * MSG_BUF_SIZE + 3] = HX_MSB_B2DW(&TxDATA[4]); /* MB0 word 3: data word 1 */
	CAN[port]->RAMn[0 * MSG_BUF_SIZE + 1] = TxID<<18; /* MB0 word 1: Tx msg with STD ID 0x555 */
	CAN[port]->RAMn[0 * MSG_BUF_SIZE + 0] = 0x0C400000 | TxLENGTH << CAN_WMBn_CS_DLC_SHIFT; /* MB0 word 0: */
	/* EDL,BRS,ESI=0: CANFD not used */
	/* CODE=0xC: Activate msg buf to transmit */
	/* IDE=0: Standard ID */
	/* SRR=1 Tx frame (not req'd for std ID) */
	/* RTR = 0: data, not remote tx request frame*/
	/* DLC = 8 bytes */
}

int FLEXCAN_receive_msg(
		int port,
		//uint32_t *RxID, /* Received message ID */
		uint32_t *RxLENGTH, /* Recieved message number of data bytes */
		uint8_t RxDATA[8]
		) { /* Receive msg from ID 0x556 using msg buffer 4 */
	uint32_t dummy;
	if ((CAN[port]->IFLAG1 >> 4) & 1) {  /* If CAN 0 MB 4 flag is set (received msg), read MB4 */
		//*RxCODE = (CAN[port]->RAMn[4 * MSG_BUF_SIZE + 0] & 0x07000000) >> 24; /* Read CODE field */
		*RxLENGTH = (CAN[port]->RAMn[4 * MSG_BUF_SIZE + 0] & CAN_WMBn_CS_DLC_MASK)>> CAN_WMBn_CS_DLC_SHIFT;
		//*RxID = (CAN[port]->RAMn[4 * MSG_BUF_SIZE + 1] & CAN_WMBn_ID_ID_MASK)>> CAN_WMBn_ID_ID_SHIFT >>18;
		uint32_t w2 = CAN[port]->RAMn[4 * MSG_BUF_SIZE + 2];
		uint32_t w3 = CAN[port]->RAMn[4 * MSG_BUF_SIZE + 3];
		HX_MSB_DW2B(w2,&RxDATA[0]);
		HX_MSB_DW2B(w3,&RxDATA[4]);
		//*RxTIMESTAMP = (CAN[port]->RAMn[0 * MSG_BUF_SIZE + 0] & 0x000FFFF);
		dummy = CAN[port]->TIMER; /* Read TIMER to unlock message buffers */
		(void)dummy;
		CAN[port]->IFLAG1 = 0x00000010; /* Clear CAN 0 MB 4 flag without clearing others*/
		return 0;
	}
	return -1;
}

