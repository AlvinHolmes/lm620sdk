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
local TARGET_NAME = "video_call"
-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Compile options
-----------------------------------------------------------------------
option("video_call")
    -- 默认关闭
	set_default(false)
	set_showmenu(true)
	set_category("video_call")
	set_description("Enable or disable middleware video call function", "  =y|n")
option_end()

add_global_includedirs(".")

-----------------------------------------------------------------------
-- Export global macro
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

if has_config("video_call") then
    -- JPEG lib
    if has_config("jpeg") then
        add_linkdirs("libjpeg")
        add_links("jpeg")
        add_global_includedirs("libjpeg")
        add_srcfile("libjpeg/jpeg.c",{public = true})
        add_global_define("USE_LIBJPEG_TURBO")
    end
    -- juphoon
    if has_config("juphoon") then
        add_linkdirs("juphoon/lib")
        add_links("jrtc0_32ZC")
        add_global_includedirs("juphoon")
        add_global_includedirs("juphoon/port")
        add_srcfile("juphoon/video_call.c",{public = true})
        add_srcfile("juphoon/port/jc.c",{public = true})
        add_srcfile("juphoon/port/mme.c",{public = true})
        add_global_define("USE_JUPHOON_SDK")
    end
    -- image convert
    add_srcfile("image_convert.c",{public = true})
    add_global_includedirs(".")
end

	--对外SDK头文件
    add_headerfiles("libjpeg/**.h", {prefixdir = "media"})
    add_headerfiles("*.h", {prefixdir = "media"})

target_end()


