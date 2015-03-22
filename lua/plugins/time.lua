----------------------------------------------------------------------
-- Copyright 2012 2013 William Gatens
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
-- Description: Plugin which can display the time.
----------------------------------------------------------------------

function time(username, serverPart, userMessage, isPrivateMessage)
  local botname = getBotName():lower()
  local things = {"snacks", "otter pictures", "herby tea", "to do Graham's thesis", "drop sconnies", "pecan pah", "pecan po", "snug snug", username .. " to be banished", username .. " to get out", username .. " to repent"}
  userMessage = userMessage:lower()
  if (string.find(userMessage, botname..": time")) then
    sendLuaMessageToSource(username,"Time for " .. things[math.random(#things)] .. " (" .. os.date() .. ")",isPrivateMessage)
  end
end

local documentation = {['Usage'] = "\"!time\" Prints the current server time and date"}

return {name="Time", description="Returns the current date and time",
	parseFunction=time, doc=documentation}
