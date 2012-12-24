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
  int outputToSource(std::string, std::string, bool);

  void init(std::string, std:: string);

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
  void loadData();

  char * timeNow();

  void sendPong(std::string buf);

  int parseMessage(std::string buf, Kiwi);
  void parsePrivateMessage(std::string buf, Kiwi);
  int checkAndParseMessages ();

};

#endif /* IRCBOT_H_ */
