/*********************************************************************
 * Copyright 2012 2013 William Gatens
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
 * Description: Contains program entry point, deals with the parsing
 *              of parameters.
 ********************************************************************/

#include <iostream>
#include <unistd.h>
#include "timer.h"
#include "ircbot.h"
#include <time.h>

using namespace std;

int main(int argc, char* argv[]) {

  // set nick and channel information
  string nick = "KaffinatedKiwi";
  string channel = "#caffeine-addicts";
  string dataFile = "./plugin-data.sav";
  string password = "";
  bool connect = true;

  int SUCCESS = 0;        // success flag
  int DISCONNECTED  = 1;  // bot has been disconnected, shut it down
  int SHUTDOWN  = 2;      // shutdown the bot

  int argumentCounter = 1;

  while (argv[argumentCounter]) {
    // convert arg to string
    std::string arg = argv[argumentCounter];

    // check arguments
    if (!arg.compare("-p") || !arg.compare("--password")) {
      if (argv[argumentCounter+1]) {
	password = argv[argumentCounter+1];
	argumentCounter+=2;
      }
      else {
	cout << "Password argument requires a parameter" << endl;
	return 1;
      }
    }
    else if (!arg.compare("-c") || !arg.compare("--channel")) {
      if (argv[argumentCounter+1]) {
	channel = argv[argumentCounter+1];
	argumentCounter += 2;
      }
      else {
	cout << "Channel argument requires a parameter" << endl;
	return 1;
      }
    }
    else if (!arg.compare("-s") || !arg.compare("--save-file")) {
      if (argv[argumentCounter+1]) {
	dataFile = argv[argumentCounter+1];
	argumentCounter += 2;
      }
      else {
	cout << "Use save file argument requires a parameter" << endl;
	return 1;
      }
    }
    else if (!arg.compare("-n") || !arg.compare("--nick")) {
      if (argv[argumentCounter+1]) {
	nick = argv[argumentCounter+1];
	argumentCounter += 2;
      }
      else {
	cout << "Nick argument requires a parameter" << endl;
	return 1;
      }
    }
    else if (!arg.compare("-d") || !arg.compare("--debug")) {
      nick = "kiwitest";
      channel = "#caffeine-addicts-test";
      argumentCounter ++;
    }
    else if (!arg.compare("--help") || !arg.compare("-?") || !arg.compare("-h")) {
      cout << "Arguments list:" << endl;
      cout << "\t-p | --password <password>: Sets the password." << endl;
      cout << "\t-c | --channel <room name>: Sets the room channel name." << endl;
      cout << "\t-s | --save-file <file>: Sets the file data is saved to. Default is ./kiwi-data.sav" << endl;
      cout << "\t-n | --nick <nickname>: Sets the nickname to <nickname>." << endl;
      cout << "\t-d | --debug: Kiwi joins channel '#caffeine-addicts-test' and has nick 'kiwitest'." << endl;
      cout << "\t-h | --help: Displays this help text." << endl;
      argumentCounter++;
      connect = false;
    }
    else {
      cout << "Unrecognised argument: " << arg << ". Use argument -h to see a list of valid arguments." << endl;
      argumentCounter++;
      connect = false;
    }
  }

  int botStatus = DISCONNECTED;
  bool initialised = false;
  int sleepTimer = 0;
  const int MAX_SLEEP_TIME = 15 * 60; // 15 minutes

  if (connect) {
	  
    while ((botStatus == SUCCESS) | (botStatus == DISCONNECTED)) {
    	
      IrcBot bot = IrcBot(nick,"USER guest tolmoon tolsun :Ronnie Regan");

      //initialise the bot on channel specified in parameter
      initialised = bot.init(channel, password, dataFile);

      /* start the main loop where we check for messages */
      if(initialised) {
          botStatus = bot.mainLoop();
          sleepTimer = 0;
      } 
      
      if (initialised && botStatus == DISCONNECTED) {
    	  
    	initialised  = false;
        cout << "DISCONNECTED status reported. Re-connecting the kiwi to the world..." << endl;
        
      } else {
        // Count up in 10 seconds up to 1 minute for reconnection tries
        if(sleepTimer < 60) {
            sleepTimer += 10;
        }
        // After 1 minute extend the sleep by another minute - up to 15 minutes
        else if(sleepTimer < MAX_SLEEP_TIME) {
            sleepTimer += 60;
        }
        
        cout << "Failed to connect kiwi. Trying again in ";
        if(sleepTimer < 60)
        	cout << sleepTimer << " seconds. Creeeee!" << endl;
        else
        	cout << (sleepTimer/60l) << " minutes. Creee Creee CREEEEEEEEEEEE!";
        
        struct timespec req = {0};
        req.tv_sec = sleepTimer;
        req.tv_nsec = 0;
        nanosleep(&req, (struct timespec *)NULL);
        
      }
    }
  }

  return botStatus;
}
