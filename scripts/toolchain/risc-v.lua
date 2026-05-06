-----------------------------------------------------------------------
-- Xmake script file
-- Copyright (C) 2023, Nanjing Innochip Technology Co.,Ltd.
-- All rights reserved.
--
-- @author      geanfeng
-- @file        xmake.lua
-----------------------------------------------------------------------

-- define toolchain
toolchain("risc-v_gcc")

    -- mark as standalone toolchain
    set_kind("standalone")

    -- toolchain directory
    if is_os("linux") then
       -- print("Linux platform")
       set_sdkdir(TOP_DIR .. "/tools/ict-build/nuclei/linux")
    elseif is_os("windows") then
       -- print("Windows platform")
       set_sdkdir(TOP_DIR .. "\\tools\\ict-build\\nuclei\\win")
    else
       print("Unknown compile platform.")
    end

    set_toolset("cc", "riscv-nuclei-elf-ict-gcc")
    set_toolset("cxx", "riscv-nuclei-elf-g++")
    set_toolset("ld", "riscv-nuclei-elf-gcc")
    set_toolset("ar", "riscv-nuclei-elf-ar")
    set_toolset("strip", "riscv-nuclei-elf-strip")
    set_toolset("as", "riscv-nuclei-elf-gcc")
    set_toolset("objcopy", "riscv-nuclei-elf-objcopy")

    on_load(function (toolchain)
        -- add flags
        toolchain:add("arflags", "-D", {force = true})
        toolchain:add("cxflags", "-march=rv32imac -mabi=ilp32 -mcmodel=medlow -mdiv -ffunction-sections -fno-builtin-printf -fno-common -fdata-sections -fshort-enums -fno-builtin -mstrict-align -MMD -MP -msave-restore -fno-unroll-loops -Werror=int-conversion -D_USE_CMPL_GCC -Dgcc -DN310  -Wno-unused-function -Wno-unknown-pragmas -Wno-builtin-macro-redefined -Wno-address -U__FILE__", {force = true})
        toolchain:add("asflags", "-march=rv32imac -mabi=ilp32", {force = true})
        toolchain:add("ldflags", "-march=rv32imac -mabi=ilp32 -Wl,--whole-archive -Wl,--start-group", {force = true})
    end)

toolchain_end()
