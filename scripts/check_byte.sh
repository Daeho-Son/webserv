#!/bin/bash

##
# check_byte.sh
##

clear

NUMBER=`ls -al ./put_test | grep file_should_exist_after | awk '{print $5}'`
printf "%'.0f\n" $NUMBER
