-- simple function that looks for 'hi kiwi'
-- we should really be looking for regexps or something, not
-- straight strings.
function animalParse(currentLine)
  if (string.find(currentLine, "hi kiwi")) then
    sendLuaMessage("hi, CREE!")
  elseif (string.find(currentLine, "cup of tea for you kiwi?")) then
    sendLuaMessage("but of course! CREE-CREE!")
  end
end