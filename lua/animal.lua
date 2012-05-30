-- simple function that looks for 'hi kiwi'
-- we should really be looking for regexps or something, not
-- straight strings.
function animalParse(currentLine)
  if (string.find(currentLine, "hi kiwi")) then
    sendLuaMessage("hi, CREE!")
  elseif (string.find(currentLine, "cup of tea for you kiwi?")) then
    sendLuaMessage("but of course! CREE-CREE!")
  elseif (string.find(currentLine, "kiwi, do your stuff.")) then
    sendLuaMessage("GOOOD LUCK SIMOWN! All the best from kiwis everywhere! You'll do great I'm sure. CREEEE!")
  elseif (string.find(currentLine, "pets kiwi")) then
    sendLuaMessage("Aww. Thanks! Wuv you! <3")
  end
end