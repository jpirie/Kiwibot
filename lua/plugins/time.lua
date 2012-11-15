function time(username, serverPart, userMessage)
  local botname = getBotName():lower()
  userMessage = userMessage:lower()
  if (string.find(userMessage, botname..": what is the time?")) then
    sendLuaMessage(os.date())
  end
end

return {name="Time", description="Returns the current date and time",
	parseFunction=time, saveDataFunction=saveTimeData, loadDataFunction=loadTimeData}
