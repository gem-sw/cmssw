#! /bin/sh

NeventsPerJob=10000
LastJob=99
StartJob=0
#StartJob=0
#NeventsPerJob=$1
#NJob=$2
#TrashDir=/tmp/jslee

#for RUN in 1 2 3 4 5
for RUN in 1
do
  JobId=$StartJob
  #for JobId in 1 69 72 87 88 90 10 12 13 16 17 19 2 20 24 26 28 33 35 4 48 5 6 61 64 66 70 73 80 85 86 9 91 92 99
  while [ $JobId -lt $LastJob ]
  do
    echo run$RUN jobid$JobId
    cp run_bsub.sh run_bsub_${RUN}_${JobId}.sh

    #For batch job
    bsub -q 1nd run_bsub.sh ${NeventsPerJob} ${RUN} ${JobId}

    #For local job
    #./run_bsub.sh ${RUN} ${NeventsPerJob} ${JobId}

    sleep 3
    rm run_bsub_${RUN}_${JobId}.sh
    JobId=`expr $JobId + 1`
  done
done
