import xlrd, openpyxl
import csv
import os, sys, io

def excel_to_csv(in_name):
    if in_name.endswith('.xls'):
        with xlrd.open_workbook(in_name) as infile:
            sh = infile.sheet_by_index(0)  # or wb.sheet_by_name('name_of_the_sheet_here')
            out_name = in_name[:-4] + '.csv'
            if sys.version_info[0] == 2: # python2
                with open(out_name, 'wb') as outfile:
                    c = csv.writer(outfile)
                    for r in range(sh.nrows):
                        c.writerow(sh.row_values(r))
            if sys.version_info[0] == 3: # python3
                with open(out_name, 'w', newline="") as outfile:
                    c = csv.writer(outfile)
                    for r in range(sh.nrows):
                        c.writerow(sh.row_values(r))

    if in_name.endswith('.xlsx'):
        infile = openpyxl.load_workbook(in_name)
        sh = infile.active
        out_name = in_name[:-5] + '.csv'
        if sys.version_info[0] == 2: # python2
            with open(out_name, 'wb') as outfile:
                c = csv.writer(outfile)
                for r in sh.rows:
                    c.writerow([cell.value for cell in r])
        if sys.version_info[0] == 3: # python3
            with open(out_name, 'w', newline="") as outfile:
                c = csv.writer(outfile)
                for r in sh.rows:
                    c.writerow([cell.value for cell in r])
                    
	print("\n")
	print("Success: " + in_name + " converted to " + outname)
	print("\n")

if __name__ == '__main__':
    input_file = sys.argv[1]
    excel_to_csv(input_file)
