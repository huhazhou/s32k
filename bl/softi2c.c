/*
 * softi2c.c
 *
 *  Created on: 2017年10月19日
 *      Author: houxd
 */

#include "softi2c.h"

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
void softi2c_init_io(const struct SoftI2C_Ports *I2C_DEV) {
	I2C_DEV->SDA_OUT();
	I2C_DEV->SCL_OUT();
}
void softi2c_start(const struct SoftI2C_Ports *I2C_DEV) {
	I2C_DEV->SET_SDA();
	I2C_DEV->SET_SCL();
	softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
	I2C_DEV->CLR_SDA();
	softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
	I2C_DEV->CLR_SCL();
	softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
}

void softi2c_stop(const struct SoftI2C_Ports *I2C_DEV) {
	I2C_DEV->CLR_SDA();
	I2C_DEV->SET_SCL();
	softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
	I2C_DEV->SET_SDA();
	softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
}

void softi2c_send_byte(const struct SoftI2C_Ports *I2C_DEV, unsigned data) {
	unsigned BitCounter = 8;
	do {
		if (data & 0x80)
			I2C_DEV->SET_SDA();
		else
			I2C_DEV->CLR_SDA();
		I2C_DEV->SET_SCL();
		softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
		I2C_DEV->CLR_SCL();
		softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
		data <<= 1;
		BitCounter--;
	} while (BitCounter);
}

int softi2c_check_ack(const struct SoftI2C_Ports *I2C_DEV) {
	unsigned b;
	I2C_DEV->SDA_IN();
	I2C_DEV->SET_SCL();
	softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
	b = I2C_DEV->SDA_VAL();
	softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
	I2C_DEV->CLR_SCL();
	softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
	I2C_DEV->SDA_OUT();
	if (b)
		return 0;
	return 1;
}

unsigned softi2c_read_byte(const struct SoftI2C_Ports *I2C_DEV) {
	unsigned count = 8, temp = 0;
	I2C_DEV->SDA_IN();
	do {
		I2C_DEV->SET_SCL();
		temp <<= 1;
		softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
		if (I2C_DEV->SDA_VAL())
			temp = temp | 0x01;
		softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
		I2C_DEV->CLR_SCL();
		softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
		count--;
	} while (count);
	I2C_DEV->SDA_OUT();
	return temp;
}

void softi2c_send_ack(const struct SoftI2C_Ports *I2C_DEV) {
	I2C_DEV->CLR_SDA();
	I2C_DEV->CLR_SCL();
	softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
	I2C_DEV->SET_SCL();
	softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
	I2C_DEV->CLR_SCL();
	softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
}

void softi2c_send_noack(const struct SoftI2C_Ports *I2C_DEV) {
//send a no ack bit
	I2C_DEV->SET_SDA();
	I2C_DEV->CLR_SCL();
	softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
	I2C_DEV->SET_SCL();
	softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
	I2C_DEV->CLR_SCL();
	softi2c_delay(I2C_DEV->HALF_CLK_CYCLE_US);
}

void softi2c_send_data(const struct SoftI2C_Ports *I2C_DEV, unsigned chipaddr,
		unsigned dataaddr, unsigned datanum, const uint8_t *data) {
	unsigned i = 0;
	softi2c_start(I2C_DEV);
	softi2c_send_byte(I2C_DEV, chipaddr & (~0x01));  // chip address
	if (softi2c_check_ack(I2C_DEV)) {
		softi2c_send_byte(I2C_DEV, dataaddr);     // data address
		if (softi2c_check_ack(I2C_DEV)) {
			while (i < datanum) {
				softi2c_send_byte(I2C_DEV, data[i++]); // temp hide
				if (softi2c_check_ack(I2C_DEV) == 0)
					break;
			}
		}
	}
	softi2c_stop(I2C_DEV);
	softi2c_delay(10);
}

void softi2c_read_data(const struct SoftI2C_Ports *I2C_DEV, unsigned chipaddr,
		unsigned dataaddr, unsigned datanum, uint8_t *data) {
	unsigned i = 0;
	softi2c_start(I2C_DEV);
	softi2c_send_byte(I2C_DEV, chipaddr);  // eprom address
	if (softi2c_check_ack(I2C_DEV)) {
		softi2c_send_byte(I2C_DEV, dataaddr);     // data address
		if (softi2c_check_ack(I2C_DEV)) {
			softi2c_start(I2C_DEV);
			softi2c_send_byte(I2C_DEV, chipaddr | 0x01);
			if (softi2c_check_ack(I2C_DEV)) {
				while (i < datanum) {
					data[i++] = softi2c_read_byte(I2C_DEV);   // temp hide
					if (i == datanum)
						softi2c_send_noack(I2C_DEV);
					else
						softi2c_send_ack(I2C_DEV);
				}
			}
		}
	}
	softi2c_stop(I2C_DEV);
}

