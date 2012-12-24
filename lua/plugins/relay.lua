-- acknowledge.lua
-- bot acknowledges user messages by sending them back to the user in a private message

namesToAcknowledge = {}
namesToRelay = {}

function relayParse(username, serverPart, userMessage, isPrivateMessage)
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

  for key,value in pairs(namesToAcknowledge) do
    if (username == value) then
      sendLuaPrivateMessage(username, userMessage)
    end
  end
  for key,value in pairs(namesToRelay) do
    if (username ~= value) then
    sendLuaPrivateMessage(value, username..": "..userMessage)
    end
  end
end

 return {name="Relay", description="relays user messages by sending them back to the user in a private message",
	parseFunction=relayParse, saveDataFunction=saveRelayData, loadDataFunction=loadRelayData}
