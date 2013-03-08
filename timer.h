/*********************************************************************
 * Copyright 2013 John Pirie
 *
 * Kiwibot is a free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kiwibot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kiwibot.  If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/

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
};

#endif /* TIMER_H_ */
