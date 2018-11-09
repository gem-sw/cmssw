from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from gemdrawconfig import *

import ROOT
from ROOT import TCanvas
from ROOT import TEfficiency
from ROOT import gROOT
from ROOT import gStyle
from ROOT import gPad

import re
import os
import sys


class Directory(object):
    def __init__(self, path, creation=True):
        self.path = path
        self._creation = creation
        if self._creation:
            os.makedirs(self.path)

    def make_subdir(self, name):
        path = os.path.join(self.path, name)
        setattr(self, name, Directory(path, creation=self._creation))

    def get_entries(self, full_path=True):
        entries = os.listdir(self.path)
        if full_path:
            entries = [os.path.join(self.path, each) for each in entries]
        return entries


def convert_title_to_name(title):
    name = re.sub('[^A-Za-z0-9]+', "_", title)
    name = name.replace(" ", "_")
    name = name.replace("__", "_")
    name = name.lower()
    name = name.rstrip("_")
    return name


def make_det_suffix(chamber_id=None, layer_id=None, roll_id=None, vfat_id=None, as_name=False):
    zipped = [("Chamber", chamber_id),
              ("Layer", layer_id),
              ("Roll", roll_id),
              ("VFAT", vfat_id)]

    zipped = [item for item in zipped if item[1] is not None]

    suffix = "("
    for i, (det_name, det_id) in enumerate(zipped):
        entry = " {} {}".format(det_name, det_id)
        if i == 0:
            entry = entry.lstrip()
        suffix += entry
    suffix += ")"

    if as_name:
        suffix = convert_title_to_name(suffix)
    return suffix


def loop_over_chamber_layer(obj, **kwargs):
    for chamber_id in range(MIN_CHAMBER_ID, MAX_CHAMBER_ID + 1, 2):
        for layer_id in range(1, NUM_LAYER + 1):
            obj(chamber_id=chamber_id, layer_id=layer_id, **kwargs)


class BasePainter(object):
    def __init__(self,
                 sim_dir,
                 out_dir,
                 chamber_id=None,
                 layer_id=None,
                 roll_id=None,
                 vfat_id=None,
                 **kwargs):
        """
        Args:
          sim_dir:
          out_dir:
          chamber_id:
        """
        self.sim_dir = sim_dir
        self.out_dir = out_dir

        self.chamber_id = chamber_id
        self.layer_id = layer_id
        self.roll_id = roll_id
        self.vfat_id = vfat_id

        self.kwargs = kwargs

        self.det_id = {
            "chamber_id": chamber_id,
            "layer_id": layer_id,
            "roll_id": roll_id,
            "vfat_id": vfat_id}

        self.can = TCanvas()
        self.can.cd()
        self._draw()
        self._makeup()

        out_fmt = self._make_out_fmt()
        self.can.SaveAs(out_fmt.format(ext="png"))
        self.can.SaveAs(out_fmt.format(ext="pdf"))

    def _draw(self):
        raise NotImplementedError()

    def _makeup(self):
        raise NotImplementedError()

    def _make_out_fmt(self):
        out_fmt = os.path.join(self.out_dir, self.name + ".{ext}")
        return out_fmt


class BaseHistPainter(BasePainter):
    def __init__(self,
                 name,
                 sim_dir,
                 out_dir,
                 chamber_id=None,
                 layer_id=None,
                 roll_id=None,
                 vfat_id=None,
                 **kwargs):
        self.name = name
        super(BaseHistPainter, self).__init__(
            sim_dir, out_dir, chamber_id, layer_id, roll_id, vfat_id, **kwargs)

    def _draw(self):
        self.hist = self.sim_dir.Get(self.name)

    def _makeup(self):
        raise NotImplementedError()


class BaseEffPainter(BasePainter):
    def __init__(self,
                 passed_name,
                 total_name,
                 name,
                 sim_dir,
                 out_dir,
                 chamber_id=None,
                 layer_id=None,
                 roll_id=None,
                 vfat_id=None,
                 **kwargs):
        self.passed_name = passed_name
        self.total_name = total_name
        self.name = name
        super(BaseEffPainter, self).__init__(
            sim_dir, out_dir, chamber_id, layer_id, roll_id, vfat_id, **kwargs)

    def _draw(self):
        passed = self.sim_dir.Get(self.passed_name)
        total = self.sim_dir.Get(self.total_name)
        if not TEfficiency.CheckConsistency(passed, total):
            return None
        eff = TEfficiency(passed, total)
        self.hist = eff.CreateHistogram()

    def _makeup(self):
        raise NotImplementedError()


class Hist1DPainter(BaseHistPainter):
    def _draw(self):
        self.hist = self.sim_dir.Get(self.name)
    def _makeup(self):
        # Histogram
        if self.kwargs.has_key("draw_opt"):
            self.hist.Draw(self.kwargs["draw_opt"])
        else:
            self.hist.Draw("HIST E")
        if self.kwargs.has_key("title"):
            self.hist.SetTitle(self.kwargs["title"])
        if self.kwargs.has_key("x_title"):
            self.hist.GetXaxis().SetTitle(self.kwargs["x_title"])
        if self.kwargs.has_key("y_title"):
            self.hist.GetYaxis().SetTitle(self.kwargs["y_title"])
        if self.kwargs.has_key("line_color"):
            self.hist.SetFillColor(self.kwargs["line_color"])
        if self.kwargs.has_key("line_width"):
            self.hist.SetLineWidth(self.kwargs["line_width"])
        else:
            self.hist.SetLineWidth(3)
        if self.kwargs.has_key("line_color"):
            self.hist.SetFillColor(self.kwargs["line_color"])
        if self.kwargs.has_key("line_color_alpha"):
            self.hist.SetFillColorAlpha(*self.kwargs["line_color_alpha"])
        # Canvas
        if self.kwargs.has_key("log_y"):
            self.can.SetLogy()
        if self.kwargs.has_key("grid"):
            self.can.SetGrid()
        # Style


class Hist2DPainter(BaseHistPainter):
    def _makeup(self):
        self.hist.Draw("COLZ TEXT")
        gStyle.SetPaintTextFormat("g")


class Eff1DPainter(BaseEffPainter):
    def _makeup(self):
        self.hist.Draw("HIST")


class Eff2DPainter(BaseEffPainter):
    def _makeup(self):
        self.hist.Draw("COLZ TEXT")
        gStyle.SetPaintTextFormat(".3f")
