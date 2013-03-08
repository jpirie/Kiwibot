----------------------------------------------------------------------
-- Copyright 2012 2013 William Gatens
-- Copyright 2012 2013 John Pirie
--
-- Kiwibot is a free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- Kiwibot is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with Kiwibot.  If not, see <http://www.gnu.org/licenses/>.
--
-- Description: Plugin has two functions: 1) Allows the user to
--              receive messages they send to the channel back through
--              the Kiwi in a private message. Useful for verification
--              that messages are getting through. 2) Will ping users
--              who opt-in when any messages arrive on the channel
--              (useful for getting notifications for all channel
--              messages when using AndChat).
----------------------------------------------------------------------

-- acknowledge.lua
-- bot acknowledges user messages by sending them back to the user in a private message

namesToAcknowledge = {}
namesToRelay = {}

function relayParse(username, serverPart, userMessage, isPrivateMessage)
  if userMessage == "" or username == "NickServ" or username == "" then
    print "returning."
    return
  end

  -- make the current line lower case
  botname = getBotName():lower()
  userMessage = userMessage:lower()

  if (string.find(userMessage, botname..": acknowledge")) then
    table.insert(namesToAcknowledge, username)
    sendLuaMessageToSource(username, "I will acknowledge your messages.", isPrivateMessage)
  elseif (string.find(userMessage, botname..": relay")) then
    table.insert(namesToRelay, username)
    sendLuaMessageToSource(username, "I will relay messages privately to you.", isPrivateMessage)
  elseif (string.find(userMessage, botname..": stop acknowledge")) then
    local findUser = false
    for key,value in pairs(namesToAcknowledge) do
      if (value == username) then
	table.remove(namesToAcknowledge, key)
	findUser = true
	sendLuaMessageToSource(username, "Acknowledgement stopped.", isPrivateMessage)
      end
    end
    if (findUser == false) then
      sendLuaMessageToSource(username, "I wasn't acknowledging your messages!", isPrivateMessage)
    end
  elseif (string.find(userMessage, botname..": stop relay")) then
    local findUser = false
    for key,value in pairs(namesToRelay) do
      if (value == username) then
	table.remove(namesToRelay, key)
	findUser = true
	sendLuaMessageToSource(username, "Relay stopped.", isPrivateMessage)
      end
    end
    if (findUser == false) then
      sendLuaMessageToSource(username, "I wasn't relaying messages!", isPrivateMessage)
    end
  end

  -- the user has asked us to acknowledge messages. If we see a message from that user, acknowledge it
  for key,value in pairs(namesToAcknowledge) do
    if (username == value) then
      sendLuaPrivateMessage(username, userMessage)
    end
  end

  -- the user has asked us to relay messages. If they weren't the sender, then relay the message
  for key,value in pairs(namesToRelay) do
    if (username ~= value) then
    sendLuaPrivateMessage(value, username..": "..userMessage)
    end
  end
end

 return {name="Relay", description="relays user messages by sending them back to the user in a private message",
	parseFunction=relayParse, saveDataFunction=saveRelayData, loadDataFunction=loadRelayData}
