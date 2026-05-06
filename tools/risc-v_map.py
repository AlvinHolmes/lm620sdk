from asyncio.windows_events import NULL
import xlwt
import re

print('Analysis map file to excel file!\n')

book = xlwt.Workbook()

section_sheet = book.add_sheet('section')
section_row_number = 0
section_col_number = 0

symbol_sheet = book.add_sheet('symbol')
symbol_row_number = 0
symbol_col_number = 0

PATH,SHEET_NAME,TEXT_NUM,NON_TEXT_NUM,SHEETOBJECT,ROW = 'path','sheet_name','text_num','non_text_num',"sheet_object","row"
path_sheet =[{PATH:"build/components/net/ip/lwip-2.1.2",
            SHEET_NAME:"LWIP",
            TEXT_NUM:0,
            NON_TEXT_NUM:0,
            SHEETOBJECT:None,
            ROW:0
            }]

def init_sheet():
    """初始化sheet"""
    for item in path_sheet:
        item[SHEETOBJECT] = book.add_sheet(item.get(SHEET_NAME))

def symbol_filter(func): #symbol过滤器
    def wrapper(*args, **kwargs):
        symbol_info= args[0]
        judge_path(symbol_info)
        return func(*args, **kwargs)
    return wrapper

def judge_path(symbol_info:list):
    """判断不同路径写入对应的表格"""
    for sheet in path_sheet:
        if symbol_info[3].startswith(sheet[PATH]): #判断路径是否符合
            col = 0
            for number in symbol_info:
                if number != '':  
                    if col == 0: 
                        if number.startswith(".text"): #第一列判断是否为text,是给text_num加
                            sheet[TEXT_NUM] += int(symbol_info[2],16)
                        else:
                            sheet[NON_TEXT_NUM] += int(symbol_info[2],16)
                    if col ==2:
                        number = int(number,16) #第三列保存格式为10进制
                    sheet[SHEETOBJECT].write(sheet[ROW],col,number) #写入对应的表格
                    col +=1
            sheet[ROW] +=1

def write_count_text():
    """将text数量和data数量写入对应的表格"""
    for sheet in path_sheet:
        sheet[SHEETOBJECT].write(0,5,"text")
        sheet[SHEETOBJECT].write(1,5,sheet[TEXT_NUM])
        sheet[SHEETOBJECT].write(0,6,"data")
        sheet[SHEETOBJECT].write(1,6,sheet[NON_TEXT_NUM])

def section_write(x):
    global section_row_number
    col = 0
    for number in x:
        if number != '':
            section_sheet.write(section_row_number, col, number)
            col = col + 1
    section_row_number = section_row_number + 1
    return

@symbol_filter #symbol过滤器装饰器
def symbol_write(x):
    global symbol_row_number
    col = 0
    for number in x:
        if number != '':
            if col ==2:
                number = int(number,16) #第三列保存格式为10进制
            symbol_sheet.write(symbol_row_number, col, number)
            col = col + 1
    symbol_row_number = symbol_row_number + 1
    return

with open('cpu-ap.map') as file_object:
    init_sheet()
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
    write_count_text()

book.save('cpu-ap_map.xls')