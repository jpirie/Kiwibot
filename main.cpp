#include <iostream>
#include "ircbot.h"

using namespace std;

int main(int argc, char* argv[]) {

  // set nick and channel information
  string nick = "KaffinatedKiwi";
  string channel = "#caffeine-addicts";
  string password = "";

  int argumentCounter = 1;

  while (argv[argumentCounter]) {
    // convert arg to string
    std::string arg = argv[argumentCounter];

    // check arguments
    if (arg == "-p" || arg == "--password") {
      if (argv[argumentCounter+1]) {
	password = argv[argumentCounter+1];
	argumentCounter += 2;
      }
      else {
	cout << "Password argument requires a parameter" << endl;
	return 1;
      }
    }
    if (arg == "-c" || arg == "--channel") {
      if (argv[argumentCounter+1]) {
	channel = argv[argumentCounter+1];
	argumentCounter += 2;
      }
      else {
	cout << "Channel argument requires a parameter" << endl;
	return 1;
      }
    }
    if (arg == "-n" || arg == "--nick") {
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
    }
  }

  IrcBot bot = IrcBot(nick,"USER guest tolmoon tolsun :Ronnie Regan");

  //initialise the bot on channel specified in parameter
  bot.init(channel, password);

  /* start the main loop where we look at the messages coming in and out */
  int botStatus = bot.mainLoop();

  return botStatus;
}
