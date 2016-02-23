# Font Tools

## Introduction

UTFT library only supports low resolution monospaced fonts.
Here we describe a solution to support high resolution,
monospaced and proportional fonts.
Font is extracted from the BDF format.
You can use free programs to convert TTF or OTF fonts to BDF format.
Once you have BDF font you can use bdf2bytes.py to prepare format which can be used
by the EEZ extension for the UTFT library which can draw these fonts on LCD screen. 

## Acquire font in BDF format

BDF is bitmap based format for fonts.
If you can't find your font of choice in BDF format,
and you probably can't because now vector formats are much more popular,
you can use third party programs to convert from TTF or OTF format to BDF format.

### TTF2BDF

TTF2BDF is program created by Computing Research Labs from New Mexico State University in 1996 through 1999.
It can be downloaded from the Internat for the various platforms.
For example, I downloaded version for the Windows from here:   

### OTF2BDF

https://www.math.nmsu.edu/~mleisher/Software/otf2bdf/

## bdf2bytes.py


## EEZ Font Extension for UTFT Library