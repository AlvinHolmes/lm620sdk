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
local TARGET_NAME = "vo-amrwbenc-0.1.3"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
--add_global_define("ICT_DRV")

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
--add_global_includedirs("inc")

-----------------------------------------------------------------------
-- Compile options
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
    --测试开关宏
--    add_global_define("MIDDLEWARE_TEST_ON")
    --add_global_define("MIDDLEWARE_TEST_AT_ON")

-----------------------------------------------------------------------
-- Target
-----------------------------------------------------------------------
target(TARGET_NAME)
-- [don't edit] ---
	set_kind("static")
    set_optimize("fastest")
	add_deps("base")
	add_global_deps(TARGET_NAME)
-- [   end    ] ---

	--私有头文件路径
	add_includedirs("common/include")
	add_includedirs("amrwbenc/inc")
    add_includedirs("amrwbenc/src")
	--私有宏定义
	--add_defines("DEBUG")
	--源文件包含
	add_srcdir("amrwbenc/src")
    add_srcdir("common")
    add_srcfile("wrapper.c")
    -- add_srcfile("amrwbenc/*.cpp")
	--对外头文件
	--add_headerfiles("inc/*.h", {prefixdir = "drv"})
target_end()
