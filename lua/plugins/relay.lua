-- acknowledge.lua
-- kiwi acknowledges user messages by sending them back to the user in a private message

namesToAcknowledge = {}
namesToRelay = {}

function relayParse(username, serverPart, userMessage, botName)
  -- make the current line lower case
  userMessage = userMessage:lower()

  if (string.find(userMessage, "kiwi: acknowledge")) then
    table.insert(namesToAcknowledge, username)
    sendLuaMessage("I will acknowledge your messages.")
  elseif (string.find(userMessage, "kiwi: relay")) then
    table.insert(namesToRelay, username)
    sendLuaMessage("I will relay messages privately to you.")
  elseif (string.find(userMessage, "kiwi: stop acknowledge")) then
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
  elseif (string.find(userMessage, "kiwi: stop relay")) then
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
    sendLuaPrivateMessage(value, username..": "..userMessage)
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
