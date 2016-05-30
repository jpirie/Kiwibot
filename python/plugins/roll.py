import re
import random
from collections import namedtuple
from plugins.BasePlugin import BasePlugin

class RollPlugin(BasePlugin):

    SUCCESS_MESSAGES = ["May kiwi luck shine apon you", "CREE CREE!", "Most honourable", "Well darn tootin' you did it!", "I can't believe it", "Hallelujah! Praise the kiwi on high!", "You are truly kiwi blessed this day", "WOWF!"]
    FAILURE_MESSAGES = ["I'm the opposite of glad", "And no argy-bargy either", "Shucks", "You bring dishonour to the kiwi family name", "Such a shame, a kiwi shame", "CROOOOO", "By failing you have angered the kiwi gods", "Stop that", "GET OUT!"]


    def __init__(self, name):
        self.Challenge = namedtuple("Challenge", ["dice", "min", "max", "predicate", "target", "challenge", "challengers"])
        self.CanITry = namedtuple("CanITry", ["username", "thing", "dice", "predicate", "target"])
        self.challenge = None
        self.points = {}
        self.dicePattern = re.compile(r"((\d+)?d(\d+)(([+|-])(\d+))?)$")
        self.canI = {}

    def updatePoints(self, username, points):
        if self.challenge and username not in self.challenge.challengers:
            self.challenge.challengers.append(username)
            if username not in self.points.keys():
                self.points[username] = 0
            totalPoints = self.points[username] + points
            if(totalPoints < 0):
                totalPoints = 0
            self.points[username] = totalPoints

    def showChallenge(self, username):
        challenge = self.challenge
        if(not challenge):
            self.sendSingleMessage(username, "No challenge set. CREEEEEEEEEE!")
            return
        self.sendSingleMessage(username, challenge.challenge + ": roll " + challenge.predicate + " " + str(challenge.target) + " on " + challenge.dice + " to succeed.")
        
    def showRanking(self, username):
        scores = sorted(self.points.items(), key=lambda points: -points[1])
        results = [uname + ": " + str(score) + " point" + ("s" if (score != 1) else "") for uname, score in scores]
        if(results):
            self.sendSingleMessage(username, (" | ".join(results)))
        else:
            self.sendSingleMessage(username, "No one has no points. Crooooo")
        
    def roll(self, rollMatch, username):
        multiplier = (rollMatch.group(2))
        dice = int(rollMatch.group(3))
        op = rollMatch.group(5)
        mod = rollMatch.group(6)
        
        if(not multiplier):
            multiplier = 1
        if(dice > 1000 or int(multiplier) > 1000):
            self.sendSingleMessage(username, "That's too many dice to roll for my wing flaps to grasp. CREEEE!")
            return ("", 0)
        if(dice < 1):
            self.sendSingleMessage(username, "Can't roll a " + str(dice) + " sided die. Crooooo")
            return ("", 0)
        total = 0
        calculation = ""
        
        for i in range(int(multiplier)):
            diceRoll = random.randint(1, int(dice))
            total = total + diceRoll
            if((int(multiplier) > 1 or op) and int(multiplier) <= 20):
                calculation += str(diceRoll) + "+"
        
        calculation = calculation[:-1]        
        if((op and mod) and int(multiplier) <= 20):
            calculation = calculation + rollMatch.group(4)
            if(op == '+'):
                total = total + int(mod)
            else:
                total = total - int(mod)     
                    
        return (calculation, total)

    def attemptChallenge(self, username):
        if(self.challenge):
            dice = self.challenge.dice
            rollMatch = self.dicePattern.match(dice)
            if(rollMatch):
                calculation, total = self.roll(rollMatch, username)
                if(total):
                    result = "Rolling a " + str(dice)
                    if(calculation):
                        result += " = " + calculation
                    result += " = " + str(total) 
                    predicate = self.challenge.predicate
                    target = self.challenge.target
                    success = self.rollSucceeds(predicate, total, target)
                    if self.challenge:
                        if(success):
                            result += ". " + username.title() + " SUCCEEDS!"
                        else:
                            result += ". " + username.title() + " FAILS!"
                        if username in self.challenge.challengers:
                            result += " No kiwi points rationed as " + username + " has already attempted this challenge."
                        else:
                            if(success):
                                self.updatePoints(username, 1)
                                result += " " + username.title() + " gains a point! CREEEE!"
                            else:
                                self.updatePoints(username, -1)    
                                result += " " + username.title() + " loses a point. CROOOO"
                    self.sendSingleMessage(username, result)
        else:
            self.sendSingleMessage(username, "No challenge set. CREEEEEEEEEE!")

    def issueChallenge(self, rawDice, predicate, target, challenge, username):
    
        rollMatch = self.dicePattern.match(rawDice)
             
        if(rollMatch):
            multiplier = (rollMatch.group(2))
            dice = int(rollMatch.group(3))
            op = rollMatch.group(5)
            mod = rollMatch.group(6)
            
            if(not multiplier):
                multiplier = 1
        
            maxRoll = int(multiplier) * dice
            minRoll = int(multiplier) * 1
        
            if((op and mod)):
                if(op == '+'):
                    minRoll += int(mod)
                    maxRoll += int(mod)
                else:
                    minRoll -= int(mod)
                    maxRoll -= int(mod)
            
            canSucceed = self.successIsPossible(minRoll, maxRoll, predicate, target)
            if(canSucceed):
                self.challenge = self.Challenge(rawDice, minRoll, maxRoll, predicate, target, challenge, [])
                self.sendSingleMessage(username, "Challenge \"" + challenge + "\" stored in my kiwi accredited tupperware for future rolls")
            else:
                self.sendSingleMessage(username, "It's not possible to roll " + predicate+str(target) + " using " + str(rawDice) + ". You tricky sausage.")
                     
        else:
            self.sendSingleMessage(username, rawDice + " doesn't look like any kind of kiwi dice to me. CREEEEEE!")
            
    def successIsPossible(self, minRoll, maxRoll, predicate, target):
        if(predicate == "<"):
            return (target > minRoll)
        if(predicate == ">"):
            return (target < maxRoll)
        else:
            return (target >= minRoll and target <= maxRoll)

    def rollSucceeds(self, predicate, total, target):
        if(predicate == ">"):
            return total > target
        if(predicate == "<"):
            return total < target
        if(predicate == "="):
            return total == target

    def printTry(self, username, thing, predicate, target, dice):
        message = username + ", roll " + predicate + " " + str(target) + " on a " + dice + " "
        message += "to find out if you can " +  thing
        self.sendSingleMessage(username, message)

    def seeIfYouCan(self, username, thing):
        dice, predicate, target = self.generateTry()
        if(username in self.canI.keys()):
            thing = self.canI[username].thing
            self.sendSingleMessage(username, "You already asked me if you could %s. Try and see if you can do that first. CREEEE!" % (thing, ))
        else:
            self.canI[username] = self.CanITry(username, thing, dice, predicate, target)
            self.printTry(username, thing, predicate, target, dice)

    def showTry(self, username):
        if(username in self.canI.keys()):
            userTry = self.canI[username]
            self.printTry(userTry.username, userTry.thing, userTry.predicate, userTry.target, userTry.dice)
        else:
            self.sendSingleMessage(username, "You didn't ask me if you could do anything! Croooo")

    def generateTry(self):
        multiplier = random.randint(0, 2)
        dice = random.randint(2, 12)
        diceString = (str(multiplier) if multiplier > 1 else "") + "d" + str(dice)
        multiplier = max(multiplier, 1)
        maxRoll = multiplier * dice
        minRoll = multiplier * 1
        target = random.randint(minRoll+1, maxRoll-1)
        predicate = random.choice(["<", ">"])
        return diceString, predicate, target

    def attemptTry(self, username, userTry):
        rollMatch = self.dicePattern.match(userTry.dice)
        if(rollMatch):
            calculation, total = self.roll(rollMatch, username)
            if(self.rollSucceeds(userTry.predicate, total, userTry.target)):
                success = "Success: Result " + calculation + " = " + str(total) + " is " + userTry.predicate + " " + str(userTry.target) + ". You can " +  userTry.thing+ ", " + username +  ". " + random.choice(RollPlugin.SUCCESS_MESSAGES)
                success = success.replace(" my", " your").replace(" me", " you")
                self.sendSingleMessage(username, success)
            else:
                failure = "Failure: Result " + calculation + " = " + str(total) + " is not " + userTry.predicate + " " + str(userTry.target) + ". You can not " + userTry.thing+ ", " + username + ". " + random.choice(RollPlugin.FAILURE_MESSAGES)
                failure = failure.replace(" my", " your").replace(" me", " you")
                self.sendSingleMessage(username,  failure)

    def parseMessage(self, username, serverPart, message, isPrivateMessage):
        match = re.search(self.getBotName() + ": roll ?(.*)", str(message).strip())
        if(match):
            dice = match.group(1)
            # !roll without any extra parameters
            if(not dice):
                self.attemptChallenge(username)
            else:
                rollMatch = self.dicePattern.match(dice)
                if(rollMatch):
                    calculation, total = self.roll(rollMatch, username)
                    if(total):
                        result = "Rolling a " + str(match.group(1))
                        if(calculation):
                            result += " = " + calculation
                        result += " = " + str(total)
                        self.sendSingleMessage(username, result) 
        match = re.search(self.getBotName() + r": challenge ?(.*)", message.strip())
        if(match):
            command = match.group(1).strip()
            if(command == "show"):
                self.showChallenge(username)
            if(command == "rank"):
                self.showRanking(username)
            else:
                chg = re.search("(.*?) ?([<|>|=])+ ?(\d+)\s*(.+)", command)
                if(chg):
                    dice = chg.group(1)
                    predicate = chg.group(2)
                    target = chg.group(3)
                    challenge = chg.group(4)
                    self.issueChallenge(dice, predicate, int(target), challenge, username)

        match = re.search(self.getBotName() + ": ([c|C]an|[M|m]ay|[M|m]ight) [I|i] (.*)\?", str(message).strip())
        if(match):
            thing = match.group(2)
            self.seeIfYouCan(username, thing)

        match = re.search(self.getBotName() + ": try (.+)", message.strip())
        if(match):
            command = match.group(1).strip()
            if(command == "show"):
                self.showTry(username)
            elif(command == "roll"):
                if username in self.canI.keys():
                    userTry = self.canI[username]
                    self.attemptTry(username, userTry)
                    del self.canI[username]
                else:
                    self.sendSingleMessage(username, "What are you trying to do? You didn't ask me anything!")
