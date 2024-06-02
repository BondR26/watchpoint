#!/bin/bash

# Source and destination paths
SOURCE_PATH="yocto/poky/build/tmp/work/qemux86-poky-linux/watchpoint/1.0-r0/watchpoint.ko"
DESTINATION_PATH="root@192.168.7.2:/tmp"

# Perform the scp command
scp $SOURCE_PATH $DESTINATION_PATH

# Check if the scp command was successful
if [ $? -eq 0 ]; then
    echo "File copied successfully."
else
    echo "Failed to copy the file."
fi

