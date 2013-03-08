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
-- Description: Plugin which corrects basic URL typos in the channel
--              and repeats the URL with corrections, making the link
--              clickable.
----------------------------------------------------------------------

urls = {}

function loadUrlHandlingData ()
  print ("Loading url-handling.lua data...")
  local pluginSaveFile = "plugin-data.sav"           -- name of plugin file
  local fileHandler = io.open(pluginSaveFile, "r")   -- open in read mode first.
  local correctSection                               -- true if parsing correct section of data file
  for line in fileHandler:lines() do                 -- go through all lines in existing file
    if (line == "[urlhandling]") then                   -- look for the lastseen tag
      correctSection = true
    elseif (line == "[end]" and correctSection) then -- we're done
      break
    elseif (correctSection) then                     -- we're in the right section, add the user to array
      table.insert(urls, line)
    end
  end
  print ("done!")
end

function saveUrlHandlingData ()
  print ("Saving last-seen.lua data...")
  local pluginSaveFile = "plugin-data.sav"         -- name of plugin file
  local fileHandler = io.open(pluginSaveFile, "r") -- open in read mode first.
  local lines = {}                                 -- holds lines we've looped through
  local remainingFile                              -- holds rest of the file we don't loop through
  local correctSection                             -- true if parsing correct section of data file
  for line in fileHandler:lines() do               -- go through all lines in existing file
    if (line == "[urlhandling]") then                 -- look for the lastseen tag
      correctSection = true
      lines[#lines + 1] = line                     -- store the current line
    elseif (line == "[end]" and correctSection) then
      -- insert the current seen people
      for key,value in pairs(namesSeen) do
	lines[#lines + 1] = value
      end
      lines[#lines + 1] = line                     -- store the current line
      for line in fileHandler:lines() do           -- go through all lines in existing file
	lines[#lines + 1] = line                   -- read the rest of the file in
      end
      break
    end

    if (not correctSection) then                   -- if we're not looking at the right section
      lines[#lines + 1] = line                     -- store the current line
    end                                            -- we don't want it otherwise
  end
  fileHandler:close()                              -- close file, we're ready to open in write mode

  if (not correctSection) then                     -- if we didn't find the section at all
    lines[#lines + 1] = "[urlhanding]"                -- put the section in
    lines[#lines + 1] = "[end]"
  end


  fileHandler = io.open(pluginSaveFile, "w")       -- open the file in write mode
  for i, line in ipairs(lines) do                  -- loop through line in our ammended array
    fileHandler:write(line, "\n")                  -- write the line
  end
  if (remainingFile ~= nil) then                   -- if we have the rest of the file
    fileHandler:write(remainingFile)               -- write the rest of the file
  end
  fileHandler:close()                              -- close the file
  print ("done!")
end

function urlhandlingParse(username, serverPart, userMessage, isPrivateMessage)
  -- make links clickable that start with a missing w
  local missingW = "^ww%.%w+%.%w+"
  if string.find(userMessage, missingW) ~= nil then
    sendLuaMessageToSource(username, "clickable: "..string.gsub(userMessage, missingW, "w%1"), isPrivateMessage)
  end

  local missingWInText = " ww%.%w+%.%w+"
  local foundLocation = string.find(userMessage, missingWInText)
  if foundLocation ~= nil then
    local replaceSpaceWithW = "w"..string.sub(userMessage, foundLocation+1)
    local findExtraText = string.find(replaceSpaceWithW, " ")
    if findExtraText ~= nil then
      replaceSpaceWithW = string.sub(replaceSpaceWithW, 0, findExtraText-1)
    end
    sendLuaMessageToSource(username, "clickable: "..replaceSpaceWithW, isPrivateMessage)
  end

  local missingStartingH = "^ttp://%w+"
  if string.find(userMessage, missingStartingH) ~= nil then
    sendLuaMessageToSource(username, "clickable: "..string.gsub(userMessage, missingStartingH, "h%1"), isPrivateMessage)
  end

  local missingHInText = " ttp://%w+%.%w+"
  local foundLocation = string.find(userMessage, missingHInText)
  if foundLocation ~= nil then
    local replaceSpaceWithH = "h"..string.sub(userMessage, foundLocation+1)
    local findExtraText = string.find(replaceSpaceWithH, " ")
    if findExtraText ~= nil then
      replaceSpaceWithH = string.sub(replaceSpaceWithH, 0, findExtraText-1)
    end
    sendLuaMessageToSource(username, "clickable: "..replaceSpaceWithH, isPrivateMessage)
  end

end

return {name="Url Handling", description="Corrects mis-pasted URLs so they are clickable",
	parseFunction=urlhandlingParse, saveDataFunction=saveUrlHandlingData, loadDataFunction=loadUrlHandlingData}


