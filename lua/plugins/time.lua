function time(line)
  if (string.find(line, "kiwi: what is the time?")) then
    sendLuaMessage(os.date())
  end
end

function saveData()
  print ("Saving time.lua data...")
end

return {name="Time", description="Returns the current date and time",
	parseFunction=time, saveDataFunction=saveData}
