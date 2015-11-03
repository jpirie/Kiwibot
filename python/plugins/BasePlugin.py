import ircbot

#TODO: Fix the interface and comment each method
class BasePlugin(object):

    def __init__(self, name):
        self.pluginName = name

    def sendMessage(self, message):
        ircbot.sendMessage(message)

    def sendChannelMessage(self, *args):
        ircbot.sendChannelMessage(*args)

    def sendSingleMessage(self, username, message):
        ircbot.sendMessageToSource(username, message, "0")

    def sendMessageToSource(self, username, message, isPrivateMessage):
        ircbot.sendMessageToSource(username, message, str(isPrivateMessage))

    def sendPrivateMessage(self, username, message):
        ircbot.sendPrivateMessage(username, message)

    def runSystemCommand(self, command):
        ircbot.runSystemCommand(command)

    def getBotName(self):
        return ircbot.getBotName().decode('utf-8')

    def getChannelName(self):
        return ircbot.getChannelName().decode('utf-8')

    def saveData():
        pass

    def loadData():
        pass
