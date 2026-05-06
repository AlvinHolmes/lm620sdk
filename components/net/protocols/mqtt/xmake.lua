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
local TARGET_NAME = "mqtt"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
--add_global_define("ICT_DRV")

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
add_global_includedirs("app_at_api")
add_global_includedirs("pahomqtt-v1.1.0/include")

-----------------------------------------------------------------------
-- Compile options
-----------------------------------------------------------------------
option("mqtt")
    -- 默认关闭
	set_default(true)
	set_showmenu(true)
	set_category("middleware")
	set_description("Enable Paho MQTT", "  =y|n")
option_end()

option("mqtt_tls")
    -- 默认关闭
	set_default(false)
	set_showmenu(true)
	set_category("middleware")
	set_description("Paho MQTT TLS Choice")
option_end()

option("mqtt_test")
    -- 默认关闭
	set_default(false)
	set_showmenu(true)
	set_category("middleware")
	set_description("Enable Paho MQTT Test", "  =y|n")
option_end()

if has_config("mqtt") then
-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
add_global_define("NET_USING_PAHO_MQTT")

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
	add_includedirs("pahomqtt-v1.1.0/MQTTPacket/src",{public = true})
	add_includedirs("pahomqtt-v1.1.0/MQTTClient/src",{public = true})
	--私有宏定义
	--add_defines("DEBUG")
	--源文件包含
	add_srcdir("app_at_api")
	add_srcdir("pahomqtt-v1.1.0/MQTTPacket/src")
	add_srcfile("pahomqtt-v1.1.0/MQTTClient/src/MQTTClient.c")
	add_srcfile("pahomqtt-v1.1.0/MQTTClient/src/MQTTPort.c")
	if has_config("mqtt_tls") then
        add_global_define("MQTT_USING_TLS")
		add_srcfile("pahomqtt-v1.1.0/MQTTClient/src/mq_tls_mbedtls.c")
	end
	if has_config("mqtt_test") then
		add_srcdir("pahomqtt-v1.1.0/MQTTClient/samples")
	end
	--对外头文件
    add_headerfiles("pahomqtt-v1.1.0/include/mqtt_api.h", {prefixdir = "mqtt"})
target_end()

end

