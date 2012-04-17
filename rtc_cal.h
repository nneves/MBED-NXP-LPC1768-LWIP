#ifndef RTC_CAL_H
#define RTC_CAL_H

#include "lpc17xx_rtc.h"

/* Definitions needed to calculate the calibration value and direction */
#define RTC_IDEAL_FREQ        32768000     //in mHz (milli Hz) (=32.768kHz)
#define RTC_ACTUAL_FREQ       32768000

void rtc_cal_init(RTC_TIME_Type *pFullTime);

void rtc_cal_config(uint32_t actual_rtc_frequency);

void rtc_cal_settime(RTC_TIME_Type *pFullTime);
uint8_t rtc_cal_gettime(RTC_TIME_Type *pFullTime);

#endif

