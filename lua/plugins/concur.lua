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
  local concur_count = 0
  if (string.find(userMessage, "concur")) then
    concur_count = concur_count + 1
  end
  if concur_count == 2 then
    sendLuaMessageToSource(username,"I concur",isPrivateMessage)
    concur_count = 0
  end
end


return {name="Time", description="Concurs in meetings"}
