import csv
import os.path
import os, sys, io

def geometry_files_creator(run_number):
    import configureRun_cfi as runConfig
    
    if (runConfig.RunNumber!=int(run_number)):
        sys.exit('configureRun_cfi file has something wrong: run rumber not matching... Check it: ' + run_number)
    
    if (os.path.exists("StandAlignmentValues_run%u.csv"%int(run_number))):
        with open("StandAlignmentValues_run%u.csv"%int(run_number)) as infile:
            for line in infile:
                line = line.split('\n')[0]
                SCtype = line.split(',')[0]
                if (SCtype=='RunNumber'):
                    if (line.split(',')[1]!=run_number):
                        sys.exit('StandAlignmentValues file has something wrong: run rumber not matching...')

    path = os.path.abspath("geometry_files_creator.py")
    pre_path = path.split('gemcrs')[0]
    geom_path = pre_path + 'gemcrs/src/Geometry/MuonCommonData/data/GEMQC8/'
    
    dx = [0,0,0,0,0,\
          0,0,0,0,0,\
          0,0,0,0,0]
    rz = [0,0,0,0,0,\
          0,0,0,0,0,\
          0,0,0,0,0]

    if (os.path.exists("StandAlignmentValues_run%u.csv"%int(run_number))):
        with open("StandAlignmentValues_run%u.csv"%int(run_number)) as infile:
            for line in infile:
                if ('\n' in line):
                    line = line.split('\n')[0]
                if ('\r' in line):
                    line = line.split('\r')[0]
                if (line.split(',')[0]!='RunNumber' and line.split(',')[0]!='Position'):
                    position = line.split(',')[0]
                    row = int(position.split('/')[0])
                    column = int(position.split('/')[1])
                    SCnumber = (5 * (column - 1)) + (row - 1)
                    dx[SCnumber] = float(line.split(',')[1])
                    rz[SCnumber] = float(line.split(',')[6])

    for i in xrange(15):
        column = int(i/5)+1
        row = int(i%5)+1
        if (runConfig.StandConfiguration[i] != '0'):
		    outfile_name = geom_path + 'gem11' + str(runConfig.StandConfiguration[i]) + '_c' + str(column) + '_r' + str(row) + '.xml'
		    with open(outfile_name,"w+") as outfile:
		        with open(geom_path + "gem11_geom_template.xml") as template:
		            for line in template:
		                if ('"dxSC"' in line):
		                    line = line.split('0')[0] + str((2-column)*56.0) + line.split('0')[1]
		                if ('"dySC"' in line):
		                    if (runConfig.StandConfiguration[i] == 'S'):
		                        line = line.split('0')[0] + str(30.0) + line.split('0')[1]
		                    if (runConfig.StandConfiguration[i] == 'L'):
		                        line = line.split('0')[0] + str(22.0) + line.split('0')[1]
		                if ('"dzSC"' in line):
		                    line = line.split('0')[0] + str(row*23.0) + line.split('0')[1]
		                if ('"dx"' in line):
		                    line = line.split('0')[0] + str(dx[i]) + line.split('0')[1]
		                if ('"rz"' in line):
		                    line = line.split('0')[0] + str(rz[i]) + line.split('0')[1]
		                if ('(first_ch_num)' in line):
		                    line = line.split('(first_ch_num)')[0] + str((i+1)*2-1) + line.split('(first_ch_num)')[1]
		                if ('(second_ch_num)' in line):
		                    line = line.split('(second_ch_num)')[0] + str((i+1)*2) + line.split('(second_ch_num)')[1]
		                if ('(type)' in line):
		                    line = line.split('(type)')[0] + str(runConfig.StandConfiguration[i]) + line.split('(type)')[1]
		                outfile.write(line)
                    
    print("\n")
    print("Success: geometry files created for run " + run_number)
    print("\n")

if __name__ == '__main__':
    run_num = sys.argv[1]
    geometry_files_creator(run_num)


