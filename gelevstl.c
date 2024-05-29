#include "gdal_priv.h"
#include "gdalwarper.h"

#include "src/STLWriter.h"
#include "src/elevation.h"
#include <cstdio>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// based on
// https://gdal.org/api/gdal_alg.html#_CPPv423GDALRasterizeGeometries12GDALDatasetHiPKiiPK12OGRGeometryH19GDALTransformerFuncPvPKd12CSLConstList16GDALProgressFuncPv
GDALDatasetH makeMEMdatasetStrip(int width, int height, GDALDataType datatype, const char *inputProjection,
                                 void **pData) {

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
  adfGeoTransform[0] = -69.093;
  adfGeoTransform[3] = 44.22;
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

  int voidCutoff2 = 0;

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

  // tris += writeLineWall(stl, nextline, width, cos(globalLat), -height, 0);

  float *prevline;
  float *nextline = &buf[0];

  for (int row = 0; row < height; row++) {
    prevline = nextline;

    // getElevationLine(nextline, width, y, lat, lng, scaleFactor, rot, waterDrop, baseHeight, stepSize);
    nextline = &buf[row * width];
    tris += writeXStrip(stl, prevline, nextline, width, cos(globalLat), -row, -row-1);
    fflush(stl);
  }

  // write other x wall
  // tris += writeLineWall(stl, nextline, width, cos(globalLat), 0, 1);

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

int main(int argc, const char *argv[]) {

  // ARGS
  if (argc != 2) {
    return EINVAL;
  }
  const char *pszFilename = argv[1];

  int outputWidth = 100;
  int outputHeight = 200;

  float userscale = 1; // TODO make this an arg!
  float stepSize = 1;  // TODO make this an arg!
                       // I think it refers to the stride of the output model in the widths of
                       // SRTM 90m pixels
  float verticalscale = 92.7;
  float scaleFactor = (userscale / verticalscale) / ((float)stepSize);

  // opening input file

  GDALDatasetH hDataset;
  GDALAllRegister();
  const GDALAccess eAccess = GA_ReadOnly;
  hDataset = GDALOpen(pszFilename, eAccess);
  if (hDataset == NULL) {
    printf("Unable to open file %s\n", pszFilename);
    return 1;
  }

  // create and open in-memory dataset
  float *strip = NULL;
  const char *inputProjection = GDALGetProjectionRef(hDataset);

  GDALDatasetH outputStripDset =
    makeMEMdatasetStrip(outputWidth, outputHeight, GDT_Float32, inputProjection, (void **)&strip);

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

  // do something with the data in strip!
  /*
  for (int i = 0; i < outputWidth; i++) {
    printf("% 5.1lf  ", strip[i]);
  }
  printf("\n");
  */

  for (int i = 0; i < outputWidth * outputHeight; i++) {
    strip[i] = strip[i] * scaleFactor;
  }
  buffToSTL(outputWidth, outputHeight, strip, "test.stl", 45.0);

  // close in-memory dataset
  GDALClose(outputStripDset);
  CPLFree(strip);
  return 0;
}
