import csv
import os, sys, io

def configMaker(run_number):
	
	runPath = os.path.abspath("launcher_sim.py").split('gemcrs')[0] + 'gemcrs/src/Validation/GEMCosmicMuonStand/test/'
	
	infileName = runPath + "StandGeometryConfiguration_run" + run_number + ".csv"
	
	with open(infileName) as infile:
		for line in infile:
			line = line.split('\n')[0]
			SCtype = line.split(',')[0]
			if (SCtype=='RunNumber'):
				if (line.split(',')[1]!=run_number):
					sys.exit('StandGeometryConfiguration file has something wrong: run rumber not matching...')

	in_name = 'run'
	for i in range(8-len(run_number)):
	    in_name = in_name + '0'
	in_name = in_name + run_number + '_Dummy_Dummy_2018.dat'

	out_name = 'out_run_'
	for i in range(8-len(run_number)):
	    out_name = out_name + '0'
	out_name = out_name + run_number + '.root'

	outfileName = runPath + "configureRun_cfi.py"

	outfile = open(outfileName,"w")

	outfile.write('RunNumber = ' + run_number + '\n\n')

	outfile.write('# Input and output files name definition\n')
	outfile.write('InputFileName = \'' + in_name + '\'\n')
	outfile.write('OutputFileName = \'' + out_name + '\'\n\n')

	outfile.write('# Parameters definition\n')
	outfile.write('minClusterSize = 1\n')
	outfile.write('maxClusterSize = 10\n')
	outfile.write('maxResidual = 5.0 # cm\n')
	outfile.write('trackChi2 = 3\n')
	outfile.write('trackResX = 0.2\n')
	outfile.write('trackResY = 0.3697\n')
	outfile.write('MulSigmaOnWindow = 5\n\n')

	outfile.write('# Stand configuration definition\n')
	StandConfiguration = ['0','0','0','0','0',\
	                      '0','0','0','0','0',\
	                      '0','0','0','0','0']
                          
	with open(infileName) as infile:
	    for line in infile:
	        line = line.split('\n')[0]
	        SCtype = line.split(',')[0]
	        if (SCtype!='RunNumber' and SCtype!='ChamberName'):
	            position = line.split(',')[1]
	            row = int(position.split('/')[0])
	            column = int(position.split('/')[1])
	            SCnumber = (5 * (column - 1)) + (row - 1)
	            StandConfiguration[SCnumber] = (SCtype)[8]			

	outfile.write('StandConfiguration = [\\\n')
	for entry in range(15):
	    if (entry==4 or entry==9):
	            outfile.write('\'' + StandConfiguration[entry] + '\',\\\n')
	    elif (entry==14):
	        outfile.write('\'' + StandConfiguration[entry] + '\']')
	    else:
	        outfile.write('\'' + StandConfiguration[entry] + '\',')

	outfile.close()

	print("\n")
	print("Success: configuration file created for run " + run_number)
	print("\n")

if __name__ == '__main__':
    run_num = sys.argv[1]
    configMaker(run_num)
