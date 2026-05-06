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

-----------------------------------------------------------------------
-- Configuration
-----------------------------------------------------------------------
local TARGET_NAME = "utilities"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
---add_global_define("ICT_DRV")
-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
--add_global_includedirs("inc")
	add_global_includedirs("cJSON")
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
	--add_includedirs("generated", {public = true})

	--私有宏定义
	--add_defines("LV_CONF_INCLUDE_SIMPLE")

	
	--源文件包含
	--add_srcdir("src", {public = true})

	    add_srcdir("cJSON")
	--对外头文件
	--add_headerfiles("inc/*.h", {prefixdir = "drivers"})
target_end()
