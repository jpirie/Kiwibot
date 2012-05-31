function time(line) 
  if (string.find(line, "kiwi: what is the time?")) then
    sendLuaMessage(os.date())
  end
end


return time

