#include <iostream>
#include "ircbot.h"

using namespace std;

int main() {

  /* sets nickname and user information for the bot */
  IrcBot bot = IrcBot("NICK test11235","USER guest tolmoon tolsun :Ronnie Regan");

  /* initialise the bot on channel specified in parameter*/
  bot.init("#caffeine-addicts-test");

  /* start the main loop where we look at the messages coming in and out */
  bot.mainLoop();

  return 0;
}
