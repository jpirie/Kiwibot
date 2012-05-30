-- global variables for plugin control
animalPlugin=true

function pluginLoader (currentLine)

  if (string.find(currentLine, "kiwi: plugin list")) then
    sendLuaMessage("Plugged in to this super kiwi is:")
    if animalPlugin
    then sendLuaMessage ("Animal Plugin [enabled]")
    else sendLuaMessage ("Animal Plugin [disabled]")
    end
  end

  -- if the animal plugin is currently activated
  if animalPlugin
  then
    -- bring the result of executing the file into the current environment
    dofile("lua/animal.lua")
    -- givi kiwi the attention it deserves, pass the current user text in
    animalParse(currentLine)
  end
end