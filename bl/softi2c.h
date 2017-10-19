/*
 * softi2c.h
 *
 *  Created on: 2017年10月19日
 *      Author: houxd
 *
 *============================================
 * 1.fix DELAY_CALIBRATE_PARAM and use
  softi2c_test_delay_1s() to test
 * 2.port template:
void SET_SCL()	{PTB->PSOR |= 1 << 9;	   }
void CLR_SCL()	{PTB->PCOR |= 1 << 9;      }
void SET_SDA()	{PTB->PSOR |= 1 << 10;     }
void CLR_SDA()	{PTB->PCOR |= 1 << 10;     }
void SCL_OUT()	{PTB->PDDR |= 1 << 9;      }
void SCL_IN ()	{PTB->PDDR &= ~(1 << 9);   }
void SDA_OUT()	{PTB->PDDR |= 1 << 10;     }
void SDA_IN ()	{PTB->PDDR &= ~(1 << 10);  }
int  SDA_VAL()	{return PTB->PDIR & (1<<10);}

const struct SoftI2C_Ports I2C0 = {
	.SET_SCL 	=SET_SCL,
	.CLR_SCL 	=CLR_SCL,
	.SET_SDA 	=SET_SDA,
	.CLR_SDA 	=CLR_SDA,
	.SCL_OUT 	=SCL_OUT,
	.SCL_IN 	=SCL_IN,
	.SDA_OUT	=SDA_OUT,
	.SDA_IN		=SDA_IN,
	.SDA_VAL	=SDA_VAL,
	.HALF_CLK_CYCLE_US	=	10,
};
 *============================================
 */

#ifndef SOFTI2C_H_
#define SOFTI2C_H_
#include <stdint.h>

#define DELAY_CALIBRATE_PARAM	(5)	//soft delay calibrate param
struct SoftI2C_Ports {
	void (*SET_SCL)();
	void (*CLR_SCL)();
	void (*SET_SDA)();
	void (*CLR_SDA)();
	void (*SCL_OUT)();
	void (*SCL_IN )();
	void (*SDA_OUT)();
	void  (*SDA_IN )();
	int (*SDA_VAL)();
	int HALF_CLK_CYCLE_US;	// unit is us.
};
extern void softi2c_test_delay_1s();	//use for calibrate soft delay
extern void softi2c_init_io(const struct SoftI2C_Ports *I2C_DEV);
extern void softi2c_start(const struct SoftI2C_Ports *I2C_DEV);
extern void softi2c_stop(const struct SoftI2C_Ports *I2C_DEV);
extern void softi2c_send_byte(const struct SoftI2C_Ports *I2C_DEV, unsigned data);
extern int softi2c_check_ack(const struct SoftI2C_Ports *I2C_DEV);
extern unsigned softi2c_read_byte(const struct SoftI2C_Ports *I2C_DEV);
extern void softi2c_send_ack(const struct SoftI2C_Ports *I2C_DEV);
extern void softi2c_send_noack(const struct SoftI2C_Ports *I2C_DEV);
extern void softi2c_send_data(const struct SoftI2C_Ports *I2C_DEV, unsigned chipaddr,
		unsigned dataaddr, unsigned datanum, const uint8_t *data);
extern void softi2c_read_data(const struct SoftI2C_Ports *I2C_DEV, unsigned chipaddr,
		unsigned dataaddr, unsigned datanum, uint8_t *data);
#endif /* SOFTI2C_H_ */
