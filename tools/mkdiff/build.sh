#!/bin/sh

if [ $# != 1 ]; then
    echo "USAGE: $0 dest path"
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

make IST_DIR=${OUTDIR} clean && make IST_DIR=${OUTDIR} install

#resotre current work directory
cd ${SPWD}

exit 0
