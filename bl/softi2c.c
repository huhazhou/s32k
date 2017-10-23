/*
 * softi2c.c
 *
 *  Created on: 2017年10月19日
 *      Author: houxd
 */

#include "softi2c.h"

static const struct SoftI2C* this_dev;
static void softi2c_delay(int _us) {
	int us = _us;
	while (us--)
		for (int i = DELAY_CALIBRATE_PARAM; i; --i)
			;
}
void softi2c_test_delay_1s()
{
	softi2c_delay(1000000);
}
void softi2c_init_io(const struct SoftI2C *I2C_DEV) {
	this_dev = I2C_DEV;
	this_dev->SDA_OUT();
	this_dev->SCL_OUT();
}
void softi2c_start() {
	this_dev->SDA_SET();
	this_dev->SCL_SET();
	softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
	this_dev->SDA_CLR();
	softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
	this_dev->SCL_CLR();
	softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
}

void softi2c_stop() {
	this_dev->SDA_CLR();
	this_dev->SCL_SET();
	softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
	this_dev->SDA_SET();
	softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
}

void softi2c_send_byte(unsigned _data) {
	unsigned data = _data;
	unsigned BitCounter = 8;
	do {
		if (data & 0x80)
			this_dev->SDA_SET();
		else
			this_dev->SDA_CLR();
		this_dev->SCL_SET();
		softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
		this_dev->SCL_CLR();
		softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
		data <<= 1;
		BitCounter--;
	} while (BitCounter);
}

int softi2c_check_ack() {
	unsigned b;
	this_dev->SDA_IN();
	this_dev->SCL_SET();
	softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
	b = this_dev->SDA_VAL();
	softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
	this_dev->SCL_CLR();
	softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
	this_dev->SDA_OUT();
	if (b)
		return 0;
	return 1;
}

unsigned softi2c_read_byte() {
	unsigned count = 8, temp = 0;
	this_dev->SDA_IN();
	do {
		this_dev->SCL_SET();
		temp <<= 1;
		softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
		if (this_dev->SDA_VAL())
			temp = temp | 0x01;
		softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
		this_dev->SCL_CLR();
		softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
		count--;
	} while (count);
	this_dev->SDA_OUT();
	return temp;
}

void softi2c_send_ack() {
	this_dev->SDA_CLR();
	this_dev->SCL_CLR();
	softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
	this_dev->SCL_SET();
	softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
	this_dev->SCL_CLR();
	softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
}

void softi2c_send_noack() {
//send a no ack bit
	this_dev->SDA_SET();
	this_dev->SCL_CLR();
	softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
	this_dev->SCL_SET();
	softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
	this_dev->SCL_CLR();
	softi2c_delay(this_dev->HALF_CLK_CYCLE_US);
}

void softi2c_send_data(unsigned chipaddr, unsigned dataaddr, unsigned datanum,
		const uint8_t *data) {
	unsigned i = 0;
	softi2c_start();
	softi2c_send_byte(chipaddr & (~0x01));  // chip address
	if (softi2c_check_ack()) {
		softi2c_send_byte(dataaddr);     // data address
		if (softi2c_check_ack()) {
			while (i < datanum) {
				softi2c_send_byte(data[i++]); // temp hide
				if (softi2c_check_ack() == 0)
					break;
			}
		}
	}
	softi2c_stop(this_dev);
	softi2c_delay(10);
}

int softi2c_read_data(unsigned chipaddr,unsigned dataaddr, unsigned datanum,
		uint8_t *data) {
	unsigned i = 0;
	softi2c_start();
	softi2c_send_byte(chipaddr);  // eprom address
	if (softi2c_check_ack()) {
		softi2c_send_byte(dataaddr);     // data address
		if (softi2c_check_ack()) {
			softi2c_start();
			softi2c_send_byte(chipaddr | 0x01);
			if (softi2c_check_ack()) {
				while (i < datanum) {
					data[i++] = softi2c_read_byte();   // temp hide
					if (i == datanum)
						softi2c_send_noack();
					else
						softi2c_send_ack();
				}
				return 0;
			}
		}
	}
	softi2c_stop();
	return -1;
}

