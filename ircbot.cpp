#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <netdb.h>
#include <stdlib.h>
#include <map>
#include <vector>

#include "ircbot.h"

// headers for lua
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

using namespace std;

// define a data size for the input line
#define DATA_SIZE 256

bool connected;

/* this needs to be a global variable because of a funky issue with the fact
   that C++ functions need specific signatures for lua to call them */
string channelSendString = "";

int connectionSocket;

/* integer return codes from functions.
 * 0 is standard success message
 * 1 means kiwibot has been asked to be rebooted
 */
int SUCCESS = 0;
int REBOOT  = 1;
int SHUTDOWN  = 2;

IrcBot::IrcBot(string nick, string user) {
  this->nick = nick;
  this->user  = user;

  // a map from file name to hash of the file
  map<string, string> luaFileHashes;

  message = "";
}

// we close the connection when deconstructing the bot
IrcBot::~IrcBot() {
  // close down the lua plugin
  lua_close(luaState);

  // close the socket
  close (connectionSocket);
}

/* same as sendMessage but called by the Lua code
 * we need a new function because C++ functions must have a specific signature
 * if lua functions are to call them */
int sendLuaMessage(lua_State *luaState) {

  // the channel name should not be hard coded here, change this.
  string msg = channelSendString;
  lua_gettop(luaState);

  // 1 is the first index of the array sent back (we only send one string to this function)
  msg  += lua_tostring(luaState, 1);

  // add carridge return and new line to the end of messages
  msg += "\r\n";

  send(connectionSocket,msg.c_str(),msg.length(),0);
  return 0;
}

std::string runProcessWithReturn(const char* cmd) {
    FILE* pipe = popen(cmd, "r");
    if (!pipe)
      return "an error occurred";

    char buffer[256];
    string stdout = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
                stdout += buffer;
    }
    pclose(pipe);
    return stdout;
}

/* initialises the irc bot. Gives birth to a new kiwi,
   and sets of sockets for communication */
void IrcBot::init(string channel) {

  channelSendString = "PRIVMSG "+channel+" :";

  // set up our fun friend - the kiwi
  Kiwi kiwi = Kiwi();
  this->kiwi = kiwi;

  // load the plugin system
  this->luaState = lua_open();
  luaL_openlibs(luaState);
  lua_register(luaState, "sendLuaMessage", sendLuaMessage);
  std::cerr << "-- Loading plugin: " << "lua/loader.lua" << std::endl;
  int status = luaL_loadfile(luaState, "lua/loader.lua");

  // check if there was an error loading the plugin loader
  if (status) {
    std::cerr << "-- error: " << lua_tostring(luaState, -1) << std::endl;
    lua_pop(luaState, 1); // remove error message
  }

  // run pcall to set up the state
  lua_pcall(luaState, 0, 0, 0);


  // initialise connections to the IRC server
  struct addrinfo hints, *serverInfo;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((getaddrinfo("irc.freenode.net","6667",&hints,&serverInfo)) != 0) {
    cout << "Error getting address information. Exiting.";
    return;
  }

  // set up the socket
  if ((connectionSocket = socket(serverInfo->ai_family,serverInfo->ai_socktype,serverInfo->ai_protocol)) < 0) {
    cout << "Error with initialising the socket. Exiting.";
    return;
  }

  // connect to the server
  if (connect(connectionSocket,serverInfo->ai_addr, serverInfo->ai_addrlen) < 0) {
      cout << "Error with connecting to the server. Exiting.";
      close (connectionSocket);
      return;
  }

  // send nick information
  sendMessage(nick);
  checkAndParseMessages();

  // send user information
  sendMessage(user);
  checkAndParseMessages();

  string joinMessage = "JOIN ";
  joinMessage += channel;

  // join the channel
  sendMessage(joinMessage);
  checkAndParseMessages();
}

/* checks for any messages from the server, and parses any that it finds. It
 * also responds to PING request with PONG message. */
int IrcBot::checkAndParseMessages() {
  char buffer[DATA_SIZE];

  // recieve messegase from the server
  message=checkServerMessages(buffer, sizeof(buffer));

  int botStatus;

  // handle the message
  if (message != "")
    botStatus = parseMessage(message, this->kiwi);

  // check for ping messages
  if (stringSearch(message,"PING "))
    sendPong(message);

  return botStatus;
}

/* the main loop
 * we just keep checking and parsing messages forever */
int IrcBot::mainLoop () {
  char buffer [DATA_SIZE];

  while (1) {
    int botStatus = checkAndParseMessages();
    if (botStatus == REBOOT || botStatus == SHUTDOWN)
      return botStatus;
  }
}

// searches for a string in another given string
bool IrcBot::stringSearch(string toSearch, string searchFor) {
  return (toSearch.find(searchFor) != string::npos);
}

// checks for incoming server messages
string IrcBot::checkServerMessages(char* buffer, size_t size) {
  // get any messages that are incoming
  size_t total = ::recv(connectionSocket, buffer, size-1, 0);

  // add the string termination character
  buffer[total] = '\0';

  // convert to std::string and return
  string str = "";
  str.assign(buffer);
  if (str != "")
    cout << "received message: " << str << "\n" << endl;
  return str;
}

