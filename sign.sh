#!/bin/bash

###################################################################################
#
#  * Copyright (c) 2022, Nanjing Innochip Technology Co.,Ltd.
#  *
#  * Licensed under the Apache License, Version 2.0 (the "License"); 
#  * you may not use this file except in compliance with the License. 
#  * You may obtain a copy of the License at
#  *
#  *     http://www.apache.org/licenses/LICENSE-2.0
#  *
#  *  Date                Author                Notes
#  *  2023-05-18          Edward Wang           First version
#  *  
#  *  History:
#  *         V1.2.3 @2024/07/18   删除冗余语句等
#  *         V1.2.2 @2024/05/29   增加NvUpdater和多BIN合一等关联需求等
#  *         V1.2.1 @2024/05/25   增加兼容不同工程编译版本的外发需求
#  *         V1.2.0 @2024/05/21   增加兼容ZCnuclei和nuclei编译
#  *         V1.1.9 @2024/05/16   增加MTS版本编译NV镜像文件
#  *         V1.1.8 @2024/02/23   增加MINI版本编译用于自动化测试
#  *         V1.1.7 @2024/01/17   解决mts-rule编译时boot镜像被删除的问题
#  *         V1.1.6 @2024/01/11   增加工具侧支持增量编译方式
#  *         V1.1.5 @2023/12/19   解决主脚本项目不对应问题等
#  * 
###################################################################################

# ---------------------------------------------------
# default config
# ---------------------------------------------------
CERT_VER="0"
CERT_CMD=""
SIGN_ARG=""
FILE_PATH=""

# ---------------------------------------------------
# project config
# ---------------------------------------------------
PRODUCT="ict2210"

# ---------------------------------------------------
# mbl config
# ---------------------------------------------------
MBL_TEXT_BASE=0xC0247000
START_SOC_ID=0x0
END_SOC_ID=0xFFFFFFFFFFFFFFFF
BOOT_DELAY=0x64
UART_BOOT_DELAY=0xC8

# ---------------------------------------------------
# xboot config
# ---------------------------------------------------
XBOOT_TEXT_BASE=0x80000000

# ---------------------------------------------------
# kernel config
# ---------------------------------------------------
CP_LOADADDR=0x80000000
CP_ENTRY=0xF5000000
AP_LOADADDR=0x800FE800
AP_ENTRY=0x00008000

# ---------------------------------------------------
# Get current directory.
# ---------------------------------------------------
ROOTDIR=$(dirname $(readlink -f "$0"))
cd ${ROOTDIR}
#echo "===> ROOTDIR is ${ROOTDIR}"

SRC_PATH=${ROOTDIR}/../bin/version
IMG_PATH=${ROOTDIR}/../bin/image
DEST_PATH=${ROOTDIR}/../bin/release

#tools
TOOLS_PATH=${ROOTDIR}/tools/mkimg
MKCERT=${TOOLS_PATH}/mk_cert_openssl
FILECERT=${TOOLS_PATH}/file_cert
#MKCERT_PRI=${TOOLS_PATH}/mk_cert_openssl_pri
#FILECERT_PRI=${TOOLS_PATH}/file_cert_pri
MKCERT_PRI=${TOOLS_PATH}/mk_cert_openssl
FILECERT_PRI=${TOOLS_PATH}/file_cert
GENDBGCERT=${TOOLS_PATH}/gendbgcert
MKJOINT=${TOOLS_PATH}/joint
MKMBL=${TOOLS_PATH}/mkmbl
MKIMAGE=${TOOLS_PATH}/mkimage

