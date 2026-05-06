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
local TARGET_NAME = "http"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
--add_global_define("ICT_DRV")

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
add_global_includedirs("httpclient-v1.1.0/include")

-----------------------------------------------------------------------
-- Compile options
-----------------------------------------------------------------------
option("http")
    -- 默认关闭
	set_default(true)
	set_showmenu(true)
	set_category("middleware")
	set_description("Enable Paho http", "  =y|n")
option_end()

option("http_test")
    -- 默认关闭
	set_default(false)
	set_showmenu(true)
	set_category("middleware")
	set_description("Enable Paho http Test", "  =y|n")
option_end()

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
	add_includedirs("httpclient-v1.1.0/include")
	--私有宏定义
	--add_defines("DEBUG")
	--源文件包含
	add_srcdir("httpclient-v1.1.0/src")
	add_srcdir("httpclient-v1.1.0/port")
	add_srcfile("httpclient-v1.1.0/port/sample/http_client_get_file.c")
	--对外头文件
	add_headerfiles("httpclient-v1.1.0/include/**.h", {prefixdir = "http"})
target_end()


