/*
 * ds3231.h
 *
 *  Created on: 2017年10月19日
 *      Author: houxd
 */

#ifndef DS3231_H_
#define DS3231_H_

struct DS3231_Time{
	uint8_t sec;	//00-59
	uint8_t min;	//00-59
	uint8_t hour;	//00-23
	uint8_t wday;	//1~7
	uint8_t mday;	//1~32
	uint8_t mon;	//1~12
	uint8_t year;	//0~99
};
extern int ds3231_init();
extern float ds3231_gettempr();
extern void ds3231_settime(const struct DS3231_Time *dsdat);
extern void ds3231_gettime(struct DS3231_Time *dsdat);


#endif /* DS3231_H_ */
