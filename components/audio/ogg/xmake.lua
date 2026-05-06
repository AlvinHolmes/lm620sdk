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
local TARGET_NAME = "ogg"
-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Compile options
-----------------------------------------------------------------------
option("ogg")
    -- 默认关闭
	set_default(false)
	set_showmenu(true)
	set_category("ogg")
	set_description("Enable or disable ogg function", "  =y|n")
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
    set_optimize("fastest")
	add_global_deps(TARGET_NAME)
-- [   end    ] ---

    add_global_includedirs('libogg-1.3.5/include/ogg')
    add_global_includedirs(".")
    add_srcfile('libogg-1.3.5/src/*.c', {public = true})
    add_srcfile('*.c', {public = true})

	--对外SDK头文件
target_end()

