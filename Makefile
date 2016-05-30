######################################################################
# Copyright 2012 2013 William Gatens
# Copyright 2012 2013 John Pirie
# Copyright 2015 2015 Peter Gatens
#
# Kiwibot is a free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Kiwibot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Kiwibot.  If not, see <http://www.gnu.org/licenses/>.
#
# Description: Makefile for kiwibot.
######################################################################

# Includes are different between Kiwi home machine and my local machine. May need changes to get it compiling locally on your machine (should work for Kiwi though).
LUA_LINK=llua -ldl
LUA_INCLUDE=
PYTHON_C_FLAGS=`python3-config --cflags`
PYTHON_LD_FLAGS=`python3-config --ldflags`
FILES=main.cpp lua-interface.cpp python-interface.cpp system-utils.cpp ircbot.cpp timer.cpp  

default:
	@echo "The make options are:"
	@echo "  - make kiwibot: Remeves an existing kiwibot binary and builds a new one"
	@echo "  - make clean:   Removes an existing kiwibot binary and backup (*~) files"

kiwibot:
	rm -f kiwibot
	g++ --output kiwibot $(LUA_INCLUDE) $(PYTHON_C_FLAGS) $(FILES) $(PYTHON_LD_FLAGS) $(LUA_LINK)

clean:
	rm -f kiwibot *~
