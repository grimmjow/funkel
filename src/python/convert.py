#!/bin/python

import os,sys
import math
from stl import Stl


class Convert:

    bound_min = [] #x, z, y
    bound_max = []
    trian = []
    resolution = {}
    offset = []
    ratio = 1

    def __init__(self, trian, resolution):
        self.trian = trian
        self.resolution = resolution

    def getBoundries(self):
        self.bound_min = [self.trian[0][0][0], self.trian[0][0][1], self.trian[0][0][2]] #erster punkt im ersten dreieck
        self.bound_max = [self.trian[0][0][0], self.trian[0][0][1], self.trian[0][0][2]] #erster punkt im ersten dreieck

        for t in trian:
            for p in t:
                self.bound_min[0] = min(self.bound_min[0], p[0])
                self.bound_min[1] = min(self.bound_min[1], p[1])
                self.bound_min[2] = min(self.bound_min[2], p[2])

                self.bound_max[0] = max(self.bound_max[0], p[0])
                self.bound_max[1] = max(self.bound_max[1], p[1])
                self.bound_max[2] = max(self.bound_max[2], p[2])

        self.offset = []
        for ii in range(3):
            self.offset.append((self.bound_min[ii] + self.bound_max[ii]) / 2)

        self.ratio = min(self.resolution["x"] * 2 / (self.bound_max[0] - self.bound_min[0]),
                         self.resolution["x"] * 2 / (self.bound_max[1] - self.bound_min[1]),
                         self.resolution["y"] / (self.bound_max[2] - self.bound_min[2]))

        print self.ratio
        print self.bound_min
        print self.bound_max
        print self.offset

def printHelp():
    print "use: convert.py -stl:file.stl -t:52 -x:2"


stl_file = "test.stl"
template = "imgdata.h.template"
outputFile = "imgdata.h"
resolution = {"y": 32, "x": 16, "t": 32}
layer = 0

for arg in sys.argv:
    if (arg[:5] == "-stl:"):
        imageFile = arg[5:]

    elif (arg[:3] == "-t:") and (arg[3:].isdigit()):
        resolution["t"] = int(arg[3:])

    elif (arg[:3] == "-x:") and (arg[3:].isdigit()):
        layer = int(arg[3:])

    elif (arg == "-h") or (arg == "--help"):
        printHelp()


t_offset = (resolution["t"] * 1.0 / resolution["x"]) * layer;

print "file:", stl_file
print "y:", resolution["y"]
print "x:", resolution["x"]
print "t:", resolution["t"]
print "layer:", layer
print "t_offset:", t_offset

trian = Stl.read(stl_file)

if not len(trian):
    exit -1

converter = Convert(trian, resolution)
converter.getBoundries()


print trian

