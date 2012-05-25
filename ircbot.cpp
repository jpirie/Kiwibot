#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <netdb.h>

#include "ircbot.h"

using namespace std;

// define a data size for the input line
#define DATA_SIZE 256

bool connected;

IrcBot::IrcBot(string nick, string user) {
  this->nick = nick;
  this->user  = user;

  message = "";
}

// we close the connection when deconstructing the bot
IrcBot::~IrcBot() {
  close (connectionSocket);
}

/* initialises the irc bot. Gives birth to a new kiwi,
   and sets of sockets for communication */
void IrcBot::init(string channel) {

  // set up our fun friend - the kiwi
  Kiwi kiwi = Kiwi();
  this->kiwi = kiwi;

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

  if (stringSearch(str, "End of /NAMES list")) {
    connected = true;
  }

  if (connected) {
    string jpirieSend = "PRIVMSG jpirie :"+str;
    sendMessage(jpirieSend);
  }


  /* channel names SHOULD NOT be hardcoded! Get rid of this. */

  if (stringSearch(str,"hi kiwi"))
    sendMessage("PRIVMSG #caffeine-addicts :hi, CREE!");
  else if (stringSearch(str,"cup of tea for you kiwi?"))
    sendMessage("PRIVMSG #caffeine-addicts :but of course! CREE-CREE!");

  // sends the kiwi's hunger level to the channel
  else if (stringSearch(str,"kiwi hungerLevel")) {
    string message = "PRIVMSG #caffeine-addicts :current huger level is ";

    // we should really include a template or something for adding numbers to strings properly
    std::stringstream stream;
    stream << kiwi.getHungerLevel();
    std::string arraystring = stream.str();
    message += arraystring;

    // send the message out to the user
    sendMessage(message);
  }
}
