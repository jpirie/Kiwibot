import re
import random
from plugins.BasePlugin import BasePlugin

class RollPlugin(BasePlugin):

    def parseMessage(self, username, serverPart, message, isPrivateMessage):
        rollPattern = re.compile(self.getBotName() + ": roll " +  r"((\d+)?d(\d+)([+|-])?(\d+)?)")
        rollMatch = rollPattern.match(str(message))
        if(rollMatch):
            output = "Rolling " + str(rollMatch.group(1)) + ". "
            multiplier = (rollMatch.group(2))
            dice = int(rollMatch.group(3))
            op = rollMatch.group(4)
            mod = rollMatch.group(5)
            if(not multiplier):
                multiplier = 1
            if(dice < 1):
                self.sendMessageToSource(username, "Can't roll a " + str(dice) + " sided die. Crooooo", isPrivateMessage)
                return
            total = 0
            for i in range(int(multiplier)):
                total = total + random.randint(1, int(dice))
            if(op and mod):
                if(op == '+'):
                    total = total + int(mod)
                else:
                    total = total - int(mod)
            output = output + "Result = " + str(total)
            self.sendMessageToSource(username, output,  isPrivateMessage)




        



    



