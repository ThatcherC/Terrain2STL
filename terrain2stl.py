
import sys
import subprocess



def main(lat, lon, scale, out, waterDrop=0, baseHeight=1, gridX=1, gridY=1):
    """
    The capture box has 0,0 x,y at the top-left corner:  +X right, +Y down.
    Lat increases N of the equator, decreases S of it.
    Lon increases E of Greenwich UK, decreases W of it

    Download HTG files from:
    http://dds.cr.usgs.gov/srtm/version2_1/SRTM3/

    """
    if not out.endswith(".stl"):
        out += ".stl"

    # Call to the compiled c++ (on Mac)
    subprocess.call(["./elevstl", str(lat), str(lon), str(scale), out, str(waterDrop), str(baseHeight)])


if __name__ == "__main__":
    main(38.9683, -105.0804, 27, "testerX.stl")