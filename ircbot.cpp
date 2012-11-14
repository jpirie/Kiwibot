#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <netdb.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <ctime>
#include <iomanip>

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

// a flag to let us know if we are now connected to channel (not receiving more startup messages)
bool connected;

// set to false when we have loaded the data from plugins for the first time
bool firstPluginLoad = true;

// a boolean to toggle whether history is being logged
bool loggingHistory = true;

vector<string> authenticatedUsernames;

/* this needs to be a global variable because of a funky issue with the fact
   that C++ functions need specific signatures for lua to call them */
string channelSendString = "";
string channelName = "";
string ircbotName = "";

int connectionSocket;

// integer return codes from functions.
int SUCCESS = 0;   // success flag
int SHUTDOWN  = 2; // shutdown the kiwi

IrcBot::IrcBot(string nick, string user) {
  this->nick = nick;
  ircbotName = nick;
  this->user  = user;

  map<string, string> luaFileHashes; // a map from file name to hash of the file

  message = "";
}

// we close the connection when deconstructing the bot
IrcBot::~IrcBot() {
  lua_close(luaState);      // close down the lua plugin
  close (connectionSocket); // close the socket
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

// searches for a string in another given string
bool stringSearch(string toSearch, string searchFor) {
  return (toSearch.find(searchFor) != string::npos);
}


/* check to see if there is a history log file available
 * if there isn't, then create one and add the new string received
 * if there is, open it in append mode and add the new string received */
void writeHistory(string username, string serverInfo, string userMessage) {

   if (!loggingHistory || !connected || stringSearch(userMessage, ircbotName+": history start") || stringSearch(userMessage, ircbotName+": history stop"))
     return;

   // the line that will be output to the history log
   string historyString = "";

   runProcessWithReturn("mkdir -p history/");
   // current date/time based on current system
   time_t now = time(0);
   tm *ltm = localtime(&now);
   int year = 1900 + ltm->tm_year;
   int month = 1 + ltm->tm_mon;
   int day = ltm->tm_mday;
   int hour = ltm->tm_hour;
   int minute = ltm->tm_min;
   int second = ltm->tm_sec;

   // figure out the name of the current history log file
   std::ostringstream buffer;
   buffer << "history/" << year;
   if (month >= 10)  buffer << "-" << month;
   else              buffer << "-0" << month;
   if (day >= 10)    buffer << "-" << day;
   else              buffer << "-0" << day;
   buffer << ".log";

   // set it to a permanant string so the ostringstreams don't overwrite one another
   string filename = buffer.str();
   const char* filenamePerm = filename.c_str();

   // prefix the time to the string received
   std::ostringstream historyEntry;
   if (hour >= 10)   historyEntry << hour;
   else              historyEntry << "0" << hour;
   if (minute >= 10) historyEntry << ":" << minute;
   else              historyEntry << ":0" << minute;
   if (second >= 10) historyEntry << ":" << second;
   else              historyEntry << ":0" << second;

   if (userMessage != "")
      historyString = username + ": " + userMessage;

   // there is no user message text, perhaps they are joining the channel
   if (serverInfo.find("JOIN") != string::npos)
      historyString = "=> " + username + " joined the channel\r\n";

   // for detecting them leaving the channel
   if (serverInfo.find("PART") != string::npos)
      historyString = "<= " + username + " left the channel\r\n";

   // detection of leaving IRC altogether via QUIT
   if (serverInfo.find("QUIT") != string::npos)
      historyString = "<= " + username + " quit the channel\r\n";

   // detection of leaving IRC altogether via QUIT
   size_t nickMessage = serverInfo.find("NICK");
   if (nickMessage != string::npos)
      // we use 6 here because the length of NICK: <new nickname> between N and < is 6.
      historyString = "+ " + username + " changed nickname to " + serverInfo.substr(int(nickMessage)+6);

   if (historyString != "") {
     historyEntry << " " << historyString;

     FILE * pFile;
     pFile = fopen (filenamePerm,"a");
     if (pFile!=NULL) {
       fputs (historyEntry.str().c_str(),pFile);
       fclose (pFile);
     }
   }
}

// returns the name of the kiwibot, called by lua
int getBotName(lua_State *luaState) {
  lua_pushstring(luaState, ircbotName.c_str());
  return 1;
}

/* same as sendMessage but called by the Lua code
 * we need a new function because C++ functions must have a specific signature
 * if lua functions are to call them */
int sendLuaMessage(lua_State *luaState) {
  string msg = channelSendString;
  lua_gettop(luaState);

  // 1 is the first index of the array sent back (we only send one string to this function)
  string messageOnly = lua_tostring(luaState, 1);
  msg  += messageOnly;

  // add carridge return and new line to the end of messages
  msg += "\r\n";

  // update the history file
  writeHistory(ircbotName, "", messageOnly + "\r\n");

  send(connectionSocket,msg.c_str(),msg.length(),0);
  return 0;
}

// same as sendLuaMessage but for private messages
int sendLuaPrivateMessage(lua_State *luaState) {
  string msg = "PRIVMSG ";
  lua_gettop(luaState);

  // 1 is the first index of the argument array sent back, the username
  string username = lua_tostring(luaState, 1);
  msg  += username;
  msg  += " :";
  // 2 is the second index of the argument array sent back, the string
  string message = lua_tostring(luaState, 2);
  msg  += message;

  // add carridge return and new line to the end of messages
  msg += "\r\n";

  send(connectionSocket,msg.c_str(),msg.length(),0);
  return 0;
}

/* initialises the irc bot. Gives birth to a new kiwi,
   and sets of sockets for communication */
void IrcBot::init(string channel, string password) {

  channelSendString = "PRIVMSG "+channel+" :";
  channelName = channel;

  // set up our fun friend - the kiwi
  Kiwi kiwi = Kiwi();
  this->kiwi = kiwi;

  // load the plugin system
  this->luaState = lua_open();
  luaL_openlibs(luaState);
  lua_register(luaState, "sendLuaMessage", sendLuaMessage);
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


  // initialise connections to the IRC server
  struct addrinfo hints, *serverInfo;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  cout << "connecting to server..." << endl;

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
  sendMessage("NICK "+nick);
  checkAndParseMessages();

  // send user information
  sendMessage(user);
  checkAndParseMessages();

  // send authentication to NickServ if kiwi set with password
  if (password != "") {
    string authMessage = "PRIVMSG NickServ : identify " + nick + " " + password;
    sendMessage(authMessage);
  }
  checkAndParseMessages();

  // join the channel
  string joinMessage = "JOIN ";
  joinMessage += channel;
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

  // check for ping messages
  if (stringSearch(message,"PING "))
    sendPong(message);
  else if (message != "")
    botStatus = parseMessage(message, this->kiwi);

  return botStatus;
}

/* the main loop
 * we just keep checking and parsing messages forever */
int IrcBot::mainLoop () {
  char buffer [DATA_SIZE];

  while (1) {
    int botStatus = checkAndParseMessages();
    if (botStatus == SHUTDOWN)
      return botStatus;
  }
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
    cout << "received message: " << str << endl;
  return str;
}

// sends an outgoing message to the IRC server
int IrcBot::sendMessage(string msg) {
  msg += "\r\n";
  // send the message to the server
  return send(connectionSocket,msg.c_str(),msg.length(),0);
}

// outputs a message to the irc channel
int IrcBot::outputToChannel(string msg) {
  writeHistory(this->nick.c_str(), "", msg+"\r\n");
  sendMessage(channelSendString+msg);
}

// outputs a message to the irc channel
int IrcBot::outputToUser(string username, string msg) {
  sendMessage("PRIVMSG "+username+" :"+msg);
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

void IrcBot::saveData() {
    // get ready to call the "main" function
    lua_getglobal(luaState, "savePluginData");

    // call the global function that's been assigned (0 denotes the number of parameters)
    int errors = lua_pcall(luaState, 0, 0, 0);

    if ( errors!=0 ) {
      std::cerr << "-- ERROR: " << lua_tostring(luaState, -1) << std::endl;
      lua_pop(luaState, 1); // remove error message
    }
}

void IrcBot::loadData() {
    // get ready to call the "main" function
    lua_getglobal(luaState, "loadPluginData");

    // call the global function that's been assigned (0 denotes the number of parameters)
    int errors = lua_pcall(luaState, 0, 0, 0);

    if ( errors!=0 ) {
      std::cerr << "-- ERROR: " << lua_tostring(luaState, -1) << std::endl;
      lua_pop(luaState, 1); // remove error message
    }
}

// returns the username of the full server message
std::string getUsername(string fullMessage) {
  string username = "";
  size_t findUsername = fullMessage.find("!");
  if (findUsername != string::npos)
    username = fullMessage.substr(1, findUsername-1); // we leave out the first character, it's irrelevant
  return username;
}

// returns the server information part of the full server message
std::string getServerInfo(string fullMessage) {
  size_t findEndOfServerInfo = fullMessage.find(channelName);
  if (findEndOfServerInfo != string::npos)
    return fullMessage.substr(1, int(findEndOfServerInfo)+channelName.size());
  else
      return fullMessage;
}

std::string getUserMessage(string fullMessage) {
   size_t findEndOfServerInfo = fullMessage.find(channelName);
   if (findEndOfServerInfo != string::npos)
      return fullMessage.substr(int(findEndOfServerInfo)+channelName.size()+2);
   else
      return "";
}

/* the function which parses the messages that the user sends
 * this should really be in its own file
 */
int IrcBot::parseMessage(string str, Kiwi kiwi) {

  string username = getUsername(str);
  string serverInfo = getServerInfo(str);
  string userMessage = getUserMessage(str);

  /* the returned str is just what the user text is without the IRC prefix information
   * note: this should not be done this way, a function should be called which has the
   *       sole purpose of getting the user text from the full message */
  writeHistory(username, serverInfo, userMessage);

  if (stringSearch(str, "End of /NAMES list"))
    connected = true;  // intro messages to the server have finished

  if (serverInfo.find("JOIN") != string::npos) {
    string checkAccess = "PRIVMSG NickServ : ACC " + username;
    sendMessage(checkAccess);
  }
  // for detecting them leaving the channel
  if (serverInfo.find("PART") != string::npos || serverInfo.find("QUIT") != string::npos) {
    std::vector<string>::iterator position = std::find(authenticatedUsernames.begin(), authenticatedUsernames.end(), username);
    if (position != authenticatedUsernames.end()) // == vector.end() means the element was not found
      authenticatedUsernames.erase(position);
  }

  if (stringSearch(userMessage, ircbotName+": help")) {
    outputToUser(username, "I have all kinds of fun features! Syntax: \""+ircbotName+": <command>\" on the following commands:");
    outputToUser(username, "\"check authentication\". Checks user access information via NickServ.");
    outputToUser(username, "\"give history\". Creates tarball of history and puts link on web. (Must be simown, sadger, or jpirie)");
    outputToUser(username, "\"hide history\". Removes existing tarball of history on web (Must be simown, sadger, or jpirie)");
    outputToUser(username, "\"give op status\". Gives op status (must be sadger, simown, or jpirie)");
    outputToUser(username, "\"take op status\". Take op status away (must be sadger, simown, or jpirie)");
    outputToUser(username, "\"update repo\". Updates the repository I sit in by pulling from the public http link.");
    outputToUser(username, "\"save data\". Saves data of all "+ircbotName+" plugins.");
    outputToUser(username, "\"shutdown\". Shuts me down. I won't come back though, please don't do that to me. :(");
    outputToUser(username, "\"history <start|stop>\". Starts/stops logging channel conversation.");
    outputToUser(username, "\"plugin list\". Lists the current plugins and their activation status.");
  }
  else if (stringSearch(userMessage, ircbotName+": update repo")) {
    cout << "Updating repository..." << endl;
    outputToChannel("Update the repo? Sure thing!");
    system("cd ~/repos/kiwibot; git pull http master; git pull http dynamic-plugins");
    cout << "done updating repository." << endl;
    outputToChannel("All done boss! <3");
  }
  else if (stringSearch(userMessage, ircbotName+": save data")) {
    cout << "Saving all "+ircbotName+" data..." << endl;
    saveData();
    cout << "Saving all "+ircbotName+" data..." << endl;
  }
  else if (stringSearch(userMessage, ircbotName+": load data")) {
    cout << "Loading all "+ircbotName+" data..." << endl;
    loadData();
    cout << "done!..." << endl;
  }
  else if (stringSearch(userMessage, ircbotName+": give history")) {
    if (username == "sadger" || username == "jpirie" || username == "simown") {
      if (find(authenticatedUsernames.begin(), authenticatedUsernames.end(), username) != authenticatedUsernames.end()) {
	string historyCommand = "tar -czf kiwi-history.tar.gz history/; mv kiwi-history.tar.gz ~/public_html/";
	runProcessWithReturn(historyCommand.c_str());
	outputToUser(username, "Latest history tarball available at http://www.macs.hw.ac.uk/~jp95/kiwi-history.tar.gz.");
      }
      else
	outputToUser(username, "You must be authenticated with NickServ to use this command.");
    }
    else
      outputToUser(username, "Only sadger, jpirie, or simown can use this command.");
  }
  else if (stringSearch(userMessage, ircbotName+": hide history")) {
    if (username == "sadger" || username == "jpirie" || username == "simown") {
      if (find(authenticatedUsernames.begin(), authenticatedUsernames.end(), username) != authenticatedUsernames.end()) {
	string historyCommand = "rm ~/public_html/kiwi-history.tar.gz";
	runProcessWithReturn(historyCommand.c_str());
	outputToUser(username, "Tarball deleted.");
      }
      else
	outputToUser(username, "You must be authenticated with NickServ to use this command.");
    }
    else
      outputToUser(username, "Only sadger, jpirie, or simown can use this command.");

  }
  else if (stringSearch(userMessage, ircbotName+": give op status")) {
    if (username == "sadger" || username == "jpirie" || username == "simown") {
      if (find(authenticatedUsernames.begin(), authenticatedUsernames.end(), username) != authenticatedUsernames.end())
	sendMessage("MODE "+channelName+" +o "+username);
      else
	outputToUser(username, "You must be authenticated with NickServ to use this command.");
    }
    else
      outputToUser(username, "Only sadger, jpirie, or simown can use this command.");
  }
  else if (stringSearch(userMessage, ircbotName+": take op status")) {
    if (username == "sadger" || username == "jpirie" || username == "simown") {
      if (find(authenticatedUsernames.begin(), authenticatedUsernames.end(), username) != authenticatedUsernames.end())
	sendMessage("MODE "+channelName+" -o "+username);
      else
	outputToUser(username, "You must be authenticated with NickServ to use this command.");
    }
    else
      outputToUser(username, "Only sadger, jpirie, or simown can use this command.");


  }
  else if (stringSearch(userMessage, ircbotName+": shutdown")) {
    outputToChannel("Oh I get it. It's fine, I'm a pain sometimes I guess. Croo.");
    sendMessage("QUIT");
    close (connectionSocket);  //close the open socket
    cout << "Shutting down...";
    return SHUTDOWN;
  }
  else if (stringSearch(userMessage, ircbotName+": history start")) {
    if (loggingHistory) {
       loggingHistory = false;
       outputToChannel("I'm already keeping my ears peeled for tasty parsable information!");
       loggingHistory = true;
    }
    else {
       outputToChannel("My ears are now open, I shall store future conversations for tasty parsing later.");
       loggingHistory = true;
    }
  }
  else if (stringSearch(userMessage, ircbotName+": history stop")) {
    if (loggingHistory) {
       loggingHistory = false;
       outputToChannel("Ah I see, cunning secret conversations. I know not of what you speak, your sneakrets are safe with me.");
    }
    else
       outputToChannel("My ears are already shut to your sneakiness. I'll keep it secret. I'll keep it safe.");
  }
  else if (stringSearch(userMessage, ircbotName+": check authentication")) {
    outputToChannel("I'll get on the dog and bone to NickServ straight away.");
    string checkAccess = "PRIVMSG NickServ : ACC " + username;
    sendMessage(checkAccess);
  }
  /* we check the whole string here (str) not the userMessage, because that's not set up
   * right for NickServ requests */
  else if (stringSearch(str, "ACC 3") && username == "NickServ") {
    size_t separatingColon = str.substr(1, str.length()).find(":");
    // we add two to nicknameStart becuase we ignore the starting colon in the message
    string newAuthenticatedMember = str.substr(separatingColon+2, str.length()-str.find("ACC")-1);
    authenticatedUsernames.push_back(newAuthenticatedMember);
    cout << "New authenticated username: '" << newAuthenticatedMember << "'" << endl;
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

    lua_pushstring(luaState, username.c_str());   // parameter one: the user's name who may be talking
    lua_pushstring(luaState, serverInfo.c_str()); // parameter two: the server part of the message
    lua_pushstring(luaState, userMessage.c_str()); // parameter three: the user message

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

    // parameter five: a new table for storing the plugins that have been deleted
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
    int errors = lua_pcall(luaState, 5, 0, 0);

    if (firstPluginLoad) {
      loadData();
      firstPluginLoad = false;
    }

    if ( errors!=0 ) {
      std::cerr << "-- ERROR: " << lua_tostring(luaState, -1) << std::endl;
      lua_pop(luaState, 1); // remove error message
    }

  }

  return SUCCESS;
}
