-----------------------------------------------------------------------
-- Xmake script file
-- Copyright (C) 2023, Nanjing Innochip Technology Co.,Ltd.
-- All rights reserved.
--
-- @author      geanfeng
-- @file        xmake.lua
-----------------------------------------------------------------------

-- define toolchain
toolchain("risc-v-dl_gcc")

    -- mark as standalone toolchain
    set_kind("standalone")

    -- toolchain directory
    if is_os("linux") then
       -- print("Linux platform")
       set_sdkdir(TOP_DIR .. "/tools/ict-build/nuclei_glibc/linux")
    elseif is_os("windows") then
       -- print("Windows platform")
       set_sdkdir(TOP_DIR .. "\\tools\\ict-build\\nuclei_glibc\\win")
    else
       print("Unknown compile platform.")
    end

    set_toolset("cc", "riscv-nuclei-linux-gnu-gcc")
    set_toolset("cxx", "riscv-nuclei-linux-gnu-g++")
    set_toolset("ld", "riscv-nuclei-linux-gnu-gcc")
    set_toolset("ar", "riscv-nuclei-linux-gnu-ar")
    set_toolset("strip", "riscv-nuclei-linux-gnu-strip")
    set_toolset("as", "riscv-nuclei-linux-gnu-gcc")
    set_toolset("objcopy", "riscv-nuclei-linux-gnu-objcopy")

    on_load(function (toolchain)
        -- add flags
        toolchain:add("cxflags", "-march=rv32imac -mabi=ilp32 -mcmodel=medlow -mdiv -ffunction-sections -fno-builtin-printf -fno-common -fdata-sections -fshort-enums -fno-builtin -mstrict-align -MMD -MP -fno-unroll-loops -Werror=int-conversion -D_USE_CMPL_GCC -Dgcc -DN310  -Wno-unused-function -Wno-unknown-pragmas -Wno-builtin-macro-redefined -Wno-address -fPIC -U__FILE__", {force = true})
        toolchain:add("asflags", "-march=rv32imac -mabi=ilp32", {force = true})
        toolchain:add("ldflags", "-march=rv32imac -mabi=ilp32 -Wl,--gc-sections,-z,max-page-size=0x4 -shared -fPIC -nostartfiles -nostdlib -static-libgcc -Wl,--whole-archive -Wl,--start-group", {force = true})
    end)

toolchain_end()
