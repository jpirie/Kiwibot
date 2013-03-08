#!/usr/bin/python

################################################################################
# Name: generate-comics.py
#
# Desc: Looks through the history of kiwibot, and when it comes across
#       humerous lines, will make a comic out of them. Thanks go to
#       Paul Mutton who wrote ComicTest.java situated in this directory.
#
################################################################################

import os
import sys

# the first argument is the path of the script
if len(sys.argv) != 2:
    print("Error: you must pass this script an argument specifying the location of history log files.")
    sys.exit(1)

# length of the datestamp at the start of the lines
DATE_LENGTH = 9

def strip (string):
    return string[DATE_LENGTH:len(string)-1]

# gets the name of the user
def getName (string):
    if string.find(":"):
        return string[0:string.find(":")]
    else:
        return ""

# calls the shell command which actually prints the png comic
def printComic(string1, string2, string3, string4, string5, string6):
    os.system ("java CartoonWrapper \"" + string1 + "\" \"" + string2 + "\" \"" + string3 + "\" \"" + string4 + "\" \"" + string5 + "\" \"" + string6 + "\"")

# history director comes from the argument
historyDir = sys.argv[1]

# go through all of the history files
for root, dirs, filenames in os.walk(historyDir):
    for f in filenames:
        fileHandler = open(os.path.join(root, f), 'r')
        fileContents = fileHandler.readlines()
        lineIter = iter(fileContents)

        counter = 0
        for line in lineIter:
            counter = counter + 1
            # look for funny lines
            # it's a longer line, include the line including 'lol'
            if (line.lower().find("lol") != -1) and len(line) > (DATE_LENGTH + 15):
                printComic(strip(fileContents[counter-6]),strip(fileContents[counter-5]),strip(fileContents[counter-4]),
                           strip(fileContents[counter-3]),strip(fileContents[counter-2]),strip(fileContents[counter-1]))
            # otherwise, don't show the 'lol' line
            elif (line.lower().find("lol") != -1):
                printComic(strip(fileContents[counter-7]),strip(fileContents[counter-6]),strip(fileContents[counter-5]),
                           strip(fileContents[counter-4]),strip(fileContents[counter-3]),strip(fileContents[counter-2]))

