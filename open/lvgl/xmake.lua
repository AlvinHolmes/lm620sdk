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
local TARGET_NAME = "lvgl"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
--add_global_define("LV_CONF_INCLUDE_SIMPLE")
--add_global_define("LV_LVGL_H_INCLUDE_SIMPLE")

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
add_global_includedirs("origin")
add_global_includedirs("port")


-----------------------------------------------------------------------
-- Target
-----------------------------------------------------------------------
if has_config("lvgl") then

target(TARGET_NAME)
-- [don't edit] ---
    set_kind("static")
	add_deps("base")
	add_global_deps(TARGET_NAME)
-- [   end    ] ---

    --私有头文件路径
	add_includedirs("origin/src", {public = true})
	add_includedirs("origin/src/core", {public = true})
    add_includedirs("origin/src/draw", {public = true})
	add_includedirs("origin/src/font", {public = true})
	add_includedirs("origin/src/hal", {public = true})
	add_includedirs("origin/src/misc", {public = true})
	add_includedirs("origin/src/widgets", {public = true})
	add_includedirs("origin/src/extra", {public = true})
    add_includedirs("port", {public = true})

	--私有宏定义
	--add_defines("LV_CONF_INCLUDE_SIMPLE")
	--源文件包含
	--add_srcdir("src", {public = true})
	add_srcdir("origin/src/font")
	add_srcdir("origin/src/hal")
	add_srcdir("origin/src/draw")
	add_srcdir("origin/src/draw/sw")
	add_srcdir("origin/src/widgets")
	add_srcdir("origin/src/misc")
	add_srcdir("origin/src/core")
	add_srcdirall("origin/src/extra")
	-- demos
	-- add_srcdirall("origin/demos")
	add_srcdirall("origin/examples")
	-- port
	add_srcdir("port")
	--对外头文件
	--add_headerfiles("inc/*.h", {prefixdir = "drivers"})
target_end()

end