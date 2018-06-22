#! /bin/sh

runFile=eff.c

srcDir=/afs/cern.ch/user/j/jslee/gemcrs/src
workDir=$srcDir/Validation/GEMCR/test

cd $srcDir
eval `scramv1 runtime -sh`
#eval `scramv1 runtime -csh`
cd $workDir

root -b -q ${runFile} >> log.txt

