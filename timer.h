#ifndef TIMER_H_
#define TIMER_H_

#include <time.h>

class Timer {
  private:
      unsigned long begTime;

  public:
      Timer();
      void start();
      unsigned long elapsedTime();
      bool isTimeout(unsigned long seconds);
};

#endif /* TIMER_H_ */
