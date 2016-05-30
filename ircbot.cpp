/*********************************************************************
 * Copyright 2012 2013 William Gatens
 * Copyright 2012 2013 John Pirie
 * Copyright 2015 2016 Peter Gatens 
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
 * Description: Class containing the code for the kiwibot. The main
 *              program loop, lua initialisation, and servers messages are done
 *              here, along with other important functions.
 ********************************************************************/

#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <poll.h>
#include <algorithm>
#include <netdb.h>
#include <stdlib.h>
#include <stdexcept>
#include <map>
#include <vector>
#include <ctime>
#include <iomanip>
#include <dirent.h>
#include <fcntl.h>

#include "lua-interface.h"
#include "python-interface.h"
#include "timer.h"
#include "system-utils.h"
#include "ircbot.h"

// headers for lua
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

using namespace std;

// define a data size for the input line
#define DATA_SIZE 512

// a flag to let us know if we are now connected to channel (not receiving more startup messages)
bool connected;

// a boolean to toggle whether history is being logged
bool loggingHistory = true;

vector<string> connectedUsernames;
vector<string> authenticatedUsernames;

LuaInterface luaInterface;
PythonInterface pythonInterface;
SystemUtils systemUtils;

/* this needs to be a global variable because of a funky issue with the fact
   that C++ functions need specific signatures for lua to call them */
string channelSendString = "";
string channelName = "";
string ircbotName = "";
string dataFile = "";

// integer return codes from functions.
int SUCCESS = 0;       // success flag
int DISCONNECTED  = 1; // bot has been disconnected, shut it down
int SHUTDOWN  = 2;     // shutdown the bot

// timeout and messages
const int MAX_PING = 300;
const int POLL_FREQUENCY = 30 * 1000; // 30 seconds in milliseconds


// maximum lines to send to the user at any one time
int MAX_LINES_OUTPUT = 5;

Timer pingTimer;    // this perhaps shouldn't be done this way, rather check for 0 return on recv call for disconnections
Timer uptimeTimer;

// holds information for private messaging to users (e.g. history searching)
struct userTextManagement {
  int textPosition;
  int currentPage;
  int numPages;
  vector<string> text;
};

map<string, userTextManagement> directUserText; // a map from file name to hash of the file

IrcBot::IrcBot(string nick, string user) {
  this->nick = nick;
  ircbotName = nick;
  this->user  = user;

  message = "";
}

// we close the connection when deconstructing the bot
IrcBot::~IrcBot() {
  luaInterface.closeState();
  pythonInterface.closeState();
  close (connectionSocket); // close the socket
}

// searches for a string in another given string
bool stringSearch(string toSearch, string searchFor) {
  return (toSearch.find(searchFor) != string::npos);
}

string IrcBot::getNick() {
  return this->nick;
}

string IrcBot::getChannel() {
  return this->channel;
}

/* check to see if there is a history log file available
 * if there isn't, then create one and add the new string received
 * if there is, open it in append mode and add the new string received */
