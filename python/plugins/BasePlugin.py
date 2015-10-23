import ircbot

class BasePlugin(object):

    def __init__(self, name):
        self.pluginName = name

    def sendMessage(self, *args):
        ircbot.sendMessage(args)

    def sendChannelMessage(self, *args):
        ircbot.sendChannelMessage(*args)

    def sendMessageToSource(self, username, message, isPrivateMessage):
        print("Sending message to source!")
        ircbot.sendMessageToSource(username, message, str(isPrivateMessage))

    def sendPrivateMessage(self, *args):
        ircbot.sendPrivateMessage(*args)

    def runSystemCommand(self, *args):
        ircbot.runSystemCommand(*args)

    def getBotName(self):
        return ircbot.getBotName().decode('utf-8')

    def getChannelName(self):
        return ircbot.getChannelName().decode('utf-8')

    def parseMessage(username, serverPart, usermessage, isPrivateMessage):
        print("Here for some reason?!")

    def saveData():
        pass

    def loadData():
        pass
