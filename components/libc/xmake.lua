-----------------------------------------------------------------------
-- Xmake script file
-- SPDX-FileCopyrightText: 2025 南京创芯慧联技术有限公司
-- SPDX-License-Identifier: Apache-2.0
--
-- @author      your name
-- @file        xmake.lua
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Configuration
-----------------------------------------------------------------------
local TARGET_NAME = "posix"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
add_global_define("_POSIX_TIMERS")
add_global_define("_POSIX_THREADS")
add_global_define("_POSIX_THREAD_PROCESS_SHARED")
add_global_define("_POSIX_REALTIME_SIGNALS")
-- add_global_define("_POSIX_THREAD_PRIO_PROTECT")
add_global_define("_UNIX98_THREAD_MUTEX_ATTRIBUTES")
add_global_define("__POSIX_VISIBLE=199506")
add_global_define("_POSIX_TIMEOUTS")

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------

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
    -- add_includedirs("include", {public = true})
    add_includedirs("pthreads")
    --私有宏定义
    --add_defines("DEBUG")
    --源文件包含
    add_srcdir("pthreads", {public = true})
    add_srcdir("time", {public = true})
    add_srcdir("timer", {public = true})
    --对外头文件
    add_global_includedirs("pthreads")
target_end()
