-----------------------------------------------------------------------
-- Xmake script file
-- Copyright (C) 2023, Nanjing Innochip Technology Co.,Ltd.
-- All rights reserved.
--
-- @author      geanfeng
-- @file        xmake.lua
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Configuration
-----------------------------------------------------------------------
local TARGET_NAME = "mp3"
-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Compile options
-----------------------------------------------------------------------
option("mp3")
    -- 默认关闭
	set_default(false)
	set_showmenu(true)
	set_category("mp3")
	set_description("Enable or disable mp3 function", "  =y|n")
option_end()

----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Target
-----------------------------------------------------------------------
target(TARGET_NAME)
-- [don't edit] ---
	set_kind("static")
	add_deps("base")
    -- set_optimize("fastest")
	add_global_deps(TARGET_NAME)
-- [   end    ] ---
    add_srcfile("real/*.c", {public = true})
    add_srcfile("*.c", {public = true})
    add_global_includedirs("pub")
    add_global_includedirs("real")

	--对外SDK头文件
target_end()

