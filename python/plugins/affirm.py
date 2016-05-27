import os
from random import shuffle
from plugins.BasePlugin import BasePlugin

class AffirmPlugin(BasePlugin):

    def __init__(self, name):
        print("Loaded affirm")
        plugin_loc = os.path.dirname(__file__)
        with open(plugin_loc + '/data/affirm.txt', 'rb') as f:
            self.aff = f.read().decode('utf-8').splitlines()
        shuffle(self.aff)
        self.index = 0
        self.count = len(self.aff)

    def parseMessage(self, username, serverPart, message, isPrivateMessage):
        if message.strip() == (self.getBotName() + ": affirm") :
            if self.index < self.count :
                self.sendSingleMessage(username, self.aff[self.index])
                self.index+=1
            else:
                shuffle(self.aff)
                self.index = 0
                self.sendSingleMessage(username, self.aff[self.index])

