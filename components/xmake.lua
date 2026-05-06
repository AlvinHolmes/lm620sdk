-----------------------------------------------------------------------
-- Xmake script file
-- Copyright (C) 2023, Nanjing Innochip Technology Co.,Ltd.
-- All rights reserved.
--
-- @author      geanfeng
-- @file        xmake.lua
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Include subdirs
-----------------------------------------------------------------------
if get_config("cpu_type") == "cpu-ap" then
    includes("driver")
    includes("utilities")
	
	if has_config("slapp") then
        includes("libapp")
    end

    if has_config("posix") then
        includes("libc")
    end

    if has_config("cm") then
    end

    if not has_config("mts", "mini") then
        includes("net/pub")
        includes("net/lwip")
        includes("net/protocols/mqtt")
        includes("net/protocols/websocket")
        includes("net/protocols/mbedtls")
        includes("net/protocols/http")
        includes("net/protocols/ftp")
        includes("firmwareupdate")
        includes("audio")
        if has_config("CMDM") then
        end

        if has_config("volte") then
        end

        if has_config("volte") or has_config("volte-sms") then
        end

        if has_config("video_call") then
            includes("video_call")
        end

        if has_config("TTS") then
        end
    end

end

if get_config("cpu_type") == "cpu-cp" then
    includes("driver")
end

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
