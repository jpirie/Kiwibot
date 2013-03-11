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
 * Description: Contains
 ********************************************************************/

#include <iostream>
#include <string>
#include <netdb.h>
#include <sstream>
#include <vector>
#include <algorithm>

#include "ircbot.h"
#include "lua-interface.h"

using namespace std;

// set to false when we have loaded the data from plugins for the first time
bool firstPluginLoad = true;

IrcBot* LuaInterface::ircbot;
SystemUtils* LuaInterface::systemUtils;

/* same as sendMessage but called by the Lua code
 * we need a new function because C++ functions must have a specific signature
 * if lua functions are to call them */
int LuaInterface::sendLuaMessage(lua_State *luaState) {
  IrcBot thisIrcBot = (*ircbot);
  string msg = (thisIrcBot).getChannelSendString();
  lua_gettop(luaState);

  // 1 is the first index of the array sent back (we only send one string to this function)
  string messageOnly = lua_tostring(luaState, 1);
  msg  += messageOnly;

  // add carridge return and new line to the end of messages
  msg += "\r\n";

  // update the history file
  (thisIrcBot).writeHistory((thisIrcBot).getNick(), "", messageOnly + "\r\n");

  send((thisIrcBot).connectionSocket,msg.c_str(),msg.length(),0);
  return 0;
}

// returns the name of the bot, called by lua
int LuaInterface::getBotName(lua_State *luaState) {
  lua_pushstring(luaState, ircbot->getNick().c_str());
  return 1;
}

// same as sendLuaMessage but for private messages
int LuaInterface::sendLuaPrivateMessage(lua_State *luaState) {
  lua_gettop(luaState);
  // first parameter: username, second parameter: message
  ircbot->outputToUser(lua_tostring(luaState, 1), lua_tostring(luaState, 2));
  return 0;
}

LuaInterface::LuaInterface() {
  map<string, string> luaFileHashes; // a map from file name to hash of the file
}

void LuaInterface::closeState() {
  lua_close(luaState);
}

void LuaInterface::savePluginData() {
    // get ready to call the "main" function
    lua_getglobal(luaState, "savePluginData");

    // call the global function that's been assigned (0 denotes the number of parameters)
    int errors = lua_pcall(luaState, 0, 0, 0);

    if ( errors!=0 ) {
      std::cerr << "-- ERROR: " << lua_tostring(luaState, -1) << std::endl;
      lua_pop(luaState, 1); // remove error message
    }
}

void LuaInterface::loadPluginData() {
    // get ready to call the "main" function
    lua_getglobal(luaState, "loadPluginData");

    // call the global function that's been assigned (0 denotes the number of parameters)
    int errors = lua_pcall(luaState, 0, 0, 0);

    if ( errors!=0 ) {
      std::cerr << "-- ERROR: " << lua_tostring(luaState, -1) << std::endl;
      lua_pop(luaState, 1); // remove error message
    }
}

