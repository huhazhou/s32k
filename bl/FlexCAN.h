/* FlexCAN.h              (c) 2015 Freescale Semiconductor, Inc.
 * Descriptions: FTM example code.
 * 16 Sep 2016 SM: Initial version
 */


#ifndef FLEXCAN_H_
#define FLEXCAN_H_

#include <stdint.h>
extern void FLEXCAN_init(int port, uint32_t ID, uint32_t bps);
extern void FLEXCAN_transmit_msg(
		int port,
		uint32_t TxID,
		uint32_t TxLENGTH,
		const uint8_t TxDATA[8]);
extern int FLEXCAN_receive_msg(
		int port,
		uint32_t *RxLENGTH, /* Recieved message number of data bytes */
		uint8_t RxDATA[8]
		);
#endif /* FLEXCAN_H_ */
