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
includes("driver")
--includes("app")
includes("interface")

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
add_global_define("ZLOG_DEBUG")
add_global_define("ZLOG_COLOR")
--add_global_define("ZLOG_TO_SLOG")

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
add_global_includedirs("include")