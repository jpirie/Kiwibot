#include <iostream>
#include "ircbot.h"

using namespace std;

int main(int argc, char* argv[]) {

  // set nick and channel information
  string nick = "KaffinatedKiwi";
  string channel = "#caffeine-addicts";
  string password = "";

  /* the first argument is the password for kiwi to authenticate
   * note: the arguments shouldn't be done this way, there should
   *       be some kind of nice system where we have
   *       ./kiwibot -p <password> --somethingElse <whatever> */
  if (argv[1])
    password = argv[1];

  IrcBot bot = IrcBot(nick,"USER guest tolmoon tolsun :Ronnie Regan");

  //initialise the bot on channel specified in parameter
  bot.init(channel, password);

  /* start the main loop where we look at the messages coming in and out */
  int botStatus = bot.mainLoop();

  return botStatus;
}
