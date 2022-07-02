#!/bin/bash

##
# test.sh
##

clear

PID=`ps | grep ./webserv | grep -v grep | awk '{print $1}'`
echo -n "Please enter the number of times to test: "
read NUMBER
if [[ -n ${PID} ]]; then
	python3 ./test/test.py $NUMBER
else
	echo "./webserv가 실행중이 아닙니다."
fi
