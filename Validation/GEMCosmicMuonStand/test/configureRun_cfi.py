#Configuration for unpacker
import csv,os

#RAWFileName="/cms/scratch/quark2930/Work/gemcr/gemcr/src/Validation/GEMCR/test/run000002_LocalRun_CERNP5_2017-06-17.dat"
RAWFileName="/afs/cern.ch/work/g/gmocelli/PhD/CRS/gemcrs/src/Validation/GEMCR/run000001_Dummy_Dummy_2018.dat"
MaxEvents=-1

arrRAW = [
    "", 
    "", 
    "/cms/scratch/quark2930/Work/gemcr/gemcr/src/Validation/GEMCR/test/run000002_LocalRun_CERNP5_2017-06-17.dat", 
    "/cms/scratch/quark2930/Work/gemcr/gemcr/src/Validation/GEMCR/test/run000003_LocalRun_CERNP5_2017-06-17.dat", 
    "/cms/scratch/quark2930/Work/gemcr/gemcr/src/Validation/GEMCR/test/run000004_LocalRun_CERNP5_2017-06-19.dat", 
    "/cms/scratch/quark2930/Work/gemcr/gemcr/src/Validation/GEMCR/test/run000005_LocalRun_CERNP5_2017-06-19.dat", 
    "/cms/scratch/quark2930/Work/gemcr/gemcr/src/Validation/GEMCR/test/run000006_LocalRun_CERNP5_2017-06-19.dat", 
    "/cms/scratch/quark2930/Work/gemcr/gemcr/src/Validation/GEMCR/test/run000007_LocalRun_CERNP5_2017-06-20.dat", 
]

if os.path.exists("rawidx.txt"):
  nIdx = int(open("rawidx.txt", "r").read())
  if nIdx < len(arrRAW): 
    RAWFileName = arrRAW[ nIdx ]
    MaxEvents = os.path.getsize(RAWFileName) / 320
  else: 
    RAWFileName = "run%i"%nIdx

makeTrack = True
minClusterSize = 1
maxClusterSize = 10
maxResidual = 5.0 # cm 
trackChi2 = 3
trackResX = 0.3792
trackResY = 0.3697
MulSigmaOnWindow = 5
tag = ""
########################################################################################
# 
########################################################################################
RunNumber=int(RAWFileName.split("/")[-1].split("_")[0][3:])

ratePlot = False
GEMMask = "Validation/GEMCR/data/GEMMaskVecRun%06d.dat"%RunNumber
GEMHot = "Validation/GEMCR/data/GEMHotVecRun%06d.dat"%RunNumber

if os.path.isfile(os.environ['SRT_CMSSW_BASE_SCRAMRTDEL']+"/src/"+GEMHot):
   runWithMasking = True
else : 
  runWithMasking = False
if runWithMasking :
   makeMaskList = False
else :
   makeMaskList = True
if not runWithMasking :
  GEMMask = "Validation/GEMCR/data/GEMMaskEmp.dat" 
  GEMHot = "Validation/GEMCR/data/GEMMaskEmp.dat"

runWithMasking = False
makeMaskList = False

if runWithMasking : print "This run is running with masking"
else : print  "This run dose not have mask lists" 

if makeTrack and runWithMasking : OutputFileName='Reco_Run%06d.root'%RunNumber
elif runWithMasking : OutputFileName='Reco_Run%06d_maskedRecHit.root'%RunNumber
else : OutputFileName='Reco_Run%06d_test.root'%RunNumber

if ratePlot : OutputFileName='Reco_Run%06d_RandomTrigger.root'%RunNumber

if not tag == "":
  OutputFileName.replace(".root", "_%s.root"%tag)

print OutputFileName, " will be generated."
sqlite_file = os.environ['SRT_CMSSW_BASE_SCRAMRTDEL']+'/src/EventFilter/GEMRawToDigi/test/GEMEMap_CosmicStand_8Nov2016.db'

