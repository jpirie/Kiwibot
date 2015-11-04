from os import listdir, chdir, remove
from os.path import isfile, join
from importlib import import_module
from imp import reload
from plugins.BasePlugin import BasePlugin
from collections import namedtuple
import sys

plugins = {}
Plugin = namedtuple("Plugin", ["hashcode", "instance"])

def compareHashes(filepath, hashcode):
    fileHash = getFileHash(filepath)
    if(hashcode and (fileHash == hashcode)):
        return False
    return True
    
def getFileHash(filepath):
    moduleFile = None
    try:
        moduleFile = open(filepath, "r") 
        fileLines = "".join(moduleFile.readlines())
        # simple hash is probably fine for this
        return hash(fileLines)
    except Exception as e:
        print(str(e))
        return None
    finally:
        if(moduleFile):
            moduleFile.close()

def updatePlugins():

    path = "./python/plugins/"
    pythonModules = [ f.split(".")[0] for f in listdir(path) if isfile(join(path,f)) if f.endswith(".py")]        

    global plugins

    toRemove = []
    for plugin in plugins.keys():
        if plugin not in pythonModules:
            toRemove.append(plugin)
    for r in toRemove:
        del plugins[toRemove]

    toLoad = []
    for module in pythonModules:
        if(module not in plugins.keys()):
            toLoad.append(module)
        else:
            filepath = path + module + ".py"
            plugin = plugins.get(module, None)
            hashChanged = compareHashes(filepath, plugin.hashcode)
            if(plugin and hashChanged):
                toLoad.append(module)
    try:
        for module in toLoad:
            try:
                mod = import_module("plugins." + str(module))
                if module in plugins.keys():
                    mod = reload(mod)
                packageClasses  = [getattr(mod, m) for m in dir(mod)]
                packageClasses = [m for m in packageClasses if str(type(m)) == "<class 'type'>"]
                pluginClasses = [p for p in packageClasses if issubclass(p, BasePlugin) and p is not BasePlugin]
                for p in pluginClasses:
                    hashcode = getFileHash(path + module + ".py")
                    global plugins
                    plugins[str(module)] = Plugin(hashcode, p(module))
                    print("Succeeded loading: " + module)
            except Exception as e:
                print("Failed to load " + module)
                print(str(e))
    except Exception as e:
        print(e)
    return 0

def main(username, serverPart, message, isPrivateMessage):
    updatePlugins()
    global plugins
    for plugin in plugins.values():
        try:
            plugin.instance.parseMessage(username, serverPart, message, isPrivateMessage)
        except Exception as e:
            print("Parsing message failed")
            print(str(e))
