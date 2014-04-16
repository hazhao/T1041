# Run WC_Reader 
# this is just a wrapper for the ROOT C++ code
# will replace w/ a simple GUI
# Usage: python WC_Reader.py file.root
# Created 4/12/2014 B.Hirosky: Initial release

import os, re, glob, sys, array
from ROOT import *
from TBUtils import *

logger=Logger()

if len(sys.argv)<2:
    logger.Fatal("No ROOT file given")


runDat=sys.argv[1]

print "Processing file:",runDat

gROOT.ProcessLine(".L TBEvent.cc+")
gROOT.ProcessLine(".L dqm/dqmPlots.C+")
gROOT.ProcessLine(".L WC_Reader.C+")

WC_Reader(runDat)

hit_continue('Hit any key to exit')












