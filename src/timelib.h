/*
 * timelib.h
 *
 *  Created on: 2017年10月19日
 *      Author: houxd
 */

#ifndef TIMELIB_H_
#define TIMELIB_H_
#include <stdint.h>
#define _TIMELIB_TZ_DEF			(+8*60*60) 	/* default time zone: +8 */
#define _TIMELIB_TICK_HZ		(1000)		/* unit with __timelib_tick_count */
#define _TIMELIB_ASC_USE_CN		(1)			/* for asctime output ctrl */
typedef uint32_t time_t;
struct tm
{
	int	tm_sec;		/** [0..60], allows for a positive leap second */
	int	tm_min;		/** [0..59] */
	int	tm_hour;	/** [0..23] */
	int	tm_mday;	/** [1..31] */
	int	tm_mon;		/** [0..11], months since January */
	int	tm_year;	/** years since 1900 */
	int	tm_wday;	/** [0..6], days since Sunday */
	/*  not support. */
	/*	int	tm_yday;	 [0..365], days since January 1 */
	/*	int	tm_isdst;	 Daylight Saving Time flag */
};
extern volatile uint64_t __timelib_tick_count;	/* must be call interrupt */
extern time_t time(const time_t* t);
extern time_t mktime(struct tm * pt);
extern void localtime_z(const time_t *ptime, long timezone, struct tm *tm_time);
extern struct tm *localtime_s(const time_t * pt, struct tm*ltm);
extern long __timezone(long *tz);
extern char *asctime_s(const struct tm * tp, char *buf, int buflen);
extern char *asctime(const struct tm * tp);
#endif /* TIMELIB_H_ */