// sends an outgoing message to the IRC server
int IrcBot::sendMessage(string msg) {
  // add carridge return and new line to the end of messages
  msg += "\r\n";

  // send the message to the server
  return send(connectionSocket,msg.c_str(),msg.length(),0);
}

int IrcBot::outputToChannel(string msg) {
  sendMessage(channelSendString+msg);
}

// sends the pong packet so the IRC server knows that we're alive
void IrcBot::sendPong(string buf) {
  // ping and pong messages
  string pingMessage = "PING";
  string pongMessage = "PONG";

  // get the position of PING in the message
  int positionFound = buf.find("PING");

  // construct a pong reply
  string reply = pongMessage.append(buf.substr(positionFound+pongMessage.size(), buf.length()));
  cout << "sending message: " << reply << "\n" << endl;
  sendMessage(reply);
}

/* the function which parses the messages that the user sends
 * this should really be in its own file
 */
int IrcBot::parseMessage(string str, Kiwi kiwi) {
  cout << "Handling this string: " << str << endl;

  if (stringSearch(str, "End of /NAMES list"))
    connected = true;

  //if (connected && !stringSearch(str, "PING")) {
    /*
     * jpirie: this needs to be fixed. It seems to copy what the user enters
     *         and flood the channel. Maybe need to check that I'm actually
     *         logged on (it should do this anyway, even if that is not the bug)?
     *
     * NOTE: notifying me when i happen to be on a phone should be part of a plugin
     *       for kiwi most likely.
     */
    //string jpirieSend = "PRIVMSG jpirie :"+str;
    //sendMessage(jpirieSend);
  //}


  if (stringSearch(str, "kiwi: help")) {
    outputToChannel("I have all kinds of fun features! Syntax: \"kiwi: <command>\" on the following commands:");
    outputToChannel("\"update repo\". Updates the repository I sit in by pulling from the public http link.");
    outputToChannel("\"restart\". Shuts me down, rebuilds my binary (make clean && make kiwibot), and then me up again.");
    outputToChannel("\"shutdown\". Shuts me down. I won't come back though, please don't do that to me. :(");

    // kiwi: plugin list is handled in the plugin loader
    outputToChannel("\"plugin list\". Lists the current plugins and their activation status.");
  }
  else if (stringSearch(str, "kiwi: update repo")) {
    cout << "Updating repository..." << endl;
    outputToChannel("Update the repo? Sure thing!");
    system("cd ~/repos/kiwibot; git pull http master");
    cout << "done updating repository." << endl;
    outputToChannel("All done boss! <3");
  }
  else if (stringSearch(str, "kiwi: restart")) {
    outputToChannel("Kiwi's restarting! Maybe gonna get some tasty updates! Ooh!");
    sendMessage("QUIT");
    close (connectionSocket);  //close the open socket
    cout << "Restarting...";
    return REBOOT;
  }
  else if (stringSearch(str, "kiwi: shutdown")) {
    outputToChannel("Oh I get it. It's fine, I'm a pain sometimes I guess. Croo.");
    sendMessage("QUIT");
    close (connectionSocket);  //close the open socket
    cout << "Shutting down...";
    return SHUTDOWN;
  }

  /* it is important to check that we are passed all the initial server messages!
   * if we send some messages in the middle of the process of this, we seem to end
   * up in infinite loops (probably get caught sending messages to the server, which it
   * sends back with an error message, which we then send back for whatever reason) */
  if (connected) {
    /* we need to find out which lua files have changed since we last
     * ran them so that we don't have to relead the files again */

    // run a command to get all the lua files in the plugins folder
    string luaFilesCommand = "find lua/plugins -name \"*.lua\"";
    string luaFiles = runProcessWithReturn(luaFilesCommand.c_str());

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
      string sumOfFile = runProcessWithReturn(sumOfFileCommand.c_str());

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

    // parameter one: the current string from the server
    lua_pushstring(luaState, str.c_str());

    // parameter two: a new table for storing the updated files list
    lua_createtable(luaState, updatedFiles.size(), 0);
    int newTable = lua_gettop(luaState);
    int index = 1;
    for(vector<string>::iterator iter = updatedFiles.begin(); iter != updatedFiles.end(); ++iter) {
      // push the updated file's path onto the table
      lua_pushstring(luaState, (*iter).c_str());
      lua_rawseti(luaState, newTable, index);
      index++;
    }

    // parameter three: a new table for storing the plugins that have been deleted
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


    // call the global function that's been assigned (2 denotes the number of parameters)
    int errors = lua_pcall(luaState, 3, 0, 0);

    if ( errors!=0 ) {
      std::cerr << "-- ERROR: " << lua_tostring(luaState, -1) << std::endl;
      lua_pop(luaState, 1); // remove error message
    }

  }

  return SUCCESS;
}
