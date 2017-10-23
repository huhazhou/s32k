/*
 * softonewire.h
 *
 *  Created on: 2017年10月19日
 *      Author: houxd
 */

#ifndef SOFTONEWIRE_H_
#define SOFTONEWIRE_H_
#include <stdint.h>
enum SoftOneWire_TimingTblIndex{	/* 定义1-wire标准时序中各种操作所需的延迟，单位时us */
	OW_RESET_BEGAIN = 0,         /* 主机初始化出发时序                  */
	OW_RESET_WAIT_ACK,           /* 主机等待初始化应答                  */
	OW_RESET_END,                /* 完成初始化时序，480-70              */
	OW_WRITE_1_L,                /* 启动写 '1' 的时序                  */
	OW_WRITE_1_H,                /* 完成写 '1' 的时序                  */
	OW_WRITE_0_L,                /* 写'0'需要保持60us的低电平           */
	OW_WRITE_0_H,                /* 写'0',在拉高电平后保持10us          */
	OW_READ_LOW,                 /* 读时隙开始时拉低                    */
	OW_READ_BEFORE_SAMPLE,       /* 读时隙开始后15us内采样, 6+8=14us    */
	OW_READ_END,                 /* 一个读时隙需要至少60us，60-6-8 = 56  */
	_OW_INDEX_MAX,				 /* the TIMING_TABLE[] size max      */
};
struct SoftOneWire {
	void (*IO_OUT_SET)();	/* !!! set i/o mode Output and SET pin value */
	void (*IO_OUT_CLR)();	/* !!! set i/o mode Output and SET pin value */
	int  (*IO_IN_VAL)();	/* !!! set i/o mode Input and CLR pin value */
	const unsigned *TIMING_TABLE; /* length is _OW_INDEX_MAX */;
};
extern const unsigned onewire_def_timing_tbl[_OW_INDEX_MAX];
extern void onewrie_io_init(const struct SoftOneWire* OW_DEV);
extern int onewrie_reset(void);
extern void onewrite_write_bit(int v);
extern int onewire_read_bit(void);
extern void onewire_write_byte(uint8_t v);
extern uint8_t onewire_read_byte(void);
extern uint8_t onewrie_crc8(uint8_t *dat, uint8_t length);
extern int onewire_read(uint8_t slave_addr, int len, uint8_t *rbuf);

#endif /* SOFTONEWIRE_H_ */
