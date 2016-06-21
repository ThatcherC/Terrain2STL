Terrain2STL
===========

A web-based service that converts publicly available HGT files to useful STL files.

###Try it Out!
This project is running on a Google Compute Engine server at [jthatch.com](http://jthatch.com/terrain2stl). It's updated soon after any commit to this repository. 

You can also download everything here and start the server with: 

```node terrainServer.js```

It will run at localhost:8080.

A few .HGT files are included, but not the whole set. You can get the rest from [the USGS](http://dds.cr.usgs.gov/srtm/version2_1/SRTM3/), or I could send you few.


###To-Do List
- [x] Make dataset voids not just a fixed negative value.
- [x] Make models watertight.
  - [x] Fix voids in edges of models
- [x] Increase maximum box size.
- [x] Fill voids in dataset.
- [x] Make the 3 arcsecond resolution available in all sizes.
- [x] Add ability to cross tiles. !!!
- [x] Make Print and Download buttons disappear or defocus when the selection box is moved
- [x] Make the selection box reflect changes in the Lat/Long boxes 
- [x] Decide on a maximum file size
  - [ ] Add a resolution slider
- [x] Add longitude compensation to correct for the distortion caused by the Mercator Projection
- [ ] Draw the northern and southern boundaries of the data set
- [x] Normalize model height to remove vertical offset
- [ ] Change view to fit lat/long
- [x] Vertical scale slider
  - [ ] Increase scale range (maybe to 4x)
- [ ] Add a negative version for molds
- [ ] Make a manifold surface option (bigger file, but better performance)
- [ ] Compress STL files

###Descriptions of Files
* /hgt_files

  * This directory contains some sample HGT files for testing. Included are Mt. Everest; Rockport, Maine; and San Franciso Bay.
  * The whole dataset is 40GB, so only a few files are included.

* elevstl.cpp

  * This is the source code for the part of the program that actually creates the STL files. Accepts command-line arguments

* gmaps-gridlines.js

  * This script provides the gridlines for the map. It is a slightly modified version of a package found here: https://github.com/alexcheng1982/google-maps-gridlines

* terrain2stl.html

  * The HTML source of the website.

* terrainServer.js

  * Node.js server. Takes parameters from the HTML page and passes them to elevstl to create the STL.
  * Also handles client downloading of pages and files.

