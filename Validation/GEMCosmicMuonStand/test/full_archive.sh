workspacename=$1

mainpath=`pwd`
workspacepath=$CMSSW_BASE

tar cf ${mainpath}/job.tar $workspacepath --xform="s:$CMSSW_BASE:$workspacename:" -P --exclude tmp --exclude 'job.tar' --exclude 'job.tar.gz' --exclude 'job_old.tar.gz' --exclude-vcs --exclude-tag-all=.create-batch --exclude-tag-all=.requestcache --exclude=*.root --exclude=*.log --exclude=*.err --exclude=*.png
#tar ff ${mainpath}/job.tar ${workspacepath}/$1 --xform="s:$CMSSW_BASE:$workspacename:" -P --exclude tmp --exclude 'job.tar' --exclude 'job.tar.gz' --exclude-vcs --exclude=*.root

echo "@# Starting compression"

mv job.tar.gz job_old.tar.gz
gzip job.tar 
rm job_old.tar.gz
