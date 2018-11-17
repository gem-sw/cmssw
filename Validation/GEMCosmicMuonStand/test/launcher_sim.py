import csv
import os
import sys
import io
import subprocess
import time

if __name__ == '__main__':

    run_number = sys.argv[1]
    xlsx_csv_conversion_flag = sys.argv[2]
    
    # Different paths definition
    srcPath = os.path.abspath("launcher_sim.py").split('gemcrs')[0]+'gemcrs/src/'
    pyhtonModulesPath = os.path.abspath("launcher_sim.py").split('gemcrs')[0]+'gemcrs/src/Validation/GEMCosmicMuonStand/python/'
    runPath = os.path.abspath("launcher_sim.py").split('gemcrs')[0] + 'gemcrs/src/Validation/GEMCosmicMuonStand/test/'
    resDirPath = os.path.abspath("launcher_sim.py").split('gemcrs')[0]
    
    sys.path.insert(0,pyhtonModulesPath)
    
    import configureRun_cfi as runConfig
    import config_creator
    import geometry_files_creator
    
    # Conversion from excel to csv files
    if (xlsx_csv_conversion_flag == "xlsxTOcsv=ON"):
    	import excel_to_csv
    	fileToBeConverted = runPath + "StandGeometryConfiguration_run" + run_number + ".xlsx"
    	excel_to_csv.conversion(fileToBeConverted)
    	fileToBeConverted = runPath + "StandAlignmentValues_run" + run_number + ".xlsx"
    	excel_to_csv.conversion(fileToBeConverted)
    
    # Generate configuration file
    config_creator.configMaker(run_number)
    time.sleep(1)
    
    # Generate geometry files
    geometry_files_creator.geomMaker(run_number)
    time.sleep(1)
    
    # Compiling after the generation of the geometry files
    scramCommand = "scramv1 b -j 8"
    scramming = subprocess.Popen(scramCommand.split(),stdout=subprocess.PIPE,universal_newlines=True,cwd=srcPath)
    while scramming.poll() is None:
    	line = scramming.stdout.readline()
    	print(line)
	print scramming.stdout.read()
    scramming.communicate()
    time.sleep(1)
    
    # Running the CMSSW code
    runCommand = "cmsRun runGEMCosmicStand_sim.py"
    running = subprocess.Popen(runCommand.split(),stdout=subprocess.PIPE,universal_newlines=True,cwd=runPath)
    while running.poll() is None:
    	line = running.stdout.readline()
    	print(line)
	print running.stdout.read()
    running.communicate()
    time.sleep(1)
    
    # Creating folder outside the CMMSW release to put the output files and plots
    outDirName = "Results_QC8_run_"+run_number
    #---# Remove old version if want to recreate
    if (os.path.exists(resDirPath+outDirName)):
    	rmDirCommand = "rm -rf "+outDirName
    	rmDir = subprocess.Popen(rmDirCommand.split(),stdout=subprocess.PIPE,universal_newlines=True,cwd=resDirPath)
    	rmDir.communicate()
    #---# Create the new empty folder
    resDirCommand = "mkdir "+outDirName
    resDir = subprocess.Popen(resDirCommand.split(),stdout=subprocess.PIPE,universal_newlines=True,cwd=resDirPath)
    resDir.communicate()
    time.sleep(1)
    
    # Create folders for ouput plots per chamber
    SuperChType = runConfig.StandConfiguration
    effoutDir = os.path.abspath("launcher_sim.py").split('gemcrs')[0] + outDirName
    for i in range (0,30):
    	if (SuperChType[int(i/2)] != '0'):
    		plotsDirCommand = "mkdir outPlots_Chamber_Pos_" + str(i)
    		plotsDirChamber = subprocess.Popen(plotsDirCommand.split(),stdout=subprocess.PIPE,universal_newlines=True,cwd=effoutDir)
    		plotsDirChamber.communicate()
    time.sleep(1)
    
    # Selecting the correct output file, changing the name and moving to the output folder
    out_name = 'out_run_'
    for i in range(8-len(run_number)):
        out_name = out_name + '0'
    out_name = out_name + run_number + '.root'
    
    mvCommand = "mv temp_" + out_name + " " + out_name
    moving = subprocess.Popen(mvCommand.split(),stdout=subprocess.PIPE,universal_newlines=True,cwd=runPath)
    moving.communicate()
    time.sleep(1)
    
    mvToDirCommand = "mv " + out_name + " " + resDirPath+outDirName + "/" + out_name
    movingToDir = subprocess.Popen(mvToDirCommand.split(),stdout=subprocess.PIPE,universal_newlines=True,cwd=runPath)
    movingToDir.communicate()
    time.sleep(1)
    
    # Efficiency computation & output
    effCommand = "root -l -q " + runPath + "efficiency_calculation.c(" + run_number + ",\"" + runPath + "\")"
    efficiency = subprocess.Popen(effCommand.split(),stdout=subprocess.PIPE,universal_newlines=True,cwd=effoutDir)
    while efficiency.poll() is None:
    	line = efficiency.stdout.readline()
    	print(line)
	print efficiency.stdout.read()
    efficiency.communicate()
    time.sleep(1)
    
