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
local TARGET_NAME = "open-drv"

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
    --add_includedirs("/inc", {public = true})
    --私有宏定义
    --add_defines("DEBUG")
    --源文件包含
    --add_srcdir("src", {public = true})
    if get_config("chip") then
        if get_config("cpu_type") == "cpu-ap" then

            if get_config("codec_es8311") then
                add_srcdir("src/es8311")
            end
            if get_config("codec_pt8311") then
                add_srcdir("src/pt8311")
            end

            add_srcdir("src/gc032a")
            add_srcdir("src/pa")
            add_srcdir("src/disp")
            add_srcdir("src/input")
            add_srcdir("src/camera")
            add_srcdir("src/st7789v")
            add_srcdir("src/i2c")
            add_srcdir("src/lcd_te")
            add_srcdir("src/res_tree")

            add_srcdir("src/onewire")
            --add_srcdir("src/extint")
            add_srcdir("src/pwrkey")

            if get_config("video") then
                add_srcdir("src/video")
            end
        end
    end
    --对外头文件
    --add_headerfiles("inc/*.h", {prefixdir = "drivers"})
target_end()
