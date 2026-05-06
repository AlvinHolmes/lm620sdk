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
local TARGET_NAME = "ota"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
---add_global_define("OS_USING_OTA")

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
    add_includedirs("include", {public = true})
    --私有宏定义
    --add_defines("DEBUG")
    --源文件包含
    add_srcdir("src", {public = true})
    --对外头文件
    add_headerfiles("include/*.h", {prefixdir = "ota"})
target_end()
