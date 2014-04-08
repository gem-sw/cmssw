#!/usr/bin/env python
from ROOT import *

from drawPlots import *
import os
import sys
sys.argv.append( '-b' )

def draw_occ(target_dir, h, ext =".png", opt = ""):
  gStyle.SetStatStyle(0)
  gStyle.SetOptStat(1110)
  c = TCanvas(h.GetTitle(),h.GetName(),600,600)
  c_title = c.GetTitle()
  c.Clear()
  if not h:
    sys.exit('h does not exist')
  h.SetLineWidth(2)
  h.SetLineColor(kBlue)
  h.Draw(opt)
  c.SaveAs(target_dir + c_title + ext)

def draw_bx(target_dir, h , ext = ".png", opt = ""):
  gStyle.SetStatStyle(0)
  gStyle.SetOptStat(1110)
  c = TCanvas(h.GetTitle(),h.GetName(),600,600)
  c_title = c.GetTitle()
  gPad.SetLogy()
  h.SetTitle(title)
  h.SetLineWidth(2)
  h.SetLineColor(kBlue)
  h.Draw(opt)
  h.SetMinimum(1.)
  c.SaveAs(target_dir + c_title + ext)

def draw_col(target_dir, h, ext =".png", opt = "col"):
  gStyle.SetStatStyle(0)
  gStyle.SetOptStat(1110)
  c = TCanvas(h.GetTitle(),h.GetName(),600,600)
  c_title = c.GetTitle()
  c.Clear()
  if not h:
    sys.exit('h does not exist')
  h.SetLineWidth(2)
  h.SetLineColor(kBlue)
  h.Draw(opt)
  c.SaveAs(target_dir + c_title + ext)


PlotDir = "t_plot"
PlotDir = PlotDir+'/'
#os.mkdir(PlotDir)

c = TCanvas("c","c",600,600)
dqm_file = TFile("DQM_V0001_R000000001__Global__CMSSW_X_Y_Z__RECO.root")
d1 = dqm_file.Get("DQMData/Run 1/MuonGEMHitsV/Run summary/GEMHitsTask")

key_list =[]
for x in d1.GetListOfKeys() :
	key_list.append(x.GetName())

for hist in key_list :
	if hist.find("eff") != -1 :
		continue
	if (hist.find("lx") !=-1 or hist.find("ly") != -1 ) :
		continue
	if ( hist.find("bx") != -1 ) :
		draw_bx( PlotDir, d1.Get(hist)  )
	elif ( hist.find("xy") !=-1 or hist.find("zr") !=-1) :
		draw_col( PlotDir, d1.Get(hist) )
	else :
		draw_occ( PlotDir, d1.Get(hist) )
#c.SaveAs("h1.png")
#draw_plot(hi1)
