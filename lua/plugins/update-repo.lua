----------------------------------------------------------------------
-- Copyright 2013 John Pirie
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
-- Description: A plugin to update the repository that Kiwibot is running in
----------------------------------------------------------------------

updateRepoCommand = getBotName()..": update repo"

function parseUpdateRepo(username, serverPart, userMessage, isPrivateMessage)
    if (string.find(userMessage, updateRepoCommand)) then
      sendLuaMessageToSource(username, "Updating now...")
      runSystemCommand("cd ~/repos/kiwibot; git pull http master; git pull http dynamic-plugins")
      sendLuaMessageToSource(username, "Done!")
    end
end

function loadUpdateRepo()
end

function saveUpdateRepo()
end

return {name="UpdateRepo", description="Allows Kiwibot updates to be pulled from the repository.",
	parseFunction=parseUpdateRepo, loadDataFunction=loadUpdateRepo, saveDataFunction=saveUpdateRepo}
