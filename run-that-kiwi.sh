############################################################
# This is the file that should be executed to run kiwibot
#
# It will listen to the return signals, and respond appropriately
# by rebooting/shutting down/etc our nice kiwi
############################################################

#!/bin/bash

echo "run-that-kiwi.sh started" > run-that-kiwi-log

# do forever
while [ 1 ]
do
    date=`date '+%Y-%m-%d'`

    echo "starting kiwi..." >> run-that-kiwi-log

    # run kiwi bot
    # if you want to run this script on a machine where this path doesn't exist,
    # maybe add a switch to test the hostname or something? Can't do ./kiwibot
    # when this runs on lxultra8 machine because apparently it explodes
    ./kiwibot > "$date-log"

    # get the return value
    returnValue="$?"

    echo "kiwi exited with return value $returnValue" >> run-that-kiwi-log

    # code 1 = reboot the kiwi
    if [ "$returnValue" == 1 ]
    then
	# this is currently disabled, kiwi cannot restart
        # this is because when using an ssh connection to spawn a process, which is then disowned,
        # which then tries to spawn processes of its own, access is denied. Probably some user issue.
	echo "it wants a reboot"
	echo "restart value returned. make clean and kiwibot..." >> run-that-kiwi-log
	make clean && make kiwibot
	echo "done!" >> run-that-kiwi-log
    # code 2 = shut down the kiwi
    elif [ "$returnValue" == 2 ]
    then
	echo "shutdown value returned from kiwibot" >> run-that-kiwi-log
	echo "shutdown value returned from kiwibot"
	exit;
    else
	echo "WARNING: unknown return code!" >> run-that-kiwi-log
	echo "WARNING: unknown return code!"
	exit;
    fi
done
