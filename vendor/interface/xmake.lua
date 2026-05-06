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
option("at_tool")
    -- 默认关闭
	set_default(false)
	set_showmenu(true)
	set_category("interface")
	set_description("Enable uart", "  =y|n")
option_end()

option("fota")
    -- 默认关闭
	set_default(false)
	set_showmenu(true)
	set_category("interface")
	set_description("Enable uart", "  =y|n")
option_end()

option("sntp")
    -- 默认关闭
	set_default(false)
	set_showmenu(true)
	set_category("interface")
	set_description("Enable uart", "  =y|n")
option_end()

-----------------------------------------------------------------------
-- Include subdirs
-----------------------------------------------------------------------
if has_config("at_tool") then
	includes("at_tool")
end

if has_config("fota") then
	includes("fota")
end

if has_config("sntp") then
	includes("sntp")
end



