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

default:
	@echo "The make options are:"
	@echo "  - make kiwibot: Remeves an existing kiwibot binary and builds a new one"
	@echo "  - make clean:   Removes an existing kiwibot binary and backup (*~) files"

kiwibot:
	rm -f kiwibot
	# jpirie: the -llua and -l dl bits MUST go at the end, because if they don't
	#         this doesn't build on my office machine. The -L and -I parameters are
	#	  also there because the lua libraries are not installed by default on
	#         my office machine, but it won't throw errors if those directories don't
        #	  exist ;)
	g++ --output kiwibot -L/u1/pg/jp95/software/lua-5.1.5/lib -I/u1/pg/jp95/software/lua-5.1.5/include  main.cpp ircbot.cpp timer.cpp -llua -ldl

clean:
	rm -f kiwibot *~
