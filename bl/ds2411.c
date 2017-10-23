/*
 * ds2411.c
 *
 *  Created on: 2017年10月19日
 *      Author: houxd
 */

#include <ds2411.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "S32K144.h"
#include "softonewire.h"

void IO_OUT_SET() {
	PTA->PDDR |= 1 << 16;
	PTA-> PSOR |= 1<<16;
}
void IO_OUT_CLR() {
	PTA->PDDR |= 1 << 16;
	PTA-> PCOR |= 1<<16;
}

int IO_IN_VAL() {
	PTA->PDDR &= ~(1 << 16);
	return PTA->PDIR & 1<<16;
}
const unsigned DS2411_TIMING_TABLE[_OW_INDEX_MAX] = {
	[OW_RESET_BEGAIN] = 480,
	[OW_RESET_WAIT_ACK] = 70,
	[OW_RESET_END] = 410,
	[OW_WRITE_1_L] = 6,
	[OW_WRITE_1_H] = 64,
	[OW_WRITE_0_L] = 60,
	[OW_WRITE_0_H] = 10,
	[OW_READ_LOW] = 4,
	[OW_READ_BEFORE_SAMPLE] = 8,
	[OW_READ_END] = 65,
};
const struct SoftOneWire OneWireDev = {
		.IO_OUT_SET = IO_OUT_SET,
		.IO_OUT_CLR = IO_OUT_CLR,
		.IO_IN_VAL = IO_IN_VAL,
		.TIMING_TABLE = DS2411_TIMING_TABLE,
};

int ds2411_getsn(struct DS2411_SN* psn) {
	uint8_t *sn = (uint8_t*)psn;
	onewrie_io_init(&OneWireDev);
	int res = onewire_read(0x33,8,sn);
	if(res)
		return -1;
	if (sn[7] != onewrie_crc8(sn, 7)) {
		return -2;
	}
	return 0;
}



