#!/bin/python

import os,sys
import Image
import math


def printHelp():
    print "use: convert.py -img:file.jpg"


imageFile = "test.jpg"
template = "imgdata.h.template"
outputFile = "imgdata.h"
resolution = {"x": 16, "z": 32, "t": 1}

for arg in sys.argv:
    if (arg[:5] == "-img:"):
        imageFile = arg[5:]

    elif (arg[:3] == "-x:") and (arg[3:].isdigit()):
        if int(arg[3:]) % 16 == 0:
            resolution["x"] = int(arg[3:])

    elif (arg[:3] == "-z:") and (arg[3:].isdigit()):
        resolution["z"] = int(arg[3:])

    elif (arg[:3] == "-t:") and (arg[3:].isdigit()):
        resolution["t"] = int(arg[3:])

    elif (arg == "-h") or (arg == "--help"):
        printHelp()

print "file:", file
print "x:", resolution["x"]
print "z:", resolution["z"]

img = Image.open(imageFile)
pix = img.load()
size = img.size
radius = min(size[0] / 2, size[1] / 2)

imgx = 50
imgy = 50

a = imgx - (size[0] / 2)
b = imgy - (size[1] / 2)
c = math.sqrt(math.pow(a, 2) + math.pow(b, 2))

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
        if pix[imgx, imgy] == 0:
            continue

        a = imgx - (size[0] / 2)
        b = imgy - (size[1] / 2)
        c = math.sqrt(math.pow(a, 2) + math.pow(b, 2))

        if c > radius or c == 0:
            continue

        deg = math.degrees(math.asin(a/c))
        if b > 0:
            if a > 0:
                deg += 90
            else:
                deg -= 90

        z = int(round(((360 + deg) % 360) * (resolution["z"]-1) / 360))
        x = int(round(c * (resolution["x"]-1) / radius))
        output[z][x][0] += pix[imgx, imgy]
        output[z][x][1] += 1

# average value determination
for z in range(resolution["z"]):
    for x in range(resolution["x"]):
        if output[z][x][1] > 0:
            output[z][x][0] = output[z][x][0] / output[z][x][1]
        output[z][x][0] = int(round(output[z][x][0] / 128))

# export array
imgdata = "{\n"
for z in range(resolution["z"]):

    imgdata += "{0b"

    for x in range(resolution["x"] / 2):
        imgdata += str(output[z][x * 2 + 1][0])

    imgdata += ", 0b"

    for x in range(resolution["x"] / 2):
        imgdata += str(output[(z + resolution["z"] / 2) % resolution["z"]][x * 2][0])

    imgdata += "},\n"

imgdata = imgdata[:-2] + "}"

f = open(template, "r")
content = f.read()
f.close()

content = content.replace("$IMGDATA", imgdata)
content = content.replace("$RES_X", str(resolution["x"]))
content = content.replace("$RES_Z", str(resolution["z"]))
content = content.replace("$RES_T", str(resolution["t"]))

f = open(outputFile, "w")
f.write(content)
f.close()
