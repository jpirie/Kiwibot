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
 ********************************************************************/

#ifndef LUA_INTERFACE_H
#define LUA_INTERFACE_H

#include "ircbot.h"
#include "system-utils.h"

// headers for lua
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

class LuaInterface {
 private:
  // set to false when we have loaded the data from plugins for the first time
  bool firstPluginLoad;
  std::map<std::string, std::string> luaFileHashes;
  static IrcBot* ircbot;
  static SystemUtils* systemUtils;
  static int sendLuaMessage(lua_State*);
  static int sendLuaChannelMessage(lua_State*);
  static int sendLuaMessageToSource(lua_State*);
  static int getPluginData(lua_State *luaState);
  static int setPluginData(lua_State *luaState);
  static int runSystemCommand(lua_State*);
  static int runSystemCommandWithOutput(lua_State*);
  static int getBotName(lua_State*);
  static int getChannelName(lua_State*);
  static int sendLuaPrivateMessage(lua_State*);
  static int getAuthenticatedUsernames(lua_State *luaState);

 public:
  LuaInterface();
  LuaInterface(std::string);
  lua_State *luaState;
  void initState(IrcBot*);
  void savePluginData();
  void loadPluginData();
  void closeState();
  void runPlugins(std::string,std::string,std::string,bool);
  lua_State* getState();
};

#endif /* LUA_INTERFACE_H_ */
