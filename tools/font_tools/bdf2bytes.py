'''
Copyright: Envox Experimental Zone https://www.envox.hr/eez, 2015
'''

from __future__ import print_function
import sys, os, re, argparse

parser = argparse.ArgumentParser(description="Convert BDF font into bytes ready to be used by the EEZ font drawing extension for the UTFT library.")

parser.add_argument('input', type=file)
parser.add_argument('output', type=argparse.FileType("w"))
parser.add_argument('--varname', help="Name of the variable containing font data (default: FONT_DATA)", default="FONT_DATA")
parser.add_argument('--startenc', help="Start encoding (default: 32)", type=int, default=32)
parser.add_argument('--endenc', help="End encoding (default: 127)", type=int, default=127)

args = parser.parse_args()

state = "FONT"
font = {
    "glyphs": {}
}

with args.input as f:
    lines = f.readlines()
    for line in lines:
        parts = line.split()
        name = parts[0]

        if state == "FONT":
            if name == "FONT_ASCENT":
                font["ascent"] = int(parts[1])
            elif name == "FONT_DESCENT":
                font["descent"] = int(parts[1])
            elif name == "STARTCHAR":
                glyph = {}
                state = "CHAR"
        elif state == "CHAR":
            if name == "ENCODING":
                glyph["encoding"] = int(parts[1])
            elif name == "BBX":
                glyph["bbxWidth"] = int(parts[1])
                glyph["bbxHeight"] = int(parts[2])
                glyph["bbxXOff"] = int(parts[3])
                glyph["bbxYOff"] = int(parts[4])
            elif name == "DWIDTH":
                glyph["dwidth"] = int(parts[1])
            elif name == "BITMAP":
                glyph["bitmap"] = []
                state = "BITMAP"
        elif state == "BITMAP":
            if name == "ENDCHAR":
                if glyph["encoding"] >= args.startenc and glyph["encoding"] <= args.endenc:
                    font["glyphs"][glyph["encoding"]] = glyph
                state = "FONT"
            else:
                for byteHexString in re.findall('..', line):
                    glyph["bitmap"].append(int(byteHexString, 16))

def appendUInt8(buffer, value):
    assert value >= 0 and value <= 255
    buffer.append(value)

def appendInt8(buffer, value):
    #assert value >= -128 and value <= 127
    buffer.append(value)

def appendUint16(buffer, value):
    assert value >= 0 and value <= 65535
    buffer.append(value >> 8)
    buffer.append(value & 0xFF)

outputBytes = []

appendUInt8(outputBytes, font["ascent"])
appendUInt8(outputBytes, font["descent"])
appendUInt8(outputBytes, args.startenc)
appendUInt8(outputBytes, args.endenc)
for i in range(args.startenc, args.endenc + 1):
    appendUint16(outputBytes, 0)

for i in range(args.startenc, args.endenc + 1):
    offsetIndex = 4 + (i - args.startenc) * 2 
    offset = len(outputBytes)
    outputBytes[offsetIndex] =  offset >> 8
    outputBytes[offsetIndex + 1] =  offset & 0xFF

    if i in font["glyphs"]:
        glyph = font["glyphs"][i]

        appendInt8(outputBytes, glyph["dwidth"])
        appendUInt8(outputBytes, glyph["bbxWidth"])
        appendUInt8(outputBytes, glyph["bbxHeight"])
        appendInt8(outputBytes, glyph["bbxXOff"])
        appendInt8(outputBytes, glyph["bbxYOff"])

        outputBytes.extend(glyph["bitmap"])
    else:
        appendUInt8(outputBytes, 255)

with args.output as f:
    outputBytesStr = ", ".join(str(x) for x in outputBytes)
    outputStr = "static const uint8_t {0}[{1}] PROGMEM = {{ {2} }};".format(args.varname, len(outputBytes), outputBytesStr)
    print(outputStr, file=f)

print("Total", args.endenc - args.startenc + 1 , "glyphs generated, which resulted in", len(outputBytes), "bytes.");