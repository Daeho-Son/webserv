#!/bin/bash

##
# done.sh
##

clear

PID=`ps | grep ./webserv | grep -v grep | awk '{print $1}'`

if [[ -n ${PID} ]]; then
	kill -9 ${PID}
	echo "${PID}: 프로세스가 종료되었습니다."
else
	echo "./webserv는 실행중인 프로세스가 아닙니다."
fi
