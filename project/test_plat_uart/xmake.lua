-----------------------------------------------------------------------
-- Xmake project: test_plat_uart
-- Compiles the plat UART unit tests for lm620sdk (ICT2110/openphone).
--
-- Usage (from SDK root):
--   PROJECT_NAME=test_plat_uart xmake -r -w
--
-- On hardware: connect serial console and type "uart_test<Enter>"
-- to run the Unity test suite.
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Options
-----------------------------------------------------------------------
includes("cfg/base_option.lua")
includes("cfg/cust_option.lua")

-----------------------------------------------------------------------
-- Paths relative to SDK root (TOP_DIR = os.curdir() = SDK root)
-----------------------------------------------------------------------
local REPO_ROOT       = TOP_DIR .. "/../../.."
local UNITY_SRC       = REPO_ROOT .. "/third_party/unity/src"
local PLAT_INCLUDE    = REPO_ROOT .. "/platform/include"
local LM620_UART_SRC  = REPO_ROOT .. "/platform/implement/dev/lm620"
local TEST_SRC        = REPO_ROOT .. "/tests/unit/test_plat_uart"
local DRV_INC         = TOP_DIR  .. "/components/driver/include"
local DRV_CHIP_INC    = TOP_DIR  .. "/components/driver/include/chip"
local OS_INC          = TOP_DIR  .. "/library/volte/include/os"
local CMN_INC         = TOP_DIR  .. "/library/volte/include/cmn"
local BOARD_INC       = TOP_DIR  .. "/board/openphone"

-----------------------------------------------------------------------
-- Global NV include
-----------------------------------------------------------------------
add_global_includedirs("nv")

-----------------------------------------------------------------------
-- Target: main (standard SDK user-app static lib)
-----------------------------------------------------------------------
target("main")
-- [don't edit] ---
    set_kind("static")
    add_deps("base")
    add_global_deps("main")
-- [   end    ] ---

    -- plat public headers
    add_includedirs(PLAT_INCLUDE, {public = false})

    -- Unity test framework
    add_files(UNITY_SRC .. "/unity.c", {defines = '__FILE__="unity"'})
    add_includedirs(UNITY_SRC, {public = false})

    -- plat_uart lm620sdk backend
    add_files(LM620_UART_SRC .. "/lm620_uart.c", {defines = '__FILE__="lm620_uart"'})
    add_includedirs(DRV_INC, {public = false})
    add_includedirs(DRV_CHIP_INC, {public = false})
    add_includedirs(OS_INC, {public = false})
    add_includedirs(CMN_INC, {public = false})
    add_includedirs(BOARD_INC, {public = false})

    -- Unity test file (compiled with PLAT_LM620SDK_TEST → NR_SHELL_CMD_EXPORT entry)
    add_defines("PLAT_LM620SDK_TEST")
    add_files(TEST_SRC .. "/test_plat_uart.c", {defines = '__FILE__="test_plat_uart"'})

target_end()