# ---------------------------------------------------
# Use regular expression to parse signature command&arg.
# ---------------------------------------------------
for arg in $*
do
    if [[ ${arg} =~ ^pro=.* ]]; then
        PRODUCT=${arg:4}
    elif [[ ${arg} =~ ^certV=.* ]]; then
        CERT_VER=${arg:6}
    elif [[ ${arg} =~ ^file=.* ]]; then
        FILE_PATH=${arg:5}
    elif [[ ${arg} =~ ^src=.* ]]; then
        SRC_PATH=${arg:4}
    elif [[ ${arg} =~ ^img=.* ]]; then
        IMG_PATH=${arg:4}
    elif [[ ${arg} =~ ^dest=.* ]]; then
        DEST_PATH=${arg:5}
    else
        if [[ ${CERT_CMD} == "" ]]; then
            CERT_CMD=${arg}
        else
            SIGN_ARG="${SIGN_ARG} ${arg}"
        fi
    fi
done

function usage() {
    echo "                                                                 "
    echo "Usage:                                                           "
    echo "     ./sign.sh img              (Use cmd to sign image)          "
    exit 0
}

function fail_result() {
    echo "###################################################################"
    echo "############### ERROR:   no files to be signed!   #################"
    echo "###################################################################"
}

function clear_image() {
    if [ -e "${IMG_PATH}" ]; then
        rm -fr ${IMG_PATH}
    fi
}

function clear_release() {
    if [ -e "${DEST_PATH}" ]; then
        rm -fr ${DEST_PATH}
    fi
}

function sign_mbl_img() {
    if [ ! -e "${SRC_PATH}/mbl.bin" ]; then
        fail_result
        exit 1
    fi

    if [ ! -e "${IMG_PATH}" ]; then
        mkdir -p ${IMG_PATH}
    fi

    #移除mbl.bin下载标记和对齐补充
    ${MKMBL} --product=NULL --binFile=${SRC_PATH}/mbl.bin --output=${IMG_PATH}/mbl.bin
    #对mbl.img镜像启动验签进行签名
    ${MKCERT_PRI} --certVer=${CERT_VER} --sign_key=${TOOLS_PATH}/signing_key.pem --startSocID=${START_SOC_ID} --endSocID=${END_SOC_ID} --srcFile=${IMG_PATH}/mbl.bin --signature=${IMG_PATH}/mbl_cert.bin

    if [ -e "${ROOTDIR}/boot/project/boot.config" ]; then
        ${MKMBL} --bootDelay=${BOOT_DELAY} --uartBootDelay=${UART_BOOT_DELAY} --binFile=${IMG_PATH}/mbl.bin  --dataLoadAddr=${MBL_TEXT_BASE} --loadEntry=${MBL_TEXT_BASE} --xipEntry=0x0 --certFile=${IMG_PATH}/mbl_cert.bin --output=${IMG_PATH}/mbl.img --configFile=${ROOTDIR}/boot/project/boot.config --outputConfig=${IMG_PATH}/config.bin
    else
        #生成带MBL的签名证书打包镜像文件
        ${MKMBL} --bootDelay=${BOOT_DELAY} --uartBootDelay=${UART_BOOT_DELAY} --binFile=${IMG_PATH}/mbl.bin  --dataLoadAddr=${MBL_TEXT_BASE} --loadEntry=${MBL_TEXT_BASE} --xipEntry=0x0 --certFile=${IMG_PATH}/mbl_cert.bin --output=${IMG_PATH}/mbl.img
    fi

    #生成debug证书
    ${GENDBGCERT} --socId=0x1800 --dLoad=0x1 --jTag=0x1 --output=${IMG_PATH}/debug.bin
    #对debug证书启动验签进行签名
    ${MKCERT_PRI} --certVer=${CERT_VER} --sign_key=${TOOLS_PATH}/signing_key.pem --mergeHead=0x1 --srcFile=${IMG_PATH}/debug.bin --signature=${IMG_PATH}/debug_sig.bin
    if [ -e "${IMG_PATH}/mbl.img" ]; then
        ${GENDBGCERT} --binFile=${IMG_PATH}/debug.bin --certFile=${IMG_PATH}/debug_sig.bin --output=${IMG_PATH}/mbl_$(printf "0x%X" $(find ${IMG_PATH}/mbl.img -printf "%s")).img
    else
        ${GENDBGCERT} --binFile=${IMG_PATH}/debug.bin --certFile=${IMG_PATH}/debug_sig.bin --output=${IMG_PATH}/debug.img
    fi

    rm -fr ${IMG_PATH}/mbl_cert.bin
    rm -fr ${IMG_PATH}/debug_sig.bin
    rm -fr ${IMG_PATH}/debug.bin
}

