-----------------------------------------------------------------------
-- Xmake script file
-- Copyright (C) 2023, Nanjing Innochip Technology Co.,Ltd.
-- All rights reserved.
--
-- @author      geanfeng
-- @file        xmake.lua
-----------------------------------------------------------------------

-- define toolchain
toolchain("ZCrisc-v_gcc")

    -- mark as standalone toolchain
    set_kind("standalone")

    -- toolchain directory
    if is_os("linux") then
       -- print("Linux platform")
       set_sdkdir(TOP_DIR .. "/tools/ict-build/ZCnuclei/linux")
    elseif is_os("windows") then
       -- print("Windows platform")
       set_sdkdir(TOP_DIR .. "\\tools\\ict-build\\ZCnuclei\\win")
    else
       print("Unknown compile platform.")
    end

    -- toolset
    set_toolset("cc", "riscv64-unknown-elf-ict-gcc")
    set_toolset("cxx", "riscv64-unknown-elf-g++")
    set_toolset("ld", "riscv64-unknown-elf-gcc")
    set_toolset("ar", "riscv64-unknown-elf-ar")
    set_toolset("strip", "riscv64-unknown-elf-strip")
    set_toolset("as", "riscv64-unknown-elf-gcc")
    set_toolset("objcopy", "riscv64-unknown-elf-objcopy")

    on_load(function (toolchain)
        -- add flags
        toolchain:add("arflags", "-D", {force = true})
        toolchain:add("cxflags", "-march=rv32ima_zca_zcb_zcmp_zcmt_zba_zbb_zbc_zbs_xxldspn3x -mabi=ilp32 -mcmodel=medlow -mdiv -ffunction-sections -fno-builtin-printf -fno-common -fdata-sections -fshort-enums -fno-builtin -mstrict-align -MMD -MP -msave-restore -fno-unroll-loops -fno-strict-aliasing -Werror=int-conversion -D_USE_CMPL_GCC -Dgcc -DN310 -DRISCV_DSP -D_GLIBCXX_INCLUDE_NEXT_C_HEADERS -Wno-unused-function -Wno-unknown-pragmas -Wno-builtin-macro-redefined -Wno-address -Wno-comment -U__FILE__ -gdwarf-4 -gstrict-dwarf", {force = true})
        toolchain:add("asflags", "-march=rv32ima_zca_zcb_zcmp_zcmt_zba_zbb_zbc_zbs_xxldspn3x -mabi=ilp32 -gdwarf-4 -gstrict-dwarf", {force = true})
        toolchain:add("ldflags", "-march=rv32ima_zca_zcb_zcmp_zcmt_zba_zbb_zbc_zbs -mabi=ilp32 -gdwarf-4 -gstrict-dwarf -Wl,--whole-archive -Wl,--start-group", {force = true})
    end)

toolchain_end()
