#include <am.h>
#include <nemu.h>

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  // 从 RTC_ADDR 读取低32位
  uint32_t low = inl(RTC_ADDR);
  
  // 从 RTC_ADDR + 4 读取高32位
  uint32_t high = inl(RTC_ADDR + 4);
  
  // 将高低32位组合成一个64位微秒数
  uptime->us = ((uint64_t)high << 32) | low;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
