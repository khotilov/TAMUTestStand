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

/opt/xdaq/bin/xdaq.exe -p 20012 -h localhost -c $PWD/xml/tamu_test.xml >& test_cf.log &
#/opt/xdaq/bin/xdaq.exe -p 20012 -h localhost -c $HOME/TriDAS/emu/emuDCS/TAMUTestStand/xml/tamu_test.xml >& test_cf.log &
#/opt/xdaq/bin/xdaq.exe -l DEBUG -p 20012 -h localhost -c $HOME/TriDAS/emu/emuDCS/TAMUTestStand/xml/tamu_test.xml >& test_debug.log &

echo "started xdaq for TAMUTestStand with PID", $!
