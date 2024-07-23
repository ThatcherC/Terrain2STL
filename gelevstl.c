#include "gdal_priv.h"
#include "gdal_utils.h"
#include "gdalwarper.h"
#include "ogrsf_frmts.h"

#include "src/STLWriter.h"
#include "src/elevation.h"
#include <cstdio>
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// based on
// https://gdal.org/api/gdal_alg.html#_CPPv423GDALRasterizeGeometries12GDALDatasetHiPKiiPK12OGRGeometryH19GDALTransformerFuncPvPKd12CSLConstList16GDALProgressFuncPv
GDALDatasetH makeMEMdatasetStrip(double northwestlat, double northwestlng, int width, int height, GDALDataType datatype,
                                 const char *inputProjection, void **pData) {

  // check that databuffer point pData == 0 or NULL -
  // we're doing the allocation here so we want to make sure where not
  // dropping a pointer to some already-allocated memory
  if (*pData != NULL) {
    // todo - return an error or something more graceful
    printf("Expected pData to be NULL! We'll allocate memory for it automatically.\n");
    exit(1); // for now, quit
  }

  int nBufXSize = width;
  int nBufYSize = height;
  int nBandCount = 1;
  GDALDataType eType = datatype; // let the output buffer be float32s - good for interpolation
  int nDataTypeSize = GDALGetDataTypeSizeBytes(eType);

  *pData = CPLCalloc(nBufXSize * nBufYSize * nBandCount, nDataTypeSize);
  char memdsetpath[1024];
  sprintf(memdsetpath,
          "MEM:::DATAPOINTER=0x%p,PIXELS=%d,LINES=%d,"
          "BANDS=%d,DATATYPE=%s,PIXELOFFSET=%d,LINEOFFSET=%d",
          *pData, nBufXSize, nBufYSize, nBandCount, GDALGetDataTypeName(eType), nBandCount * nDataTypeSize,
          nBufXSize * nBandCount * nDataTypeSize);

  // Open Memory Dataset
  GDALDatasetH hMemDset = GDALOpen(memdsetpath, GA_Update);

  double adfGeoTransform[6];
  // TODO: implement a correct geo transform here!!!!!!
  adfGeoTransform[0] = northwestlng;
  adfGeoTransform[3] = northwestlat;
  adfGeoTransform[1] = 0.000833;
  adfGeoTransform[5] = -0.000833;
  adfGeoTransform[2] = 0;
  adfGeoTransform[4] = 0;

  //
  GDALSetGeoTransform(hMemDset, adfGeoTransform);
  GDALSetProjection(hMemDset, inputProjection); // TODO pick a projection

  return hMemDset;
}

void printDatasetInfo(GDALDatasetH hDataset) {

  // printing input file info

  GDALDriverH hDriver;
  double adfGeoTransform[6];
  hDriver = GDALGetDatasetDriver(hDataset);
  printf("Driver: %s/%s\n", GDALGetDriverShortName(hDriver), GDALGetDriverLongName(hDriver));
  printf("Size is %dx%dx%d\n", GDALGetRasterXSize(hDataset), GDALGetRasterYSize(hDataset),
         GDALGetRasterCount(hDataset));
  if (GDALGetProjectionRef(hDataset) != NULL)
    printf("Projection is `%s'\n", GDALGetProjectionRef(hDataset));
  if (GDALGetGeoTransform(hDataset, adfGeoTransform) == CE_None) {
    printf("Origin = (%.6f,%.6f)\n", adfGeoTransform[0], adfGeoTransform[3]);
    printf("Pixel Size = (%.6f,%.6f)\n", adfGeoTransform[1], adfGeoTransform[5]);
  }
}

