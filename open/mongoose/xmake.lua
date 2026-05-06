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
local TARGET_NAME = "mongoose"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
---add_global_define("ICT_DRV")

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
-- add_global_includedirs("origin")
-- add_global_includedirs("port")

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
    add_includedirs("origin/src", {public = true})
    add_includedirs("origin", {public = true})
    add_includedirs("port", {public = true})

	--私有宏定义
    add_defines("MG_CONF_INCLUDE")
	--源文件包含
	--add_srcdir("src", {public = true})
    add_srcdir("origin/src", {public = true})
    add_srcdir("port", {public = true})

	--对外头文件
	--add_headerfiles("inc/*.h", {prefixdir = "drivers"})
target_end()
