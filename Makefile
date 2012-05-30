default:
	@echo "The make options are:"
	@echo "  - make kiwibot: Remeves an existing kiwibot binary and builds a new one"
	@echo "  - make clean:   Removes an existing kiwibot binary and backup (*~) files"

kiwibot:
	rm -f kiwibot
	g++ --output kiwibot -I/usr/include/luabind -llua main.cpp ircbot.cpp kiwi.cpp

clean:
	rm -f kiwibot *~