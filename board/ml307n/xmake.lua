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
local TARGET_NAME = "board"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
--add_global_define("ICT_DRV")

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
--add_global_includedirs("inc")
add_global_includedirs(".")
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
    add_includedirs("./",{public = true})
    if os.exists(TOP_DIR .. "/plat/os/src/portable/risc-v/n310") then
	add_includedirs(TOP_DIR .. "/plat/os/src/portable/risc-v/n310",{public = true})
    end
	--私有宏定义
	--add_defines("DEBUG")

	--源文件包含
    add_srcfile("board.c")
    if get_config("cpu_type") == "cpu-ap" then
        add_srcfile("pinmap.c")
    end
	--对外头文件
	--add_headerfiles("inc/*.h", {prefixdir = "drv"})
target_end()
