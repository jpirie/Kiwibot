#ifndef IRCBOT_H_
#define IRCBOT_H_

#include <string.h>

#include "kiwi.h"

// headers for lua
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

class IrcBot {
public:
  IrcBot(std::string, std::string);
  virtual ~IrcBot();

  bool setup;

  void init(std::string);

  void mainLoop();

 private:
  std::string message;
  char *port;

  Kiwi kiwi;

  lua_State *luaState;

  std::string nick;
  std::string user;

  bool stringSearch(std::string toSearch, std::string searchFor);
  std::string checkServerMessages (char*, size_t);

  char * timeNow();

  int sendMessage(std::string);
  //int sendLuaMessage(lua_State*);

  void sendPong(std::string buf);

  void parseMessage(std::string buf, Kiwi);
  void checkAndParseMessages ();

};

#endif /* IRCBOT_H_ */
