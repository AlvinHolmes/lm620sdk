-----------------------------------------------------------------------
-- Xmake script file
-- Copyright (C) 2023, Nanjing Innochip Technology Co.,Ltd.
-- All rights reserved.
--
-- @author      geanfeng
-- @file        xmake.lua
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Version
-----------------------------------------------------------------------
set_project("ICT2110")
set_version("0.0.2", {build = "%Y%m%d%H%M"})
set_xmakever("2.7.8")
add_rules("mode.release", "mode.debug")
set_defaultmode("debug")

-----------------------------------------------------------------------
-- Project name
-----------------------------------------------------------------------
local USER_PROJECT_NAME = "mdl"
if os.getenv("PROJECT_NAME") then
        USER_PROJECT_NAME = os.getenv("PROJECT_NAME")
end
G_USER_PROJECT_NAME = USER_PROJECT_NAME

-----------------------------------------------------------------------
-- Path definition
-----------------------------------------------------------------------
TOP_DIR  = os.curdir()

-----------------------------------------------------------------------
-- XIP address
-----------------------------------------------------------------------
local XIP_BASE = 0xC8000000
local XKNL_BEGIN = 0

-----------------------------------------------------------------------
-- XIP section header struct size, used for calculatting
-- the actual running offset of XIP
-----------------------------------------------------------------------
local XIP_HEAD_LEN = 0x20

-----------------------------------------------------------------------
-- XIP joint align
-----------------------------------------------------------------------
local XIP_JOINT_ALIGN = 0x400

-----------------------------------------------------------------------
-- XIP or normal type
-----------------------------------------------------------------------
local XIP_IMG_EXIST = '1'
if os.getenv("XIP_IMG_SWITCH") == "normal" then
        XIP_IMG_EXIST = '0'
end

-----------------------------------------------------------------------
-- Configuration
-----------------------------------------------------------------------
set_plat("cross")
set_config("chip","ict2110")
set_config("cpu_arch","ZCrisc-v")
set_config("cpu_type","cpu-ap")
set_config("buildir", "bin/.build/cpu-ap")
set_optimize("smallest")
if os.getenv("PROJECT_NAME") == "zr-volte" then
    set_warnings("all")
else
    set_warnings("all", "error")
end


if os.getenv("CROSS_TOOLCHAIN") == "nuclei" then
    --print("===> NOTICE: Cross toolchain is nuclei @AP!!")
    set_config("cpu_arch","risc-v")
elseif os.getenv("CROSS_TOOLCHAIN") == "ZCnuclei" then
    set_config("cpu_arch","ZCrisc-v")
    --print("===> NOTICE: Cross toolchain is ZCnuclei @AP!!")
end

-----------------------------------------------------------------------
-- Compile options
-----------------------------------------------------------------------
if os.exists("project/"..USER_PROJECT_NAME.."/cfg/base_option.lua") then
  includes("project/"..USER_PROJECT_NAME.."/cfg/base_option.lua")
end

if is_mode("debug") then
    add_defines("USE_TOP_DEBUG")
else
    add_defines("USE_TOP_RELEASE")
end

if has_config("sdkver") then
	set_installdir("library/" .. get_config("sdkver"))
end

-----------------------------------------------------------------------
-- Defines
-----------------------------------------------------------------------
add_defines("_CHIP_ICT2110",
            "_CPU_AP")

-----------------------------------------------------------------------
-- Flash2 switch
-- If use flash2, you should comment out MuxProxy_SlaveInit
-- first in app_main.c, and switch on CONFIG_USE_FLASH2
-----------------------------------------------------------------------
add_defines("CONFIG_USE_FLASH2")

-----------------------------------------------------------------------
-- Include subdirs
-----------------------------------------------------------------------
includes(TOP_DIR .. "/scripts")

-----------------------------------------------------------------------
-- Toolchain
-----------------------------------------------------------------------
if get_config("cpu_arch") == "arm" then
        set_arch("arm")
        add_requires("gnu_rm 2021.10")
        set_toolchains("gnu-rm@gnu_rm")
elseif get_config("cpu_arch") == "risc-v" then
        set_arch("risc-v")
        set_toolchains("risc-v_gcc")