def configureRun(SLOTLIST=[], VFATLIST=[], COLUMNLIST=[], ROWLIST=[], LAYERLIST=[], chamber=[], columnStand=[], rowStand=[], layerSC=[], GE11slot=[], opticallink=[], typechamber=[]):

    fileVFATS=os.environ.get('CMSSW_BASE')+"/src/EventFilter/GEMRawToDigi/data/VFAT2LIST.csv"

    schamber=[]
    slot=[]
    barcode=[]
    vfataddr=[]

    #Configuration of the Stand: write down every VFAT
    #The ones below are a editted version with respect to what is there in the elog

    NUMBEROFDETECTORS=10
    
    ################
    ## For new geo
    ################

    opticallink.append(0)
    GE11slot.append(1)
    schamber.append("GE1/1-SCS-002")
    chamber.append("GE1/1-VII-S-CERN-0001") # GEB0
    typechamber.append("short")
    slot.append([0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23])
    barcode.append(["#326","#331","#297","#034","#049","#046","#314","#072","#192","#142","#333","#334","#292","#268","#305","#102","#055","#280","#307","#332","#265","#324","#078","#313"])
    vfataddr.append(["0xfebb", "0xfec0", "0xfe38", "0xf957", "0xf774", "0xff74", "0xfebc", "0xf77c", "0xfec4", "0xfef4", "0xffb3", "0xfa0c", "0xff94", "0xfe28", "0xf9b7", "0xf92b", "0xf737", "0xfeb3", "0xf9a0", "0xf993", "0xf75c", "0xfedb", "0xffd4", "0xfe24"])
    columnStand.append(2)
    rowStand.append(2)
    layerSC.append(1) #Interior?

    opticallink.append(1)
    GE11slot.append(1)
    schamber.append("GE1/1-SCS-002")
    chamber.append("GE1/1-VII-S-CERN-0002") # GEB1
    typechamber.append("short")
    slot.append([0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23])
    barcode.append(["#060","#306","#057","#289","#030","#047","#053","#315","#327","#311","#144","#108","#256","#323","#264","#328","#254","#004","#225","#277","#287","#285","#041","#317"])
    vfataddr.append(["0xffac", "0xfa38", "0xff7f", "0xfefb", "0xff73", "0xf71c", "0xfff7", "0xfe2c", "0xf9eb", "0xfed7", "0xfa37", "0xf9bb", "0xf96b", "0xfef3", "0xff78", "0xf9bc", "0xfecc", "0xff34", "0xf6bb", "0xfa47", "0xff30", "0xf74b", "0xff9f", "0xfed4"])
    columnStand.append(2)
    rowStand.append(2)
    layerSC.append(2) #Interior?

    opticallink.append(2)
    GE11slot.append(27)
    schamber.append("GE1/1-SCS-003")
    chamber.append("GE1/1-VII-S-CERN-0004") # GEB2
    typechamber.append("short")
    slot.append([0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23])
    barcode.append(["#146","#204","#296","#284","#263","#093","#090","#035","#027","#088","#138","#336","#098","#040","#077","#211","#082","#029","#076","#048","#066","#290","#286","#316"])
    vfataddr.append(["0xf758", "0xff68", "0xff2b", "0xf73c", "0xff27", "0xff7c", "0xf72c", "0xff14", "0xff6f", "0xf788", "0xffaf", "0xf73b", "0xf6ac", "0xf6d8", "0xf97b", "0xfec3", "0xf750", "0xfff3", "0xf76f", "0xffd0", "0xffc0", "0xfe3b", "0xf9fc", "0xf9ac"])
    columnStand.append(2)
    rowStand.append(3)
    layerSC.append(1) #Interior?

    opticallink.append(3)
    GE11slot.append(27)
    schamber.append("GE1/1-SCS-003")
    chamber.append("GE1/1-VII-S-CERN-0003") # GEB3
    typechamber.append("short")
    slot.append([0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23])
    barcode.append(["#071","#319","#294","#321","#139","#096","#258","#271","#075","#269","#251","#325","#111","#318","#295","#274","#001","#240","#310","#288","#188","#273","#058","#070"])
    vfataddr.append(["0xff97", "0xf9c4", "0xf6f3", "0xfa03", "0xf964", "0xf6c3", "0xfa53", "0xf99f", "0xf718", "0xf9a7", "0xf6b3", "0xf9f4", "0xf93f", "0xf9f3", "0xf748", "0xf70b", "0xff18", "0xfa48", "0xf9dc", "0xfa4f", "0xf988", "0xf723", "0xff38", "0xf730"])
    columnStand.append(2)
    rowStand.append(3)
    layerSC.append(2) #Interior?

    opticallink.append(4)
    GE11slot.append(28)
    schamber.append("GE1/1-SCL-001")
    chamber.append("GE1/1-VII-L-CERN-0002") # GEB4
    typechamber.append("long")
    slot.append([0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23])
    barcode.append(["#121","#036","#031","#164","#045","#028","#170","#180","#257","#169","#183","#168","#127","#161","#255","#157","#172","#120","#167","#154","#330","#182","#026","#038"])
    vfataddr.append(["0xf943", "0xf743", "0xf787", "0xf6cb", "0xffdf", "0xff44", "0xf783", "0xfa13", "0xf9f7", "0xf6b0", "0xf6cc", "0xfe27", "0xf920", "0xf968", "0xf77b", "0xff3c", "0xf6ec", "0xff67", "0xffc7", "0xf6dc", "0xf9c8", "0xf6a4", "0xff58", "0xffd7"])
    columnStand.append(2)
    rowStand.append(4)
    layerSC.append(1) #Interior?

    opticallink.append(5)
    GE11slot.append(28)
    schamber.append("GE1/1-SCL-001")
    chamber.append("GE1/1-VII-L-CERN-0004") # GEB5
    typechamber.append("long")
    slot.append([0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23])
    barcode.append(["#155","#119","#118","#228","#205","#099","#131","#067","#200","#109","#226","#262","#223","#106","#133","#110","#125","#148","#115","#136","#220","#094","#147","#191"])
    vfataddr.append(["0xfe47", "0xf6b4", "0xf6a3", "0xf72f", "0xfa07", "0xff9c", "0xf77f", "0xff98", "0xf98b", "0xf6e3", "0xff64", "0xffbb", "0xfe43", "0xf98f", "0xf784", "0xf940", "0xff54", "0xff17", "0xf944", "0xf6bf", "0xf9e8", "0xf6ff", "0xf720", "0xf770"])
    columnStand.append(2)
    rowStand.append(4)
    layerSC.append(2) #Interior?

    opticallink.append(6)
    GE11slot.append(29)
    schamber.append("GE1/1-SCS-001")
    chamber.append("GE1/1-VII-S-CERN-0006") # GEB6
    typechamber.append("short")
    slot.append([0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23])
    barcode.append(["#339","#308","#051","#068","#322","#337","#299","#197","#056","#303","#087","#301","#266","#195","#270","#050","#252","#135","#275","#253","#061","#140","#329","#215"])
    vfataddr.append(["0xfebf", "0xfe34", "0xffb8", "0xffd3", "0xf9d4", "0xfedf", "0xff1c", "0xfe1f", "0xf738", "0xf9cc", "0xffeb", "0xff70", "0xfa14", "0xff0c", "0xfa24", "0xffe0", "0xff57", "0xfee7", "0xfe37", "0xf9ff", "0xfff8", "0xf707", "0xf9f8", "0xf724"])
    columnStand.append(2)
    rowStand.append(1)
    layerSC.append(1) #Interior?
    
    opticallink.append(7)
    GE11slot.append(29)
    schamber.append("GE1/1-SCS-001")
    chamber.append("GE1/1-VII-S-CERN-0005") # GEB7
    typechamber.append("short")
    slot.append([0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23])
    barcode.append(["#242","#302","#304","#069","#281","#059","#283","#100","#338","#259","#309","#073","#089","#214","#272","#312","#145","#217","#282","#086","#279","#278","#234","#246"])
    vfataddr.append(["0xf6b8", "0xfa10", "0xf9c0", "0xffc8", "0xfe3c", "0xf773", "0xf9b8", "0xffa0", "0xfa2b", "0xfa33", "0xf708", "0xf71f", "0xf767", "0xff10", "0xf96f", "0xfe4b", "0xfee8", "0xf714", "0xfe3f", "0xf6f8", "0xf973", "0xfe20", "0xfee0", "0xf6d4"])
    columnStand.append(2)
    rowStand.append(1)
    layerSC.append(2) #Interior?

    opticallink.append(8)
    GE11slot.append(30)
    schamber.append("GE1/1-SCL-002")
    chamber.append("GE1/1-VII-L-CERN-0003") # GEB8
    typechamber.append("long")
    slot.append([0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23])
    barcode.append(["#117","#212","#363","#239","#063","#237","#360","#104","#128","#202","#130","#221","#189","#193","#187","#235","#349","#222","#201","#198","#213","#105","#194","#236"])
    vfataddr.append(["0xf6e7", "0xfec8", "0xf98c", "0xf9e0", "0xffc4", "0xf704", "0xff5f", "0xf978", "0xfe44", "0xf78b", "0xf6df", "0xf777", "0xfa5b", "0xf91b", "0xfed0", "0xfe23", "0xfa1f", "0xf6e8", "0xfa1c", "0xff28", "0xfe2b", "0xffa3", "0xff6c", "0xf953"])
    columnStand.append(2)
    rowStand.append(5)
    layerSC.append(1) #Interior?

    opticallink.append(9)
    GE11slot.append(30)
    schamber.append("GE1/1-SCL-002")
    chamber.append("GE1/1-VII-L-CERN-0001") # GEB9
    typechamber.append("long")
    slot.append([0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23])
    barcode.append(["#171","#186","#362","#163","#358","#174","#364","#361","#152","#175","#177","#150","#179","#210","#178","#042","#173","#003","#165","#181","#122","#153","#149","#176"])
    vfataddr.append(["0xf950", "0xf6a8", "0xfed8", "0xf763", "0xf977", "0xf6ab", "0xfa50", "0xf9cb", "0xfeec", "0xf733", "0xfee4", "0xf717", "0xfef8", "0xf734", "0xf924", "0xffb4", "0xf6f4", "0xfa34", "0xf6b7", "0xf72b", "0xffa7", "0xf95f", "0xf6af", "0xff3f"])
    columnStand.append(2)
    rowStand.append(5)
    layerSC.append(2) #Interior?

    VFATHEX=[]
    BARCODE=[]

    for i in range(0, NUMBEROFDETECTORS):
      for j in range(0, len(vfataddr[ i ])):
        VFATLIST.append(int(vfataddr[ i ][ j ], 16))
        VFATHEX.append(vfataddr[ i ][ j ])
        BARCODE.append(barcode[ i ][ j ])
        SLOTLIST.append(slot[ i ][ j ])
        COLUMNLIST.append(columnStand[i])
        ROWLIST.append(rowStand[i])
        LAYERLIST.append(layerSC[i])

