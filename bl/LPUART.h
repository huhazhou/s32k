/*
 * LPUART.h
 *
 *  Created on: Mar 17, 2016
 *      Author: B46911
 */

#ifndef LPUART_H_
#define LPUART_H_
#include "S32K144.h"
void LPUART_init(LPUART_Type *LPUART, uint32_t bps);
void LPUART_transmit_char(LPUART_Type *LPUART, char send);
int LPUART_receive_char(LPUART_Type *LPUART);
#endif /* LPUART_H_ */