elseif get_config("cpu_arch") == "ZCrisc-v" then
        set_arch("risc-v")
        set_toolchains("ZCrisc-v_gcc")
end

-----------------------------------------------------------------------
-- Pre_build, pre-processiong compilation parameters
-----------------------------------------------------------------------
pre_build = function()
		local BIN_DIR = TOP_DIR.."/bin/"..USER_PROJECT_NAME
        before_build(function(target)
				if os.getenv("USE_SDK") == "y" then
					CP_BIN_DIR = target:installdir() .. "/cp"
				else
					CP_BIN_DIR = BIN_DIR .. "/cp"
				end
                local GCC = target:tool("cc")
                local PARTITION_NAME = "xpartition.ini"
                if XIP_IMG_EXIST ~= '1' then
                        PARTITION_NAME = "npartition.ini"
                end

                local PARTITION_PATH = "board/"..get_config("board").."/cfg/"
                local LINKSCRIPT_PATH = "board/"..get_config("board").."/cfg/"

                if os.exists("board/"..get_config("board").."/cfg/"..PARTITION_NAME) then
                    PARTITION_PATH = "board/"..get_config("board").."/cfg/"
                    --print("===>NOTICE: Partition cfg: "..PARTITION_PATH)
                end
                if os.exists("project/"..USER_PROJECT_NAME.."/cfg/"..PARTITION_NAME) then
                    PARTITION_PATH = "project/"..USER_PROJECT_NAME.."/cfg/"
                    print("===>NOTICE: Partition cfg: "..PARTITION_PATH)
                end
                if os.exists("project/"..USER_PROJECT_NAME.."/cfg/cpu-ap.lds") then
                    LINKSCRIPT_PATH = "project/"..USER_PROJECT_NAME.."/cfg/"
                    print("===>NOTICE: Link scripts cfg: "..LINKSCRIPT_PATH)
                end
                if os.exists(PARTITION_PATH..PARTITION_NAME) then
                        local PRE_PART_TBL = io.open(PARTITION_PATH..PARTITION_NAME, "r")
                        for LINE in PRE_PART_TBL:lines() do
                                local PRE_PART_NAME = string.match(LINE, "xknl")
                                local WORDS = {}
                                if PRE_PART_NAME == "xknl" then
                                        for W in string.gmatch(LINE, "%w+") do
                                                WORDS[#WORDS + 1] = W
                                        end
                                        XKNL_BEGIN = tonumber(WORDS[2])
                                end
                        end
                        PRE_PART_TBL:close()
                end
                if not os.exists("$(buildir)/"..USER_PROJECT_NAME) then
                        os.mkdir("$(buildir)/"..USER_PROJECT_NAME)
                end
                if os.exists(CP_BIN_DIR.."/xcp.bin") then
                        local CP_XIP_FILE = io.open(CP_BIN_DIR.."/xcp.bin", "rb")
                        local CONTENT = CP_XIP_FILE:read("*a")
                        local CP_XIP_LENGTH = 0
                        if CONTENT ~= '' then
                                CP_XIP_LENGTH = CP_XIP_FILE:seek("end")
                                if (CP_XIP_LENGTH % 4) ~= 0 then
                                        CP_XIP_LENGTH = CP_XIP_LENGTH + 4 - (CP_XIP_LENGTH % 4)
                                end
                                CP_XIP_LENGTH = CP_XIP_LENGTH + XIP_HEAD_LEN
                        end
                        CP_XIP_FILE:close()

                        if (CP_XIP_LENGTH % XIP_JOINT_ALIGN) ~= 0 then
                                CP_XIP_LENGTH = CP_XIP_LENGTH + XIP_JOINT_ALIGN - (CP_XIP_LENGTH % XIP_JOINT_ALIGN)
                        end

                        if CP_XIP_LENGTH == 0 then
                                CP_XIP_LENGTH = XIP_HEAD_LEN
                        end
                        local AP_XIP_OFFSET = CP_XIP_LENGTH + XIP_BASE + XKNL_BEGIN
                        os.cp(LINKSCRIPT_PATH.."/cpu-ap.lds", LINKSCRIPT_PATH.."/cpu-ap.h")
                        os.exec(GCC.." -E -DAP_XIP_START="..string.format("0x%X", AP_XIP_OFFSET).." -DAP_XIP_ON="..XIP_IMG_EXIST.." -P "..LINKSCRIPT_PATH.."/cpu-ap.h -o $(buildir)/"..USER_PROJECT_NAME.."/cpu-ap.ld\n")
                        os.rm(LINKSCRIPT_PATH.."/cpu-ap.h")
                else
						print("Make AP Not found CP.BIN .................................................")
                        local AP_XIP_OFFSET = XIP_BASE + XIP_HEAD_LEN + XKNL_BEGIN
                        os.cp(LINKSCRIPT_PATH.."/cpu-ap.lds", LINKSCRIPT_PATH.."/cpu-ap.h")
                        os.exec(GCC.." -E -DAP_XIP_START="..string.format("0x%X", AP_XIP_OFFSET).." -DAP_XIP_ON="..XIP_IMG_EXIST.." -P "..LINKSCRIPT_PATH.."/cpu-ap.h -o $(buildir)/"..USER_PROJECT_NAME.."/cpu-ap.ld\n")
                        os.rm(LINKSCRIPT_PATH.."/cpu-ap.h")
                end			
        end)
end

-----------------------------------------------------------------------
-- Build target
-----------------------------------------------------------------------
--target(USER_PROJECT_NAME..".elf")
target("cpu-ap.elf")
        set_kind("binary")
        add_deps(GLOBAL_DEPS)
		-- use sdk
        if os.getenv("USE_SDK") == "y" and has_config("sdkver") then
			add_linkdirs("library/" .. get_config("sdkver") .. "/lib")
			for _, filepath in ipairs(os.files("library/" .. get_config("sdkver") .. "/lib/*.a")) do
				add_links(string.sub(path.basename(filepath),4))
			end
        end

        set_targetdir("$(buildir)/"..USER_PROJECT_NAME)
        -- ld flags
        add_ldflags("-lm -Wl,--end-group -Wl,--no-whole-archive", {force = true})

        local MKROOT = TOP_DIR.."/tools/mkimg"
        local MKMERGE = MKROOT.."/merge"
        local MKIMAGE = MKROOT.."/mkimage"
        local BIN_DIR = TOP_DIR.."/bin/"..USER_PROJECT_NAME.."/ap"
        local VERSION_DIR = TOP_DIR.."/bin/"..USER_PROJECT_NAME.."/version"
        local MKJOINT = MKROOT.."/joint"
        local PARTITION_NAME = "xpartition.ini"
        if XIP_IMG_EXIST ~= '1' then
                PARTITION_NAME = "npartition.ini"
        end

        if is_os("windows") then
                MKMERGE = MKROOT.."/merge.exe"
                MKIMAGE = MKROOT.."/mkimage.exe"
                MKJOINT = MKROOT.."/joint.exe"
        end

        if get_config("board") then
                pre_build()
                add_ldflags("-Xlinker --gc-sections -Wl,-Map,$(buildir)/"..USER_PROJECT_NAME.."/cpu-ap.map --specs=nano.specs --specs=nosys.specs -T $(buildir)/"..USER_PROJECT_NAME.."/cpu-ap.ld", {force = true})
        end

        local PARTITION_PATH = "board/"
        if get_config("board") then
            if os.exists("board/"..get_config("board").."/cfg/"..PARTITION_NAME) then
                PARTITION_PATH = "board/"..get_config("board").."/cfg/"
                --print("===>[AP]Current cfg: "..PARTITION_PATH)
            end
            if os.getenv("PROJECT_NAME") ~= nil then
                if os.exists("project/"..os.getenv("PROJECT_NAME").."/cfg/"..PARTITION_NAME) then
                    PARTITION_PATH = "project/"..os.getenv("PROJECT_NAME").."/cfg/"
                    print("===>[AP]Current cfg: "..PARTITION_PATH)
                end
            end
        end

        after_build(function(target)
				if os.getenv("USE_SDK") == "y" then
					CP_BIN_DIR = target:installdir() .. "/cp"
				else
					CP_BIN_DIR = BIN_DIR .. "/../cp"
				end
                if not os.exists(BIN_DIR) then
                        os.mkdir(BIN_DIR)
                end
                if not os.exists(VERSION_DIR) then
                        os.mkdir(VERSION_DIR)
                end
				if os.getenv("USE_SDK") == "y" then
					sdk_libs = os.files("library/lib/*.a")
					new_libs = os.files("$(buildir)/".."cross/risc-v/debug/*.a")
					lib_err = false
					for k1,v1 in ipairs(new_libs) do
						for k2,v2 in ipairs(sdk_libs) do
							if path.basename(v1) == path.basename(v2) then
								print("Link: new library [" .. path.basename(v1) .. "] is already in SDK libraries")
								lib_err = true
							end
						end
					end
					if lib_err then
						raise("LINK ERROR: there are duplicate libraries in the link.")
					end
				end
                local XIPIMG_EXIST = 1
                local ZKNL_SIZE = ""
                local XKNL_SIZE = ""
                if os.exists(PARTITION_PATH..PARTITION_NAME) then
                        local PART_TBL = io.open(PARTITION_PATH..PARTITION_NAME, "r")
                        for LINE in PART_TBL:lines() do
                                local PART_NAME = string.match(LINE, "%w+")
                                local WORDS = {}
                                if PART_NAME == "zknl" then
                                        for W in string.gmatch(LINE, "%w+") do
                                                WORDS[#WORDS + 1] = W
                                        end
                                        ZKNL_SIZE = WORDS[3]
                                elseif PART_NAME == "xknl" then
                                        for W in string.gmatch(LINE, "%w+") do
                                                WORDS[#WORDS + 1] = W
                                        end
                                        XKNL_SIZE = WORDS[3]
                                end
                        end
                        PART_TBL:close()
                end

                local OBJCPY = target:tool("objcopy")
                if os.exists(PARTITION_PATH..PARTITION_NAME) then
                    os.exec("python tools/partition.py "..PARTITION_PATH..PARTITION_NAME.." "..BIN_DIR.."/partition.cfg")
                    os.cp(PARTITION_PATH..PARTITION_NAME, VERSION_DIR.."/partition.ini")
                end
                if has_config("dlmo") then
                    if XIP_IMG_EXIST == '1' then
                        os.exec(OBJCPY.." -j .xip_dlmo -I elf32-littleriscv "..target:targetfile().." -O binary "..BIN_DIR.."/module.symvers\n")
                    else
                        os.exec(OBJCPY.." -j .dlmo -I elf32-littleriscv "..target:targetfile().." -O binary "..BIN_DIR.."/module.symvers\n")
                    end
                    os.cp(BIN_DIR.."/module.symvers", VERSION_DIR)
                end
                os.exec(OBJCPY.." -j .itcm -I elf32-littleriscv "..target:targetfile().." -O binary "..BIN_DIR.."/ap_ilm.bin\n")
                os.exec(OBJCPY.." -j .dtcm -I elf32-littleriscv "..target:targetfile().." -O binary "..BIN_DIR.."/ap_dlm.bin\n")
                os.exec(OBJCPY.." -j .iram0_psm_re -I elf32-littleriscv "..target:targetfile().." -O binary "..BIN_DIR.."/ap_psm.bin\n")
                os.exec(OBJCPY.." -j .iram2 -I elf32-littleriscv "..target:targetfile().." -O binary "..BIN_DIR.."/ap_iram2.bin\n")
                os.exec(OBJCPY.." -j .psram -j .text -j .rodata -j .dlmo -j .data -I elf32-littleriscv "..target:targetfile().." -O binary "..BIN_DIR.."/ap_ddr.bin\n")
                os.exec(MKMERGE.." --addr=0x800FE800 --bin="..BIN_DIR.."/ap_ddr.bin --addr=0x00008000 --bin="..BIN_DIR.."/ap_ilm.bin --addr=0x00010000 --bin="..BIN_DIR.."/ap_dlm.bin --addr=0xC0200800 --bin="..BIN_DIR.."/ap_psm.bin --addr=0xC026C800 --bin="..BIN_DIR.."/ap_iram2.bin --output="..BIN_DIR.."/zap.bin \n")
                os.rm(BIN_DIR.."/ap_ilm.bin")
                os.rm(BIN_DIR.."/ap_dlm.bin")
                os.rm(BIN_DIR.."/ap_psm.bin")
                os.rm(BIN_DIR.."/ap_iram2.bin")
                os.rm(BIN_DIR.."/ap_ddr.bin")
                if XIP_IMG_EXIST == '1' then
                        os.exec(OBJCPY.." -j .xip_text -j .xip_rodata -j .xip_dlmo -I elf32-littleriscv "..target:targetfile().." -O binary "..BIN_DIR.."/xap.bin\n")
                else
                        XIPIMG_EXIST = 0
                end
                if XIP_IMG_EXIST == '1' then
                        if not os.exists(CP_BIN_DIR.."/xcp.bin") then
                                assert(false, "If making xip image, you should first building cp with IMG=xip.")
                        end
                else
                        if os.exists(CP_BIN_DIR.."/xcp.bin") then
                                assert(false, "If making normal image, you should first building cp with IMG=normal.")
                        end
                end
                if os.exists(CP_BIN_DIR.."/zcp.bin") then
                        os.exec(MKIMAGE.." --size="..ZKNL_SIZE.." --cpuCp="..CP_BIN_DIR.."/zcp.bin --cpuCpLoadAddr=0x80000000 --cpuCpEntry=0xF5000000 --cpuCpMultiSections --cpuAp="..BIN_DIR.."/zap.bin --cpuApLoadAddr=0x800FE800 --cpuApEntry=0x00008000 --cpuApMultiSections --packSize=0x40 --output="..BIN_DIR.."/zknl.img \n")
                        if XIP_IMG_EXIST == '1' then
                                if os.exists(CP_BIN_DIR.."/xcp.bin") then
                                        local CP_XIP_FILE = io.open(CP_BIN_DIR.."/xcp.bin", "rb")
                                        local CONTENT = CP_XIP_FILE:read("*a")
                                        if CONTENT ~= '' then
                                                os.exec(MKJOINT.." --size="..XKNL_SIZE.." --bin1="..CP_BIN_DIR.."/xcp.bin --bin2="..BIN_DIR.."/xap.bin --align="..string.format("0x%X", XIP_JOINT_ALIGN).." --output="..BIN_DIR.."/xknl.img\n")
                                        else
                                                os.exec(MKJOINT.." --size="..XKNL_SIZE.." --bin1="..BIN_DIR.."/xap.bin --output="..BIN_DIR.."/xknl.img\n")
                                        end
                                        CP_XIP_FILE:close()
                                elseif os.exists(BIN_DIR.."/xap.bin") then
                                        os.exec(MKJOINT.." --size="..XKNL_SIZE.." --bin1="..BIN_DIR.."/xap.bin --output="..BIN_DIR.."/xknl.img\n")
                                end
                        end
                        if get_config("board") then
                          if os.exists("board/"..get_config("board").."/nv/nvd_roPhyuser_0x00000000.bin") then
                            os.cp("board/"..get_config("board").."/nv/nvd_roPhyuser_0x00000000.bin", VERSION_DIR)
                          end
                        end
						if os.exists("project/"..USER_PROJECT_NAME.."/nv/nvd_roPhyuser_0x00000000.bin") then
							os.cp("project/"..USER_PROJECT_NAME.."/nv/nvd_roPhyuser_0x00000000.bin", VERSION_DIR)
						end
                        os.cp("$(buildir)/"..USER_PROJECT_NAME.."/*.elf", BIN_DIR)
                        os.cp("$(buildir)/"..USER_PROJECT_NAME.."/*.map", BIN_DIR)
                        os.cp(BIN_DIR.."/*.elf", VERSION_DIR)
                        os.cp(BIN_DIR.."/*.img", VERSION_DIR)
                        os.cp(BIN_DIR.."/*.cfg", VERSION_DIR)

                        os.exec("python scripts/analysis_map.py $(buildir)/"..USER_PROJECT_NAME.."/cpu-ap.map $(buildir)/cross/risc-v/debug")
                        if os.exists("bin/SLOG_IDs.inf") then
                            os.exec("tools/ict-gcc/slog-checker bin/SLOG_IDs.inf")
                        end
                else
                        assert(false, "Make zknl and xknl depend on cp, please build cp first.")
                end
        end)

        local save_defines = get_global_define()
        after_install(function (target)
                print("save sdk defines")
                io.save(target:installdir() .. "/defines.txt", save_defines)
        os.rm(target:installdir() .. "/bin")
    end)
target_end()
