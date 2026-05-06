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
local TARGET_NAME = "ftp"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
--add_global_define("ICT_DRV")

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
add_global_includedirs("ftplib-4.0-1/inc")

-----------------------------------------------------------------------
-- Compile options
-----------------------------------------------------------------------
option("ftp")
    -- 默认关闭
	set_default(true)
	set_showmenu(true)
	set_category("middleware")
	set_description("Enable ftp library", "  =y|n")
option_end()

option("ftp_test")
    -- 默认关闭
	set_default(false)
	set_showmenu(true)
	set_category("middleware")
	set_description("Enable ftp library Test", "  =y|n")
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
	--私有宏定义
	add_defines("FTP_RTOS")
	--源文件包含
	add_srcfile("ftplib-4.0-1/src/ftplib.c")
	--对外头文件
	--add_headerfiles("ftplib-4.0-1/inc/*.h", {prefixdir = "net"})
target_end()


