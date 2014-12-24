"""
terrain2stl.py
Author : Eric Pavey - warpcat@gmail.com - www.akeric.com
Overview:  Companion tool to the compiled elevstl.cpp code.
Find all source : https://github.com/ThatcherC/Terrain2STL

Currently only been tested on OSX.

To compile the c++ on the osx, I had to run:
$ sudo clang++ -std=c++11 -stdlib=libc++ -o elevstl elevstl.cpp

Example usage:
$ python terrain2STL.py -104.93 38.87 9 "newTest.stl" --waterDrop 3 --baseHeight 6 --gridX 4 --gridY 4
"""

import sys
import argparse
import subprocess

def main(lon, lat,  scale, out, waterDrop=0, baseHeight=1, gridX=1, gridY=1):
    """
    The capture box has 0,0 x,y at the top-left corner:  +X right, +Y down.
    Lat increases N of the equator, decreases S of it.
    Lon increases E of Greenwich UK, decreases W of it

    Download additional HTG files from:
    http://dds.cr.usgs.gov/srtm/version2_1/SRTM3/

    Dependencies: The compiled elevstl.cpp code.

    Parameters:
    lon : float : Longitude X
    lat : float : Latitude Y
    scale : int : Scale in arc div/sec (mult of 3)
    out : string : Name of .stl file to save.
    waterDrop : int : Default 0 : How much to 'lower' the water relative to the coast.
    baseHeight : int : Default 1 : Extra height to add to the base.
    gridX : int : Default 1 : How many times to sample in X E->W.
    gridY : int : Default 1 : How many times to sample in Y N->S.

    Return : None
    """
    if not out.endswith(".stl"):
        out += ".stl"

    # Call to the compiled c++ (on Mac)
    if gridX == 1 and gridY == 1:
        subprocess.call(["./elevstl", str(lat), str(lon), str(scale), out, str(waterDrop), str(baseHeight)])
        return

    step = scale/90.0
    for x in range(gridX):
        for y in range(gridY):
            newLon = lon + step*x
            newLat = lat + -step*y
            baseName = out.split(".")[0]
            newName = '%s_x%sy%s.stl'%(baseName, x, y)
            subprocess.call(["./elevstl", str(newLat), str(newLon), str(scale), newName, str(waterDrop), str(baseHeight)])

if __name__ == "__main__":
    """
    When executed from the commandline...
    """
    parser = argparse.ArgumentParser(description='Create stl files(s) from coordinates.')
    parser.add_argument('lon', type=float, help='Longitude')
    parser.add_argument('lat', type=float, help='Latitude')
    parser.add_argument('scale', type=int, help='Scale, in arc div/sec (mult of 3)')
    parser.add_argument('out', help='Output stl filename')
    parser.add_argument('--waterDrop', type=int, default=0, help='How much to lower the water', metavar="0")
    parser.add_argument('--baseHeight', type=int, default=1, help='Add extra height to the base', metavar="1")
    parser.add_argument('--gridX', type=int, default=1, help='How many times to sample in X E->W.', metavar="1")
    parser.add_argument('--gridY', type=int, default=1, help='How many times to sample in Y N->S.', metavar="1")
    args = parser.parse_args()

    main(args.lon, args.lat,  args.scale, args.out, waterDrop=args.waterDrop,
         baseHeight=args.baseHeight, gridX=args.gridX, gridY=args.gridY)