// lh is lower heights, ie those along a strip with a lower y component
// uh is upper height, those along a strip with a higher y component
int writeXStrip(FILE *file, float *lh, float *uh, int width, float xScale, float lyval, float hyval) {
  int numtris = 0;

  // build left wall
  struct _vect3 u = {0, hyval, uh[0]};
  struct _vect3 l = {0, lyval, lh[0]};
  struct _vect3 ub = {0, hyval, 0};
  struct _vect3 lb = {0, lyval, 0};
  addTriangle(file, createTriangle(u, ub, lb));
  addTriangle(file, createTriangle(u, lb, l));
  numtris += 2;

  // build right wall
  u = (struct _vect3){(width - 1) * xScale, hyval, uh[width - 1]};
  l = (struct _vect3){(width - 1) * xScale, lyval, lh[width - 1]};
  ub = (struct _vect3){(width - 1) * xScale, hyval, 0};
  lb = (struct _vect3){(width - 1) * xScale, lyval, 0};
  addTriangle(file, createTriangle(u, lb, ub));
  addTriangle(file, createTriangle(u, l, lb));
  numtris += 2;

  int voidCutoff2 = -100;

  for (int x = 1; x < width; x++) {
    float ha = uh[x];
    float hb = lh[x];     // d---a
    float hc = lh[x - 1]; // |   |
    float hd = uh[x - 1]; // c---b

    if (ha > voidCutoff2 && hb > voidCutoff2 && hc > voidCutoff2 && hd > voidCutoff2) {
      struct _vect3 a = {x * xScale, hyval, ha};
      struct _vect3 b = {x * xScale, lyval, hb};
      struct _vect3 c = {(x - 1) * xScale, lyval, hc};
      struct _vect3 d = {(x - 1) * xScale, hyval, hd};

      // choose where to split the square based on local curvature
      if (fabs(hd - hb) < fabs(ha - hc)) {
        addTriangle(file, createTriangle(a, d, b));
        addTriangle(file, createTriangle(c, b, d));
      } else {
        addTriangle(file, createTriangle(a, d, c));
        addTriangle(file, createTriangle(a, c, b));
      }
      numtris += 2;
    }
  }

  return numtris;
}

int writeLineWall(FILE *file, float *heights, int width, float xScale, float yval, int flipNormal) {
  int numtris = 0;

  for (int i = 1; i < width; i++) {
    if (heights[i] > voidCutoff && heights[i - 1] > voidCutoff) {
      struct _vect3 a = {i * xScale, yval, heights[i]};
      struct _vect3 b = {(i - 1) * xScale, yval, heights[i - 1]};

      struct _vect3 w = {i * xScale, yval, 0};
      struct _vect3 z = {(i - 1) * xScale, yval, 0};

      if (flipNormal) {
        addTriangle(file, createTriangle(z, b, a)); // model walls
        addTriangle(file, createTriangle(a, w, z));
      } else {
        addTriangle(file, createTriangle(b, z, a)); // model walls
        addTriangle(file, createTriangle(w, a, z));
      }

      numtris += 2;
    }
  }

  return numtris;
}

void buffToSTL(int width, int height, float *buf, char *outputName, float globalLat) {

  int tris = 0;
  FILE *stl = fopen(outputName, "w");
  // fopen returns a null pointer if file can't be opened
  if (stl == NULL) {
    fprintf(stderr, "Unable to open '%s' for output!\n", outputName);
    exit(1);
  }
  startSTLfile(stl, 4);

  // get zeroth line
  // old line: getElevationLine(nextline, width, -height, lat, lng, scaleFactor, rot, waterDrop, baseHeight, stepSize);

  float *prevline;
  float *nextline = &buf[0];

  // tris += writeLineWall(stl, nextline, width, cos(globalLat), -height, 0);
  tris += writeLineWall(stl, nextline, width, cos(globalLat), 0, 0);

  for (int row = 0; row < height; row++) {
    prevline = nextline;

    // getElevationLine(nextline, width, y, lat, lng, scaleFactor, rot, waterDrop, baseHeight, stepSize);
    nextline = &buf[row * width];
    tris += writeXStrip(stl, nextline, prevline, width, cos(globalLat), -row - 1, -row);
    fflush(stl);
  }

  // write other x wall
  // tris += writeLineWall(stl, nextline, width, cos(globalLat), 0, 1);
  tris += writeLineWall(stl, nextline, width, cos(globalLat), -height, 1);

  // add in the bottom of the model
  /*
  float xScale = cos(globalLat);
  struct _vect3 o = {10, -10, 0};
  for (int y = -height + 1; y <= 0; y++) {
    struct _vect3 lowerleft = {0, y - 1, 0};
    struct _vect3 upperleft = {0, y, 0};
    struct _vect3 lowerright = {(width - 1) * cos(globalLat), y - 1, 0};
    struct _vect3 upperright = {(width - 1) * cos(globalLat), y, 0};
    addTriangle(stl, createTriangle(lowerleft, upperleft, o));
    addTriangle(stl, createTriangle(upperright, lowerright, o));
    tris += 2;
  }
  for (int x = 0; x < width - 1; x++) {
    struct _vect3 upperleft = {x * xScale, 0, 0};
    struct _vect3 upperright = {(x + 1) * xScale, 0, 0};
    struct _vect3 lowerleft = {x * xScale, -height, 0};
    struct _vect3 lowerright = {(x + 1) * xScale, -height, 0};
    addTriangle(stl, createTriangle(upperleft, upperright, o));
    addTriangle(stl, createTriangle(lowerright, lowerleft, o));
    tris += 2;
  }
  */

  // set the number of triangles in the header to tris
  setSTLtriangles(stl, tris);
  fclose(stl);
}

