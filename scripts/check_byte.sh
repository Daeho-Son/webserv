#!/bin/bash

##
# check_byte.sh
##

clear

ls -al ./put_test | grep file_should_exist_after | awk '{print $5}'
