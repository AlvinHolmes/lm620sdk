-----------------------------------------------------------------------
-- Xmake script file
-- Copyright (C) 2023, Nanjing Innochip Technology Co.,Ltd.
-- All rights reserved.
--
-- @author      geanfeng
-- @file        xmake.lua
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Option
-----------------------------------------------------------------------
includes("cfg/base_option.lua")
includes("cfg/cust_option.lua")

-----------------------------------------------------------------------
-- Configuration
-----------------------------------------------------------------------
local TARGET_NAME = "main"
local APP_ROOT = path.absolute(path.join(TOP_DIR, "../../.."))
local APP_SRC_DIR = path.join(APP_ROOT, "app")
local APP_BUILD_DIR = path.join(APP_ROOT, "build", "app")
local APP_LIB_NAME = "app"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------
  add_global_define("IMS_SUPPORT")

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
add_global_includedirs("nv")

-----------------------------------------------------------------------
-- Target
-----------------------------------------------------------------------
target(TARGET_NAME)
-- [don't edit] ---
    set_kind("static")
	add_deps("base")
	add_global_deps(TARGET_NAME)
-- [   end    ] ---

    -- Build and link external CMake app library from project root.
    before_build(function (target)
        if not os.exists(path.join(APP_SRC_DIR, "CMakeLists.txt")) then
            os.raise("Error: CMake app source not found: " .. APP_SRC_DIR)
        end

        if not os.exists(APP_BUILD_DIR) then
            os.mkdir(APP_BUILD_DIR)
        end

        local cc = target:tool("cc")
        if not cc then
            os.raise("Error: cannot get C compiler from xmake toolchain")
        end

        local config_args = {
            "-S", APP_SRC_DIR,
            "-B", APP_BUILD_DIR,
            "-DCMAKE_BUILD_TYPE=Release",
            "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY",
            "-DCMAKE_C_COMPILER=" .. cc,
            "-DCMAKE_C_FLAGS=-march=rv32ima_zca_zcb_zcmp_zcmt_zba_zbb_zbc_zbs -mabi=ilp32"
        }

        os.execv("cmake", config_args)
        os.execv("cmake", {"--build", APP_BUILD_DIR, "--target", APP_LIB_NAME, "-j"})
    end)

    add_linkdirs(APP_BUILD_DIR, {public = true})
    add_links(APP_LIB_NAME, {public = true})

    --私有头文件路径
    add_includedirs("inc",{public = true})
	--私有宏定义
	--add_defines("DEBUG")
	--源文件包含
    -- add_includedirs("inc",{public = true})
    add_srcdir("src")
target_end()
