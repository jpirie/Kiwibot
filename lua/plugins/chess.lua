local gameInProgress = false
local whiteToMove = true
local currentBoard = {}

-- returns a fresh board state
function createFreshBoard()
  return {"R", "N", "B", "Q", "K", "B", "N", "R", -- 1st rank
    "P", "P", "P", "P", "P", "P", "P", "P", -- 2nd rank
    " ", " ", " ", " ", " ", " ", " ", " ", -- 3rd rank
    " ", " ", " ", " ", " ", " ", " ", " ", -- 4th rank
    " ", " ", " ", " ", " ", " ", " ", " ", -- 5th rank
    " ", " ", " ", " ", " ", " ", " ", " ", -- 6th rank
    "p", "p", "p", "p", "p", "p", "p", "p", -- 7th rank
    "r", "n", "b", "q", "k", "b", "n", "r"} -- 8th rank
end

function squareToArrayValue(square)
  local splitMove = {}
  local i = 1
  for word in square:gmatch('.') do
    splitMove[i] = word
    i = i + 1
  end

  if (tonumber(splitMove[2]) < 1 or tonumber(splitMove[2]) > 8) then
    error("Invalid move!")
  end

  numberOfFile=0
  if (splitMove[1] == "a") then numberOfFile=1
  elseif (splitMove[1] == "b") then numberOfFile=2
  elseif (splitMove[1] == "c") then numberOfFile=3
  elseif (splitMove[1] == "d") then numberOfFile=4
  elseif (splitMove[1] == "e") then numberOfFile=5
  elseif (splitMove[1] == "f") then numberOfFile=6
  elseif (splitMove[1] == "g") then numberOfFile=7
  elseif (splitMove[1] == "h") then numberOfFile=8
  else
    error("Invalid move!")
  end

  arrayValue = numberOfFile + (tonumber(splitMove[2])-1) * 8
  return arrayValue
end

function pawnMove(move)
  local targetSquare = squareToArrayValue(move)
  local currentBoardValue = currentBoard[targetSquare]
  sendLuaMessage("Square has currently on it "..currentBoardValue)
  sendLuaMessage("Pawn to "..move)
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

    -- strip out irrelevant text and trim whitespace
    local move = string.sub(currentLine, startOfUserText+string.len(moveText)+1):gsub("^%s*(.-)%s*$", "%1")

    if (string.len(move) == 2) then
      pawnMove(move)
    else
      sendLuaMessage(move)
    end

  end
end

function saveData()
  print ("Saving chess.lua data...")
end

return {name="Chess", description="Allows two users to play chess on the channel",
	parseFunction=chessParse, saveDataFunction=saveData}