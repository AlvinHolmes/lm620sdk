import os
import sys
import re

if len(sys.argv) != 3:
    print("!!input file error")
    sys.exit(0)
 
map_file = sys.argv[1]
if os.path.exists(map_file) == False:
    print("!!not found map file")
    sys.exit(0)

lib_path = sys.argv[2]
if os.path.exists(lib_path) == False:
    print("!!not found library path")
    sys.exit(0)

lib_sum_list = []

ref_map = {
    "cpu-ap.map":
    {
    "libboard.a":[363,29,300],
    "libshell.a":[3644,419,437],
    "libos.a":[22926,3441,6784],
    "libcpu.a":[5192,818,2132],
    "libfs.a":[18449,1104,1024],
    "libagt.a":[12803,1739,2048],
    "libdrv.a":[47336,5060,13467],
    "libnvm.a":[1350,93,24],
    "libramdump.a":[2078,350,68],
    "libpsm.a":[4034,1186,94],
    "libpub.a":[1976,327,128],
    "libps.a":[361223,29768,32680],
    "liblwip.a":[78141,3304,25157],
    "libnet_startup.a":[20831,6525,360],
    "libbusmux.a":[12947,6523,2840],
    "libat_ps.a":[51795,7260,104],
    "libat_command.a":[12949,1816,26],
    "libota.a":[1008,271,0],
    "libhttp.a":[7470,2276,33],
    "libmqtt.a":[6667,569,4],
    "libcommon.a":[2704,40,64],
    "libgcc.a":[7717,338,0],
    "libm_nano.a":[14,8,0],
    "libg_nano.a":[15062,3770,777],
    "libnosys.a":[120,257,4],
    },
    "cpu-cp.map":
    {
    "libboard.a":[236,29,200],
    "libshell.a":[2914,321,421],
    "libos.a":[14259,2613,1501],
    "libcpu.a":[3778,802,2044],
    "libagt.a":[3944,469,352],
    "libdrv.a":[11124,1104,596],
    "libramdump.a":[16,35,0],
    "libpsm.a":[2098,1014,72],
    "libgcc.a":[4640,316,0],
    "libg_nano.a":[592,257,80],
    "libphy.a":[318101,17183,73074],
    },
}

map_name = map_file.split('/')[-1]
ref_dict = ref_map.get(map_name,{})
for lib_name in os.listdir(lib_path):
    lib_cfg = ref_dict.get(lib_name,[0,0,0])
    lib_sum_list.append({'name':lib_name, 'TEXT':0, 'TEXT_TARGET':lib_cfg[0], 'RODATA': 0, 'RODATA_TARGET': lib_cfg[1], 'DATA': 0, 'DATA_TARGET': lib_cfg[2], 'DEBUG': 0, 'OTHER': 0})

lib_name = "libgcc.a"
lib_cfg = ref_dict.get(lib_name,[0,0,0])
lib_sum_list.append({'name':lib_name, 'TEXT':0, 'TEXT_TARGET':lib_cfg[0], 'RODATA': 0, 'RODATA_TARGET': lib_cfg[1], 'DATA': 0, 'DATA_TARGET': lib_cfg[2], 'DEBUG': 0, 'OTHER': 0})

lib_name = "libm_nano.a"
lib_cfg = ref_dict.get(lib_name,[0,0,0])
lib_sum_list.append({'name':lib_name, 'TEXT':0, 'TEXT_TARGET':lib_cfg[0], 'RODATA': 0, 'RODATA_TARGET': lib_cfg[1], 'DATA': 0, 'DATA_TARGET': lib_cfg[2], 'DEBUG': 0, 'OTHER': 0})

lib_name = "libg_nano.a"
lib_cfg = ref_dict.get(lib_name,[0,0,0])
lib_sum_list.append({'name':lib_name, 'TEXT':0, 'TEXT_TARGET':lib_cfg[0], 'RODATA': 0, 'RODATA_TARGET': lib_cfg[1], 'DATA': 0, 'DATA_TARGET': lib_cfg[2], 'DEBUG': 0, 'OTHER': 0})

lib_name = "libnosys.a"
lib_cfg = ref_dict.get(lib_name,[0,0,0])
lib_sum_list.append({'name':lib_name, 'TEXT':0, 'TEXT_TARGET':lib_cfg[0], 'RODATA': 0, 'RODATA_TARGET': lib_cfg[1], 'DATA': 0, 'DATA_TARGET': lib_cfg[2], 'DEBUG': 0, 'OTHER': 0})

