-----------------------------------------------------------------------
-- Xmake script file
-- Copyright (C) 2023, Nanjing Innochip Technology Co.,Ltd.
-- All rights reserved.
--
-- @author      geanfeng
-- @file        xmake.lua
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Global variable
-----------------------------------------------------------------------
GLOBAL_DEPS = {}
GLOBAL_INCLUDE = {}
GLOBAL_DEFINE = {}

-----------------------------------------------------------------------
-- Functions
-----------------------------------------------------------------------
function add_global_includedirs(path)
	if path then
		table.insert(GLOBAL_INCLUDE, os.curdir() .. "/" .. path)
	end
end

function get_global_includedirs()
	return GLOBAL_INCLUDE
end

function add_global_define(define)
	if define then
		table.insert(GLOBAL_DEFINE, define)
	end
end

function get_global_define()
	return GLOBAL_DEFINE
end

function add_global_deps(target)
	if target then
		table.insert(GLOBAL_DEPS, target)
	end
end

function add_srcfile(filepath)
    add_files(filepath, {defines = "__FILE__=" .. "\"" .. path.basename(filepath) .. "\""})
end

function add_srcdir(dirpath)
    for _, filepath in ipairs(os.files(dirpath .. "/*.c")) do
    	add_files(filepath, {defines = "__FILE__=" .. "\"" .. path.basename(filepath) .. "\""})
    end
end

function add_srcdirall(dirpath)
    for _, filepath in ipairs(os.files(dirpath .. "/**.c")) do
    	add_files(filepath, {defines = "__FILE__=" .. "\"" .. path.basename(filepath) .. "\""})
    end
end

-----------------------------------------------------------------------
-- Global include directory
-----------------------------------------------------------------------
if os.getenv("USE_SDK") ~= "y" then
    --全版本
	add_includedirs(TOP_DIR .. "/components",{public = true})
	add_includedirs(TOP_DIR .. "/modem",{public = true})
	add_includedirs(TOP_DIR .. "/modem/ps",{public = true})
	add_includedirs(TOP_DIR .. "/modem/phy",{public = true})
	add_includedirs(TOP_DIR .. "/modem/pub/inc",{public = true})
	add_includedirs(TOP_DIR .. "/modem/pub/inc/nv",{public = true})
	
else
    --sdk版本
	add_includedirs(TOP_DIR .. "/components",{public = true})
	add_includedirs(TOP_DIR .. "/components/audio",{public = true})
	if has_config("sdkver") then
		add_includedirs("../library/" .. get_config("sdkver") .. "/include",{public = true})
		add_includedirs(os.dirs("../library/" .. get_config("sdkver") .. "/include/*"),{public = true})
		add_includedirs("../library/" .. get_config("sdkver") .. "/include/drv/chip",{public = true})
		add_includedirs("../library/" .. get_config("sdkver") .. "/include/modem/nv",{public = true})
		if os.exists("../library/" .. get_config("sdkver") .. "/include/lwip") then
			add_includedirs("../library/" .. get_config("sdkver") .. "/include/lwip/port",{public = true})
			add_includedirs("../library/" .. get_config("sdkver") .. "/include/lwip/port/arch/include",{public = true})
		end
	end
end

-----------------------------------------------------------------------
-- Include subdirs
-----------------------------------------------------------------------
includes(TOP_DIR .. "/scripts/toolchain")
includes(TOP_DIR .. "/components")

if os.getenv("USE_SDK") ~= "y" then
	if get_config("cpu_type") == "cpu-ap" then
		includes(TOP_DIR .. "/modem/pub")	
		if not has_config("mts", "mini") then
			includes(TOP_DIR .. "/modem/ps")
		end
	end
	if get_config("cpu_type") == "cpu-cp" then
		if not has_config("mini") then
		includes(TOP_DIR .. "/modem/phy")
		end
	end	
else
    if get_config("cpu_type") == "cpu-ap" then
        add_global_define("BUILD_IN_SDK")
    end
end

includes(TOP_DIR .. "/open")
includes(TOP_DIR .. "/board")
if G_USER_PROJECT_NAME then
    includes(TOP_DIR .. "/project/" .. G_USER_PROJECT_NAME)
end

if get_config("cpu_type") == "cpu-ap" then
	includes(TOP_DIR .. "/vendor")
end

-----------------------------------------------------------------------
-- Global definition
-----------------------------------------------------------------------
includes("top_option.lua")

-----------------------------------------------------------------------
-- Base target
-----------------------------------------------------------------------
target("base")
	set_kind("headeronly")
	add_defines(get_global_define(), {public = true})
	add_includedirs(get_global_includedirs(), {public = true})

    local top_dir = TOP_DIR
	
    on_config(function (target)
		if os.getenv("USE_SDK") == "y" then
			if os.exists(target:installdir() .. "/defines.txt") then
				local defines = io.load(target:installdir() .. "/defines.txt")
				target:add("defines", defines, {public = true})
			else
				print("installdir=$(installdir)".. target:installdir())
				os.raise("Error: sdk version need defines.txt")
			end
		end

		try
		{
			function ()
				git_commit = os.iorun("git rev-parse HEAD")
			end,
			catch
			{
				function (errors)
					git_commit = "nogit123456789123456789" 
				end
			}
		}
		git_commit = git_commit:gsub("\n", "") 
		-- 打印Git信息
		-- print("Current Git Commit: " .. git_commit)
		target:add("defines", "OS_GIT_COMMIT=".."\""..git_commit.."\"", {public = true})
    end)
target_end()
