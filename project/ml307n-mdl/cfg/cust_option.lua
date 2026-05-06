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
-- set_config("pa_aw87390", true)
-- set_config("codec_es8311", true)


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

-----------------------------------------------------------------------
-- vendor interface options
-----------------------------------------------------------------------
--set_config("at_tool", true)
--set_config("fota", true)
-- set_config("sntp", true)