void LuaInterface::runPlugins(string username, string serverInfo, string userMessage, bool isPrivateMessage) {
    /* we need to find out which lua files have changed since we last
     * ran them so that we don't have to relead the files again */

    // run a command to get all the lua files in the plugins folder
    string luaFilesCommand = "find lua/plugins -name \"*.lua\"";
    string luaFiles = (*systemUtils).runProcessWithReturn(luaFilesCommand.c_str());

    std::istringstream stream(luaFiles);
    std::string line;
    map<string,string>::iterator iter;


    // a vector to hold the list of plugins that have been updated
    vector<string> updatedFiles;

    // a vector to hold the list of plugins that have been deleted
    vector<string> deletedFiles;
    for (iter=luaFileHashes.begin(); iter!=luaFileHashes.end(); ++iter)
      deletedFiles.push_back(iter->first);

    // loop through all the plugins found
    while(std::getline(stream, line)) {
      // this file exists, removed it from the deletedFiles vector
      vector<string>::iterator vectorIter = find(deletedFiles.begin(), deletedFiles.end(), line);
      if (vectorIter != deletedFiles.end())
	deletedFiles.erase(vectorIter);

      // get the hash of the current file we are processing
      string sumOfFileCommand = "md5sum ";
      sumOfFileCommand += line;
      sumOfFileCommand += " | awk '{print $1}'";
      string sumOfFile = (*systemUtils).runProcessWithReturn(sumOfFileCommand.c_str());

      // get the existing hash we already have of the file we are processing
      iter = luaFileHashes.find(line);

      if (iter != luaFileHashes.end()) {
	//we found a hash, compare it to the newly generated hash and see if they are the same
	if (sumOfFile.compare((*iter).second) != 0) {
	  cout << "File change detected in file: " << line << ". Replacing hash." << endl;
	  luaFileHashes[line] = sumOfFile;
	  updatedFiles.push_back(line);
	}
      }
      else {
	// we didn't have an existing hash, this is a new plugin
	cout << "New plugin detected: " << line << endl;
	luaFileHashes[line] = sumOfFile;
	updatedFiles.push_back(line);
      }
    }

    // get ready to call the "main" function
    lua_getglobal(luaState, "main");

    lua_pushstring(luaState, username.c_str());   // parameter one: the user's name who may be talking
    lua_pushstring(luaState, serverInfo.c_str()); // parameter two: the server part of the message
    lua_pushstring(luaState, userMessage.c_str()); // parameter three: the user message
    lua_pushboolean(luaState, isPrivateMessage); // parameter four: whether the message is privately sent

    // parameter five: give new table for storing the updated files list
    lua_createtable(luaState, updatedFiles.size(), 0);
    int newTable = lua_gettop(luaState);
    int index = 1;
    for(vector<string>::iterator iter = updatedFiles.begin(); iter != updatedFiles.end(); ++iter) {
      // push the updated file's path onto the table
      lua_pushstring(luaState, (*iter).c_str());
      lua_rawseti(luaState, newTable, index);
      index++;
    }

    // parameter six: a new table for storing the plugins that have been deleted
    lua_createtable(luaState, deletedFiles.size(), 0);
    int deletedFilesTable = lua_gettop(luaState);
    index = 1;
    for(vector<string>::iterator iter = deletedFiles.begin(); iter != deletedFiles.end(); ++iter) {
      // the plugin has been deleted, remove it from the luaFileHashes map
      map<string,string>::iterator deletedIter = luaFileHashes.find((*iter));
      if (deletedIter != luaFileHashes.end())
      	luaFileHashes.erase(deletedIter);
      else
      	cout << "Something has gone wrong! The user has deleted a plugin that we didn't even know existed!" << endl;

      cout << "Deleted plugin file detected: " << (*iter) << endl;
      // push the updated file's path onto the table
      lua_pushstring(luaState, (*iter).c_str());
      lua_rawseti(luaState, deletedFilesTable, index);
      index++;
    }


    // call the global function that's been assigned (5 denotes the number of parameters)
    int errors = lua_pcall(luaState, 6, 0, 0);

    if (firstPluginLoad) {
      loadPluginData();
      firstPluginLoad = false;
    }

    if ( errors!=0 ) {
      std::cerr << "-- ERROR: " << lua_tostring(luaState, -1) << std::endl;
      lua_pop(luaState, 1); // remove error message
    }
}


void LuaInterface::initState(IrcBot* bot) {
  this->ircbot = bot;

  // load the plugin system
  this->luaState = luaL_newstate();
  luaL_openlibs(luaState);
  lua_register(luaState, "sendLuaMessage", sendLuaMessage);
  lua_register(luaState, "sendLuaMessageToSource", sendLuaMessageToSource);
  lua_register(luaState, "getBotName", getBotName);
  lua_register(luaState, "sendLuaPrivateMessage", sendLuaPrivateMessage);
  std::cerr << "-- Loading plugin: " << "lua/loader.lua" << std::endl;
  int status = luaL_loadfile(luaState, "lua/loader.lua");

  // check if there was an error loading the plugin loader
  if (status) {
    std::cerr << "-- error: " << lua_tostring(luaState, -1) << std::endl;
    lua_pop(luaState, 1); // remove error message
  }

  // run pcall to set up the state
  lua_pcall(luaState, 0, 0, 0);
}

/* same as sendMessage but called by the Lua code and sent to a user
 * if message was sent privately */
int LuaInterface::sendLuaMessageToSource(lua_State *luaState) {
string msg = ircbot->getChannelSendString();
  lua_gettop(luaState);

  string username = lua_tostring(luaState, 1);

  string messageOnly = lua_tostring(luaState, 2);
  msg  += messageOnly;

  bool isPrivateMessage = lua_toboolean(luaState, 3);

  // add carridge return and new line to the end of messages
  msg += "\r\n";

  if (isPrivateMessage)
    sendLuaPrivateMessage(luaState);
  else {
    // update the history file
    ircbot->writeHistory(ircbot->getNick(), "", messageOnly + "\r\n");
    send(ircbot->connectionSocket,msg.c_str(),msg.length(),0);
  }
  return 0;
}

lua_State* LuaInterface::getState() {
  return this->luaState;
}
