#! /bin/sh

runFile=eff.c

srcDir=/home/home1/institut_3a/mocellin/gemcrs/src
workDir=$srcDir/Validation/GEMCR/test

cd $srcDir
eval `scramv1 runtime -sh`
cd $workDir

root -b -q ${runFile} >> log.txt

