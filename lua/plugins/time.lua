function time(line, botName)
  if (string.find(line, "kiwi: what is the time?")) then
    sendLuaMessage(os.date())
  end
end

return {name="Time", description="Returns the current date and time", parseFunction=time}

