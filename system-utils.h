/*********************************************************************
 * Copyright 2012 2013 John Pirie
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
 ********************************************************************/

#ifndef SYSTEM_UTILS_H_
#define SYSTEM_UTILS_H_

#include <iostream>
#include <string>

class SystemUtils {
public:
  SystemUtils();
  std::string createTempFile();
  std::string runProcessWithReturn(const char*);
};

#endif /* SYSTEM_UTILS_H_ */
