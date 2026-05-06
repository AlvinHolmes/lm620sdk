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
add_global_includedirs("src")

-- set_config("test_type", "pcm_raw")
-- set_config("test_type", "amr_file_data")
-- set_config("test_type", "mp3_file_data")
-- set_config("test_type", "wav_file_data")
-- set_config("test_type", "opus_file_data")
set_config("test_type", "record_play")

-- audio codec
-- set_config("amr",true)
set_config("mp3", true)
-- set_config("pcm_u_a", true)
-- set_config("opus", true)
-- set_config("ogg", true)

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
    add_srcdir("./src")
    
    if get_config("test_type") == "pcm_raw" then
        add_files("./audio_src/test_data_pcm.c")

    elseif get_config("test_type") == "amr_file_data" then
        add_files("./audio_src/test_data_amr.c")

    elseif get_config("test_type") == "mp3_file_data" then
        add_files("./audio_src/test_data_mp3.c")
        
    elseif get_config("test_type") == "wav_file_data" then
        add_files("./audio_src/test_data_wav.c")

    elseif get_config("test_type") == "opus_file_data" then
        add_files("./audio_src/test_data_opus.c")

    elseif get_config("test_type") == "record_play" then
        add_files("./audio_src/test_record_play.c")

    end
    
target_end()
