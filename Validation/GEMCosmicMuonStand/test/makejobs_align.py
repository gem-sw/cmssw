#import sys, opt
import sys, os

strJobNum = sys.argv[ 1 ]
strJobName = "res_" + strJobNum

strNumJob = "40"
strEvtNum = "1000"

if not os.path.isdir(strJobName): 
  os.mkdir(strJobName)
  open(strJobName + "/.create-batch", "w").write("")

open("run_align.sh", "w").write(open("run_template_align.sh", "r").read()%{"runnumber": strJobNum, "samplesize": strEvtNum})
open("submit.jds", "w").write(open("submit_template.jds", "r").read()%{"jobname": strJobName, "jobnum": strNumJob})

os.system("./full_archive.sh gemcr ; echo '@@ Archiving is complete' ; condor_submit submit.jds"%{"runnumber": strJobNum})


