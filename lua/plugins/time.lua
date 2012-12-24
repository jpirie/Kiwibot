function time(username, serverPart, userMessage, isPrivateMessage)
  local botname = getBotName():lower()
  userMessage = userMessage:lower()
  if (string.find(userMessage, botname..": what is the time?")) then
    sendLuaMessageToSource(username,os.date(),isPrivateMessage)
  end
end

local documentation = {['Usage'] = "\"<botname> : what is the time?\" Prints the current server time and date"}

return {name="Time", description="Returns the current date and time",
	parseFunction=time, doc=documentation}
