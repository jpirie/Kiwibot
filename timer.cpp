#include <iostream>
#include <time.h>

#include "timer.h"

Timer::Timer() {
  begTime = 0;
}

void Timer::start() {
  begTime = time(NULL);
}

unsigned long Timer::elapsedTime() {
  return ((unsigned long) time(NULL) - begTime);
}

bool Timer::isTimeout(unsigned long seconds) {
  return seconds >= elapsedTime();
}
