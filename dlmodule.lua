-----------------------------------------------------------------------
-- Xmake script file
-- Copyright (C) 2023, Nanjing Innochip Technology Co.,Ltd.
-- All rights reserved.
--
-- @author      geanfeng
-- @file        dlmoudle.lua
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
-- Configuration
-----------------------------------------------------------------------
set_plat("cross")
set_config("chip","ict2110")
set_config("cpu_arch","risc-v-dl")
set_config("cpu_type","cpu-ap")
set_config("buildir", "bin/.build/dlmoudle")
set_optimize("smallest")
set_warnings("all")
--set_warnings("all", "error")

--if os.getenv("CROSS_TOOLCHAIN") == "nuclei" then
--    set_config("cpu_arch","risc-v")
--elseif os.getenv("CROSS_TOOLCHAIN") == "ZCnuclei" then
--    set_config("cpu_arch","ZCrisc-v")
--end

-----------------------------------------------------------------------
-- Compile options
-----------------------------------------------------------------------
if os.exists("project/"..USER_PROJECT_NAME.."/cfg/base_option.lua") then
  includes("project/"..USER_PROJECT_NAME.."/cfg/base_option.lua")
end

if has_config("dlmo") then

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
-- Include subdirs
-----------------------------------------------------------------------
includes(TOP_DIR .. "/scripts/xmake-dl.lua")

-----------------------------------------------------------------------
-- Toolchain
-----------------------------------------------------------------------
if get_config("cpu_arch") == "risc-v" then
    set_arch("risc-v")
    set_toolchains("risc-v_gcc")
elseif get_config("cpu_arch") == "ZCrisc-v" then
    set_arch("risc-v")
    set_toolchains("ZCrisc-v_gcc")
elseif get_config("cpu_arch") == "risc-v-dl" then
    set_arch("risc-v")
    set_toolchains("risc-v-dl_gcc")
end

-----------------------------------------------------------------------
-- Pre_build, pre-processiong compilation parameters
-----------------------------------------------------------------------
pre_build = function()
    before_build(function(target)
        if not os.exists("$(buildir)/"..USER_PROJECT_NAME) then
            os.mkdir("$(buildir)/"..USER_PROJECT_NAME)
        end
    end)
end

-----------------------------------------------------------------------
-- Build target
-----------------------------------------------------------------------
--target(USER_PROJECT_NAME .. ".elf")
target("dlmodule.elf")
    set_kind("binary")
    add_deps(GLOBAL_DEPS)
    set_targetdir("$(buildir)/"..USER_PROJECT_NAME)
    -- ld flags
    add_ldflags("-Wl,--end-group -Wl,--no-whole-archive", {force = true})

    local MKROOT = TOP_DIR.."/tools/mkimg"
    local MKDLMO = MKROOT.."/mkdm"
    local BIN_DIR = TOP_DIR.."/bin/"..USER_PROJECT_NAME.."/dlmodule"
    local VERSION_DIR = TOP_DIR.."/bin/"..USER_PROJECT_NAME.."/version"

    if is_os("windows") then
        MKDLMO = MKROOT.."/mkdm.exe"
    end


    pre_build()
    add_ldflags("-e main -Wl,-Map,$(buildir)/"..USER_PROJECT_NAME.."/dlmodule.map", {force = true})

    after_build(function(target)
        if has_config("dlmo") then
            if not os.exists(BIN_DIR) then
                os.mkdir(BIN_DIR)
            end
            if not os.exists(VERSION_DIR) then
                os.mkdir(VERSION_DIR)
            end
            local STRIP = target:tool("strip")
            os.exec(STRIP.." -R .hash "..target:targetfile().." -o "..BIN_DIR.."/dlmodule.mo\n")
            os.exec(MKDLMO.." -s "..VERSION_DIR.."/module.symvers -i "..BIN_DIR.."/dlmodule.mo -o "..VERSION_DIR.."/dlmo.img\n")

            os.cp("$(buildir)/"..USER_PROJECT_NAME.."/*.elf", BIN_DIR)
            os.cp("$(buildir)/"..USER_PROJECT_NAME.."/*.map", BIN_DIR)

            os.cp(BIN_DIR.."/*.elf", VERSION_DIR)
            os.cp(BIN_DIR.."/*.mo", VERSION_DIR)

        else
            os.rm("$(buildir)")
        end
    end)
target_end()

end
