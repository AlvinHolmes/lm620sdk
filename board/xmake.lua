-----------------------------------------------------------------------
-- Xmake script file
-- Copyright (C) 2023, Nanjing Innochip Technology Co.,Ltd.
-- All rights reserved.
--
-- @author      geanfeng
-- @file        xmake.lua
-----------------------------------------------------------------------


-----------------------------------------------------------------------
-- Compile options
-----------------------------------------------------------------------
option("board")
    -- 默认选择evb
	set_default("evb")
	set_showmenu(true)
	set_category("board")
	set_description("Select board type")
	-- 可选的板级配置
	set_values("evb", "evt", "emu", "openphone")
option_end()

-- include source directory
includes(get_config("board"))
