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
local TARGET_NAME = "websocket"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
--add_global_define("ICT_DRV")

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Compile options
-----------------------------------------------------------------------
option("websocket")
    -- 默认关闭
	set_default(true)
	set_showmenu(true)
	set_category("middleware")
	set_description("Enable librws webwocket", "  =y|n")
option_end()

option("websocket_example")
    -- 默认关闭
	set_default(false)
	set_showmenu(true)
	set_category("websocket")
	set_description("Enable librws webwocket example", "  =y|n")
option_end()

if has_config("websocket") then
-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
add_global_includedirs("librws_1.3.0/inc")
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
    add_defines("LIBRWS_USING_MBED_TLS")
    --使用信号量阻塞RWS工作线程
    add_defines("RWS_THREAD_BLOCK")
	--源文件包含
	add_srcdir("librws_1.3.0/src")

    if has_config("websocket_example") then
        add_srcdir("librws_1.3.0/examples")
    end
	--对外头文件
    
target_end()

end