def section_write(section_info):
    section_name = section_info[0]
    section_base = section_info[1]
    section_size = section_info[2]
    print("%-32s" % section_name + "%-32s" % section_base + "%-32s" % section_size + "%-32s" % (int(section_size,16)) )
    return

def symbol_write(symbol_info):
    for lib_item in lib_sum_list:
        if symbol_info[3].find(lib_item['name']) > 0 : #判断路径是否符合
            if symbol_info[0].startswith(".text") or symbol_info[0].startswith(".itcm") or symbol_info[0].startswith(".iram.code") or symbol_info[0].startswith(".iram.psm_recode") or symbol_info[0].startswith(".iram.recode") or symbol_info[0].startswith(".init") or symbol_info[0].startswith(".psram.code") : #第一列判断是否为text,是给text_num加
                lib_item['TEXT'] += int(symbol_info[2],16)
            elif symbol_info[0].startswith(".rodata") or symbol_info[0].startswith(".srodata") or symbol_info[0].startswith(".dtcm.rodata") or symbol_info[0].startswith(".iram.rodata") or symbol_info[0].startswith(".nr_shell_cmd") or symbol_info[0].startswith(".at_cmd_table") or symbol_info[0].startswith(".psm_") or symbol_info[0].startswith(".psram.rodata") or symbol_info[0].startswith(".shutdown_call") or symbol_info[0].startswith(".xip.rodata"):
                lib_item['RODATA'] += int(symbol_info[2],16)
            elif symbol_info[0].startswith(".data") or symbol_info[0].startswith(".sdata") or symbol_info[0].startswith(".bss") or symbol_info[0].startswith(".sbss") or symbol_info[0].startswith(".dtcm.srdata") or symbol_info[0].startswith(".dtcm.rtdata") or symbol_info[0].startswith(".iram.srdata") or symbol_info[0].startswith(".iram.rtdata") or symbol_info[0].startswith(".iram.sharedata") or symbol_info[0].startswith(".iram.psm_redata") or symbol_info[0].startswith(".iram.redata") :
                lib_item['DATA'] += int(symbol_info[2],16)
            elif symbol_info[0].startswith(".psram.debugbss"):
                lib_item['DEBUG'] += int(symbol_info[2],16)
            else:
                lib_item['OTHER'] += int(symbol_info[2],16)
                print('OTHER: ' + lib_item['name'] + '->' + symbol_info[0])
    return

total_map = {
    "cpu-ap.map":
    {
    "plat":[0,0,0,0],
    "lwip":[0,0,0,0],
    "ps":[0,0,0,0],
    },
    "cpu-cp.map":
    {
    "plat":[0,0,0,0],
    "phy":[0,0,0,0],
    },
}

