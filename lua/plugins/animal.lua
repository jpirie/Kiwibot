-- lists of synonyms
-- jpirie: should these be in their own plugin file so everything can use synonym lists?
local helloSynonyms = {"hi", "hello", "ahoy", "avast", "hallo", "bonjour", "pew", "pow", "hey", "evening", "morning", "afternoon", "prevening", "greetings", "howdy", "hiya"}
local byeSynonyms = {"bye", "night", "n8", "ciao", "later", "in a bit", "nate"}
local thanksSynonyms = {"WOW! Thanks!", "omnomnomnom", "OMNOM", "Ooh, delicious", "tasty tastiness", "mm mm mmmmm", "for me? Woah. <3", "Aww, thanks!"}
local hungrySynonymsLow = {"Getting a bit peckish I have to say", "I would sure love a tasty treat!", "When's my dinner I wonder...", "Bacon roll for kiwi perhaps?"}
local hungrySynonymsModerate = {"Certainly hurgry now, any chance of some food?", "I would really love some tastiness about now", "Oops, that was my stomach rumbling!", "Is dinner for your favourite kiwi on the horizon?"}
local hungrySynonymsHigh = {"AaAaH! Please feeeeed meeeee", "Help... help me. I'm.... so hungry...", "Please, any food? The... hunger...", "CREEEEEEEEEE! Ima really pretty hurgry."}

local HUNGER_MAX = 1500
local hunger = HUNGER_MAX
local whineLevel = 400
local linesEncountered = 0

-- we should really be looking for regexps or something, not
-- straight strings.
function animalParse(username, serverPart, userMessage, isPrivateMessage)
  -- make the current line lower case
  userMessage = userMessage:lower()
  botname = getBotName():lower()

  if (string.find(userMessage, botname)) then
    if (string.find(userMessage, "cup of tea")) then
      sendLuaMessageToSource(username, "but of course! CREE-CREE!", isPrivateMessage)
    elseif (string.find(userMessage, "pets")) then
      sendLuaMessageToSource(username, "Aww. Thanks! Wuv you! <3", isPrivateMessage)
    elseif (string.find(userMessage, "silly")) then
      sendLuaMessageToSource(username, "Oops I am a bit daft sometimes aren't I. CREE!", isPrivateMessage)
    end
  end
  if (string.find(userMessage, "feeds "..botname)) then
    if (string.find(userMessage, "turd") or string.find(userMessage, "poo")) then
      sendLuaMessageToSource(username, string.upper("NO "..username.."! BAD "..username.."!"), isPrivateMessage)
    else
      -- we should add points to the user that feeds kiwi so that it loves that user more
      sendLuaMessageToSource(username, thanksSynonyms[math.random(#thanksSynonyms)], isPrivateMessage)
      if hunger < 500 then
        hunger = hunger + 500;
      else
        hunger = HUNGER_MAX
      end
      whineLevel = math.floor(hunger / 2)
    end
  else
    -- check for hello messages
    for _,greeting in pairs(helloSynonyms) do
      if (string.find(userMessage, greeting.." "..botname)) then
	sendLuaMessageToSource(username, "hi, CREE!", isPrivateMessage)
      end
    end

    -- check for goodbye messages
    for _,farewells in pairs(byeSynonyms) do
      if (string.find(userMessage, farewells.." "..botname)) then
	sendLuaMessageToSource(username, "Laters, I'll keep this channel in check for you, have no fear!", isPrivateMessage)
      end
    end
  end

  hunger = hunger - 1
  if hunger == whineLevel then
    if hunger <= 50 then
      sendLuaMessage(hungrySynonymsHigh[math.random(#hungrySynonymsLow)])
    elseif hunger <=  150 then
      sendLuaMessage(hungrySynonymsModerate[math.random(#hungrySynonymsLow)])
    elseif hunger <= 400 then
      sendLuaMessage(hungrySynonymsLow[math.random(#hungrySynonymsLow)])
    end
    whineLevel = math.floor(whineLevel / 2)
  end
end

return {name="Animal Parser", description="Parses greetings for the kiwi",
	parseFunction=animalParse, saveDataFunction=saveData, loadDataFunction=loadData}


