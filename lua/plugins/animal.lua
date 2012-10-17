-- lists of synonyms
-- jpirie: should these be in their own plugin file so everything can use synonym lists?
local helloSynonyms = {"hi", "hello", "ahoy", "avast", "hallo", "bonjour", "pew", "pow", "hey", "evening", "morning", "afternoon", "prevening", "greetings", "howdy", "hiya"}
local byeSynonyms = {"bye", "night", "n8", "ciao", "later", "in a bit", "nate"}
local thanksSynonyms = {"WOW! Thanks!", "omnomnomnom", "OMNOM", "Ooh, delicious", "tasty tastiness", "mm mm mmmmm", "for me? Woah. <3", "Aww, thanks!"}

-- simple function that looks for 'hi kiwi'
-- we should really be looking for regexps or something, not
-- straight strings.
function animalParse(currentLine, botName)
  -- make the current line lower case
  currentLine = currentLine:lower()

  if (string.find(currentLine, "kiwi")) then
    if (string.find(currentLine, "cup of tea")) then
      sendLuaMessage("but of course! CREE-CREE!")
    elseif (string.find(currentLine, "pets")) then
      sendLuaMessage("Aww. Thanks! Wuv you! <3")
    elseif (string.find(currentLine, "silly")) then
      sendLuaMessage("Oops I am a bit daft sometimes aren't I. CREE!")
    end
  end
  if (string.find(currentLine, "feeds kiwi with")) then
    -- we should add points to the user that feeds kiwi so that it loves that user more
    sendLuaMessage(thanksSynonyms[math.random(#thanksSynonyms)])
  elseif (string.find(currentLine, "kiwi: what is your name")) then
    sendLuaMessage(botName)
  else
    -- check for hello messages
    for _,greeting in pairs(helloSynonyms) do
      if (string.find(currentLine, greeting.." kiwi")) then
	sendLuaMessage("hi, CREE!")
      end
    end

    -- check for goodbye messages
    for _,farewells in pairs(byeSynonyms) do
      if (string.find(currentLine, farewells.." kiwi")) then
	sendLuaMessage("Laters, I'll keep this channel in check for you, have no fear!")
      end
    end
  end
end

function saveData ()
  print ("Saving animal.lua data...")
end

function loadData ()
  print ("Loading animal.lua data...")
  print ("done!")
end

return {name="Animal Parser", description="Parses greetings for the kiwi",
	parseFunction=animalParse, saveDataFunction=saveData, loadDataFunction=loadData}


