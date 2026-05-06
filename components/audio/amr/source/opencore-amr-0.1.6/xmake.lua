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
local TARGET_NAME = "opencore-amr-0.1.6"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------

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
	add_includedirs("opencore/codecs_v2/audio/gsm_amr/amr_nb/common/include")
	add_includedirs("opencore/codecs_v2/audio/gsm_amr/amr_nb/dec/include")
    add_includedirs("opencore/codecs_v2/audio/gsm_amr/amr_nb/dec/src")
	add_includedirs("opencore/codecs_v2/audio/gsm_amr/amr_nb/enc/include")
    add_includedirs("opencore/codecs_v2/audio/gsm_amr/amr_nb/enc/src")
	add_includedirs("opencore/codecs_v2/audio/gsm_amr/amr_wb/dec/include")
	add_includedirs("opencore/codecs_v2/audio/gsm_amr/common/dec/include")
    add_includedirs("opencore/codecs_v2/audio/gsm_amr/amr_wb/dec/src")
    add_includedirs("oscl")
	--私有宏定义
	--add_defines("DEBUG")
	--源文件包含
    add_srcfile("amrnb/wrapper.cpp")
    add_srcfile("amrwb/wrapper.cpp")
    add_srcfile("opencore/codecs_v2/audio/gsm_amr/amr_nb/enc/src/*.cpp")
    add_srcfile("opencore/codecs_v2/audio/gsm_amr/amr_nb/dec/src/*.cpp")
    add_srcfile("opencore/codecs_v2/audio/gsm_amr/amr_nb/common/src/*.cpp")

	add_srcdir("opencore/codecs_v2/audio/gsm_amr/amr_nb/common/src")
	add_srcdir("opencore/codecs_v2/audio/gsm_amr/amr_nb/dec/src")
	add_srcdir("opencore/codecs_v2/audio/gsm_amr/amr_nb/enc/src")
    
	add_srcdir("opencore/codecs_v2/audio/gsm_amr/amr_wb/dec/src")
	-- add_srcdir("opencore/codecs_v2/audio/gsm_amr/common/dec/src")
    add_srcfile("opencore/codecs_v2/audio/gsm_amr/amr_wb/dec/src/*.cpp")


	--对外头文件
	add_headerfiles("../../include/*.h", {prefixdir = "amr"})
target_end()
