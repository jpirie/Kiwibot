############################################################
# This is the file that should be executed to run kiwibot
#
# It will listen to the return signals, and respond appropriately
# by rebooting/shutting down/etc our nice kiwi
############################################################

#!/bin/bash

# do forever
while [ 1 ]
do
    # run kiwi bot
    ./kiwibot

    # get the return value
    returnValue="$?"

    echo "Got return value $returnValue"

    # code 1 = reboot the kiwi
    if [ "$returnValue" == 1 ]
    then
	echo "it wants a reboot"
	make clean && make kiwibot
    # code 2 = shut down the kiwi
    elif [ "$returnValue" == 2 ]
    then
	echo "shutdown value returned from kiwibot"
	break;
    else
	echo "WARNING: unknown return code!"
    fi
done