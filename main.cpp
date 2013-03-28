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

using namespace std;

int main(int argc, char* argv[]) {

  // set nick and channel information
  string nick = "KaffinatedKiwi";
  string channel = "#caffeine-addicts";
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
    else if (arg == "-c" || arg == "--channel") {
      if (argv[argumentCounter+1]) {
	channel = argv[argumentCounter+1];
	argumentCounter += 2;
      }
      else {
	cout << "Channel argument requires a parameter" << endl;
	return 1;
      }
    }
    else if (arg == "-n" || arg == "--nick") {
      if (argv[argumentCounter+1]) {
	nick = argv[argumentCounter+1];
	argumentCounter += 2;
      }
      else {
	cout << "Nick argument requires a parameter" << endl;
	return 1;
      }
    }
    else if (arg == "-d" || arg == "--debug") {
      nick = "kiwitest";
      channel = "#caffeine-addicts-test";
      argumentCounter ++;
    }
    else if (arg == "--help" || arg == "-?" || arg == "-h") {
      cout << "Arguments list:" << endl;
      cout << "\t-p | --password <password>: Sets the password" << endl;
      cout << "\t-c | --channel <room name>: Sets the room channel name to " << endl;
      cout << "\t-n | --nick <nickname>: Sets the nickname to <nickname>" << endl;
      cout << "\t-d | --debug: Kiwi joins channel '#caffeine-addicts-test' and has nick 'kiwitest'" << endl;
      cout << "\t-h | --help: Displays this help text" << endl;
      argumentCounter++;
      connect = false;
    }
    else {
      cout << "Unrecognised argument: " << arg << endl;
      argumentCounter++;
      connect = false;
    }
  }

  int botStatus = SUCCESS;
  if (connect) {
    while (botStatus == SUCCESS | botStatus == DISCONNECTED) {
      IrcBot bot = IrcBot(nick,"USER guest tolmoon tolsun :Ronnie Regan");

      //initialise the bot on channel specified in parameter
      bot.init(channel, password);

      /* start the main loop where we check for messages */
      botStatus = bot.mainLoop();

      if (botStatus == DISCONNECTED)
	cout << "DISCONNECTED status reported. Re-connecting the kiwi to the world..." << endl;
    }
  }

  return botStatus;
}
