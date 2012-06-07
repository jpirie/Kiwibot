local gameInProgress = false
local whiteToMove = true
local currentBoard = {}

-- returns a fresh board state
function createFreshBoard()
  local board = {"R", "N", "B", "Q", "K", "B", "N", "R", -- 1st rank
		 "P", "P", "P", "P", "P", "P", "P", "P", -- 2nd rank
		 " ", " ", " ", " ", " ", " ", " ", " ", -- 3rd rank
		 " ", " ", " ", " ", " ", " ", " ", " ", -- 4th rank
		 " ", " ", " ", " ", " ", " ", " ", " ", -- 5th rank
		 " ", " ", " ", " ", " ", " ", " ", " ", -- 6th rank
		 "P", "P", "P", "P", "P", "P", "P", "P", -- 7th rank
		 "R", "N", "B", "Q", "K", "B", "N", "R"} -- 8th rank
end

function chessParse(currentLine)
  if (string.find(currentLine, "kiwi: new chess game")) then
    if (gameInProgress) then
      sendLuaMessage("A game is already in progress. To restart the game, use the command \"restart chess game\" (not yet implemented!)")
    else
      gameInProgress = true
      currentBoard = createFreshBoard()
      sendLuaMessage("Got all the pieces off the floor, ready to go. White to move.")
    end
  elseif (string.find(currentLine, "kiwi: move ")) then
    -- we should clean this up so we don't have to do it twice
    local moveText = "kiwi: move"
    local startOfUserText = string.find(currentLine, moveText)
    local move = string.sub(currentLine, startOfUserText+string.len(moveText)+1)

    sendLuaMessage(move)
  end
end


return {name="Chess", description="Allows two users to play chess on the channel", parseFunction=chessParse}