void usage(char *prog) {
  printf("Usage: %s --north-west-corner <latitude>,<longitude>\n"
         "    --source <input-file.dem> --shape <input-shapfile.shp>\n"
         "    --rows <rows> --cols <columns> --stepsize <stepsize TODO>\n"
         "    --vscale <vertical scale> --rotation <rotation>\n"
         "    --waterdrop <water drop TODO units> --baseheight <base height mm>\n"
         "    --output <output file name>\n",
         prog);
}

int main(int argc, char **argv) {

  char pszFilename[100];
  char pszShapeFilename[100];
  char outputName[100] = {0};

  char opt;
  char *latlong;
  double lat, lng = 0.0;
  int rows, columns = 0;
  double userscale;
  double rot;
  int stepSize = 1;
  int waterDrop = -2; // millimeters
  int baseHeight = 2; // millimeters

  bool cornerIsSet = false;

  struct option long_options[] = {{"source", required_argument, 0, 's'},            // source DEM file
                                  {"shape", required_argument, 0, 'p'},             // source shape vector file
                                  {"north-west-corner", required_argument, 0, 'o'}, // 'o' for origin
                                  {"rows", required_argument, 0, 'r'},
                                  {"cols", required_argument, 0, 'c'},
                                  {"stepsize", required_argument, 0, 'z'},
                                  {"vscale", required_argument, 0, 'v'}, // vertical scale
                                  {"rotation", required_argument, 0, 'a'},
                                  {"waterdrop", required_argument, 0, 'w'},
                                  {"baseheight", required_argument, 0, 'b'},
                                  {"output", required_argument, 0, 'f'},
                                  {0, 0, 0, 0}};

  while ((opt = getopt_long(argc, argv, "", long_options, NULL)) != -1) {

    switch (opt) {
    case 'o':
      latlong = optarg;
      if (sscanf(latlong, "%lf,%lf", &lat, &lng) != 2) {
        fprintf(stderr, "Invalid format for --north-west-corner: expected <lat>,<lng>\n");
        return 1;
      }
      cornerIsSet = true;
      break;
    // Handle other options...
    case 's':
      // TODO handle snprintf failure (case of very long source file name)
      snprintf(pszFilename, 99, "%s", optarg);
      break;
    case 'p':
      // TODO handle snprintf failure (case of very long shape file name)
      snprintf(pszShapeFilename, 99, "%s", optarg);
      break;
    case 'f':
      // TODO handle snprintf failure (case of very long output file name)
      snprintf(outputName, 99, "%s", optarg);
      break;
    case 'r':
      rows = atoi(optarg);
      break;
    case 'c':
      columns = atoi(optarg);
      break;
    default:
      usage(argv[0]);
      return 1;
    }
  }

  if (cornerIsSet == false) {
    fprintf(stderr, "Latitude and Longitude must be provided.\n");
    usage(argv[0]);
    return 1;
  }

  if (rows < 1 || columns < 1) {
    fprintf(stderr, "Rows and columns but must be provided and greater than zero.\n");
    usage(argv[0]);
    return 1;
  }

  printf("Latitude: %lf, Longitude: %lf\n", lat, lng);

  int outputWidth = columns;
  int outputHeight = rows;

  // I think it refers to the stride of the output model in the widths of
  // SRTM 90m pixels
  // float true_verticalscale = 92.7;	//meters/arcsecond at equator
  // old vertical scale was 23.2
  double verticalscale = 92.7; // true_verticalscale gives models that are too flat to be interesting
  double scaleFactor = (userscale / verticalscale) / ((double)stepSize);

  // opening input file
  GDALDatasetH hDataset;
  GDALAllRegister();
  const GDALAccess eAccess = GA_ReadOnly;
  hDataset = GDALOpen(pszFilename, eAccess);
  if (hDataset == NULL) {
    printf("Unable to open file %s\n", pszFilename);
    return 1;
  }

  // opening input shape file
  GDALDatasetH hShapeDataset;
  hShapeDataset = GDALOpenEx(pszShapeFilename, GDAL_OF_VECTOR, NULL, NULL, NULL);

  if (hShapeDataset == NULL) {
    printf("Unable to open file %s\n", pszShapeFilename);
    return 1;
  }

  // create and open in-memory dataset
  float *strip = NULL;
  const char *inputProjection = GDALGetProjectionRef(hDataset);

  GDALDatasetH outputStripDset =
    makeMEMdatasetStrip(lat, lng, outputWidth, outputHeight, GDT_Float32, inputProjection, (void **)&strip);

  printDatasetInfo(hDataset);
  printDatasetInfo(outputStripDset);

  // warp code from: https://gdal.org/tutorials/warp_tut.html#a-simple-reprojection-case

  GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
  psWarpOptions->hSrcDS = hDataset;
  psWarpOptions->hDstDS = outputStripDset;
  psWarpOptions->nBandCount = 1;
  psWarpOptions->panSrcBands = (int *)CPLMalloc(sizeof(int) * psWarpOptions->nBandCount);
  psWarpOptions->panSrcBands[0] = 1;
  psWarpOptions->panDstBands = (int *)CPLMalloc(sizeof(int) * psWarpOptions->nBandCount);
  psWarpOptions->panDstBands[0] = 1;
  psWarpOptions->pfnProgress = GDALTermProgress; // TODO change this to minimize progress counter

  // Establish reprojection transformer.
  psWarpOptions->pTransformerArg = GDALCreateGenImgProjTransformer(
    hDataset, GDALGetProjectionRef(hDataset), outputStripDset, GDALGetProjectionRef(outputStripDset), FALSE, 0.0, 1);
  psWarpOptions->pfnTransformer = GDALGenImgProjTransform;

  // Initialize and execute the warp operation.
  GDALWarpOperation oOperation;
  oOperation.Initialize(psWarpOptions);
  oOperation.ChunkAndWarpImage(0, 0, GDALGetRasterXSize(outputStripDset), GDALGetRasterYSize(outputStripDset));
  GDALDestroyGenImgProjTransformer(psWarpOptions->pTransformerArg);
  GDALDestroyWarpOptions(psWarpOptions);

  // create rasterization output raster
  char *shape = NULL;
  GDALDatasetH shapeRaster =
    makeMEMdatasetStrip(lat, lng, outputWidth, outputHeight, GDT_Byte, inputProjection, (void **)&shape);

  // create rasterization options
  const char *rasterArgs[] = {"-burn", "1", "-l", "nz-lake-polygons-topo-1500k", NULL};
  GDALRasterizeOptions *psOptions = GDALRasterizeOptionsNew((char **)rasterArgs, NULL);
  GDALRasterizeOptionsSetProgress(psOptions, GDALTermProgress, nullptr);
  int bUsageError = FALSE;
  printf("Beginning vector burn\n");
  GDALDatasetH hRetDS = GDALRasterize(NULL, shapeRaster, hShapeDataset, psOptions, &bUsageError);
  if (bUsageError == TRUE) {
    printf("Rasterization error!\n");
    return 1;
  }
  GDALRasterizeOptionsFree(psOptions);

  for (int i = 0; i < outputWidth * outputHeight; i++) {
    strip[i] = strip[i] * scaleFactor - shape[i] * 10;
  }
  buffToSTL(outputWidth, outputHeight, strip, outputName, 45.0);

  // close in-memory dataset
  GDALClose(outputStripDset);
  CPLFree(strip);
  return 0;
}
