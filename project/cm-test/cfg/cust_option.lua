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

set_config("cm", true)
set_config("xtiny", true)
set_config("all_src", true)

set_config("pa_aw87390", false)
set_config("codec_pt8311", true)
set_config("audio",true)

-- audio codec
set_config("amr",true)
-- set_config("mp3", true)
set_config("pcm_u_a", true)
-- set_config("opus", true)
-- set_config("ogg", true)

set_config("TTS", true)
set_config("AT_CMDSET_Q", true)