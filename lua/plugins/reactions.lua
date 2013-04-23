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

function reaction(username, serverPart, userMessage, isPrivateMessage)
  local botname = getBotName():lower()
  userMessage = userMessage:lower()
  if (string.find(userMessage, "oh boy")) then
    sendLuaMessageToSource(username, "http://www.jpirie.com/files/oh-boy.gif", isPrivateMessage)
  end
--  if (string.find(userMessage, "imgur")) then
--    sendLuaMessageToSource(username, "http://emotibot.net/pix/3455.gif", isPrivateMessage)
  --  end
  if (string.find(userMessage, "wowzers")) then
    sendLuaMessageToSource(username, "http://i.imgur.com/xDOfr.gif", isPrivateMessage)
  end

end


return {name="Reactions", description="Posts gif reactions to common phrases",
	parseFunction=reaction}
