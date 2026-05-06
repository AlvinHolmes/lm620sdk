import hashlib
import os
import platform
import struct
import sys


def bin_to_one(file_list:list,saveOneBinPath:str):
    with open(saveOneBinPath,"wb") as f:
        fileNum = len(file_list) #合并文件数量
        #print(f"Merged File Number:{fileNum}")
        fileNum2Byte = struct.pack('I',fileNum)
        fileContext = fileNum2Byte
        fileAddr = 0x0
        dataInfo = {}
        addrInfo = []
        for filepath in file_list:
            addrInfo.append(fileAddr)
            #t = filepath.split("/")
            #file_name = t[-1]
            file_name = os.path.basename(filepath)

            if(len(file_name) >= 36):
                raise "file_name can't over 36 characters"

            file_size = os.path.getsize(filepath)
            fileContext+=struct.pack("36s2I",bytes(file_name,"UTF-8"),fileAddr,file_size)
            dataInfo[fileAddr] = [filepath,file_size]
            fileAddr += file_size
        i = 1
        for addr in addrInfo:
            with open(dataInfo[addr][0],"rb") as f_o:
                fileContext+=f_o.read(dataInfo[addr][1])
                f_o.close()
                #print(f"No. {i} : {f_o} Will be Merged.")
                i+=1

        m = hashlib.md5()
        m.update(fileContext)
        hexMsg = m.hexdigest()
        headMsg = hexMsg.encode('utf-8')
        f.write(headMsg+fileContext)
        f.close()
        #print("Merged Completed.")

def read_ini_path(ini_file,one_bin_dir):
    with open(ini_file,"r",encoding="utf-8") as file:
        lines = file.readlines()
        paths = [os.path.join(one_bin_dir, line.strip()) for line in lines]

    return paths

if __name__ == "__main__":
    #for arg in sys.argv[1:]:
    #    print(f"===> Argument: {arg}")

    if len(sys.argv) != 2:
        print("===> ERROR: Not input valid parameter.")
        sys.exit(1)

    input_dir = sys.argv[1]
    print('===> input dir:'   + input_dir)

    version_dir = os.path.join(input_dir, 'bin/version')
    target_dir  = os.path.join(input_dir, 'bin/')
    target_file = os.path.join(input_dir, 'bin/one.bin')
    print('===> version dir:' + version_dir)
    print('===> target dir:'  + target_dir)

    if os.path.exists(version_dir) == True:
        file_list = read_ini_path(os.path.join('','tools/binfiles.ini'), version_dir)
        print(f"File List: {file_list}")

        #linux_format_filelist_array = []
        #for filepath in file_list:
        #    linux_format_filelist = filepath.replace('\\', '/')
        #    linux_format_filelist_array.append(linux_format_filelist)

        #print(f"[ Linux Format ]File List: {linux_format_filelist_array}")
        #bin_to_one(linux_format_filelist_array, target_file)

        bin_to_one(file_list, target_file)

        print("===> making one.bin successed.")
        sys.exit(0)
    else:
        print("===> making one.bin failed.")
        sys.exit(1)

