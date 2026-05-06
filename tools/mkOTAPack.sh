#!/bin/bash


# ---------------------------------------------------
# default config
# ---------------------------------------------------
TMPSIG=sig.tmp

# ---------------------------------------------------
# Arguments
# ---------------------------------------------------
OLD_VERNUM=""       # number of old version: -o 
OLD_IMGPATH=""      # path of old version directory: -s 
NEW_VERNUM=""       # number of new version: -n 
NEW_IMGPATH=""      # path of new version directory: -d 
PACKAGE_PATH=""     # path of FOTA package file: -p 
SECBOOT_VER="0"     # secure boot version: -b 
SIGNATURE=0         # signature flag: -k


# ---------------------------------------------------
# Tools
# ---------------------------------------------------

# Get SHELL directory.
TOOLDIR=$(dirname $(readlink -f "$0"))

# mkdiff
MKDIFF=$TOOLDIR/mkdiff/mkdiff

# signature
MKCERT=$TOOLDIR/mkimg/mk_cert_openssl
FLCERT=$TOOLDIR/mkimg/file_cert
SIGKEY_FILE=$TOOLDIR/mkimg/signing_key.pem


# ---------------------------------------------------
# Functions ......
# ---------------------------------------------------

function usage() {
    echo "                                                                                                      "
    echo "Usage:                                                                                                "
    echo "  mkOTAPack.sh <-o old_ver> <-s old_path> <-n new_ver> <-d new_path> <-p pack_path> [-b secboot] [-k] "
    echo "                                                                                                      "
}

function judge_run_result() {
    if [ $? -ne 0 ]; then
        rm -fr $TMPSIG
        exit 1
    fi
}

function mkdiff() {
    echo "==================================== make OTA package ===================================="
    ${MKDIFF} -f patch -o $OLD_VERNUM -s $OLD_IMGPATH -n $NEW_VERNUM -d $NEW_IMGPATH -p $PACKAGE_PATH -b $SECBOOT_VER
    judge_run_result
}

function signature() {
    echo "================================= signature OTA package ==================================="
    ${MKCERT} --certVer=$SECBOOT_VER --sign_key=$SIGKEY_FILE --mergeHead=0x1 --srcFile=$PACKAGE_PATH --signature=$TMPSIG
    judge_run_result
    ${FLCERT} --srcFile=$PACKAGE_PATH --certFile=$TMPSIG --outFile=$PACKAGE_PATH
    judge_run_result
    rm -fr $TMPSIG
}

# ---------------------------------------------------
# Parse arguments.
# ---------------------------------------------------
while getopts 'o:s:n:d:p:b:kh' OPT; do
    case $OPT in
        o) OLD_VERNUM="$OPTARG"
        ;;
        s) OLD_IMGPATH="$OPTARG"
        ;;
        n) NEW_VERNUM="$OPTARG"
        ;;
        d) NEW_IMGPATH="$OPTARG"
        ;;
        p) PACKAGE_PATH="$OPTARG"
        ;;
        b) SECBOOT_VER="$OPTARG"
        ;;
        k) SIGNATURE=1
        ;;
        h)
        usage
        exit 0;
        ;;      
    esac    
done

if [[ x$OLD_VERNUM = x || x$OLD_IMGPATH = x || x$NEW_VERNUM = x || x$NEW_IMGPATH = x || x$PACKAGE_PATH = x ]]; then
    usage
    exit 1;
fi

# ---------------------------------------------------
# Generate OTA package.
# ---------------------------------------------------
mkdiff

# ---------------------------------------------------
# Signature OTA package.
# ---------------------------------------------------
if [ $SIGNATURE != 0 ]; then
    signature
fi