def print_lib_sum():
    map_name = map_file.split('/')[-1]
    total_dict = total_map.get(map_name,{})

    print("%-32s" % 'NAME' + "%-16s" % 'TEXT' + "%-16s" % 'TARGET' +  "%-16s" % 'RODATA' +  "%-16s" % 'TARGET'+ "%-16s" % 'DATA' + "%-16s" % 'TARGET' + "%-16s" % 'DEBUG' + "%-16s" % 'OTHER')
    for lib_item in lib_sum_list:
        if lib_item['TEXT'] > 0 or lib_item['RODATA'] > 0 or lib_item['DATA'] > 0 or lib_item['DEBUG'] > 0 or lib_item['OTHER'] > 0:
            print("%-32s" % lib_item['name'] + "%-16s" % lib_item['TEXT'] + "%-16s" % lib_item['TEXT_TARGET'] + "%-16s" % lib_item['RODATA'] + "%-16s" % lib_item['RODATA_TARGET'] + "%-16s" % lib_item['DATA'] + "%-16s" % lib_item['DATA_TARGET'] + "%-16s" % lib_item['DEBUG'] + "%-16s" % lib_item['OTHER'])
        if map_name == "cpu-ap.map":
            if lib_item['name'] == "libps.a" or lib_item['name'] == "libat_ps.a":
                module_dict = total_dict.get("ps",[0,0,0,0])
                module_dict[0] += lib_item['TEXT'] + lib_item['RODATA']
                module_dict[1] += lib_item['TEXT_TARGET'] + lib_item['RODATA_TARGET']
                module_dict[2] += lib_item['DATA']
                module_dict[3] += lib_item['DATA_TARGET']
            elif lib_item['name'] == "liblwip.a":
                module_dict = total_dict.get("lwip",[0,0,0,0])
                module_dict[0] += lib_item['TEXT'] + lib_item['RODATA']
                module_dict[1] += lib_item['TEXT_TARGET'] + lib_item['RODATA_TARGET']
                module_dict[2] += lib_item['DATA']
                module_dict[3] += lib_item['DATA_TARGET']
            else:
                module_dict = total_dict.get("plat",[0,0,0,0])
                module_dict[0] += lib_item['TEXT'] + lib_item['RODATA']
                module_dict[1] += lib_item['TEXT_TARGET'] + lib_item['RODATA_TARGET']
                module_dict[2] += lib_item['DATA']
                module_dict[3] += lib_item['DATA_TARGET']
        else:
            if lib_item['name'] == "libphy.a":
                module_dict = total_dict.get("phy",[0,0,0,0])
                module_dict[0] += lib_item['TEXT'] + lib_item['RODATA']
                module_dict[1] += lib_item['TEXT_TARGET'] + lib_item['RODATA_TARGET']
                module_dict[2] += lib_item['DATA']
                module_dict[3] += lib_item['DATA_TARGET']
            else:
                module_dict = total_dict.get("plat",[0,0,0,0])
                module_dict[0] += lib_item['TEXT'] + lib_item['RODATA']
                module_dict[1] += lib_item['TEXT_TARGET'] + lib_item['RODATA_TARGET']
                module_dict[2] += lib_item['DATA']
                module_dict[3] += lib_item['DATA_TARGET']
    print("\n                                             <<<< TOTAL (UNIT: BYTE) >>>>")
    print("%-32s" % 'NAME' + "%-16s" % 'TEXT+RODATA' + "%-16s" % 'TARGET' +  "%-16s" % 'DATA' +  "%-16s" % 'TARGET')
    if map_name == "cpu-ap.map":
        module_dict = total_dict.get("plat",[0,0,0,0])
        print("%-32s" % 'plat' + "%-16s" % module_dict[0] + "%-16s" % module_dict[1] + "%-16s" % module_dict[2] + "%-16s" % module_dict[3])
        module_dict = total_dict.get("lwip",[0,0,0,0])
        print("%-32s" % 'lwip' + "%-16s" % module_dict[0] + "%-16s" % module_dict[1] + "%-16s" % module_dict[2] + "%-16s" % module_dict[3])
        module_dict = total_dict.get("ps",[0,0,0,0])
        print("%-32s" % 'ps' + "%-16s" % module_dict[0] + "%-16s" % module_dict[1] + "%-16s" % module_dict[2] + "%-16s" % module_dict[3])
    else:
        module_dict = total_dict.get("plat",[0,0,0,0])
        print("%-32s" % 'plat' + "%-16s" % module_dict[0] + "%-16s" % module_dict[1] + "%-16s" % module_dict[2] + "%-16s" % module_dict[3])
        module_dict = total_dict.get("phy",[0,0,0,0])
        print("%-32s" % 'phy' + "%-16s" % module_dict[0] + "%-16s" % module_dict[1] + "%-16s" % module_dict[2] + "%-16s" % module_dict[3])
    return

print("\n                                             <<<< SECTIONS (UNIT: BYTE) >>>>")
print("%-32s" % 'NAME' + "%-32s" % 'BASE (HEX)' + "%-32s" % 'SIZE (HEX)' + "%-32s" % 'SIZE (DEC)')

with open(map_file) as file_object:
    last_line = ''
    for line in file_object:
        #  匹配段
        if re.match( r'^\S+\s+0x\S+', line, re.I):
            x = re.split(r'\s+', line)
            if int(x[1], 16) > 0  and  int(x[2], 16) > 0 :
                # print(x)
                section_write(x)
        
        #  匹配符号
        if last_line:
            x = re.split(r'\s+', last_line.strip() + line)
            if re.match(r'^0x', x[1]) and re.match(r'^0x', x[2]) and int(x[1], 16) > 0  and  int(x[2], 16) > 0 :
                symbol_write(x)

        if re.match( r'^\s\S+\n', line, re.I):
            last_line = line
        else:
            last_line = ''

        if re.match( r'^\s\S+', line, re.I):
            x = re.split(r'\s+', line.strip())
            if len(x) > 3 and re.match(r'^0x', x[1]) and re.match(r'^0x', x[2]) and int(x[1], 16) > 0  and  int(x[2], 16) > 0 :
                symbol_write(x)

        if re.match( r'^OUTPUT', line, 0):
            break

print("\n                                             <<<< LIBS (UNIT: BYTE) >>>>")
print_lib_sum()
print("")