function sign_xboot_img() {
    if [ ! -e "${SRC_PATH}/xboot.bin" ] || [ ! -e "${SRC_PATH}/xboot.img" ]; then
        fail_result
        exit 1
    fi

    if [ ! -e "${IMG_PATH}" ]; then
        mkdir -p ${IMG_PATH}
    fi

    cp -fr ${SRC_PATH}/xboot.bin  ${IMG_PATH}/xboot.bin
    cp -fr ${SRC_PATH}/xboot.img  ${IMG_PATH}/xboot.img
}

function sign_zknl_img() {
    if [ ! -e "${SRC_PATH}/zknl.img" ] || [ ! -e "${SRC_PATH}/partition.ini" ]; then
        fail_result
        exit 1
    fi

    if [ ! -e "${IMG_PATH}" ]; then
        mkdir -p ${IMG_PATH}
    fi

    #解压没有签名的zknl.img
    ${MKIMAGE} --zip=dec --input=${SRC_PATH}/zknl.img --output=${IMG_PATH}/zknl.bin
    #对zknl.img启动验签进行签名
    ${MKCERT} --certVer=${CERT_VER} --sign_key=${TOOLS_PATH}/signing_key.pem --mergeHead=0x2 --srcFile=${IMG_PATH}/zknl.bin --signature=${IMG_PATH}/zknl_sig.bin
    #合并zknl.bin原始数据和证书
    ${FILECERT} --imgCert=0x1 --srcFile=${IMG_PATH}/zknl.bin --certFile=${IMG_PATH}/zknl_sig.bin --outFile=${IMG_PATH}/zknl.bin
    #重新压缩已签名的zknl.img，并对镜像文件做是否超分区检查
    ${MKIMAGE} --zip=enc --size=$(grep zknl ${SRC_PATH}/partition.ini | awk -v keyword="zknl" '{print $3}') --input=${IMG_PATH}/zknl.bin --packSize=0x40 --output=${IMG_PATH}/zknl.img

    rm -fr ${IMG_PATH}/zknl_sig.bin
    rm -fr ${IMG_PATH}/zknl.bin
}

function sign_xknl_img() {
    if [ ! -e "${SRC_PATH}/xknl.img" ] || [ ! -e "${SRC_PATH}/partition.ini" ]; then
        fail_result
        exit 1
    fi

    if [ ! -e "${IMG_PATH}" ]; then
        mkdir -p ${IMG_PATH}
    fi

    #对xknl.img启动验签进行签名
    ${MKCERT} --certVer=${CERT_VER} --sign_key=${TOOLS_PATH}/signing_key.pem --mergeHead=0x2 --srcFile=${SRC_PATH}/xknl.img --signature=${IMG_PATH}/xknl_sig.bin
    #合并xknl.bin原始数据和证书
    ${FILECERT} --imgCert=0x1 --srcFile=${SRC_PATH}/xknl.img --certFile=${IMG_PATH}/xknl_sig.bin --outFile=${IMG_PATH}/xknl.img
    #对镜像文件做是否超分区检查
    ${MKJOINT} --size=$(grep xknl ${SRC_PATH}/partition.ini | awk -v keyword="xknl" '{print $3}') --input=${IMG_PATH}/xknl.img

    rm -fr ${IMG_PATH}/xknl_sig.bin
}

function sign_partition_img() {
    if [ ! -e "${SRC_PATH}/partition.cfg" ]; then
        fail_result
        exit 1
    fi

    if [ ! -e "${IMG_PATH}" ]; then
        mkdir -p ${IMG_PATH}
    fi

    cp -fr ${SRC_PATH}/partition.cfg  ${IMG_PATH}/partition.cfg
}

