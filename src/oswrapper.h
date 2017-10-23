/*
 * oswrapper.h
 *
 *  Created on: 2017年10月23日
 *      Author: houxd
 */

#ifndef OSWRAPPER_H_
#define OSWRAPPER_H_

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define OS_ASSERT(s)											\
	if((s)==0){											\
		os_trace("fault in %s call: %s\n",__FUNCTION__,#s);	\
		for(;;);												\
	}
extern void os_trace_init(void);
extern void os_trace(const char *fmt, ...);
extern char *os_console_gets(char *buf, int bufsize);

typedef SemaphoreHandle_t OS_Mutex;
extern void os_mutex_create(OS_Mutex *pMtx);
extern void os_mutex_lock(OS_Mutex *pMtx);
extern void os_mutex_unlock(OS_Mutex *pMtx);
extern void os_mutex_release(OS_Mutex *pMtx);

extern void os_sleep_ms(int ms);
extern void os_suspend(void);

#endif /* OSWRAPPER_H_ */
