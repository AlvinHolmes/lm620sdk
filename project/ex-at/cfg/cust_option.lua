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

set_config("at tcpip", false)
set_config("at mqtt", false)
set_config("at http", false)
set_config("at ssl", false)
set_config("at vfs", false)
set_config("at fota", false)


