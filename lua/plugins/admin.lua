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
-- Description: A plugin to handle kiwibot administration
----------------------------------------------------------------------

admins = {}                                    -- the table of all admins
authenticatedUsernames = {}                    -- list of authenticated admins
adminCommands = {}                             -- list of commands that are to be run
adminTag = "[admin]"                           -- admin tag in save file
listCommand   = getBotName()..": admins"       -- command to print admins
kickCommand   = getBotName()..": kick"         -- command to kick a user
muteCommand   = getBotName()..": mute"         -- command to mute a user
topicCommand  = getBotName()..": topic"        -- command for changing topic
lockCommand   = getBotName()..": lock"         -- lock channel, no users may join
unlockCommand = getBotName()..": unlock"       -- unlock channel
opCommand     = getBotName()..": op"           -- give op status
deopCommand   = getBotName()..": deop"         -- take op status
saveCommand   = getBotName()..": save data"    -- save data command
loadCommand   = getBotName()..": load data"    -- load data command
historyCommand= getBotName()..": history"      -- history command

function runAdminCommand(username, command)
  local authenticated = false
  for key,value in pairs(authenticatedUsernames)
  do
    if value == username then
      authenticated=true
    end
  end

  if authenticated then
    if (string.find(command,topicCommand)) then
      local topic = command:sub(string.len(topicCommand) + 2, string.len(command))
      sendLuaPrivateMessage("ChanServ", "TOPIC "..getChannelName()..command);
    elseif (string.find(command,opCommand)) then
      sendLuaMessage("MODE "..getChannelName().." +o "..username)
    elseif (string.find(command,deopCommand)) then
      sendLuaMessage("MODE "..getChannelName().." -o "..username)
    elseif (string.find(command,saveCommand)) then
      savePluginData()
    elseif (string.find(command,loadCommand)) then
      loadPluginData()
    elseif (string.find(command,historyCommand)) then
      local date = command:sub(string.len(historyCommand)+2,string.len(command)-2)
      local uploadString = "if [ -f history/"..date.." ]; then curl -d private=True -d \"lang=Plain Text&submit=Submit\" --data-urlencode code@history/"..date.." codepad.org | grep -o \"http://codepad.org/[a-Z0-9]*\" | head -n 1; else echo \"Could not find history file.\"; fi"
      sendLuaMessageToSource(username, "Uploading history file...)")
      local returnString = runSystemCommandWithOutput(uploadString)
      sendLuaMessageToSource(username, returnString)
    end
  else
    sendLuaPrivateMessage("NickServ", "ACC " .. username)
    adminCommands[username] = command
  end
end

function parseAdminCommand(username, serverPart, userMessage, isPrivateMessage)
  -- print admins if the user requests it
  if (string.find(userMessage, listCommand)) then
    printAdmins(username, isPrivateMessage)
  elseif (string.find(userMessage, "ACC 3") and username == "NickServ") then
    local accUsername = userMessage:sub(0,string.find(userMessage, " ")-1)
    for key,value in pairs(admins)
    do
      if value == accUsername and adminCommands[accUsername] then
	table.insert(authenticatedUsernames, accUsername)
	runAdminCommand(accUsername, adminCommands[accUsername])
	adminCommands[accUsername] = nil
      end
    end
  elseif (string.find(userMessage, "ACC") and username == "NickServ") then
    local accUsername = userMessage:sub(0,string.find(userMessage, " ")-1)
    if accUsername ~= getBotName() and adminCommands[accUsername] then -- check it's not a message about the bot from nickserv...
      sendLuaMessageToSource(accUsername, "You need to be authenticated with NickServ to use this command.", isPrivateMessage)
    end
  elseif (string.find(userMessage, topicCommand)) then
    runAdminCommand(username, userMessage)
  elseif (string.find(userMessage, opCommand)) then
    runAdminCommand(username, userMessage)
  elseif (string.find(userMessage, deopCommand)) then
    runAdminCommand(username, userMessage)
  elseif (string.find(userMessage, saveCommand)) then
    runAdminCommand(username, userMessage)
    sendLuaMessageToSource(username, "Data saved.")
  elseif (string.find(userMessage, loadCommand)) then
    runAdminCommand(username, userMessage)
    sendLuaMessageToSource(username, "Data loaded.")
  elseif (string.find(userMessage, historyCommand)) then
    runAdminCommand(username, userMessage)
  end
end

-- function to print out admin table
function printAdmins(username, isPrivateMessage)
  print ("Printing admins...")
  sendLuaPrivateMessage(username, "Printing admins:")
  for key,value in pairs(admins)
  do
    sendLuaPrivateMessage(username,"Admin: "..value)
  end
  sendLuaPrivateMessage(username,"done!")
end

-- loads admin data
function loadAdmins()
  print ("Loading admins.lua data...")

  admins = {}

  getPluginData(adminTag);

  print("printing and loading data:")
  for i,entry in ipairs(loadedElement)
  do
    print(loadedElement[i])
    table.insert(admins, entry)
 end
  print ("done!")
end

-- save admin data
function saveAdmins ()
  print ("Saving admin.lua data...")

  local dataString = ""
  for key,value in ipairs(admins) do
    dataString = dataString..value.."\n"
  end

  -- we subtract 3 to remove extraneous characters
  setPluginData(adminTag, string.sub(dataString, 0, string.len(dataString)-3))
end

return {name="Admin", description="Provides administration commands.",
	parseFunction=parseAdminCommand, loadDataFunction=loadAdmins, saveDataFunction=saveAdmins}
