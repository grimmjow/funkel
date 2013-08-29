#!/bin/python

import os,sys
import Image
import math


def printHelp():
    print "use: convert.py -img:file.jpg"


imageFile = "test.jpg"
template = "imgdata.h.template"
outputFile = "imgdata.h"
resolution = {"y": 32, "l": 16, "t": 32}
layer = 0

for arg in sys.argv:
    if (arg[:5] == "-img:"):
        imageFile = arg[5:]

    elif (arg[:3] == "-t:") and (arg[3:].isdigit()):
        resolution["t"] = int(arg[3:])

    elif (arg[:3] == "-l:") and (arg[3:].isdigit()):
        layer = int(arg[3:])

    elif (arg == "-h") or (arg == "--help"):
        printHelp()


t_offset = (resolution["t"] * 1.0 / resolution["l"]) * layer;

print "file:", imageFile
print "y:", resolution["y"]
print "l:", resolution["l"]
print "t:", resolution["t"]
print "layer:", layer
print "t_offset:", t_offset

img = Image.open(imageFile)
pix = img.load()
size = img.size
radius = min(size[0] / 2, size[1] / 2)

output = []

# initialisiere verschachtelte Listen mit Tuples (colorcount, pixelcount)
for t in range(resolution["t"]):
    last = []
    output.append(last)
    for y in range(resolution["y"]):
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

        deg = math.degrees(math.asin(abs(a)/c))

        if (a > 0 and b < 0):
            deg = deg * -1 + 180
        if (a < 0 and b < 0):
            deg += 180
        if (a < 0  and b > 0):
            deg *= -1

        t = ((360 + deg) % 360) * (resolution["t"]-1) / 360
        t = int(round(t + t_offset)) % resolution["t"]

        l = int(round(c * (resolution["l"]-1) / radius))
        y = 0

        if l == layer:
            output[t][y][0] += pix[imgx, imgy]
            output[t][y][1] += 1

# average value determination
for t in range(resolution["t"]):
    for y in range(resolution["y"]):
        if output[t][y][1] > 0:
            output[t][y][0] = output[t][y][0] / output[t][y][1]
        output[t][y][0] = int(round(output[t][y][0] / 128))

        #sys.stdout.write(str(output[t][y][0]))
    #print ""
#print ""

# export array
imgdata = "{\n"
for t in range(resolution["t"]):

    imgdata += "{0b"

    for y in range(resolution["y"] / 8):
        if y > 0:
            imgdata += ", 0b"
            
        for i in range(8):
            # imgdata += str(output[t][y * 8 + i][0])
            imgdata += str(output[t][0][0])


    imgdata += "},\n"

imgdata = imgdata[:-2] + "}"

f = open(template, "r")
content = f.read()
f.close()

content = content.replace("$IMGDATA", imgdata)
content = content.replace("$RES_Y", str(resolution["y"]))
content = content.replace("$RES_T", str(resolution["t"]))

f = open(outputFile, "w")
f.write(content)
f.close()
