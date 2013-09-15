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
pluginTag = "[lastseen]"

function loadLastSeenData ()
  print ("Loading last-seen.lua data...")

  getPluginData(pluginTag);

  print("printing and loading data:")
  for i,entry in ipairs(loadedElement)
  do
    print(loadedElement[i])
    namesSeen[entry:sub(2,(string.find(entry, ",")-1))] = entry:sub(string.find(entry, ",")+2,string.len(entry)-1)
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
  setPluginData(pluginTag, string.sub(dataString, 0, string.len(dataString)-3))
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
    -- we remove the last two characters to remove the new line character
    local name = userMessage:sub(string.len(seenCommand)+2,string.len(userMessage)-2)
    if namesSeen[name] then
      sendLuaMessageToSource(username, name.." last seen "..namesSeen[name], isPrivateMessage);
    else
      sendLuaMessageToSource(username, "I know not of the person you speak of. Quite the mystery indeed.", isPrivateMessage);
    end
  end
end

return {name="Last Seen", description="Stores when users were last seen on the channel",
	parseFunction=lastSeenParse, saveDataFunction=saveLastSeenData, loadDataFunction=loadLastSeenData}


