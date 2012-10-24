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

    elif (arg == "-h") | (arg == "--help"):
        printHelp()

img = Image.open(imageFile)
pix = img.load()
size = img.size
radius = min(size[0] / 2, size[1] / 2)

output = []

# initialisiere verschachtelte Listen mit Tuples (colorcount, pixelcount)
for z in range(resolution["z"]):
    last = []
    output.append(last)
    for x in range(resolution["x"]):
        last.append([0, 0])

# Pixel des Bildes durchlaufen
for imgx in range(1, size[0]):
    for imgy in range(1, size[1]):
        a = imgx - (size[0] / 2)
        b = imgy - (size[1] / 2)
        c = math.sqrt(math.pow(imgx, 2) * math.pow(imgy, 2))

        if c > radius:
            continue

        z = int(round((360 + math.degrees(math.sin(a/c))) % 360 / resolution["z"]))
        x = int(round(c / resolution["x"]))
        output[z][x][0] += pix[imgx, imgy]
        output[z][x][1] += 1

# export array
for z in range(resolution["z"]):
    for x in range(resolution["x"]):
        if output[z][x][1] > 0:
            output[z][x][0] = output[z][x][0] / output[z][x][1]
        output[z][x][0] = int(round(output[z][x][0] / 128))
        print output[z][x][0]
