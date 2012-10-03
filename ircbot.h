#ifndef IRCBOT_H_
#define IRCBOT_H_

#include <string.h>
#include <map>

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
  int outputToChannel(std::string);
  int outputToUser(std::string, std::string);

  int getBotName(lua_State*);
  void init(std::string, std:: string);

  std::string getBotName();
  int mainLoop();

 private:
  std::map<std::string, std::string> luaFileHashes;

  std::string message;
  char *port;

  Kiwi kiwi;

  lua_State *luaState;

  std::string nick;
  std::string user;

  // values to help tell if joe is in his office
  bool reportJoeStatus;
  int joeStatus;

  std::string checkServerMessages (char*, size_t);
  void saveData();

  char * timeNow();

  int sendMessage(std::string);

  void sendPong(std::string buf);

  int parseMessage(std::string buf, Kiwi);
  int checkAndParseMessages ();

};

#endif /* IRCBOT_H_ */
