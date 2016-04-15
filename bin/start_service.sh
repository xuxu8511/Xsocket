#!/bin/bash

_NORMAL="\033[0m"
_YELLOW="\033[0;33m"
_CYAN="\033[1;36m"
_GREEN="\033[1;32m"
_RED="\033[1;31m"
_PERPLE="\033[0;35m"

ulimit -c unlimited

USER_NAME=`whoami`

CUR_DATE=`date +%Y%m%d%H%M`
VGD="valgrind --tool=memcheck --leak-check=full --leak-resolution=high --show-reachable=yes --log-file=valgrind.$CUR_DATE"

svr_pids=()

check_service_exist()
{
  svr_pids=()
  pid=`ps -ef | grep "$USER_NAME" | grep "./service ../config/service.xml" | grep -v grep | grep -v $0 | awk '{print $2}'`
  if [ "$pid" != "" ]
  then
    printf "${_YELLOW}service with config file \"${_PERPLE}service.xml${_YELLOW}\": pid=${_GREEN}${pid}\t${_NORMAL}[${_GREEN}RUNNING${_NORMAL}]\n"
    svr_pids["service"]=$pid
  else
    printf "${_YELLOW}service with config file \"${_PERPLE}service.xml${_YELLOW}\": \t\t${_NORMAL}[${_RED} STOPED${_NORMAL}]\n"
  fi
}

echo_success()
{
  printf "${_NORMAL}[${_GREEN}SUCCESS${_NORMAL}]\n"
}

kill_service()
{
  check_service_exist
  if [ ${#svr_pids["service"]} -gt 0 ]
  then
    printf "${_YELLOW}killing running service${_CYAN} $1 ${_YELLOW}with pid=${svr_pids["service"]}...${_NORMAL}\t\t"
    kill -9 ${svr_pids["service"]} || exit 0
    printf "[${_RED} KILLED${_NORMAL}]\n"
  fi

  sleep 1
}

start_service_fail()
{
  printf "\n${_RED}start service failed, will stop all started service${_NORMAL}\n"
  kill_service
  exit 0
}

start_service() 
{
  check_service_exist
  if [ ${#svr_pids["service"]} -lt 1 ]
  then
    printf "\t${_YELLOW} executing \"./service ../config/service.xml\"...${_NORMAL}\t\t"
    echo "executing \"./service ../config/service.xml\"...`date`" > /dev/null
    ## start service command
    nohup ./service ../config/service.xml > /dev/null 2>&1 & > /dev/null && echo_success || start_service_fail
  fi

  sleep 1

  check_service_exist
  if [ ${#svr_pids["serviice"]} -lt 1 ]
  then
    printf "${_YELLOW} service service start fail${_RED}\n"
  fi
}

if [ $# -lt 1 ];
then
  # check the un-started service and start it
  start_service
  exit 0
fi
