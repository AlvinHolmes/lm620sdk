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
local TARGET_NAME = "opus"
-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Compile options
-----------------------------------------------------------------------
option("opus")
    -- 默认关闭
	set_default(false)
	set_showmenu(true)
	set_category("opus")
	set_description("Enable or disable opus function", "  =y|n")
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
    add_srcfile('*.c', {public = true})
    add_srcfile('opus-1.5.2/src/*.c', {public = true})
    add_srcfile('opus-1.5.2/tests/ict_opus_test.c', {public = true})
    add_srcfile('opus-1.5.2/celt/*.c', {public = true})
    -- add_srcfile('opus-1.5.2/dnn/*.c', {public = true})
    add_srcfile('opus-1.5.2/silk/*.c', {public = true})
    -- add_srcfile('opus-1.5.2/silk/float/*.c', {public = true})
    add_srcfile('opus-1.5.2/silk/fixed/*.c', {public = true})

    add_global_includedirs('opus-1.5.2/include')
    add_global_includedirs('opus-1.5.2/celt')
    add_global_includedirs('opus-1.5.2')
    -- add_global_includedirs('opus-1.5.2/dnn')
    add_global_includedirs('opus-1.5.2/silk')
    -- add_global_includedirs('opus-1.5.2/silk/float')
    add_global_includedirs('opus-1.5.2/silk/fixed')
    add_global_includedirs(".")

    add_global_define("USE_ALLOCA")
    add_global_define("OPUS_BUILD")
    add_global_define("FIXED_POINT")
    add_global_define("DISABLE_FLOAT_API")
    
    -- add_global_define("HAVE_LRINTF")

	--对外SDK头文件
target_end()

