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
local TARGET_NAME = "libapp"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
add_global_includedirs("inc")

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
    add_includedirs("inc", {public = true})
    --私有宏定义
    --add_defines("OS_DL_RELOBJ")
    --源文件包含
    add_srcdir("src", {public = true})
    --对外头文件
    add_headerfiles("inc/appload.h", {prefixdir = "libapp"})
target_end()