function sign_nvd_img() {
    if [ ! -e "${SRC_PATH}/nvd_full_0x00000000.bin" ] || [ ! -e "${SRC_PATH}/nvd_roPhyuser_0x00000000.bin" ]; then
        fail_result
        exit 1
    fi

    if [ ! -e "${IMG_PATH}" ]; then
        mkdir -p ${IMG_PATH}
    fi

    cp -fr ${SRC_PATH}/nvd_full_0x00000000.bin  ${IMG_PATH}/nvd_full_0x00000000.bin
    cp -fr ${SRC_PATH}/nvd_roPhyuser_0x00000000.bin  ${IMG_PATH}/nvd_roPhyuser_0x00000000.bin
}

#对所有镜像文件进行下载验签签名
function sign_mbl_rls() {
    if [ ! -e "${IMG_PATH}/mbl.bin" ] || [ ! -e "${IMG_PATH}/mbl.img" ] || [ ! -e "${IMG_PATH}/mbl_$(printf "0x%X" $(find ${IMG_PATH}/mbl.img -printf "%s")).img" ]; then
        fail_result
        exit 1
    fi

    if [ ! -e "${DEST_PATH}" ]; then
        mkdir -p ${DEST_PATH}
    fi

    ${MKCERT_PRI} --certVer=${CERT_VER} --sign_key=${TOOLS_PATH}/signing_key.pem --mergeHead=0x1 --srcFile=${IMG_PATH}/mbl.bin --signature=${DEST_PATH}/mbl_sig.bin
    ${FILECERT_PRI} --product=${PRODUCT} --srcFile=${IMG_PATH}/mbl.bin --certFile=${DEST_PATH}/mbl_sig.bin --outFile=${DEST_PATH}/mbl.bin

    ${MKCERT} --certVer=${CERT_VER} --sign_key=${TOOLS_PATH}/signing_key.pem --mergeHead=0x1 --srcFile=${IMG_PATH}/mbl_$(printf "0x%X" $(find ${IMG_PATH}/mbl.img -printf "%s")).img --signature=${DEST_PATH}/mbl_debug_sig.bin
    ${FILECERT} --srcFile=${IMG_PATH}/mbl_$(printf "0x%X" $(find ${IMG_PATH}/mbl.img -printf "%s")).img --certFile=${DEST_PATH}/mbl_debug_sig.bin --outFile=${DEST_PATH}/mbl_$(printf "0x%X" $(find ${IMG_PATH}/mbl.img -printf "%s")).img

    ${MKCERT} --certVer=${CERT_VER} --sign_key=${TOOLS_PATH}/signing_key.pem --mergeHead=0x1 --srcFile=${IMG_PATH}/mbl.img --signature=${DEST_PATH}/mbl_sig2.bin
    ${FILECERT} --srcFile=${IMG_PATH}/mbl.img --certFile=${DEST_PATH}/mbl_sig2.bin --outFile=${DEST_PATH}/mbl.img
    rm -fr ${DEST_PATH}/mbl_sig.bin
    rm -fr ${DEST_PATH}/mbl_sig2.bin
    rm -fr ${DEST_PATH}/mbl_debug_sig.bin
}

function sign_xboot_rls() {
    if [ ! -e "${IMG_PATH}/xboot.bin" ] || [ ! -e "${IMG_PATH}/xboot.img" ]; then
        fail_result
        exit 1
    fi

    if [ ! -e "${DEST_PATH}" ]; then
        mkdir -p ${DEST_PATH}
    fi

    ${MKCERT} --certVer=${CERT_VER} --sign_key=${TOOLS_PATH}/signing_key.pem --mergeHead=0x1 --srcFile=${IMG_PATH}/xboot.bin --signature=${DEST_PATH}/xboot_sig.bin
    ${FILECERT} --srcFile=${IMG_PATH}/xboot.bin --certFile=${DEST_PATH}/xboot_sig.bin --outFile=${DEST_PATH}/xboot.bin
    ${MKCERT} --certVer=${CERT_VER} --sign_key=${TOOLS_PATH}/signing_key.pem --mergeHead=0x1 --srcFile=${IMG_PATH}/xboot.img --signature=${DEST_PATH}/xboot_sig2.bin
    ${FILECERT} --srcFile=${IMG_PATH}/xboot.img --certFile=${DEST_PATH}/xboot_sig2.bin --outFile=${DEST_PATH}/xboot.img
    rm -fr ${DEST_PATH}/xboot_sig.bin
    rm -fr ${DEST_PATH}/xboot_sig2.bin
}

