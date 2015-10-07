----------------------------------------------------------------------
-- Copyright 2015 Alex Woodcock
--
-- Kiwibot is a free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- Kiwibot is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with Kiwibot.  If not, see <http://www.gnu.org/licenses/>.
--
-- Description: Plugin which can quote Monty Python.
----------------------------------------------------------------------

function python(username, serverPart, userMessage, isPrivateMessage)
  local botname = getBotName():lower()
  local quotes = {
  {"King Arthur: 'What does it say?'",
  "Maynard: 'It reads, 'Here may be found the last words of Joseph of Arimathea. He who is valiant and pure of spirit may find the Holy Grail in the Castle of aaarrrrggh'.'",
  "King Arthur: 'What?'",
  "Maynard: '...The Castle of aaarrrrggh.'",
  "Bedevere: 'What is that?'",
  "Maynard: 'He must have died while carving it.'"},
  
  {"Customer: 'Not much of a cheese shop really, is it?'",
  "Shopkeeper: 'Finest in the district, sir.'",
  "Customer: 'And what leads you to that conclusion?'",
  "Shopkeeper: 'Well, it's so clean.'",
  "Customer: 'It's certainly uncontaminated by cheese.'"},
  
  {"'Now, you listen here! He's not the Messiah. He's a very naughty boy!'"},
  
  {"'I'm Brian, and so's my wife!'"},
  
  {"'E's not pinin'! 'E's passed on! This parrot is no more! He has ceased to be! 'E's expired and gone to meet 'is maker! 'E's a stiff! Bereft of life, 'e rests in peace! If you hadn't nailed 'im to the perch 'e'd be pushing up the daisies! 'Is metabolic processes are now 'istory! 'E's off the twig! 'E's kicked the bucket, 'e's shuffled off 'is mortal coil, run down the curtain and joined the bleedin' choir invisible!! THIS IS AN EX-PARROT!!'"},

  {"'Finally, monsieur, a wafer-thin mint.'"},
  
  {"Father: One day, lad, all this will be yours.",
  "His son: What, the curtains?"},
  
  {"Mattias: 'Look, I had a lovely supper, and I all I said to my wife was that that piece of halibut was good enough for Jehovah.'",
  "Angry mob: 'Ooooh!'",
  "Official: 'Blasphemy! He said it again!'"},
  
  {"'Listen, strange women lying in ponds distributing swords is no basis for a system of government. Supreme executive power derives from a mandate from the masses, not from some farcical aquatic ceremony.'"},
  
  {"'I fart in your general generation! Your mother was a hamster and your father smelt of elderberries!'"},
  
  {"Sir Bedevere: 'Now, why do witches burn?'",
  "Peasant: '...because they're made of... wood?'",
  "Sir Bedevere: 'Good. So how do you tell whether she is made of wood?'",
  "Peasant 2: 'Build a bridge out of her.'"},
  
  {"We are no longer the knights who say ni! We are now the knights who say ekki-ekki-ekki-pitang-zoom-boing!"}
  }
  userMessage = userMessage:lower()
  if (string.find(userMessage, "!python")) then
	local quote = quotes[math.random(#quotes)]
	for i,v ipairs(quote) do
		if(#quote[i]<400) then
			sendLuaMessageToSource(username, quote[i],isPrivateMessage)
		else
			local tempString = string.sub(quote[i], 0, string.find(quote[i], " ", 390))
			sendLuaMessageToSource(username, tempString,isPrivateMessage)
			tempString = string.sub(quote[i], string.find(quote[i], " ", 390))
			sendLuaMessageToSource(username, tempString,isPrivateMessage)
		end
	end
  end
end

local documentation = {['Usage'] = "\"!python\" Prints a random Monty Python quote"}

return {name="Python", description="Returns a Monty Python quote",
	parseFunction=python, doc=documentation}
