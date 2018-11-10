#!/bin/bash
jobName=gemcr

if [ $# != 1 ]; then
    echo "JOB SECTION NUMBER IS MISSING!!!"
    exit 1
fi
SECTION=`printf %%03d $1`

if [ _$CMS_PATH == _ ]; then
  export CMS_PATH=/cvmfs/cms.cern.ch
  source $CMS_PATH/cmsset_default.sh
fi

hostname
tar xzf job.tar.gz
cd gemcr/src/Validation/GEMCosmicMuonStand/test/


scram build ProjectRename
eval `scram runtime -sh`
echo BEGIN `date` cmsRun job_${SECTION}_cfg.py #>> /cms/scratch/quark2930/Work/gemcr/gemcr/src/Validation/GEMCR/test/submit.log
mkdir results
ls -al
time cmsRun runGEMCosmicStand_sim.py runNum=%(runnumber)s eventsPerJob=%(samplesize)s idxJob=${SECTION}
EXITCODE=$?
ls -al
if [ $EXITCODE == 0 ]; then
    echo ENDED `date` cmsRun job_${SECTION}_cfg.py #>> /cms/scratch/quark2930/Work/gemcr/gemcr/src/Validation/GEMCR/test/submit.log
else
    rm -f core.*
    echo TERMINATED_$EXITCODE `date` cmsRun job_${SECTION}_cfg.py #>> /cms/scratch/quark2930/Work/gemcr/gemcr/src/Validation/GEMCR/test/submit.log
    exit 1
fi
echo FINISHED `date` # >> /cms/scratch/quark2930/Work/gemcr/gemcr/src/Validation/GEMCR/test/submit.log
