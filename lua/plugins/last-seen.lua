-- last-seen.lua
-- a plugin which will store when a user was last seen on the channel

namesSeen = {}
seenCommand = "!seen "

-- there's an issue where any time this file is changed, the irc bot
-- will reload this file, and wipe the list of namesSeen! So this data
-- should really be saved before the file is reloaded. This problem is
-- common across all plugins.

function lastSeenParse(currentLine, botName)
  -- make the current line lower case
  if (string.find(currentLine, "QUIT")) or (string.find(currentLine, "PART")) then
    local name = ""

    for char in currentLine:gmatch "." do
      if (char == "!") then
	break
      else
	-- ignore the starting colon
	if (char ~= ":") then
	  name = name..char
	end
      end
    end
    namesSeen[name] = os.date("%H:%M on %A, %B %d, %Y")
    print("Adding "..name.." to last seen map for time "..namesSeen[name]);

  elseif (string.find(currentLine, seenCommand)) then
    -- grab the name the user supplied (we take away 2 from the length of the string because we don't want the \n part (not one char... odd)
    local name = currentLine:sub(string.find(currentLine, seenCommand)+string.len(seenCommand),string.len(currentLine)-2)
    local time = namesSeen[name]
    if namesSeen[name] then
      sendLuaMessage(name.." last seen "..namesSeen[name]);
    else
      sendLuaMessage("I know not of the person you speak of. Quite the mystery indeed.");
    end
  elseif (string.find(currentLine, "!seenall")) then
    for key,value in pairs(namesSeen) do sendLuaMessage("("..key..", "..value..")") end
  end

end

function saveData ()
  print ("Saving last-seen.lua data...")
end

return {name="Last Seen", description="Stores when users were last seen on the channel",
	parseFunction=lastSeenParse, saveDataFunction=saveData}


