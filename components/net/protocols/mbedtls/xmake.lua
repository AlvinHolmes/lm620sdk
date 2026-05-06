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
local TARGET_NAME = "mbedtls"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
--add_global_define("ICT_DRV")

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
--add_global_includedirs("inc")
    add_global_includedirs("mbedtls-v2.28.2/include")
    add_global_includedirs("ports/inc")
-----------------------------------------------------------------------
-- Private function
-----------------------------------------------------------------------

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
	add_includedirs("mbedtls-v2.28.2/configs")
	add_includedirs("mbedtls-v2.28.2/include")
	add_includedirs("mbedtls-v2.28.2/library")
	add_includedirs("ports/inc")
	--私有宏定义
	--add_defines("DEBUG")
	--源文件包含
	add_srcdir("mbedtls-v2.28.2/library")
	add_srcdir("ports/src")
	--对外头文件
	add_headerfiles("ports/(**.h)", {prefixdir = "mbedtls/ports"})
	add_headerfiles("mbedtls-v2.28.2/include/mbedtls/(**.h)", {prefixdir = "mbedtls"})
	--add_headerfiles("mbedtls-v2.28.2/include/psa/(**.h)", {prefixdir = "middleware/mbedtls/psa"})
target_end()
