#include "cpl_conv.h" /* for CPLMalloc() */
#include "gdal.h"
#include "gdalwarper.h"

#include <errno.h>

// based on
// https://gdal.org/api/gdal_alg.html#_CPPv423GDALRasterizeGeometries12GDALDatasetHiPKiiPK12OGRGeometryH19GDALTransformerFuncPvPKd12CSLConstList16GDALProgressFuncPv
GDALDatasetH makeMEMdatasetStrip(int width, const char *inputProjection, void *dataBuffer) {

  // TODO: check that databuffer == 0 or NULL -
  // we're doing the allocation here so we want to make sure where not
  // dropping a pointer to some already-allocated memory

  int nBufXSize = width;
  int nBufYSize = 1;
  int nBandCount = 1;
  GDALDataType eType = GDT_Float32; // let the output buffer be float32s - good for interpolation
  int nDataTypeSize = GDALGetDataTypeSizeBytes(eType);

  void *pData = CPLCalloc(nBufXSize * nBufYSize * nBandCount, nDataTypeSize);
  char memdsetpath[1024];
  sprintf(memdsetpath,
          "MEM:::DATAPOINTER=0x%p,PIXELS=%d,LINES=%d,"
          "BANDS=%d,DATATYPE=%s,PIXELOFFSET=%d,LINEOFFSET=%d",
          pData, nBufXSize, nBufYSize, nBandCount, GDALGetDataTypeName(eType), nBandCount * nDataTypeSize,
          nBufXSize * nBandCount * nDataTypeSize);

  // Open Memory Dataset
  GDALDatasetH hMemDset = GDALOpen(memdsetpath, GA_Update);

  double adfGeoTransform[6];
  // TODO implement geotransform
  // Assign GeoTransform parameters,Omitted here.
  //
  GDALSetGeoTransform(hMemDset, adfGeoTransform);
  GDALSetProjection(hMemDset, inputProjection); // TODO pick a projection

  return hMemDset;
}

int main(int argc, const char *argv[]) {

  // ARGS
  if (argc != 2) {
    return EINVAL;
  }
  const char *pszFilename = argv[1];

  // opening input file

  GDALDatasetH hDataset;
  GDALAllRegister();
  const GDALAccess eAccess = GA_ReadOnly;
  hDataset = GDALOpen(pszFilename, eAccess);
  if (hDataset == NULL) {
    printf("Unable to open file %s\n", pszFilename);
    return 1;
  }

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

  // create and open in-memory dataset
  GDT_Float32 *strip = calloc(10, sizeof(GDT_Float32));
  const char *inputProjection = GDALGetProjectionRef(hDataset);

  GDALDatasetH outputStripDset = makeMEMdatasetStrip(10, inputProjection, (void *)strip);

  // warp code from: https://gdal.org/tutorials/warp_tut.html#a-simple-reprojection-case

  GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
  psWarpOptions->hSrcDS = hSrcDS;
  psWarpOptions->hDstDS = hDstDS;
  psWarpOptions->nBandCount = 1;
  psWarpOptions->panSrcBands = (int *)CPLMalloc(sizeof(int) * psWarpOptions->nBandCount);
  psWarpOptions->panSrcBands[0] = 1;
  psWarpOptions->panDstBands = (int *)CPLMalloc(sizeof(int) * psWarpOptions->nBandCount);
  psWarpOptions->panDstBands[0] = 1;
  psWarpOptions->pfnProgress = GDALTermProgress;

  // Establish reprojection transformer.
  psWarpOptions->pTransformerArg = GDALCreateGenImgProjTransformer(hSrcDS, GDALGetProjectionRef(hSrcDS), hDstDS,
                                                                   GDALGetProjectionRef(hDstDS), FALSE, 0.0, 1);
  psWarpOptions->pfnTransformer = GDALGenImgProjTransform;

  // Initialize and execute the warp operation.
  GDALWarpOperation oOperation;
  oOperation.Initialize(psWarpOptions);
  oOperation.ChunkAndWarpImage(0, 0, GDALGetRasterXSize(hDstDS), GDALGetRasterYSize(hDstDS));
  GDALDestroyGenImgProjTransformer(psWarpOptions->pTransformerArg);
  GDALDestroyWarpOptions(psWarpOptions);

  // close in-memory dataset
  GDALClose(outputStripDset);
  CPLFree(strip);
  return 0;
}
