function reaction(username, serverPart, userMessage, isPrivateMessage)
  local botname = getBotName():lower()
  userMessage = userMessage:lower()
  if (string.find(userMessage, "oh boy")) then
    sendLuaMessageToSource(username, "www.jpirie.com/files/oh-boy.gif", isPrivateMessage)
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
