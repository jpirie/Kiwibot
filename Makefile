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
	g++ --output kiwibot -L/u1/pg/jp95/software/lua-5.1.5/lib -I/u1/pg/jp95/software/lua-5.1.5/include  main.cpp ircbot.cpp kiwi.cpp -llua -ldl

clean:
	rm -f kiwibot *~