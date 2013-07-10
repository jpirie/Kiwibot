----------------------------------------------------------------------
-- Copyright 2012 William Gatens
-- Copyright 2012 John Pirie
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
-- Description: A plugin which will keep track of visitors to the
--              channel. Syntax:
--
--   !seen <username>: shows last disconnected date for
--                     <username>
--   !seenall: shows full log of disconnected times for all
--             users
----------------------------------------------------------------------

-- last-seen.lua
-- a plugin which will store when a user was last seen on the channel

namesSeen = {}
seenCommand = getBotName()..": seen"
seenAllCommand = getBotName()..": seenall"

function loadData ()
  print ("Loading last-seen.lua data...")
  local pluginSaveFile = "plugin-data.sav"           -- name of plugin file
  local fileHandler = io.open(pluginSaveFile, "r")   -- open in read mode first.
  local correctSection                               -- true if parsing correct section of data file
  for line in fileHandler:lines() do                 -- go through all lines in existing file
    if (line == "[lastseen]") then                   -- look for the lastseen tag
      correctSection = true
    elseif (line == "[end]" and correctSection) then -- we're done
      break
    elseif (correctSection) then                     -- we're in the right section, add the user to array
      namesSeen[line:sub(2,(string.find(line, ",")-1))] = line:sub(string.find(line, ",")+2,string.len(line)-1)
    end
  end
  print ("done!")
end

function saveLastSeenData ()
  print ("Saving last-seen.lua data...")

  local dataString = ""
  for key,value in pairs(namesSeen) do
    dataString = dataString .. "("..key..", "..value..")\n"
  end

  -- we subtract 3 to remove the final \n characters
  saveData("[lastseen]", string.sub(dataString, 0, string.len(dataString)-3))
end

function lastSeenParse(username, serverPart, userMessage, isPrivateMessage)
  -- make the current line lower case
  if (string.find(serverPart, "QUIT")) or (string.find(serverPart, "PART")) then
    local name = ""

    for char in (serverPart..userMessage):gmatch "." do
      if (char == "!") then
	break
      else
	-- ignore the starting colon
	if (char ~= ":") then
	  name = name..char
	end
      end
    end
    namesSeen[name] = os.date("%H:%M on %A, %B %d, %Y")                       -- add the user to the seen array
    saveLastSeenData()                                                        -- update the file to reflect they have been seen
    print("Adding "..name.." to last seen map for time "..namesSeen[name]);
  elseif (string.find(userMessage, seenAllCommand)) then
    local outputString = "Here's when I last saw peeps:\n"
    for key,value in pairs(namesSeen) do
      outputString = outputString.."("..key..", "..value..")\n"
      print ("output string: "..outputString);
    end
    outputString = outputString .. "End of list."
    sendLuaPrivateMessage(username, outputString)
  elseif (string.find(userMessage, seenCommand)) then
    -- grab the name the user supplied (we take away 2 from the length of the string because we don't want the \n part (not one char... odd)
    local name = userMessage:sub(string.find(userMessage, seenCommand)+string.len(seenCommand),string.len(userMessage)-2)
    if namesSeen[name] then
      sendLuaMessageToSource(username, name.." last seen "..namesSeen[name], isPrivateMessage);
    else
      sendLuaMessageToSource(username, "I know not of the person you speak of. Quite the mystery indeed.", isPrivateMessage);
    end
  end
end

return {name="Last Seen", description="Stores when users were last seen on the channel",
	parseFunction=lastSeenParse, saveDataFunction=saveLastSeenData, loadDataFunction=loadData}


