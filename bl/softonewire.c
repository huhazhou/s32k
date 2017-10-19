/*
 * softonewire.c
 *
 *  Created on: 2017年10月19日
 *      Author: houxd
 */

#include <softonewire.h>
#include <string.h>
#include "S32K144.h"

//定义1-wire标准时序中各种操作所需的延迟，单位时us
#define	RESET_LOW 			(480)	//主机初始化出发时序
#define	WAIT_RESET_ACK 		(70)	//主机等待初始化应答
#define	RESET_END			(410)	//完成初始化时序，480-70
#define	WRITE_1_L			(6)		//启动写 '1' 的时序
#define	WRITE_1_H 	 		(64)	//完成写 '1' 的时序
#define	WRITE_0_L 	 		(60)	//写'0'需要保持60us的低电平
#define	WRITE_0_H	 		(10)	//写'0',在拉高电平后保持10us
#define	READ_LOW			(4)		//读时隙开始时拉低
#define	READ_BEFORE_SAMLPE	(8)		//读时隙开始后15us内采样, 6+8=14us
#define	READ_END			(65)	//一个读时隙需要至少60us，60-6-8 = 56

void ds2411_dir(int d)
{
	if(d) PTA->PDDR |= 1 << 16;
	else PTA->PDDR &= ~(1 << 16);
}
void ds2411_line(int v)
{
	if(v) PTA-> PSOR |= 1<<16;
	else PTA-> PCOR |= 1<<16;
}
int ds2411_line_val()
{
	return PTA->PDIR & 1<<16;
}

void IO_INIT(void) {

}

void IO_DIR(uint32_t Base, uint8_t Bit, int Dir) {
	ds2411_dir(Dir);
}

void WRITE_IO(uint32_t Base, uint8_t Bit, uint16_t Value) {
	IO_DIR(Base, Bit, 1);
	ds2411_line(Value);
}

int READ_IO_VAL(uint32_t Base, uint8_t Bit) {

	IO_DIR(Base, Bit, 0);
	return ds2411_line_val();
}

static void onewire_delay_us(int _us) {
	int us = _us;
	while (us--)
		for (int i = 5; i; --i)
			;
}
int onewrie_reset(void) {
	int result = 0;
	IO_INIT();
	//先将端口拉低480us,再拉高，等70us后采样响应信号
	WRITE_IO(0, 1, 0);
	onewire_delay_us(RESET_LOW);
	WRITE_IO(0, 1, 1);
	onewire_delay_us(WAIT_RESET_ACK);
	result = READ_IO_VAL(0, 1);
	//等待复位周期完成
	onewire_delay_us(RESET_END);
	return result;
}

void onewrite_write_bit(int v) {
	if (v) {
		//写'1'操作，先拉低，然后拉高完成时序和恢复
		WRITE_IO(0, 1, 0);
		onewire_delay_us(WRITE_1_L);
		WRITE_IO(0, 1, 1);
		onewire_delay_us(WRITE_1_H);
	} else {
		//写'0'操作，拉低并保持到时序结束，然后释放总线，留足恢复时间
		WRITE_IO(0, 1, 0);
		onewire_delay_us(WRITE_0_L);
		WRITE_IO(0, 1, 1);
		onewire_delay_us(WRITE_0_H);
	}
}

int onewire_read_bit(void) {
	int result = 0;
	//读时序，拉低后数个us，并在时隙开始后15us内采样端口数据
	WRITE_IO(0, 1, 0);
	onewire_delay_us(READ_LOW);
	WRITE_IO(0, 1, 1);
	onewire_delay_us(READ_BEFORE_SAMLPE);
	result = READ_IO_VAL(0, 1);
	onewire_delay_us(READ_END);
	return result;
}

void onewire_write_byte(uint8_t v) {
	uint8_t i = 0;
	uint8_t tmp = v;
	//用循环和移位完成每一位的写操作，从LS开始
	for (i = 0; i < 8; i++) {
		onewrite_write_bit(tmp & 0x01);
		tmp >>= 1;
	}
}

uint8_t onewire_read_byte(void) {
	uint8_t i = 0;
	uint8_t result = 0;
	for (i = 0; i < 8; i++) {
		//从低位开始读起，每读一位变向右移动一位
		result >>= 1;
		if (onewire_read_bit()) {
			//printf("read bit %d is 1 ===", i);
			result |= 0x80;
		} else {
			//printf("read bit %d is 0 ===", i);
		}
		onewire_delay_us(3);
	}
	return result;
}
