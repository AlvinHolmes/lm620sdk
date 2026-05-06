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
local TARGET_NAME = "audio"
-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Compile options
-----------------------------------------------------------------------
option("audio")
    -- 默认关闭
    set_default(false)
    set_showmenu(true)
    set_category("audio")
    set_description("Enable or disable middleware audio function", "  =y|n")
option_end()

option("codec_pt8311")
    -- 默认关闭
    set_default(false)
    set_showmenu(true)
    set_category("drivers")
    set_description("Enable or disable pt8311 codec function", "  =y|n")
option_end()

option("codec_es8311")
    -- 默认关闭
    set_default(false)
    set_showmenu(true)
    set_category("drivers")
    set_description("Enable or disable es8311 codec function", "  =y|n")
option_end()

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
if has_config("audio") then
--    add_global_define("USE_AUDIO")
end

if has_config("opus") then
    add_global_define("USE_OPUS")
    includes("opus")
end

if has_config("ogg") then
    add_global_define("USE_OGG")
    includes("ogg")
end
-----------------------------------------------------------------------
-- Target
-----------------------------------------------------------------------
target(TARGET_NAME)
-- [don't edit] ---
    set_kind("static")
    add_deps("base")
    add_global_deps(TARGET_NAME)
-- [   end    ] ---

if has_config("audio") then

    if has_config("pa_aw87390") then
        add_global_define("AUDIO_PA_AW87390")
    end

    if has_config("codec_es8311") then
        add_global_define("CODEC_ES8311")
    end

    if has_config("codec_pt8311") then
        add_global_define("CODEC_PT8311")
    end

    if has_config("codec_cjc8910") then
        add_global_define("CODEC_CJC8910")
    end

    if has_config("amr") then
        add_srcfile("amr.c",{public = true})
        includes("amr/source/opencore-amr-0.1.6") 
        includes("amr/source/vo-amrwbenc-0.1.3")    
        add_global_includedirs("amr/include")
        add_global_define("USE_AMR")
    end

    add_srcfile("wav/wav.c",{public = true})
    add_global_includedirs("wav")
    
    if has_config("mp3") then
        includes("helix_mp3")
        add_global_define("USE_MP3")
    end

    if has_config("pcm_u_a") then 
        includes("g711")
        add_global_define("USE_PCM_U_A")
    end

    if has_config("audio_debug") then 
        add_srcfile("audio_debug.c",{public = true})
        add_global_define("YMODEM_RECVFILE")
        add_global_define("USE_AUDIO_DBG")
    end 

    add_srcfile("audio_codec.c",{public = true})
    add_srcfile("audio_local.c",{public = true})
    add_srcfile("audio_monitor.c",{public = true})
    add_srcfile("audio_pa.c",{public = true})
    add_srcfile("audio_voice.c",{public = true})

    -- 测试接口代码,根据需要客户自行打开
    -- add_srcfile("test_audio.c",{public = true})
    add_global_includedirs(".")

    add_global_includedirs("enc_dec")
    add_srcdir("enc_dec")

    add_global_includedirs("dev")
    add_srcdir("dev")
    
end

    --对外SDK头文件
    add_headerfiles("audio_codec.h", {prefixdir = "audio"})
    add_headerfiles("audio_voice.h", {prefixdir = "audio"})    
    add_headerfiles("audio_pa.h", {prefixdir = "audio"})
    add_headerfiles("audio_local.h", {prefixdir = "audio"})    
    add_headerfiles("audio_monitor.h", {prefixdir = "audio"})        
    add_headerfiles("dev/audio_dev.h", {prefixdir = "audio"})    
target_end()

