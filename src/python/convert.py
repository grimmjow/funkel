#!/bin/python

import os,sys
import Image
import math


def printHelp():
    print "use: convert.py -img:file.jpg"


imageFile = "test.jpg"
outputFile = "funkel.h"
resolution = {"x": 16, "z": 32, "t": 1}

for arg in sys.argv:
    if (arg[:5] == "-img:"):
        imageFile = arg[5:]

    elif (arg[:3] == "-x:") & (arg[3:].isdigit()):
        resolution["x"] = int(arg[3:])

    elif (arg[:3] == "-z:") & (arg[3:].isdigit()):
        resolution["z"] = int(arg[3:])

    elif (arg[:3] == "-t:") & (arg[3:].isdigit()):
        resolution["t"] = int(arg[3:])

    else:
        printHelp()

img = Image.open(imageFile)
pix = img.load()
size = img.size

output = []

# initialisiere verschachtelte Listen mit Tuples (colorcount, pixelcount)
for z in range(resolution["z"]):
    last = []
    output.append(last)
    for x in range(resolution["x"]):
        last.append((0, 0))

# Pixel des Bildes durchlaufen
for x in range(size[0]):
    for y in range(size[1]):
        pass



