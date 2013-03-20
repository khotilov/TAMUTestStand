#!/bin/bash

# it is expected to be launched from the TAMUTestStand/ location
if [ `basename $PWD` != "TAMUTestStand" ];
then
  echo "The script"
  echo $0
  echo "was not started from the TAMUTestStand/ location!!!"
  echo "Exiting..."
  exit 0
fi

export XDAQ_ROOT=/opt/xdaq
export XDAQ_TEMP=/tmp
#export BUILD_HOME=/home/cscdev/TriDAS
#export LD_LIBRARY_PATH=${BUILD_HOME}/x86_64_slc5/lib:/opt/xdaq/lib
# export XDAQ_OS=linux
# export XDAQ_PLATFORM=x86_64_slc5
#export XDAQ_DOCUMENT_ROOT=${XDAQ_ROOT}/htdocs

/opt/xdaq/bin/xdaq.exe -h 165.91.181.27 -p 20014 -c $PWD/xml/tamu_test_me11dev.xml -e $PWD/xml/tamu_test.profile >& test_me11dev.log &
#/opt/xdaq/bin/xdaq.exe -l DEBUG -h 165.91.181.27 -p 20014 -c $PWD/xml/tamu_test_me11dev.xml -e $PWD/xml/tamu_test.profile >& test_debug.log &

echo "started xdaq for TAMUTestStand with PID", $!
