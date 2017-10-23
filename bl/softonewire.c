/*
 * softonewire.c
 *
 *  Created on: 2017年10月19日
 *      Author: houxd
 */

#include <softonewire.h>
#include <string.h>

const unsigned onewire_def_timing_tbl[_OW_INDEX_MAX] = {
	[OW_RESET_BEGAIN] = 1200,
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

static void onewire_delay_us(int _us) {
	int us = _us;
	while (us--)
		for (int i = 5; i; --i)
			;
}
const struct SoftOneWire* this_dev;
void onewrie_io_init(const struct SoftOneWire* OW_DEV)
{
	this_dev = OW_DEV;
	this_dev->IO_OUT_CLR();
}
int onewrie_reset(void) {
	int result = 0;
	//先将端口拉低480us,再拉高，等70us后采样响应信号
	this_dev->IO_OUT_CLR();
	onewire_delay_us(this_dev->TIMING_TABLE[OW_RESET_BEGAIN]);
	this_dev->IO_OUT_SET();
	onewire_delay_us(this_dev->TIMING_TABLE[OW_RESET_WAIT_ACK]);
	result = this_dev->IO_IN_VAL();
	//等待复位周期完成
	onewire_delay_us(this_dev->TIMING_TABLE[OW_RESET_END]);
	return result;
}

void onewrite_write_bit(int v) {
	if (v) {
		//写'1'操作，先拉低，然后拉高完成时序和恢复
		this_dev->IO_OUT_CLR();
		onewire_delay_us(this_dev->TIMING_TABLE[OW_WRITE_1_L]);
		this_dev->IO_OUT_SET();
		onewire_delay_us(this_dev->TIMING_TABLE[OW_WRITE_1_H]);
	} else {
		//写'0'操作，拉低并保持到时序结束，然后释放总线，留足恢复时间
		this_dev->IO_OUT_CLR();
		onewire_delay_us(this_dev->TIMING_TABLE[OW_WRITE_0_L]);
		this_dev->IO_OUT_SET();
		onewire_delay_us(this_dev->TIMING_TABLE[OW_WRITE_0_H]);
	}
}

int onewire_read_bit(void) {
	int result = 0;
	//读时序，拉低后数个us，并在时隙开始后15us内采样端口数据
	this_dev->IO_OUT_CLR();
	onewire_delay_us(this_dev->TIMING_TABLE[OW_READ_LOW]);
	this_dev->IO_OUT_SET();
	onewire_delay_us(this_dev->TIMING_TABLE[OW_READ_BEFORE_SAMPLE]);
	result = this_dev->IO_IN_VAL();
	onewire_delay_us(this_dev->TIMING_TABLE[OW_READ_END]);
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

int onewire_read(uint8_t slave_addr, int len, uint8_t *rbuf)
{
	int rst_val = onewrie_reset();
	if(0!=rst_val)
		return -1;
	onewire_write_byte(slave_addr);
	for (int i = 0; i < len; i++) {
		rbuf[i] = onewire_read_byte();
	}
	return 0;
}

uint8_t onewrie_crc8(uint8_t *_dat, uint8_t length) {
	uint8_t *dat = _dat;
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
