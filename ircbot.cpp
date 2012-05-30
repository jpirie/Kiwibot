#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <netdb.h>

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

int connectionSocket;

IrcBot::IrcBot(string nick, string user) {
  this->nick = nick;
  this->user  = user;

  message = "";
}

// we close the connection when deconstructing the bot
IrcBot::~IrcBot() {
  // close down the lua plugin
  lua_close(luaState);

  // close the socket
  close (connectionSocket);
}

// same as sendMessage but called by the Lua code
int sendLuaMessage(lua_State *luaState) {

  // the channel name should not be hard coded here, change this.
  string msg = "PRIVMSG #caffeine-addicts-test :";
  int message = lua_gettop(luaState);

  // 1 is the first index of the array sent back (we only send one string to this function)
  msg  += lua_tostring(luaState, 1);

  // add carridge return and new line to the end of messages
  msg += "\r\n";

  send(connectionSocket,msg.c_str(),msg.length(),0);
  return 0;
}

/* initialises the irc bot. Gives birth to a new kiwi,
   and sets of sockets for communication */
void IrcBot::init(string channel) {

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

  lua_pcall(luaState, 0, 0, 0);   // need to do the first run or nothing works it seems


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
void IrcBot::checkAndParseMessages() {
  char buffer[DATA_SIZE];

  // recieve messegase from the server
  message=checkServerMessages(buffer, sizeof(buffer));

  // handle the message
  parseMessage(message, this->kiwi);

  // check for ping messages
  if (stringSearch(message,"PING "))
    sendPong(message);
}

/* the main loop
 * we just keep checking and parsing messages forever */
void IrcBot::mainLoop () {
  char buffer [DATA_SIZE];

  while (1) {
    checkAndParseMessages();
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
void IrcBot::parseMessage(string str, Kiwi kiwi) {
  cout << "Handling this string: " << str << endl;

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

  lua_getglobal(luaState, "main");       //get ready to call the main function
  lua_pushstring(luaState, str.c_str()); //with the current string from the server as a parameter
  lua_pcall(luaState, 1, 0, 0);          //call the function!
}
