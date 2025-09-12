#include <am.h>
#include <nemu.h>

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  // 读取RTC寄存器的值，假设RTC寄存器的低32位在RTC_ADDR，高32位在RTC_ADDR + 4
  // 为了保证读取的一致性，先读高32位，再读低32位，然后再读一次高32位进行验证
  // 如果两次读到的高32位不一样，说明在读低32位时高32位发生了变化，需要重新读
  // 直到两次读到的高32位一样为止
  // 最终将高32位和低32位合并成一个64位的值，表示微秒数
  uint32_t hi, lo, hi2;
  do {
    hi = inl(RTC_ADDR + 4);
    lo = inl(RTC_ADDR);
    hi2 = inl(RTC_ADDR + 4);
  } while (hi != hi2);
  uptime->us = ((uint64_t)hi << 32) | lo;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}