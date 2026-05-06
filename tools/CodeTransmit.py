# 编码转换工具，将路径下所有指定文件都转为utf-8-bom格式
#!/usr/bin/env python3
# -*- coding:utf-8 -*-
import os 
import sys 
import codecs 
import chardet 

gErrArr = []

def convert(fileName, filePath, out_enc="utf-8-sig"): 
	try: 
		content=codecs.open(filePath,'rb').read()
		src_encoding=chardet.detect(content)['encoding']
		print("file_name:%s \t\t\t\tfile_encoding:%s" %(fileName, src_encoding))
		
		if src_encoding != None:
			if src_encoding == out_enc:
				return
			content=content.decode(src_encoding).encode(out_enc)
			codecs.open(filePath,'wb').write(content)
		else :
			if os.stat(filePath).st_size != 0:
			    gErrArr.append("can not recognize file encoding %s size %s" %(filePath, os.stat(filePath).st_size))
	except Exception as err: 
		gErrArr.append("%s:%s"%(filePath, err))
  
def explore(dir): 
	print("===============================================================")
	print("{0:50}{1}".format('file_name', 'file_encoding'))
	print("===============================================================")

	for root,dirs,files in os.walk(dir): 
		for file in files:
			suffix = os.path.splitext(file)[1]
			suffix = suffix.lower()
			if (suffix == '.h' or suffix == '.c' or suffix == '.cpp' or suffix == '.hpp'
			or suffix == '.cxx' or suffix == '.cc' or suffix == '.txt' or suffix == '.bat' or suffix == '.lua'
			or suffix == '.ini' or suffix == '.mk' or file.lower() == 'makefile' 
			or file.lower() == 'SConscript'.lower() or file.lower() == 'Kconfig'.lower()):    
			    path=os.path.join(root,file)
			    convert(file, path)
  
def main(): 
    print("\r\n########### Code Transmit Start!!! ###########\r\n")		
    #filePath=""
    #explore(filePath)
    #filePath=os.path.abspath('..') # 获取当前文件目录的父目录
    filePath="/home/dengningkun/work/2110_FPGA_temp"
    print("file_path:%s\r\n" %(filePath))
    explore(filePath)
    print('\r\n---------Error Statistics------------')
    for index,item in enumerate(gErrArr):
	    print(item)
    print('\r\n        total %d errors！'%(len(gErrArr)))
    if(len(gErrArr) > 0):
	    print("pls use notepad++ to manually convert the error files!!!")
    print('\r\n-----------------------------')
    print("\r\n########### Code Transmit End!!! ###########\r\n")

if __name__=="__main__":
    main()
		