function sign_zknl_rls() {
    if [ ! -e "${IMG_PATH}/zknl.img" ]; then
        fail_result
        exit 1
    fi

    if [ ! -e "${DEST_PATH}" ]; then
        mkdir -p ${DEST_PATH}
    fi

    ${MKCERT} --certVer=${CERT_VER} --sign_key=${TOOLS_PATH}/signing_key.pem --mergeHead=0x1 --srcFile=${IMG_PATH}/zknl.img --signature=${DEST_PATH}/zknl_sig.bin
    ${FILECERT} --srcFile=${IMG_PATH}/zknl.img --certFile=${DEST_PATH}/zknl_sig.bin --outFile=${DEST_PATH}/zknl.img

    rm -fr ${DEST_PATH}/zknl_sig.bin
}

function sign_xknl_rls() {
    if [ ! -e "${IMG_PATH}/xknl.img" ]; then
        fail_result
        exit 1
    fi

    if [ ! -e "${DEST_PATH}" ]; then
        mkdir -p ${DEST_PATH}
    fi

    ${MKCERT} --certVer=${CERT_VER} --sign_key=${TOOLS_PATH}/signing_key.pem --mergeHead=0x1 --srcFile=${IMG_PATH}/xknl.img --signature=${DEST_PATH}/xknl_sig.bin
    ${FILECERT} --srcFile=${IMG_PATH}/xknl.img --certFile=${DEST_PATH}/xknl_sig.bin --outFile=${DEST_PATH}/xknl.img
    rm -fr ${DEST_PATH}/xknl_sig.bin
}

function sign_partition_rls() {
    if [ ! -e "${IMG_PATH}/partition.cfg" ]; then
        fail_result
        exit 1
    fi

    if [ ! -e "${DEST_PATH}" ]; then
        mkdir -p ${DEST_PATH}
    fi

    ${MKCERT} --certVer=${CERT_VER} --sign_key=${TOOLS_PATH}/signing_key.pem --mergeHead=0x1 --srcFile=${IMG_PATH}/partition.cfg --signature=${DEST_PATH}/part_sig.bin
    ${FILECERT} --srcFile=${IMG_PATH}/partition.cfg --certFile=${DEST_PATH}/part_sig.bin --outFile=${DEST_PATH}/partition.cfg
    rm -fr ${DEST_PATH}/part_sig.bin
}

function sign_nvd_rls() {
    if [ ! -e "${IMG_PATH}/nvd_full_0x00000000.bin" ] || [ ! -e "${IMG_PATH}/nvd_roPhyuser_0x00000000.bin" ]; then
        fail_result
        exit 1
    fi

    if [ ! -e "${DEST_PATH}" ]; then
        mkdir -p ${DEST_PATH}
    fi

    ${MKCERT} --certVer=${CERT_VER} --sign_key=${TOOLS_PATH}/signing_key.pem --mergeHead=0x1 --srcFile=${IMG_PATH}/nvd_full_0x00000000.bin --signature=${DEST_PATH}/nvd_full_sig.bin
    ${FILECERT} --srcFile=${IMG_PATH}/nvd_full_0x00000000.bin --certFile=${DEST_PATH}/nvd_full_sig.bin --outFile=${DEST_PATH}/nvd_full_0x00000000.bin
    ${MKCERT} --certVer=${CERT_VER} --sign_key=${TOOLS_PATH}/signing_key.pem --mergeHead=0x1 --srcFile=${IMG_PATH}/nvd_roPhyuser_0x00000000.bin --signature=${DEST_PATH}/nvd_roPhyuser_sig.bin
    ${FILECERT} --srcFile=${IMG_PATH}/nvd_roPhyuser_0x00000000.bin --certFile=${DEST_PATH}/nvd_roPhyuser_sig.bin --outFile=${DEST_PATH}/nvd_roPhyuser_0x00000000.bin
    rm -fr ${DEST_PATH}/nvd_full_sig.bin
    rm -fr ${DEST_PATH}/nvd_roPhyuser_sig.bin
}

