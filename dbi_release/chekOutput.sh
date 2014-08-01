#!/bin/bash - 
#===============================================================================
#
#          FILE: chekOutput.sh
# 
#         USAGE: ./chekOutput.sh 
# 
#   DESCRIPTION: 
# 
#       OPTIONS: ---
#  REQUIREMENTS: ---
#          BUGS: ---
#         NOTES: ---
#        AUTHOR: zad, 
#  ORGANIZATION: 
#       CREATED: 29/07/2014 11:03:01 CEST
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error
if [ -f runoutput.log ]; then
  rm runoutput.log
fi
while [ 1 ]
do
  adb shell "cat /data/local/tmp/log" >tmp.t
  if [[ $? -gt 0 ]]; then
    echo "ERROR! device"
    exit 1
  fi
  cat tmp.t |sed  -e 's/\r/\n/' > tmp; diff  runoutput.log tmp --suppress-common-lines -NZ |grep ">"|sed -e "s/^> //" >> runoutput.log
  sleep 1
done

