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
 *
 * Description: A basic timer class with a start function (also
 *              resets), a function to get the time that has passed.
 ********************************************************************/

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

