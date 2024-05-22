#include "cpl_conv.h" /* for CPLMalloc() */
#include "gdal.h"

#include <errno.h>

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

  return 0;
}
