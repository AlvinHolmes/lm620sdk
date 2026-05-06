#!/bin/bash

if [[ $# -lt 1 || $# -eq 2 ]]; then
    echo "USAGE: $0 dest_path [abs_path address]"
    exit 1;
fi

#save current work directory
SPWD=`pwd`

#get abslute path of shell folder
SHELL_FOLDER=$(dirname $(readlink -f "$0"))

#change to shell folder
cd ${SHELL_FOLDER}

#get current work directory
PWD=`pwd`

if [[ $1 == /* ]]; then
    OUTDIR=$1
elif [[ $1 == ~/* ]]; then
    OUTDIR=$1
else
    OUTDIR=${SPWD}/$1
fi
if [ $# -gt 2 ]; then
    EXTRA_ARG="-f $2 -a $3"
fi

# try to merge ../../bin/nv/nv_submod/nvd_Modem_0x00000000.bin by default.
# otherwise, merge the specified bin
# check if src/nvmodem exist
SDK_VER=$(grep sdkver ../../project/${PROJECT_NAME}/cfg/base_option.lua | sed '/--.*/{s/--.*//;}' | grep -oP '(?<=\", \")[^"]*')
echo "USE PROJECT: ${PROJECT_NAME}"
echo "USE SDK Version: ${SDK_VER}"

if [ -d "${PWD}/src/nvmodem" ]; then
    MODEM_BIN=
else
    MODEM_BIN=../../library/${SDK_VER}/nv/nv_submod/nvd_Modem_0x00000000.bin
    export MODEM_DIR=../../library/${SDK_VER}/include/modem
fi

PROJECT_PATH=../../project/${PROJECT_NAME}
make IST_DIR=${PWD} clean && make IST_DIR=${PWD} MODEM_PRJDIR=${PROJECT_PATH} install 

if [ $? != 0 ]; then
    exit 1;
fi

if [ "${OS}" = "Windows_NT" ] ; then
    ./nvgen.exe ${OUTDIR} ${MODEM_BIN} ${EXTRA_ARG}
else
    ./nvgen ${OUTDIR} ${MODEM_BIN} ${EXTRA_ARG}
fi

if [ $? != 0 ]; then
    exit 1;
fi

echo "NVGEN out: ${OUTDIR}"
#resotre current work directory
cd ${SPWD}

exit 0