void IrcBot::writeHistory(string username, string serverInfo, string userMessage) {

   if (!loggingHistory || !connected || stringSearch(userMessage, ircbotName+": history start") || stringSearch(userMessage, ircbotName+": history stop"))
     return;

   // the line that will be output to the history log
   string historyString = "";

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

// sends an outgoing message to the IRC server
int IrcBot::sendMessage(string msg) {
  msg += "\r\n";
  // send the message to the server
  return send(connectionSocket,msg.c_str(),msg.length(),0);
}

// outputs a message to the irc channel
int IrcBot::outputToUser(string username, string msg) {
  cout << "Message going to " << username << ": " << msg << endl;

  /* put the lines to send in a vector, we might need it if we want to send
   * the user a lot of information */
  vector<string> linesToSend;
  istringstream ss( msg );
  while (!ss.eof()) {
      string x;
      getline( ss, x, '\n' );
      linesToSend.push_back(x);
    }

  if (linesToSend.size() <= MAX_LINES_OUTPUT)
    // let's not bother with any fanciness, they only want a few lines. Send them everything.
    for(vector<string>::iterator it = linesToSend.begin(); it != linesToSend.end(); ++it) {
      sendMessage("PRIVMSG "+username+" :"+(*it));
    }
  else {
    /* we have to consider flooding in this case so we have to do something more sophisticated
     * add the lines we need to send to the user in the right hand side of a map, with the key
     * being their username */
    cout << "Too many lines to send, adding to map." << endl;
    userTextManagement currentUser;
    currentUser.textPosition = MAX_LINES_OUTPUT;
    currentUser.text = linesToSend;

    currentUser.numPages = linesToSend.size()/MAX_LINES_OUTPUT;

    // if there is a remainder, then there is going to be one extra page
    if (linesToSend.size() % MAX_LINES_OUTPUT != 0)
      currentUser.numPages++;

    currentUser.currentPage = 1;

    for (unsigned i=0; i<MAX_LINES_OUTPUT; i++)
      sendMessage("PRIVMSG "+username+" :"+linesToSend.at(i));

    std::ostringstream pageDisplay;
    pageDisplay << "PRIVMSG "+username+" :Page ";
    pageDisplay << currentUser.currentPage;
    pageDisplay << " of ";
    pageDisplay << currentUser.numPages;
    pageDisplay << ". [ Commands: 'next' (see next page); 'prev' (see previous page); 'page N' (go to page N) ]";

    directUserText[username] = currentUser;
    sendMessage(pageDisplay.str());
  }
}

string IrcBot::getChannelSendString() {
  return channelSendString;
}

// initialises the irc bot.
bool IrcBot::init(string channel, string password, string saveFile) {

  // make directories that might not exist
  systemUtils.runProcessWithReturn("mkdir -p history/");

  const char* touchCommand = ("touch "+saveFile).c_str();
  systemUtils.runProcessWithReturn(touchCommand);
  dataFile = saveFile;

  channelSendString = "PRIVMSG "+channel+" :";
  channelName = channel;
  this->channel = channel;

  // initialise connections to the IRC server
  struct addrinfo hints, *serverInfo;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  luaInterface = LuaInterface(dataFile);
  luaInterface.initState(this);

  pythonInterface = PythonInterface(dataFile);
  pythonInterface.initState(this);

  systemUtils = SystemUtils();

  cout << "connecting to server..." << endl;

  if ((getaddrinfo("irc.freenode.net","6667",&hints,&serverInfo)) != 0) {
    cout << "Error getting address information. Exiting.";
    return false;
  }

  // set up the socket
  if ((connectionSocket = socket(serverInfo->ai_family,serverInfo->ai_socktype,serverInfo->ai_protocol)) < 0) {
    cout << "Error with initialising the socket. Exiting.";
    return false;
  }

  // connect to the server
  if (connect(connectionSocket,serverInfo->ai_addr, serverInfo->ai_addrlen) < 0) {
      cout << "Error with connecting to the server. Exiting.";
      close (connectionSocket);
      return false;
  }
  
  freeaddrinfo(serverInfo);

  // send nick information
  sendMessage("NICK "+nick);
  checkAndParseMessages();

  // send user information
  sendMessage(user);
  checkAndParseMessages();

  // send authentication to NickServ if bot has been set with password
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

  /* we're now on the channel, start the pingTimer */
  pingTimer.start();
  uptimeTimer.start();

  return true;
}

/* checks for any messages from the server, and parses any that it finds. It
 * also responds to PING request with PONG message. */
int IrcBot::checkAndParseMessages() {
  char buffer[DATA_SIZE];
  int botStatus = 0;

  // recieve messages from the server (will sleep until it receives some)
  message=checkServerMessages(buffer, sizeof(buffer));

  cout << "At beginning of checkAndParseMessages. PingTimer is currently at " << pingTimer.elapsedTime() << endl;

  // check for ping messages
  if (stringSearch(message,"PING ")) {
    sendPong(message);
    pingTimer.start();
  }
  else if (message != "") {
    botStatus = parseMessage(message);
    pingTimer.start();
  }

  // if we think we have been disconnected
  if (pingTimer.elapsedTime() > MAX_PING) {
     cout << "Ping timer: " << pingTimer.elapsedTime() << endl;
     botStatus = DISCONNECTED; // restart the bot
  }

  return botStatus;
}

/* the main loop
 * we just keep checking and parsing messages forever */
int IrcBot::mainLoop () {
  char buffer [DATA_SIZE];

  while (1) {
    int botStatus = checkAndParseMessages();
    if ((botStatus == SHUTDOWN) || (botStatus == DISCONNECTED))
      return botStatus;
  }
}

// checks for incoming server messages
string IrcBot::checkServerMessages(char* buffer, size_t size) {
  // get any messages that are incoming
  cout << "Getting server messages...." << endl;

  // Poll for server messages every 30 seconds, do not block waiting for messages!
  struct pollfd fd;
  fd.fd = connectionSocket;
  fd.events = POLLIN;
  int res = poll(&fd, 1, POLL_FREQUENCY);

  string str = "";

  if((fd.revents & POLLIN) != 0) {
      int total = recv(connectionSocket, buffer, size-1, 0);
      // add the string termination character
      buffer[total] = '\0';
      // convert to std::string and return
      str.assign(buffer);
      cout << "received message: " << str << endl;
    }
    return str;
  }

// outputs a message to the irc channel
int IrcBot::outputToChannel(string msg) {
  writeHistory(this->nick.c_str(), "", msg+"\r\n");
  sendMessage(channelSendString+msg);
}

int IrcBot::outputToSource(string msg, string username, bool isPrivateMessage) {
  if (isPrivateMessage)
    outputToUser(username, msg);
  else {
    writeHistory(this->nick.c_str(), "", msg+"\r\n");
    sendMessage(channelSendString+msg);
  }
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
   else {
     // probably a private message, hence there is no channel name
     findEndOfServerInfo = fullMessage.find(ircbotName);
     if (findEndOfServerInfo != string::npos)
       return fullMessage.substr(int(findEndOfServerInfo)+ircbotName.size()+2);
     else
       return "";
   }
}

// a private message sent to the bot directly
void IrcBot::parsePrivateMessage(string str) {
  string username = getUsername(str);
  string serverInfo = getServerInfo(str);
  string userMessage = getUserMessage(str);

  cout << "username: " << username << endl;
  cout << "serverInfo: " << serverInfo << endl;
  cout << "userMesage: " << userMessage << endl;

  string pageCommand = "page ";

  /* next/prev system the user can say 'next' and 'prev' to get more
   * output from the bot where there is a lot of information that they
   * want to know */
  if (userMessage.find("next") != string::npos) {
    userTextManagement userText = directUserText[username];

    bool saidSomething = false;

    // when a user asks for 'next' without information stored for the user
    if (userText.text.empty()) {
      sendMessage("PRIVMSG "+username+" :I don't know what you want 'next' of. Perhaps my memory isn't what it used to be...");
      return;
    }
    int startPoint = userText.textPosition;
    int endPoint = userText.textPosition+MAX_LINES_OUTPUT;
    for (unsigned i=startPoint; i<endPoint; i++) {
      try {
	sendMessage("PRIVMSG "+username+" :"+userText.text.at(i));
	saidSomething = true;
	userText.textPosition = i + 1;
      }
      catch (out_of_range& outOfRange) {
	if (saidSomething)
	  userText.currentPage++;
	directUserText[username] = userText;
	std::ostringstream endOfTextDisplay;
	endOfTextDisplay << "PRIVMSG "+username+" :End of text. Page ";
	endOfTextDisplay << userText.currentPage;
	endOfTextDisplay << " of ";
	endOfTextDisplay << userText.numPages;
	endOfTextDisplay << ". [ Commands: 'prev' (see previous page); 'page N' (go to page N) ]";
	sendMessage(endOfTextDisplay.str());
	return;
      }
    }

    userText.currentPage++;

    std::ostringstream nextPageDisplay;
    nextPageDisplay << "PRIVMSG "+username+" :Page ";
    nextPageDisplay << userText.currentPage;
    nextPageDisplay << " of ";
    nextPageDisplay << userText.numPages;
    nextPageDisplay << ". [ Commands: 'next' (see next page); 'prev' (see previous page); 'page N' (go to page N) ]";
    sendMessage(nextPageDisplay.str());

    // update the map so we know where the text position is for the user
    directUserText[username] = userText;
  }
  else if (userMessage.find("prev") != string::npos) {
    userTextManagement userText = directUserText[username];
    if (userText.text.empty()) {
      // we don't have any text stored for that user
      sendMessage("PRIVMSG "+username+" :I don't know what you want 'prev' of. Perhaps my memory isn't what it used to be...");
      return;
    }
    else if ((userText.textPosition < MAX_LINES_OUTPUT) || (userText.textPosition - MAX_LINES_OUTPUT - 1 < 0) || userText.currentPage == 1) {
      // they are still on the first page
      sendMessage("PRIVMSG "+username+" :There is no previous text I'm afraid.");
      return;
    }
    else  {
      // we are at the end of the text, so calculate our starting point
      if (userText.textPosition >= userText.text.size())
	userText.textPosition = userText.text.size() - (userText.text.size() % MAX_LINES_OUTPUT) - MAX_LINES_OUTPUT;
      else
	userText.textPosition = userText.textPosition - MAX_LINES_OUTPUT*2;

      int startPoint = userText.textPosition;
      int endingPoint = userText.textPosition+MAX_LINES_OUTPUT;
      // display the text to the user
      for (unsigned i=startPoint; i<endingPoint; i++) {
	sendMessage("PRIVMSG "+username+" :"+userText.text.at(i));
      }
      userText.textPosition = userText.textPosition + MAX_LINES_OUTPUT;

      userText.currentPage--;

      std::ostringstream prevPageDisplay;
      prevPageDisplay << "PRIVMSG "+username+" :Page ";
      prevPageDisplay << userText.currentPage;
      prevPageDisplay << " of ";
      prevPageDisplay << userText.numPages;
      prevPageDisplay << ". [ Commands: 'next' (see next page); 'prev' (see previous page); 'page N' (go to page N) ]";
      sendMessage(prevPageDisplay.str());

      // update map so we keep track of where the user is in the text
      directUserText[username] = userText;
    }
  }
  else if (userMessage.find(pageCommand) != string::npos) {
    string pageNumber = userMessage.substr(userMessage.find(pageCommand) + pageCommand.length());
    int pageNum = atoi(pageNumber.c_str());
    bool saidSomething = false;

    userTextManagement userText = directUserText[username];
    if (userText.text.empty()) {
      // we don't have any text stored for that user
      sendMessage("PRIVMSG "+username+" :I don't have any text stored for you!");
      return;
    }

    if (pageNum < 1) {
      sendMessage("PRIVMSG "+username+" :Invalid page number specified.");
      return;
    }
    else if (pageNum > userText.numPages) {
      sendMessage("PRIVMSG "+username+" :There are not that many pages in the text.");
      return;
    }
    else {
      userText.textPosition = (pageNum-1) * MAX_LINES_OUTPUT;
      int startPoint = userText.textPosition;
      int endingPoint = userText.textPosition+MAX_LINES_OUTPUT;
      // display the text to the user
      for (unsigned i=startPoint; i<endingPoint; i++)
      try {
	sendMessage("PRIVMSG "+username+" :"+userText.text.at(i));
	saidSomething = true;
	userText.textPosition = i + 1;
      }
      catch (out_of_range& outOfRange) {
	userText.currentPage = pageNum;
	directUserText[username] = userText;
	std::ostringstream endOfTextDisplay;
	endOfTextDisplay << "PRIVMSG "+username+" :End of text. Page ";
	endOfTextDisplay << userText.currentPage;
	endOfTextDisplay << " of ";
	endOfTextDisplay << userText.numPages;
	endOfTextDisplay << ". [ Commands: 'prev' (see previous page); 'page N' (go to page N) ]";
	sendMessage(endOfTextDisplay.str());
	return;
      }
    }

    userText.currentPage = pageNum;

    std::ostringstream prevPageDisplay;
    prevPageDisplay << "PRIVMSG "+username+" :Page ";
    prevPageDisplay << userText.currentPage;
    prevPageDisplay << " of ";
    prevPageDisplay << userText.numPages;
    prevPageDisplay << ". [ Commands: 'next' (see next page); 'prev' (see previous page); 'page N' (go to page N) ]";
    sendMessage(prevPageDisplay.str());

    // update map so we keep track of where the user is in the text
    directUserText[username] = userText;
  }

}

/* the function which parses the messages that the user sends
 * this should really be in its own file
 */
int IrcBot::parseMessage(string str) {

  string username = getUsername(str);
  string serverInfo = getServerInfo(str);
  string userMessage = getUserMessage(str);
  bool isPrivateMessage = false;


  // if the message starts with an exclamation mark, let the bot respond to it
  if (userMessage.substr(0, 1) == "!")
    userMessage = this->nick + ": " + userMessage.substr(1,userMessage.length());

  adminCommand thisCommand;
  thisCommand.userMessage = userMessage;
  thisCommand.isPrivateMessage = isPrivateMessage;

  if (serverInfo.find("PRIVMSG "+ircbotName) != string::npos && connected) {
    // this is a private message to the irc bot
    parsePrivateMessage(str);
    isPrivateMessage = true;
  }

  /* the returned str is just what the user text is without the IRC prefix information
   * note: this should not be done this way, a function should be called which has the
   *       sole purpose of getting the user text from the full message */
  if (!isPrivateMessage)
    writeHistory(username, serverInfo, userMessage);

  if (stringSearch(str, "End of /NAMES list")) {
    connected = true;  // intro messages to the server have finished
    string beginningTrimmed = str.substr(str.find(channelName)+channelName.size()+2);
    string namesString = beginningTrimmed.substr(0, beginningTrimmed.find(":")-2);

    vector<string> namesOfUsersConnected;
    istringstream ss( namesString );
    while (!ss.eof()) {
      string x;
      getline( ss, x, ' ' );
      namesOfUsersConnected.push_back(x);
    }
  }

  if (serverInfo.find("JOIN") != string::npos) {
    string checkAccess = "PRIVMSG NickServ : ACC " + username;
    sendMessage(checkAccess);
  }
  else if (serverInfo.find("PART") != string::npos || serverInfo.find("QUIT") != string::npos) {
    // for detecting them leaving the channel

    // wipe the username from the map responsible for relaying text
    directUserText.erase(username);

    // wipe any admin commansd the user have stored in the map so they aren't run on next login
    adminCommandsUsed.erase(username);

    std::vector<string>::iterator position = std::find(authenticatedUsernames.begin(), authenticatedUsernames.end(), username);
    if (position != authenticatedUsernames.end())
      authenticatedUsernames.erase(position);
  }

  if (stringSearch(userMessage, ircbotName+": help")) {
    // these are a list of c++ commands only, not including the lua plugin commands.
    outputToUser(username, "I have all kinds of fun features! Here's the list:\n"
		 "\"!uptime\". Displays uptime.\n"
		 "\"!history <start|stop>\". Starts/stops logging channel conversation.\n"
		 "\"!shutdown\". Shuts me down. I won't come back though, please don't do that to me. :(\n"
		 "Lua: !admins, !topic, !op, !deop, !<save|load> data, !history <date>, !seen <username>, !seenall, !reactions, !reaction enable <reaction>, !reaction disable <reaction>, !relay, !stop relay, !acknowledge, !stop acknowledge, !time, !update repo");
  }
  else if (stringSearch(userMessage, ircbotName+": uptime")) {
    int uptime = uptimeTimer.elapsedTime();
    int days = uptime / 60 / 60 / 24;
    int hours = (uptime / 60 / 60) % 24;
    int minutes = (uptime / 60) % 60;
    int seconds = uptime % 60;
    std::stringstream ss;
    ss << "I have been alive for ";
    ss << days;
    if (days == 1)
      ss << " day, ";
    else
      ss << " days, ";
    ss << hours;
    if (hours == 1)
      ss << " hour, ";
    else
      ss << " hours, ";
    ss << minutes;
    if (minutes == 1)
      ss << " minute, and ";
    else
      ss << " minutes, and ";
    ss << seconds;
    if (seconds == 1)
    ss << " second.";
    else
      ss << " seconds.";
    outputToSource(ss.str(), username, isPrivateMessage);
  }
  else if (stringSearch(userMessage, ircbotName+": history start")) {
    if (isPrivateMessage)
      outputToUser(username, "This command cannot be used in private chat.");
    else {
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
  }
  else if (stringSearch(userMessage, ircbotName+": history stop")) {
    if (isPrivateMessage)
      outputToUser(username, "This command cannot be used in private chat.");
    else {
      if (loggingHistory) {
	loggingHistory = false;
	outputToChannel("Ah I see, cunning secret conversations. I know not of what you speak, your sneakrets are safe with me.");
      }
      else
	outputToChannel("My ears are already shut to your sneakiness. I'll keep it secret. I'll keep it safe.");
    }
  }
  else if (stringSearch(userMessage, ircbotName+": shutdown")) {
    if (isPrivateMessage)
      outputToUser(username, "This command cannot be used in private chat.");
    else {
      outputToChannel("Oh I get it. It's fine, I'm a pain sometimes I guess. Croo.");
      sendMessage("QUIT");
      close (connectionSocket);  //close the open socket
      cout << "Shutting down...";
      return SHUTDOWN;
    }
  }

  /* once the connection has been established, we can run the lua and python plugins */
  if (connected) {
    luaInterface.runPlugins(username, serverInfo, userMessage, isPrivateMessage);
    pythonInterface.runPlugins(username, serverInfo, userMessage, isPrivateMessage);
  }

  return SUCCESS;
}
