----------------------------------------------------------------------
-- Copyright 2012 John Pirie
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
-- Description: A plugin which allows the user to play a game of chess
--              against another player (currently not fully working).
----------------------------------------------------------------------

local gameInProgress = false
local whiteToMove = true
local currentBoard = {}
local invalidMove = "Invalid move!"

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
    sendLuaChannelMessage(invalidMove)
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
    sendLuaChannelMessage(invalidMove)
  end

  arrayValue = numberOfFile + (tonumber(splitMove[2])-1) * 8
  return arrayValue
end

function pawnMove(move)
  local targetSquare = squareToArrayValue(move)
  local currentBoardValue = currentBoard[targetSquare]
  sendLuaChannelMessage("Square has currently on it "..currentBoardValue)

  -- check the player want to move into an empty square
  if (currentBoardValue == " ") then

    --check that the player is coming from a valid square

    sendLuaChannelMessage("Pawn to "..move)
  else
    sendLuaChannelMessage(invalidMove)
  end
end

function chessParse(username, serverPart, userMessage, isPrivateMessage)
  botname = getBotName():lower()
  if (string.find(userMessage, botname..": new chess game")) then
    if (gameInProgress) then
      sendLuaChannelMessage("A game is already in progress. To restart the game, use the command \"restart chess game\" (not yet implemented!)")
    else
      gameInProgress = true
      currentBoard = createFreshBoard()
      sendLuaChannelMessage("Got all the pieces off the floor, ready to go. White to move.")
    end
  elseif (string.find(userMessage, botname..": move ")) then
    -- we should clean this up so we don't have to do it twice
    local moveText = botname..": move"
    local startOfUserText = string.find(serverPart..userMessage, moveText)

    -- strip out irrelevant text and trim whitespace
    local move = string.sub(currentLine, startOfUserText+string.len(moveText)+1):gsub("^%s*(.-)%s*$", "%1")

    if (string.len(move) == 2) then
      pawnMove(move)
    else
      sendLuaChannelMessage(move)
    end

  end
end

return {name="Chess", description="Allows two users to play chess on the channel",
	parseFunction=chessParse, saveDataFunction=saveData, loadDataFunction=loadChessData}
