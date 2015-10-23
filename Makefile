######################################################################
# Copyright 2012 2013 William Gatens
# Copyright 2012 2013 John Pirie
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
# Description: Makefile for kiwibot. Note that there are some
#              hard-coded paths in the kiwibot target as it currently
#              runs on jpirie's office machine. These hard-coded paths
#              will be harmless on other machines.
######################################################################

LUA_LINK=
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
