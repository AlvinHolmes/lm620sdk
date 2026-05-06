-----------------------------------------------------------------------
-- Xmake script file
-- Copyright (C) 2023, Nanjing Innochip Technology Co.,Ltd.
-- All rights reserved.
--
-- @author      geanfeng
-- @file        cust_option.lua
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Custom configuration options
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- board configuration options
-----------------------------------------------------------------------
set_config("pa_aw87390", false)
-- set_config("codec_es8311", true)
set_config("codec_pt8311", true)

-----------------------------------------------------------------------
-- Set components
-----------------------------------------------------------------------
--set_config("lvgl", true)

set_config("at tcpip", true)
set_config("at mqtt", true)
set_config("at http", true)
set_config("at ssl", true)
set_config("at vfs", true)
set_config("at fota", true)
-- set_config("CMDM", true)

set_config("audio",true)
set_config("dsp", true)
set_config("SPI2USB",false)

-----------------------------------------------------------------------
-- audio configuration options
-----------------------------------------------------------------------

-- set_config("voice_proc", true)
-- set_config("audio_debug",true)

-----------------------------------------------------------------------
-- audio codec options
-----------------------------------------------------------------------
-- audio codec
-- set_config("amr",true)
-- set_config("mp3", true)
-- set_config("pcm_u_a", true)
-- set_config("opus", true)
-- set_config("ogg", true)