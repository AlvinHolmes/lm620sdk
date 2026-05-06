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
local TARGET_NAME = "vendor-sntp"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
--add_global_define("")

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
--add_global_includedirs("inc")

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
    add_includedirs("/inc", {public = true})
	--私有宏定义
	--add_defines("")
	--共有宏定义
	add_defines(get_global_define(), {public = true})
	--源文件包含
	if get_config("cpu_type") == "cpu-ap" then
		add_srcdir("src", {public = true})		
	end

	--对外头文件
	--add_headerfiles("inc/*.h", {prefixdir = "xxx"})
target_end()
