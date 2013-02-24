function reaction(username, serverPart, userMessage, isPrivateMessage)
  local botname = getBotName():lower()
  userMessage = userMessage:lower()
  if (string.find(userMessage, "oh boy")) then
    sendLuaMessageToSource(username, "http://derpface.com/wp-content/uploads/2012/11/Oh-Boy-Reaction-Gif.gif", isPrivateMessage)
  end
--  if (string.find(userMessage, "imgur")) then
--    sendLuaMessageToSource(username, "http://emotibot.net/pix/3455.gif", isPrivateMessage)
--  end
  if (string.find(userMessage, "baffling")) then
    sendLuaMessageToSource(username, "http://www.jpirie.com/files/baffling.gif", isPrivateMessage)
  end
end


return {name="Reactions", description="Posts gif reactions to common phrases",
	parseFunction=reaction}
