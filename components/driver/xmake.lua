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
local TARGET_NAME = "drv-oc"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
---add_global_define("ICT_DRV")

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
add_global_includedirs("include")

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
	--add_defines("DEBUG")
	--源文件包含
	--add_srcdir("src", {public = true})
	if get_config("cpu_type") == "cpu-ap" then
		add_srcdir("src/soc")
		add_srcdir("src/i2c")
		add_srcdir("src/i2s")
		add_srcdir("src/keypad")
		add_srcdir("src/gpio")
		add_srcdir("src/pinctrl")
		add_srcdir("src/ssp")
		add_srcdir("src/spi")
		add_srcdir("src/spi_cam")
		add_srcdir("src/uart")
		add_srcdir("src/wdt")
		add_srcdir("src/lcd")		
	end
	
	--对外头文件
	--add_headerfiles("include/*.h", {prefixdir = "drv"})
	--add_headerfiles("include/chip/**.h", {prefixdir = "drv/chip"})
target_end()