function sign_ordinary_rls() {
    if [ ! -f "${FILE_PATH}" ] || [ ! -e "${FILE_PATH}" ]; then
        fail_result
        exit 1
    fi

    ${MKCERT} --certVer=${CERT_VER} --sign_key=${TOOLS_PATH}/signing_key.pem --mergeHead=0x1 --srcFile=${FILE_PATH} --signature=$(dirname ${FILE_PATH})/ordinary_sig.bin
    ${FILECERT} --srcFile=${FILE_PATH} --certFile=$(dirname ${FILE_PATH})/ordinary_sig.bin --outFile=${FILE_PATH}
    rm -fr $(dirname ${FILE_PATH})/ordinary_sig.bin
}

function fread() {
    od -An -v -t x1 -w1 "${1}"|awk -F ' ' '{printf $1" "}'
}

function sign_test() {
    if [ ! -e "${VERSION_DIR}/partition.ini" ]; then
        fail_result
        exit 1
    fi

    #echo $(hexdump -C ${VERSION_DIR}/partition.cfg)
    #echo $(od -t x1 ${VERSION_DIR}/partition.cfg)
    #echo $(fread "${VERSION_DIR}/partition.cfg" | grep -o '657a6b6e6c')
    #echo $(strings "${VERSION_DIR}/partition.cfg" | grep -o zknl)
    #echo $(grep zknl ${VERSION_DIR}/partition.ini | awk -v keyword="zknl" '{print $3}')

    #echo $(dd if=${VERSION_DIR}/partition.cfg bs=1 skip=0 count=16 status=none | xxd -p)
    #echo $(dd if=${VERSION_DIR}/partition.cfg bs=1 skip=0 count=512 2>/dev/null | xxd -p | grep '6567' -o)
    echo $(dd if=${VERSION_DIR}/partition.cfg bs=1 skip=0 count=512 status=none | xxd -p | grep '7a6b6e6c' -o)
}

# ---------------------------------------------------
# MAIN:  need to input right arguments
# ---------------------------------------------------
while true; do
  case ${CERT_CMD} in
    help )
        usage
        exit 0
        ;;
    image )
        clear_image
        sign_mbl_img
        sign_xboot_img
        sign_zknl_img
        sign_xknl_img
        sign_partition_img
        sign_nvd_img
        exit 0
        ;;
    release )
        clear_release
        sign_mbl_rls
        sign_xboot_rls
        sign_zknl_rls
        sign_xknl_rls
        sign_partition_rls
        sign_nvd_rls
        exit 0
        ;;
    mbl )
        sign_mbl_img
        sign_mbl_rls
        exit 0
        ;;
    xboot )
        sign_xboot_img
        sign_xboot_rls
        exit 0
        ;;
    zknl )
        sign_zknl_img
        sign_zknl_rls
        exit 0
        ;;
    xknl )
        sign_xknl_img
        sign_xknl_rls
        exit 0
        ;;
    part_tbl )
        sign_partition_img
        sign_partition_rls
        exit 0
        ;;
    nvd )
        sign_nvd_img
        sign_nvd_rls
        exit 0
        ;;
    ordinary )
        sign_ordinary_rls
        exit 0
        ;;
    clean )
        clear_image
        clear_release
        exit 0
        ;;
    test )
        sign_test
        exit 0
        ;;
    * )
        printf '===> Hmmm, unknown option: "%s".\n' "${1}";
        usage;
        break
        ;;
  esac
done

