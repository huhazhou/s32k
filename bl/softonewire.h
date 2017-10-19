/*
 * softonewire.h
 *
 *  Created on: 2017年10月19日
 *      Author: houxd
 */

#ifndef SOFTONEWIRE_H_
#define SOFTONEWIRE_H_
#include <stdint.h>
extern int onewrie_reset(void);
extern void onewrite_write_bit(int v);
extern int onewire_read_bit(void);
extern void onewire_write_byte(uint8_t v);
extern uint8_t onewire_read_byte(void);

#endif /* SOFTONEWIRE_H_ */
