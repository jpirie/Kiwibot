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
 * Description: A class allowing us to run system commands (creating
 *              temp files, running processing, etc.)
 ********************************************************************/

#include <iostream>
#include <string>
#include <stdio.h>
#include <algorithm>

#include "system-utils.h"

using namespace std;

SystemUtils::SystemUtils() {}

std::string SystemUtils::runProcessWithReturn(const char* cmd) {
    FILE* pipe = popen(cmd, "r");
    if (!pipe)
      return "Failed to open a pipe to run a process.";

    char buffer[256];
    string stdout = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
                stdout += buffer;
    }
    pclose(pipe);
    return stdout;
}

std::string SystemUtils::createTempFile() {
  string output = runProcessWithReturn("mktemp");
  string::size_type i = 0;
  while (i < output.length()) {
    i = output.find('\n', i);
    if (i == string::npos)
      break;
    output.erase(i);
  }
  return output;
}
