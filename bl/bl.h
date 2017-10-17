/*
 * bl.h
 *
 *  Created on: 2017年10月10日
 *      Author: houxd
 */

#ifndef BL_H_
#define BL_H_
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "LPUART.h"
#include "FlexCAN.h"
#include "SPI_MSD0_Driver.h"
#include "clocks_and_modes.h"

extern void WDOG_disable(void);
extern void NVIC_init_IRQs (void);
extern void PORT_init (void);
extern uint32_t get_tick_count();
extern void delay_ms(int ms);
extern void buzzer_ctrl(int en);
extern void buzzer_ctrl(int en);


extern inline void ds3231_int(int v);
extern inline void ds3231_rst(int v);
extern inline void i2c_scl(int v);
extern inline void i2c_sda(int v);
extern inline void i2c_sda_dir(int d);
extern inline int i2c_sda_val();

extern inline void ds2411_dir(int d);
extern inline void ds2411_line(int v);


#ifdef __cplusplus
}
#endif

#endif /* BL_H_ */
