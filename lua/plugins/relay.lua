-- acknowledge.lua
-- bot acknowledges user messages by sending them back to the user in a private message

namesToAcknowledge = {}
namesToRelay = {}

function relayParse(username, serverPart, userMessage)
  -- make the current line lower case
  botname = getBotName():lower()
  userMessage = userMessage:lower()

  if (string.find(userMessage, botname..": acknowledge")) then
    table.insert(namesToAcknowledge, username)
    sendLuaMessage("I will acknowledge your messages.")
  elseif (string.find(userMessage, botname..": relay")) then
    table.insert(namesToRelay, username)
    sendLuaMessage("I will relay messages privately to you.")
  elseif (string.find(userMessage, botname..": stop acknowledge")) then
    local findUser = false
    for key,value in pairs(namesToAcknowledge) do
      if (value == username) then
	table.remove(namesToAcknowledge, key)
	findUser = true
	sendLuaMessage("Acknowledgement stopped.")
      end
    end
    if (findUser == false) then
      sendLuaMessage("I wasn't acknowledging your messages!")
    end
  elseif (string.find(userMessage, botname..": stop relay")) then
    local findUser = false
    for key,value in pairs(namesToRelay) do
      if (value == username) then
	table.remove(namesToRelay, key)
	findUser = true
	sendLuaMessage("Relay stopped.")
      end
    end
    if (findUser == false) then
      sendLuaMessage("I wasn't relaying messages!")
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

function saveRelayData()
  print ("Saving relay.lua data...")
  print ("done!")
end

function loadRelayData ()
  print ("Loading relay.lua data...")
  print ("done!")
end

 return {name="Relay", description="relays user messages by sending them back to the user in a private message",
	parseFunction=relayParse, saveDataFunction=saveRelayData, loadDataFunction=loadRelayData}
