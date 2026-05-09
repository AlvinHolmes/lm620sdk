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
#  *         V1.0.3 @2026/02/27   支持编译app和dm时不编译kernel而解决不同时刻编译的kernel存在差异问题。
#  *         V1.0.2 @2026/01/21   解决动态模块编译APP不生效的问题
#  *         V1.0.1 @2025/02/25   支持动态模块编译
#  *         V1.0.0 @2024/12/25   增加windows路径判断，不支持路径包含空格和中文
#  * 
###################################################################################

SCRIPT_VER='1.0.3 @2026/02/27'

# ---------------------------------------------------
# 版本号定义
# ---------------------------------------------------

MANUFACTURER="InnoChip"
PRODUCT="MDL"
HW_VERSION="MDL_HW_V1.0"
SW_VERSION=""
BUILD_VERSION=""

# ---------------------------------------------------
# default config
# ---------------------------------------------------
USE_PRJ=op-mdl      # default, op-mdl
USE_VTYPE=all       # all; ap; cp
IMG_TYPE=xip        # default
USE_BIN2ONE=yes     # no or yes
BUILD_CMD=""
BUILD_ARG=""
APP_LINK_MODE="elf" #elf, dlsym

# ---------------------------------------------------
# Update git hooks
# ---------------------------------------------------
if [ -f "tools/hooks/commit-msg" ] && [ -e ".git/hooks" ]; then
    cp tools/hooks/* .git/hooks/
fi

# ---------------------------------------------------
# ERROR TYPE
# ---------------------------------------------------
## 0 : SUECCESS
## 1 : ap compiling error in all command
## 2 : cp compiling error in all command
## 3 : ap compiling error in ap command
## 4 : cp compiling error in cp command
## 5 : failed for executing command

#G_ERR_TYPE=0


# ---------------------------------------------------
# Get current directory.
# ---------------------------------------------------
ROOTDIR="$(dirname "$(readlink -f "$0")")"
if [[ ${ROOTDIR} =~ [[:space:]] ]]; then
    echo "===> ROOTDIR is ${ROOTDIR}"
    echo "Error：The path contains spaces cannot be executed!!"
    exit 1
elif [[ ${ROOTDIR} =~ [[:cntrl:]] ]]; then
    echo "===> ROOTDIR is ${ROOTDIR}"
    echo "错误：路径包含中文，不能执行编译！！"
    exit 1
fi
cd "${ROOTDIR}"
#echo "===> ROOTDIR is ${ROOTDIR}"


# ---------------------------------------------------
# Use regular expression to parse build command&arg.
# ---------------------------------------------------
for arg in $*
do
    if [[ ${arg} =~ ^PRJ=.* ]]; then
        USE_PRJ=${arg:4}
    elif [[ ${arg} =~ ^prj=.* ]]; then
        USE_PRJ=${arg:4}
    elif [[ ${arg} =~ ^SV=.* ]]; then
        SW_VERSION=${arg:3}
    elif [[ ${arg} =~ ^sv=.* ]]; then
        SW_VERSION=${arg:3}
    elif [[ ${arg} =~ ^BSV=.* ]]; then
        BUILD_VERSION=${arg:4}
    elif [[ ${arg} =~ ^bsv=.* ]]; then
        BUILD_VERSION=${arg:4}
    elif [[ ${arg} =~ ^LINK=.* ]]; then
            APP_LINK_MODE=${arg:5}
    elif [[ ${arg} =~ ^link=.* ]]; then
        APP_LINK_MODE=${arg:5}
    else
        if [[ ${BUILD_CMD} == "" ]]; then
            BUILD_CMD=${arg}
        else
            BUILD_ARG="${BUILD_ARG} ${arg}"
        fi
    fi
done

# ---------------------------------------------------
# Environment varialbes
# ---------------------------------------------------
export PROJECT_NAME=${USE_PRJ}
export SNCSDT_LICENSE_FILE=27070@127.0.0.1
export BOOT_DIR="${ROOTDIR}/components/bootloader/boot"
export SDK_VER=$( grep sdkver "${ROOTDIR}/project/${PROJECT_NAME}/cfg/base_option.lua" | sed '/--.*/{s/--.*//;}' | grep -oP '(?<=\", \")[^"]*') 
echo "USE SDK Version: ${SDK_VER}"
export APP_LINK_MODE

if [ -z "$SW_VERSION" ]; then
    SW_VERSION=$(grep "SW_VERSION" "./open/config/inc/version_config.h" | grep -o '".*"' | sed 's/"//g')
fi

if [ -z "$BUILD_VERSION" ]; then
    BUILD_VERSION=$(git rev-parse HEAD)
    if [ $? -ne 0 ]; then
        BUILD_VERSION=$(date "+%Y%m%d%H%M%S")
    else
        BUILD_VERSION="Git:${BUILD_VERSION: 0: 32}"
    fi
fi

if [ ! -e "${ROOTDIR}/modem" ]; then
    export USE_SDK=y
fi

# ---------------------------------------------------
# Environment varialbes
# ---------------------------------------------------
make_version() {

version_header="/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        version_config.h
 *
 * @brief       版本号定义.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-04-21     ict team          创建
 ************************************************************************************
 */

#ifndef _VERSION_CONFIG_H_
#define _VERSION_CONFIG_H_

#define MANUFACTURE_NAME        \"${MANUFACTURER}\"

#define PRODUCT_NAME            \"${PRODUCT}\"

#define HW_VERSION              \"${HW_VERSION}\"

/*
 *  External software version
 */
#define SW_VERSION              \"${SW_VERSION}\"

/*
 *  Build software version
 */
#define BUILD_VERSION           \"${BUILD_VERSION}\"

#endif  /* _VERSION_CONFIG_H_ */
"

cat << EOF > "./open/config/inc/version_config.h"
${version_header}
EOF

}

# ---------------------------------------------------
# Functions ......
# ---------------------------------------------------
function version() {
    echo "                                                                                   "
    echo "The scripts version is ${SCRIPT_VER}                                               "
    echo "                                                                                   "
    echo "Copyright (c) 2022, Nanjing Innochip Technology Co.,Ltd.                           "
    echo "                                                                                   "
    echo "Licensed under the Apache License, Version 2.0 (the "License");                    "
    echo "you may not use this file except in compliance with the License.                   "
    echo "You may obtain a copy of the License at                                            "
    echo "                                                                                   "
    echo "    http://www.apache.org/licenses/LICENSE-2.0                                     "
    echo "                                                                                   "    
}

function usage() {
    echo "                                                                                   "
    echo "Usage:                                                                             "
    echo "     ./build.sh prj=project cmd [arg]                                              "
    echo "                                                                                   "
    echo "Example:                                                                           "
    echo "     ./build.sh                            (default: prj=op-mdl all)               "
    echo "     ./build.sh prj=op-mdl                 (clean build)                           "
    echo "     ./build.sh prj=op-mdl ap              (dont clean)                            "
    echo "     ./build.sh prj=op-mdl boot            (dont clean)                            "
    echo "     ./build.sh prj=op-mdl kernel          (clean)                                 "
    echo "     ./build.sh prj=op-mdl dm              (dont clean)                            "
    echo "     ./build.sh prj=ex-app app             (dont clean, link mode is elf)          "
    echo "     ./build.sh prj=ex-app link=dlsym app  (dont clean, link mode is dlsym)        "
    echo "     ./build.sh prj=op-mdl sv=外部版本号   bsv=内部编译版本号     (clean build)       "
    echo "     ./build.sh nvgen                                                              "
    echo "     ./build.sh clean                                                              "
    echo "     ./build.sh help                                                               "
    echo "     ./build.sh version                                                            "
    echo "                                                                                   "
    echo "Current project:                                                                   "
    echo "      PRJ -> ${USE_PRJ}                                                            "
    echo "                                                                                   "
    echo "Supported projects:                                                                "
                PRJS=`ls project | xargs`; echo "      ${PRJS}";
    echo "                                                                                   "
    echo "                                                                                   "
}

function start_time() {
    START_TIME=$(date +%H:%M:%S)
}

function end_time() {
    END_TIME=$(date +%H:%M:%S)
}

function elapse_time() {
    ELAPSE_TIME=$(($(date +%s -d "$END_TIME")-$(date +%s -d "$START_TIME")))
}

function success_result() {
    echo "###################################################################"
    echo "########  CONGRATULATION: Building images successed!       ########"
    echo "###################################################################"
}

function fail_result() {
    echo "###################################################################"
    echo "########### ERROR:   Building images failed!   ####################"
    echo "###################################################################"
}

function judge_run_result() {
   if [ $? -ne 0 ]; then
      fail_result
      exit 1
   else
      success_result
   fi
}

function judge_run_pipe_result() {
   if [ ${PIPESTATUS[0]} -ne 0 ]; then
      fail_result
      exit 1
   else
      success_result
   fi
}

function clean_build() {
    echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
    echo "======>>>>>>>>>> Clean build   ..............................."
    echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"

    echo "======>>>>>>>>>> clean ict   ..............................."
    if [ -e "${ROOTDIR}/.xmake" ]; then
        rm -fr "${ROOTDIR}/bin/"*
        rm -fr "${ROOTDIR}/bin/.build"
        rm -fr "${ROOTDIR}/.xmake"
        rm -fr "${ROOTDIR}/"*.log
        rm -fr "${ROOTDIR}/tools/nvgen/build"
        rm -fr "${ROOTDIR}/tools/nvgen/nvgen"
    fi
    
     echo "======>>>>>>>>>> clean boot   ..............................."
    if [ -e "${BOOT_DIR}/mbl.lua" ]; then
        cd "${BOOT_DIR}"
        chmod a+x build.sh

        ./build.sh clean-all
        cd "${ROOTDIR}"
    fi    
}

function pre_build() {    
    start_time
    make_version
}

function copy_ict_version() {
    echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
    echo "======>>>>>>>>>> Copy Bins   ..............................."
    echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"

    if [ ! -e "${ROOTDIR}/bin/version" ]; then
        mkdir -p "${ROOTDIR}/bin/version"
    fi

    if [ -e "${ROOTDIR}/bin/SLOG_IDs.inf" ] ; then
        if [ -e "${ROOTDIR}/library/${SDK_VER}/SLOG_IDs.inf" ] ; then
             echo "======>>>>>>>>>> Merge SLOG_IDs.inf ..............................."
             if [ "${OS}" = "Windows_NT" ] ; then
                ./tools/ict-gcc/slog-merge.exe -i "${ROOTDIR}/bin/SLOG_IDs.inf" -j "${ROOTDIR}/library/${SDK_VER}/SLOG_IDs.inf"  -o "${ROOTDIR}/bin/SLOG_IDs.inf"
                ./tools/ict-gcc/slog-checker.exe "${ROOTDIR}/bin/SLOG_IDs.inf"
             else
                ./tools/ict-gcc/slog-merge -i "${ROOTDIR}/bin/SLOG_IDs.inf" -j "${ROOTDIR}/library/${SDK_VER}/SLOG_IDs.inf"  -o "${ROOTDIR}/bin/SLOG_IDs.inf"
                ./tools/ict-gcc/slog-checker "${ROOTDIR}/bin/SLOG_IDs.inf"
             fi
        fi
        cp -fr "${ROOTDIR}/bin/SLOG_IDs.inf"  "${ROOTDIR}/bin/version/"
    fi

    # 暂时只支持Linux下CustomCodeParser
    if [ -e "${ROOTDIR}/tools/CustomCodeParser/CustomCodeParser" ]; then
        echo "======>>>>>>>>>> CustomCodeParser Generate new type and rule file ..............................."
        if [ "${OS}" = "Windows_NT" ] ; then
            ./tools/CustomCodeParser/CustomCodeParser.exe -type ${ROOTDIR}/library/${SDK_VER}/type.xml -rule  ${ROOTDIR}/library/${SDK_VER}/rule.xml -root ${ROOTDIR}
        else
            ./tools/CustomCodeParser/CustomCodeParser -type ${ROOTDIR}/library/${SDK_VER}/type.xml -rule  ${ROOTDIR}/library/${SDK_VER}/rule.xml -root ${ROOTDIR}
        fi
        mv "${ROOTDIR}/rule.xml"  "${ROOTDIR}/bin/version/"    
        cp -fr "${ROOTDIR}/library/${SDK_VER}/type.xml"  "${ROOTDIR}/bin/version/"        
    else
        cp -fr "${ROOTDIR}/library/${SDK_VER}/type.xml"  "${ROOTDIR}/bin/version/"
        cp -fr "${ROOTDIR}/library/${SDK_VER}/rule.xml"  "${ROOTDIR}/bin/version/"    
    fi
    
    if [ -e "${ROOTDIR}/bin/${USE_PRJ}/version" ]; then
        cp -fr "${ROOTDIR}/bin/${USE_PRJ}/version/"*  "${ROOTDIR}/bin/version/"
        cp -fr "${ROOTDIR}/library/${SDK_VER}/nvToolCfg.json"  "${ROOTDIR}/bin/version/"
    fi

    if [ -e "${ROOTDIR}/bin/version" ]; then
        ls -lshR "${ROOTDIR}/bin/version"
    fi
}

function store_kernel_version() {
    echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
    echo "======>>>>>>>>>> Store Kernel Bin    ..............................."
    echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"

    if [ ! -e "${ROOTDIR}/project/${USE_PRJ}/version" ]; then
        mkdir -p "${ROOTDIR}/project/${USE_PRJ}/version"
    fi

    if [ -e "${ROOTDIR}/project/${USE_PRJ}/version" ]; then
        cp -fr "${ROOTDIR}/bin/version/"*  "${ROOTDIR}/project/${USE_PRJ}/version/"
    fi
}

function copy_kernel_version() {
    echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
    echo "======>>>>>>>>>> Copy Kernel Bin    ..............................."
    echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"

    if [ ! -e "${ROOTDIR}/bin/version" ]; then
        mkdir -p "${ROOTDIR}/bin/version"
    fi

    if [ -e "${ROOTDIR}/project/${USE_PRJ}/version" ]; then
        cp -fr "${ROOTDIR}/project/${USE_PRJ}/version/"*  "${ROOTDIR}/bin/version/"
    fi
}

function post_build() {
    end_time
    elapse_time
    echo "                                                         "
    echo "===>Building StartTime  is : $START_TIME."
    echo "===>Building EndTime    is : $END_TIME."
    echo "===>Building ElapseTime is : $ELAPSE_TIME (Unit: Second)."
    echo "                                                         "
}

function build_ict_boot() {

    if [ ! -e "${ROOTDIR}/bin/version" ]; then
        mkdir -p "${ROOTDIR}/bin/version"
    fi
        
    if [ -e "${BOOT_DIR}/build.sh" ]; then
        echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
        echo "======>>>>>>>>>> making boot ......................................"
        echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
        cd "${BOOT_DIR}"
        chmod a+x build.sh

        ./build.sh prj=${USE_PRJ} mbl; judge_run_result
   
        ./build.sh prj=${USE_PRJ} xboot; judge_run_result

        cp "${BOOT_DIR}/out/bin/mbl.bin" "${ROOTDIR}/bin/version/"
        cp "${BOOT_DIR}/out/bin/mbl.img" "${ROOTDIR}/bin/version/"
        cp "${BOOT_DIR}/out/bin/mbl.elf" "${ROOTDIR}/bin/version/"
        cp "${BOOT_DIR}/out/bin/xboot.bin" "${ROOTDIR}/bin/version/"
        cp "${BOOT_DIR}/out/bin/xboot.img" "${ROOTDIR}/bin/version/"
        cp "${BOOT_DIR}/out/bin/xboot.elf" "${ROOTDIR}/bin/version/"

        cd "${ROOTDIR}"
    else
        cp "${ROOTDIR}/library/${SDK_VER}/boot/mbl.bin" "${ROOTDIR}/bin/version/"
        cp "${ROOTDIR}/library/${SDK_VER}/boot/mbl.img" "${ROOTDIR}/bin/version/"
        cp "${ROOTDIR}/library/${SDK_VER}/boot/mbl.elf" "${ROOTDIR}/bin/version/"
        cp "${ROOTDIR}/library/${SDK_VER}/boot/xboot.bin" "${ROOTDIR}/bin/version/"
        cp "${ROOTDIR}/library/${SDK_VER}/boot/xboot.img" "${ROOTDIR}/bin/version/"
        cp "${ROOTDIR}/library/${SDK_VER}/boot/xboot.elf" "${ROOTDIR}/bin/version/"        
    fi
}

function build_ict_nv() {
    if gcc --version &>/dev/null; then
        if [ -e "${ROOTDIR}/tools/nvgen/build.sh" ]; then
            echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
            echo "======>>>>>>>>>> making nv ........................................"
            echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
            cd "${ROOTDIR}/tools/nvgen/"
            chmod +x build.sh
            if [ -e "${ROOTDIR}/bin/${USE_PRJ}/version/nvd_roPhyuser_0x00000000.bin" ]; then
                ./build.sh "${ROOTDIR}/bin/${USE_PRJ}/version/nv/nv_submod" "${ROOTDIR}/bin/${USE_PRJ}/version/nvd_roPhyuser_0x00000000.bin" 0x00000000
            else
                ./build.sh "${ROOTDIR}/bin/${USE_PRJ}/version/nv/nv_submod"
            fi
            judge_run_result
            mv "${ROOTDIR}/bin/${USE_PRJ}/version/nv/nv_submod/nvd_full_0x00000000.bin" "${ROOTDIR}/bin/version/"
            cd "${ROOTDIR}"
        fi
    else
        echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
        echo "======>>>>>>>>>> Not found gcc , use default NV ..................."
        echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
        cp "${ROOTDIR}/library/${SDK_VER}/nv/nv_submod/nvd_Modem_0x00000000.bin" "${ROOTDIR}/bin/version/nvd_full_0x00000000.bin"
    fi
}

function build_ict_nvupdater() {

    if [ -e "${ROOTDIR}/tools/NvUpdater/build.sh" ]; then
        echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
        echo "======>>>>>>>>>> tool: Nv Updater   ..............................."
        echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
        cd "${ROOTDIR}"/tools/NvUpdater
        chmod a+x build.sh

        if [ -e "${ROOTDIR}/project/${USE_PRJ}/nv/nvCustCfg.json" ]; then
            cp -fr "${ROOTDIR}/project/${USE_PRJ}/nv/nvCustCfg.json" "${ROOTDIR}/bin/version/"
            if [ ! -e "${ROOTDIR}/bin/version/type.xml" ]; then
                echo "Error: not found type.xml file, cannot run NvUpdater!!"
                exit 1
            fi
        fi

        if [ -e "${ROOTDIR}/bin/version/nvd_full_0x00000000.bin" ] &&
           [ -e "${ROOTDIR}/bin/version/type.xml" ] &&
           [ -e "${ROOTDIR}/bin/version/nvCustCfg.json" ]; then
           ./build.sh "${ROOTDIR}/bin/version/type.xml" "${ROOTDIR}/bin/version/nvCustCfg.json" "${ROOTDIR}/bin/version/nvd_full_0x00000000.bin"
        fi

        cd "${ROOTDIR}"
   fi
}

function build_ict_dlmodule() {
   if [ -e "${ROOTDIR}/dlmodule.lua" ]; then
       echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
       echo "======>>>>>>>  making dlmodule     ..............................."
       echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
       cd ${ROOTDIR}; xmake -F dlmodule.lua -r -w | tee build_dlmodule.log; judge_run_pipe_result; cd ${ROOTDIR};
   fi
}

function build_ict_app() {
   if [ -e "${ROOTDIR}/build_app.lua" ]; then
       echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
       echo "======>>>>>>>  making app, link mode is ${APP_LINK_MODE}  ........."
       echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
       cd ${ROOTDIR}; xmake -F build_app.lua -r -w | tee build_app.log; judge_run_pipe_result; cd ${ROOTDIR};
   fi
   
   if [ -e "${ROOTDIR}/bin/${USE_PRJ}/version" ]; then
        cp -fr "${ROOTDIR}/bin/${USE_PRJ}/version/"*  "${ROOTDIR}/bin/version/"
   fi
}
function build_ict_allinone() {
   if [ -e "${ROOTDIR}/tools/bin2one.py" ]; then
           echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
           echo "======>>>>>>>  making allinone    ..............................."
           echo "======>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
       python tools/bin2one.py "${ROOTDIR}"
       judge_run_result

       cd "${ROOTDIR}"
   fi
}


# ---------------------------------------------------
# MAIN:  need to input right arguments
# ---------------------------------------------------
while true; do
  if [ ! -e "${ROOTDIR}/project/${USE_PRJ}" ]; then
       printf '===> ERROR: Hmmm, input project is not supported !!: "%s".\n' "${USE_PRJ}";
       usage;
       exit 1
  fi

  case ${BUILD_CMD} in
    help )
        usage
        exit 0
        ;;
    version )
        version
        exit 0
        ;;
    '' | all )
        pre_build
        clean_build
        build_ict_boot
        xmake -r -w $BUILD_ARG | tee build_ap.log; judge_run_pipe_result;
        xmake project -k compile_commands; judge_run_result;
        copy_ict_version
        build_ict_nv
        build_ict_nvupdater
        build_ict_allinone
        post_build
        exit 0
        ;;
	mini )
        pre_build
		clean_build
		build_ict_boot
		xmake f --mini=y; xmake -r -w $BUILD_ARG | tee build_ap.log; judge_run_pipe_result;
        xmake project -k compile_commands; judge_run_result;
		build_ict_dlmodule
		copy_ict_version
		build_ict_nv
		build_ict_allinone
        post_build
        exit 0
        ;;
    ap )
        pre_build
        xmake $BUILD_ARG -w | tee build_ap.log; judge_run_pipe_result;
        xmake project -k compile_commands; judge_run_result;
        copy_ict_version
        build_ict_nv
        build_ict_nvupdater
        build_ict_allinone
        post_build
        exit 0
        ;;
    boot )
        pre_build
        build_ict_boot
        copy_ict_version
        build_ict_nv
        build_ict_nvupdater
        build_ict_allinone
        post_build
        exit 0
        ;;
    kernel )
        pre_build
        clean_build
        build_ict_boot
        xmake $BUILD_ARG -w | tee build_ap.log; judge_run_pipe_result;
        copy_ict_version
        build_ict_nv
        build_ict_nvupdater
        build_ict_allinone
        store_kernel_version
        post_build
        exit 0
        ;;
    dm )
        pre_build
        copy_kernel_version
        build_ict_dlmodule
        #build_ict_nv
        #build_ict_nvupdater
        #build_ict_allinone
        post_build
        exit 0
        ;;
    app )
        pre_build
        copy_kernel_version
        build_ict_app
        #build_ict_nv
        #build_ict_nvupdater
        #build_ict_allinone
        post_build
        exit 0
        ;;
    nvgen )
        pre_build
        copy_ict_version
        build_ict_nv
        build_ict_nvupdater
        build_ict_allinone
        post_build
        exit 0
        ;;
    clean )
        pre_build
        clean_build
        post_build
        exit 0
        ;;
    * )
        printf "===> Hmmm, unknown option: ${BUILD_CMD} ";
        usage;
        exit 1
        break
        ;;
  esac
done

