#!/bin/bash

if [ "${OS}" = "Windows_NT" ] ; then
   ./NvUpdater.exe -xml $1 -cfg $2 -bin $3
else
   chmod +x NvUpdater
   ./NvUpdater -xml $1 -cfg $2 -bin $3
fi