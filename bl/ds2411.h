/*
 * ds2411.h
 *
 *  Created on: 2017年10月19日
 *      Author: houxd
 */

#ifndef DS2411_H_
#define DS2411_H_
#include <stdint.h>
struct DS2411_SN {
	uint8_t family_code;  	//8bit家族码，ds2411应为0x01
	uint8_t serial_no[6];  	//48bit序列码
	uint8_t crc_code;	 	//8bit校验码
};
int ds2411_getsn(struct DS2411_SN* psn);
#endif /* DS2411_H_ */
