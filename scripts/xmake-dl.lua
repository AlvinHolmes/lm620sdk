-----------------------------------------------------------------------
-- Xmake script file
-- Copyright (C) 2023, Nanjing Innochip Technology Co.,Ltd.
-- All rights reserved.
--
-- @author      geanfeng
-- @file        xmake.lua
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Global variable
-----------------------------------------------------------------------
GLOBAL_DEPS = {}
GLOBAL_INCLUDE = {}
GLOBAL_DEFINE = {}

-----------------------------------------------------------------------
-- Functions
-----------------------------------------------------------------------
function add_global_includedirs(path)
    if path then
        table.insert(GLOBAL_INCLUDE, os.curdir() .. "/" .. path)
    end
end

function get_global_includedirs()
    return GLOBAL_INCLUDE
end

function add_global_define(define)
    if define then
        table.insert(GLOBAL_DEFINE, define)
    end
end

function get_global_define()
    return GLOBAL_DEFINE
end

function add_global_deps(target)
    if target then
        table.insert(GLOBAL_DEPS, target)
    end
end

function add_srcfile(filepath)
    add_files(filepath, {defines = "__FILE__=" .. "\"" .. path.basename(filepath) .. "\""})
end

function add_srcdir(dirpath)
    for _, filepath in ipairs(os.files(dirpath .. "/*.c")) do
        add_files(filepath, {defines = "__FILE__=" .. "\"" .. path.basename(filepath) .. "\""})
    end
end

function add_srcdirall(dirpath)
    for _, filepath in ipairs(os.files(dirpath .. "/**.c")) do
        add_files(filepath, {defines = "__FILE__=" .. "\"" .. path.basename(filepath) .. "\""})
    end
end

-----------------------------------------------------------------------
-- Global include directory
-----------------------------------------------------------------------
if os.getenv("USE_SDK") ~= "y" then
    --全版本
    add_includedirs(TOP_DIR .. "/components",{public = true})
    add_includedirs(TOP_DIR .. "/components/kernel/include",{public = true})
    add_includedirs(TOP_DIR .. "/components/nvm/include",{public = true})
    add_includedirs(TOP_DIR .. "/components/fs/include",{public = true})
    add_includedirs(TOP_DIR .. "/components/fs/include/sys",{public = true})
    add_includedirs(TOP_DIR .. "/components/service/inc",{public = true})
    add_includedirs(TOP_DIR .. "/modem",{public = true})
    add_includedirs(TOP_DIR .. "/modem/ps",{public = true})
    add_includedirs(TOP_DIR .. "/modem/phy",{public = true})
    add_includedirs(TOP_DIR .. "/modem/pub/inc",{public = true})
    add_includedirs(TOP_DIR .. "/modem/pub/inc/nv",{public = true})

else
    --sdk版本
    add_includedirs(TOP_DIR .. "/components",{public = true})
	add_includedirs(TOP_DIR .. "/components/kernel/include",{public = true})
	add_includedirs(TOP_DIR .. "/components/driver/include",{public = true})
	add_includedirs(TOP_DIR .. "/components/at/inc",{public = true})
	add_includedirs(TOP_DIR .. "/components/lwip",{public = true})
    if has_config("sdkver") then
        add_includedirs("../library/" .. get_config("sdkver") .. "/include",{public = true})
        add_includedirs(os.dirs("../library/" .. get_config("sdkver") .. "/include/*"),{public = true})
        add_includedirs("../library/" .. get_config("sdkver") .. "/include/drv/chip",{public = true})
        add_includedirs("../library/" .. get_config("sdkver") .. "/include/modem/nv",{public = true})
        if os.exists("../library/" .. get_config("sdkver") .. "/include/lwip") then
            add_includedirs("../library/" .. get_config("sdkver") .. "/include/lwip/port",{public = true})
            add_includedirs("../library/" .. get_config("sdkver") .. "/include/lwip/port/arch/include",{public = true})
        end
    end
end

if G_USER_PROJECT_NAME then
    add_includedirs(TOP_DIR .. "/project/" .. G_USER_PROJECT_NAME .. "/nv",{public = true})
end

-----------------------------------------------------------------------
-- Include subdirs
-----------------------------------------------------------------------
includes(TOP_DIR .. "/scripts/toolchain/xmake.lua")

includes(TOP_DIR .. "/vendor/dlmodule/xmake.lua")

if G_USER_PROJECT_NAME then
    includes(TOP_DIR .. "/project/" .. G_USER_PROJECT_NAME .. "/cfg/base_option.lua")
    includes(TOP_DIR .. "/project/" .. G_USER_PROJECT_NAME .. "/cfg/cust_option.lua")
end

-----------------------------------------------------------------------
-- Global definition
-----------------------------------------------------------------------
includes("top_option.lua")

-----------------------------------------------------------------------
-- Base target
-----------------------------------------------------------------------
target("base")
    set_kind("headeronly")
    add_defines(get_global_define(), {public = true})
    add_includedirs(get_global_includedirs(), {public = true})

    local top_dir = TOP_DIR

    on_config(function (target)
        if os.getenv("USE_SDK") == "y" then
            if os.exists(target:installdir() .. "/defines.txt") then
                local defines = io.load(target:installdir() .. "/defines.txt")
                target:add("defines", defines, {public = true})
            else
                print("installdir=$(installdir)".. target:installdir())
                os.raise("Error: sdk version need defines.txt")
            end
        end
    end)
target_end()
