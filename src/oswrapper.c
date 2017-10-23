/*
 * oswrapper.c
 *
 *  Created on: 2017年10月23日
 *      Author: houxd
 */

#include <oswrapper.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "bl.h"
extern void uart_send_lf_crlf(int port,int len,const uint8_t* data);
extern char *uart_gets_echo_blocked(int port,char *buf, int bufsize);

static OS_Mutex trace_mutex;
void os_trace_init(void)
{
	os_mutex_create(&trace_mutex);
}
void os_trace(const char *fmt, ...)
{
	va_list va;
	va_start(va,fmt);
	os_mutex_lock(&trace_mutex);
	//vprintf(fmt,va);
	char buf[128];
	int n = vsnprintf(buf,128,fmt,va);
	uart_send_lf_crlf(UART_RS485, n, (uint8_t*)buf);
	os_mutex_unlock(&trace_mutex);
	va_end(va);
}
char *os_console_gets(char *buf, int bufsize)
{
	return uart_gets_echo_blocked(UART_RS485,buf,bufsize);
}

void os_mutex_create(OS_Mutex *pMtx)
{
	OS_ASSERT(pMtx!=NULL);
	*pMtx = xSemaphoreCreateMutex();
	xSemaphoreGive(*pMtx);
}
void os_mutex_lock(OS_Mutex *pMtx)
{
	OS_ASSERT(pMtx!=NULL);
	xSemaphoreTake(*pMtx,portMAX_DELAY);
}
void os_mutex_unlock(OS_Mutex *pMtx)
{
	OS_ASSERT(pMtx!=NULL);
	xSemaphoreGive(*pMtx);
}
void os_mutex_release(OS_Mutex *pMtx)
{
	OS_ASSERT(pMtx!=NULL);
	vSemaphoreDelete(*pMtx);
}

void os_sleep_ms(int ms)
{
	vTaskDelay(ms);
}
void os_suspend(void)
{
	vTaskDelay(portMAX_DELAY);
}
