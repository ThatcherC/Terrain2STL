Terrain2STL
===========

A web-based service that converts publicly available HGT files to useful STL files.

### Try it Out!
This project is running on a Google Compute Engine server at [jthatch.com](http://jthatch.com/terrain2stl). It's updated soon after any commit to this repository.

You can also download everything here and start the server with:

```node terrainServer.js```

It will run at localhost:8080.

To run without static file serving (only needed if you're using another static file server such as NGINX), start with static serving disable:

```NOSTATIC=true node terrainServer.js```

A few .HGT files are included, but not the whole set. You can get the rest from [the USGS](http://dds.cr.usgs.gov/srtm/version2_1/SRTM3/), or I could send you few.

### Install
Build the STL generator program with a simple `make`:
```sh
you@comp:~/.../Terrain2STL$ make
```
If that's successful, generate a test STL with
```
./celevstl 44.1928 -69.0851 40 40 1.7 0 1 3 1 test.stl
```

and you should be treated with a model of scenic Rockport harbor!

The arguments here are:
- Northwest corner latitude
- Northwest corner longitude
- Model width ("pixels")
- Model height ("pixels")
- Vertical scaling factor
- Rotation angle (degrees)
- Water drop (mm) (how much the ocean should be lowered in models)
- Base height (mm) (how much extra height to add to the base of model)
- Step size (hgt cells per model pixel)
- Output file name

#### Web Interface
To run the web interface to Terrain2STL, you'll need Nodejs. With that installed, first run install all the necessary node packages with
```
you@comp:~/.../Terrain2STL$ npm install
```
Once all the packages are install, start up the server with
```
you@comp:~/.../Terrain2STL$ node terrainServer.js
```
and the website should be up `localhost:8080/`!

### To-Do List

- [x] Add a resolution slider
  ~~Box size is in multiples of 120, so allowable downscales are 2,3,4,5,6,8,..~~
  Actually DEM resolution is 1/3, allowable downscales  are 2,4,5,8
  Probably 1/8 resolution is minimum desirable
- [ ] Draw the northern and southern boundaries of the data set
- [ ] Add a negative version for molds
- [x] Make a manifold surface option (bigger file, but better performance)

### Descriptions of Files
* `/hgt_files`

  * This directory contains some sample HGT files for testing. Included are Mt. Everest; Rockport, Maine; and San Franciso Bay.
  * The whole dataset is 40GB, so only a few files are included.
  * This could also be a made a symbolic link to another directory that contains your collection of HGT files

* `/src`

  * This is the source code for the part of the program that actually creates the STL files

  * `/src/elevstl.c` - STL-creation program. Accept command line arguments

* `terrain2stl-proto.html`

  * The prototype for the HTML source of the website. Must be edited to contain API keys.

* `terrainServer.js`

  * Node.js server. Takes parameters from the HTML page and passes them to elevstl to create the STL.
  * Also handles client downloading of pages and files.

### Done

*(Previously on to-do list)*

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
- [x] Add longitude compensation to correct for the distortion caused by the Mercator Projection
- [x] Normalize model height to remove vertical offset
- [x] Change view to fit lat/long
- [x] Vertical scale slider
  - [x] Increase scale range (maybe to 4x)
- [x] Compress STL files
