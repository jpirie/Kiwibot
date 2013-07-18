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
-- Description: A reaction plugin for the Kiwi. Will respond to key
--              phrases with .gif files.
----------------------------------------------------------------------


reactions = {}                                          -- the table of all reactions
reactionsTag = "[reactions]"                            -- reactions tag in save file
showCommand = getBotName()..": reactions"               -- command to print reactions
disabledReactions = {}                                  -- table for disabled reactions
disableCommand = getBotName()..": reaction disable "    -- command to disable a reaction
enableCommand = getBotName()..": reaction enable "      -- command to enable a reaction

function reaction(username, serverPart, userMessage, isPrivateMessage)
  local botname = getBotName():lower()

  -- print reactions if the user requests it
  if (string.find(userMessage, showCommand)) then
    printReactions(username, isPrivateMessage)
  end

  -- disable a reaction
  if (string.find(userMessage, disableCommand)) then
    local name = userMessage:sub(string.find(userMessage, disableCommand)+string.len(disableCommand),string.len(userMessage)-2)
    sendLuaMessageToSource(username, "Reaction for \""..name.."\" disabled.")
    table.insert(disabledReactions, name)
  end

  -- enable a reaction
  if (string.find(userMessage, enableCommand)) then
    local name = userMessage:sub(string.find(userMessage, enableCommand)+string.len(enableCommand),string.len(userMessage)-2)
    for key,value in ipairs(disabledReactions)
    do
      if value == name then
        table.remove(disabledReactions, key)
      end
    end
    sendLuaMessageToSource(username, "Reaction for \""..name.."\" enabled.")
    return
  end

  -- check if we need to display a reaction based on the user's message contents
  for key,value in pairs(reactions) do
    if (string.find(userMessage, key)) then
      for key2,value2 in ipairs(disabledReactions)
      do
	if (key == value2) then
	  return
	end
      end
      sendLuaMessageToSource(username, value, isPrivateMessage)
    end
  end

end

-- function to print out reaction table
function printReactions(username, isPrivateMessage)
  print ("Printing reactions...")
  sendLuaPrivateMessage(username, "Printing reactions:")
  for key,value in pairs(reactions)
  do
    sendLuaPrivateMessage(username,"Text: \""..key.. "\", Reaction text: \""..value.."\"")
  end
  sendLuaPrivateMessage(username,"done!")
end

-- loads reaction data
function loadReactions()
  print ("Loading reactions.lua data...")

  getPluginData(reactionsTag);

  print("printing and loading data:")
  for i,entry in ipairs(loadedElement)
  do
    print(loadedElement[i])
    reactions[entry:sub(2,(string.find(entry, ",")-1))] = entry:sub(string.find(entry, ",")+2,string.len(entry)-1)
 end
  print ("done!")
end

-- save reaction data
function saveReactions ()
  print ("Saving reactions.lua data...")

  local dataString = ""
  for key,value in pairs(reactions) do
    dataString = dataString .. "("..key..", "..value..")\n"
  end

  -- we subtract 3 to remove extraneous characters
  setPluginData(pluginTag, string.sub(dataString, 0, string.len(dataString)-3))
end

return {name="Reactions", description="Posts gif reactions to common phrases",
	parseFunction=reaction, loadDataFunction=loadReactions}
