-----------------------------------------------------------------------
-- Xmake script file
-- Copyright (C) 2023, Nanjing Innochip Technology Co.,Ltd.
-- All rights reserved.
--
-- @author      geanfeng
-- @file        xmake.lua
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Compile options
-----------------------------------------------------------------------
option("at tcpip")
        set_default(false)
        set_showmenu(true)
        set_category("at command")
        set_description("Enable TCPIP at command", "  =y|n")
option_end()

option("at mqtt")
        set_default(false)
        set_showmenu(true)
        set_category("at command")
        set_description("Enable MQTT at command", "  =y|n")
option_end()

option("at http")
        set_default(false)
        set_showmenu(true)
        set_category("at command")
        set_description("Enable HTTP at command", "  =y|n")
option_end()

option("at ssl")
        set_default(false)
        set_showmenu(true)
        set_category("at command")
        set_description("Enable SSL at command", "  =y|n")
option_end()

option("at vfs")
        set_default(false)
        set_showmenu(true)
        set_category("at command")
        set_description("Enable VFS at command", "  =y|n")
option_end()

option("at fota")
        set_default(false)
        set_showmenu(true)
        set_category("at command")
        set_description("Enable FOTA at command", "  =y|n")
option_end()

option("app bip")
        set_default(false)
        set_showmenu(true)
        set_category("app")
        set_description("Enable app bip", "  =y|n")
option_end()

-----------------------------------------------------------------------
-- Configuration
-----------------------------------------------------------------------
local TARGET_NAME = "open-app"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
---add_global_define("ICT_DRV")
    if os.getenv("USE_SDK") ~= "y" then -- SDK版本，不重新定义宏
        --  命令集定义
        if has_config("AT_CMDSET_M") then
            add_global_define("AT_CMDSET_M") -- 中移模组AT命令集
        elseif has_config("AT_CMDSET_Q") then
            add_global_define("AT_CMDSET_Q") -- 移远AT命令集
            if has_config("URC_MAIN_RI") then
                add_global_define("URC_MAIN_RI_SUPPORT") -- 主动上报硬件RI通知功能
            end
        else
            add_global_define("AT_CMDSET_I") -- ICT AT命令集
        end
    end
-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
  add_global_includedirs("inc")

-----------------------------------------------------------------------
-- Target
-----------------------------------------------------------------------
target(TARGET_NAME)
-- [don't edit] ---
    set_kind("static")
    add_deps("base")
    add_global_deps(TARGET_NAME)
-- [   end    ] ---

    --私有头文件路径
    --add_includedirs("/inc", {public = true})
    --私有宏定义
    if has_config("at tcpip") then
        add_defines("AT_SOCKET")
    end

    if has_config("at mqtt") then
        add_defines("AT_MQTT")
    end

    if has_config("at http") then
        add_defines("AT_HTTP")
    end

    if has_config("at ssl") then
        add_defines("AT_SSL")
    end

    if has_config("at vfs") then
        add_defines("AT_VFS")
    end

    if has_config("at fota") then
        add_defines("AT_FOTA")
    end
    
    if has_config("app bip") then
        add_defines("APP_BIP_SUPPORT")
        add_includedirs("src/bip")
    end

    -- 命令模块定义

    --源文件包含
    --add_srcdir("src", {public = true})
    if get_config("cpu_type") == "cpu-ap" then
        add_srcdirall("src")
    end
    --对外头文件
        -- add_headerfiles("inc/*.h", {prefixdir = "application"})
target_end()
