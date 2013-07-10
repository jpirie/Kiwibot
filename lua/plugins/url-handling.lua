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
end

function saveUrlHandlingData ()
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


