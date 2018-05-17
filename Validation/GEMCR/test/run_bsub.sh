#! /bin/sh

NeventsPerJob=$1
RUN=$2
JobId=$3
srcFile=runGEMCosmicStand_sim.py

srcDir=/home/home1/institut_3a/mocellin/gemcrs/src
workDir=$srcDir/Validation/GEMCR/test
tempDir=/tmp/jslee
#runDir=$workDir
#resultDir=$workDir/Tree
resultDir=$srcDir/Validation/GEMCR/test/results

runFile=runGEMCosmicStand_sim_temp_${RUN}_${JobId}.py

cd $srcDir
eval `scramv1 runtime -sh`
#eval `scramv1 runtime -csh`
#cmsenv
cd $workDir

#mkdir $resultDir

sed -e 's/"runNum",1,/"runNum",'${RUN}',/' \
    -e 's/"eventsPerJob",10000,/"eventsPerJob",'${NeventsPerJob}',/' \
    -e 's/"idxJob","-1",/"idxJob","'${JobId}'",/' \
    -e 's/out_reco_MC.root/out_reco_MC_'${RUN}'_'${JobId}'.root/' \
    -e 's#file:#file:'${tempDir}'/#' \
    -e 's#temp_#'${tempDir}'/temp_#' ${srcFile} > ${runFile}
    #-e 's/out_reco_MC.root/out_reco_MC_'${RUN}'_'${JobId}'.root/' runGEMCosmicStand_sim.py > ${runFile}

#mv $runFile $runDir
#cd $runDir
chmod 777 ${runFile}
cmsRun ${runFile}
rm ${tempDir}/out_reco_MC_${RUN}_${JobId}.root
mv ${tempDir}/temp_out_reco_MC_${RUN}_${JobId}.root ${resultDir}
rm ${runFile}

#sleep 5
fileName=`find . -type f -name 'core.*'`
for file in $fileName
do
  rm $file
done
LSFjobName=`find . -type d -name 'LSFJOB_*'`
for file in $LSFjobName
do
  rm -rf ${file}
done

