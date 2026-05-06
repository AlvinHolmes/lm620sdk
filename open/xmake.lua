-----------------------------------------------------------------------
-- Xmake script file
-- Copyright (C) 2023, Nanjing Innochip Technology Co.,Ltd.
-- All rights reserved.
--
-- @author      geanfeng
-- @file        xmake.lua
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Include subdirs
-----------------------------------------------------------------------
includes("config")

if get_config("cpu_type") == "cpu-ap" then
	includes("drivers")
    includes("fs")
	if not has_config("mts", "mini") then
		includes("appstart")
		includes("net")
	end
	includes("lvgl")
end
-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
