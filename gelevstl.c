#include "gdal.h"
#include "cpl_conv.h" /* for CPLMalloc() */

#include <errno.h>

int main(int argc, const char* argv[])
{
    if (argc != 2) {
        return EINVAL;
    }
    const char* pszFilename = argv[1];

    GDALDatasetH  hDataset;
    GDALAllRegister();
    const GDALAccess eAccess = GA_ReadOnly;
    hDataset = GDALOpen( pszFilename, eAccess );
    if( hDataset == NULL )
    {
//        ...; // handle error
    }
    return 0;
}
