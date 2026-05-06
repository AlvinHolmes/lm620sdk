-----------------------------------------------------------------------
-- Xmake script file
-- Copyright (C) 2023, Nanjing Innochip Technology Co.,Ltd.
-- All rights reserved.
--
-- @author      geanfeng
-- @file        top.lua
-----------------------------------------------------------------------


-----------------------------------------------------------------------
-- SDK Version Configuration
-----------------------------------------------------------------------

option("sdkver")
    -- 默认数传版本
    set_default("mdl")
    set_showmenu(true)
    set_category("top")
	set_description("Select sdk lib version")
	-- 可选的软件基础包
	set_values("mdl", "volte")
option_end()

-----------------------------------------------------------------------
-- Top config options
-----------------------------------------------------------------------
option("asic")
    -- 默认关闭ASIC配置,采用FPGA配置
        set_default(true)
        set_showmenu(true)
        set_category("top")
        set_description("Enable or disable asic function macro", "  =y|n")
option_end()


option("dlmo")
    -- 默认关闭
    set_default(false)
    set_showmenu(true)
    set_category("top")
    set_description("Enable or disable dynamic loading module", "  =y|n")
option_end()

option("mts")
    -- 默认关闭
        set_default(false)
        set_showmenu(true)
        set_category("top")
        set_description("Enable or disable mts version", "  =y|n")
option_end()

option("mini")
    -- 默认关闭
        set_default(false)
        set_showmenu(true)
        set_category("top")
        set_description("Enable or disable mini version", "  =y|n")
option_end()

option("wifiscan")
    -- 默认开启WIFISCAN配置
        set_default(true)
        set_showmenu(true)
        set_category("top")
        set_description("Enable or disable WIFISCAN function", "  =y|n")
option_end()

option("volte")
    -- 默认关闭
    set_default(false)
    set_showmenu(true)
    set_category("top")
    set_description("Enable or disable volte function", "  =y|n")
option_end()

option("video")
    -- 默认关闭
    set_default(false)
    set_showmenu(true)
    set_category("top")
    set_description("Enable or disable video function", "  =y|n")
option_end()

option("ssc")
    -- 默认关闭
    set_default(false)
    set_showmenu(true)
    set_category("top")
    set_description("Enable or disable ssc function", "  =y|n")
option_end()

option("bootup_standby")
    -- 默认关闭
    set_default(false)
    set_showmenu(true)
    set_category("top")
    set_description("Enable or disable bootup psm standby mode function", "  =y|n")
option_end()


-- Top-Level macros

if has_config("asic") then
    add_global_define("USE_TOP_ASIC")
else
    add_global_define("USE_TOP_FPGA")
end

if has_config("mts") then
    add_global_define("USE_TOP_MTS")
else
    add_global_define("USE_TOP_PM")
end

if has_config("mini") then
    add_global_define("USE_TOP_MINI")
end

if not has_config("mini","mts") then
    add_global_define("ENABLE_SPI_SEND_LOG")
end

if has_config("wifiscan") and not has_config("mts") then
    add_global_define("USE_TOP_WIFISCAN")
end

if has_config("volte") then
    add_global_define("USE_TOP_VOLTE")
end

if has_config("volte-sms") then
    add_global_define("USE_TOP_VOLTE_SMS")
end

if has_config("ppp") then
    add_global_define("USE_TOP_PPP")
end

if has_config("video") then
    add_global_define("USE_TOP_VIDEO")
    add_global_define("USE_LOG_WITHOUT_HDLC")
else
    -- add_global_define("USE_DUAL_CARD")
end

if has_config("ssc") then
    add_global_define("USE_TOP_SSC")
end

if has_config("bootup_standby") then
    add_global_define("USE_BOOT_PSM_STANDBY")
end

if has_config("audio") then
    add_global_define("USE_AUDIO")
end
