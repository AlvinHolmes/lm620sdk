-----------------------------------------------------------------------
-- Xmake script file
-- Copyright (C) 2023, Nanjing Innochip Technology Co.,Ltd.
-- All rights reserved.
--
-- @author      geanfeng
-- @file        xmake.lua
-----------------------------------------------------------------------

package("gnu_rm")
	set_kind("toolchain")
	set_homepage("https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm")
	set_description("GNU Arm Embedded Toolchain")
	local version_map = {
		["2021.10"] = "10.3-2021.10"
	}
	if is_host("windows") then
		set_urls("http://cdndownload.openluat.com/xmake/toolchains/gcc-arm/gcc-arm-none-eabi-$(version)-win32.zip", {version = function (version)
			return version_map[tostring(version)]
		end})
		add_versions("2021.10", "d287439b3090843f3f4e29c7c41f81d958a5323aecefcf705c203bfd8ae3f2e7")
	elseif is_host("linux") then
		set_urls("http://cdndownload.openluat.com/xmake/toolchains/gcc-arm/gcc-arm-none-eabi-$(version)-x86_64-linux.tar.bz2", {version = function (version)
			return version_map[tostring(version)]
		end})
		add_versions("2021.10", "97dbb4f019ad1650b732faffcc881689cedc14e2b7ee863d390e0a41ef16c9a3")
	end
	on_install("@windows", "@linux", function (package)
		os.vcp("*", package:installdir())
	end)
package_end()