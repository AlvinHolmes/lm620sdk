-----------------------------------------------------------------------
-- Xmake script file
-- Copyright (C) 2023, Nanjing Innochip Technology Co.,Ltd.
-- All rights reserved.
--
-- @author      geanfeng
-- @file        xmake.lua
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Option
-----------------------------------------------------------------------
includes("cfg/base_option.lua")
includes("cfg/cust_option.lua")

-----------------------------------------------------------------------
-- Configuration
-----------------------------------------------------------------------
local TARGET_NAME = "main"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
--add_global_define("ICT_DRV")

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
add_global_includedirs("nv")

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
    add_includedirs("inc",{public = true})
    --私有宏定义
    --add_defines("DEBUG")
    --源文件包含
    -- add_includedirs("inc",{public = true})
    -- add_srcfile("src/adc_main.c")
    -- add_srcfile("src/asocket_main.c")
    -- add_srcfile("src/audio_main.c")
    -- add_srcfile("src/dns_main.c")
    add_srcfile("src/eloop_main.c")
    -- add_srcfile("src/fs_main.c")
    -- add_srcfile("src/gpio_main.c")
    -- add_srcfile("src/http_main.c")
    -- add_srcfile("src/i2c_main.c")
    -- add_srcfile("src/lcd_main.c")
    -- add_srcfile("src/mobile_main.c")
    -- add_srcfile("src/modem_main.c")
    -- add_srcfile("src/mqtt_main.c")
    -- add_srcfile("src/ntp_main.c")
    -- add_srcfile("src/ping_main.c")
    -- add_srcfile("src/pm_main.c")
    -- add_srcfile("src/pwm_main.c")
    -- add_srcfile("src/rtc_main.c")
    -- add_srcfile("src/sms_main.c")
    -- add_srcfile("src/socket_main.c")
    -- add_srcfile("src/spi_main.c")
    -- add_srcfile("src/ssl_main.c")
    -- add_srcfile("src/tts_main.c")
    -- add_srcfile("src/uart_main.c")
    -- add_srcfile("src/virt_at_main.c")
    -- add_srcfile("src/wifiscan_main.c")
target_end()
