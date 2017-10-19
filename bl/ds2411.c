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

//定义存放SN的基地址和偏移地址
#define SN_DEV_ADDR		0xA0
#define SN_OFFSET		0xF0

#define DEBUGMSG(...)

uint8_t calc_crc8(uint8_t *dat, uint8_t length) {
	uint8_t i, j, aa, check, flag;
	check = 0;
	for (i = 0; i < length; i++) {
		aa = *dat++;
		for (j = 0; j < 8; j++) {
			flag = (check ^ aa) & 1;
			check >>= 1;
			if (flag) {
				check ^= 0x0c;
				check |= 0x80;
			}
			aa >>= 1;
		}
	}
	return check;
}
int ds2411_getsn(struct DS2411_SN* psn) {
	uint8_t *sn = (uint8_t*)psn;
	int rst_val = onewrie_reset();
	if(0!=rst_val)
		return -1;
	onewire_write_byte(0x33); //启动读取过程
	for (int i = 0; i < 8; i++) {
		sn[i] = onewire_read_byte();
	}
	if (sn[7] != calc_crc8(sn, 7)) {
		return -2;
	}
	return 0;
}

int main()
{
	WDOG_disable();
	SOSC_init_8MHz(); /* Initialize system oscilator for 8 MHz xtal */
	SPLL_init_160MHz(); /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	NormalRUNmode_80MHz(); /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
	NVIC_init_IRQs();        /* Enable desired interrupts and priorities */
	PORT_init();             /* Configure ports */

	while(1){
		char asc[128];
		char buf[8];
		int res = ds2411_getsn((struct DS2411_SN*)buf);
		printf("res=%d\n",res);
		printf("%s\n",hx_dumphex2str(buf,8,asc));
	}

}


