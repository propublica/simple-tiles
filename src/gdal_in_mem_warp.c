#include "gdal_in_mem_warp.h"

GDALDatasetH
GDALWarpCreateOutput( GDALDatasetH hSrcDS, char *dstPtr,
                      const char *pszFormat, const char *pszSourceSRS,
                      const char *pszTargetSRS, int nOrder,
                      char **papszCreateOptions,
                      double dfMinX, double dfMinY, double dfMaxX, double dfMaxY,
                      double dfXRes, double dfYRes,
                      int nForcePixels, int nForceLines
                    )


{
    GDALDriverH hDriver;
    GDALDatasetH hDstDS;
    void *hTransformArg;
    double adfDstGeoTransform[6];
    int nPixels=0, nLines=0;
    GDALColorTableH hCT;

/* -------------------------------------------------------------------- */
/*      Find the output driver.                                         */
/* -------------------------------------------------------------------- */
    hDriver = GDALGetDriverByName( pszFormat );
    if( hDriver == NULL
        || GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATE, NULL ) == NULL )
    {
        int iDr;

        printf( "Output driver `%s' not recognised or does not support\n",
                pszFormat );
        printf( "direct output file creation.  The following format drivers are configured\n"
                "and support direct output:\n" );

        for( iDr = 0; iDr < GDALGetDriverCount(); iDr++ )
        {
            GDALDriverH hDriver = GDALGetDriver(iDr);

            if( GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATE, NULL) != NULL )
            {
                printf( "  %s: %s\n",
                        GDALGetDriverShortName( hDriver  ),
                        GDALGetDriverLongName( hDriver ) );
            }
        }
        printf( "\n" );
        exit( 1 );
    }

/* -------------------------------------------------------------------- */
/*      Create a transformation object from the source to               */
/*      destination coordinate system.                                  */
/* -------------------------------------------------------------------- */
    hTransformArg =
        GDALCreateGenImgProjTransformer( hSrcDS, pszSourceSRS,
                                         NULL, pszTargetSRS,
                                         TRUE, 1000.0, nOrder );

    if( hTransformArg == NULL )
        return NULL;

/* -------------------------------------------------------------------- */
/*      Get approximate output definition.                              */
/* -------------------------------------------------------------------- */
    if( GDALSuggestedWarpOutput( hSrcDS,
                                 GDALGenImgProjTransform, hTransformArg,
                                 adfDstGeoTransform, &nPixels, &nLines )
        != CE_None )
        return NULL;

    GDALDestroyGenImgProjTransformer( hTransformArg );

/* -------------------------------------------------------------------- */
/*      Did the user override some parameters?                          */
/* -------------------------------------------------------------------- */
    if( dfXRes != 0.0 && dfYRes != 0.0 )
    {
        CPLAssert( nPixels == 0 && nLines == 0 );
        if( dfMinX == 0.0 && dfMinY == 0.0 && dfMaxX == 0.0 && dfMaxY == 0.0 )
        {
            dfMinX = adfDstGeoTransform[0];
            dfMaxX = adfDstGeoTransform[0] + adfDstGeoTransform[1] * nPixels;
            dfMaxY = adfDstGeoTransform[3];
            dfMinY = adfDstGeoTransform[3] + adfDstGeoTransform[5] * nLines;
        }

        nPixels = (int) ((dfMaxX - dfMinX + (dfXRes/2.0)) / dfXRes);
        nLines = (int) ((dfMaxY - dfMinY + (dfYRes/2.0)) / dfYRes);
        adfDstGeoTransform[0] = dfMinX;
        adfDstGeoTransform[3] = dfMaxY;
        adfDstGeoTransform[1] = dfXRes;
        adfDstGeoTransform[5] = -dfYRes;
    }

    else if( nForcePixels != 0 && nForceLines != 0 )
    {
        if( dfMinX == 0.0 && dfMinY == 0.0 && dfMaxX == 0.0 && dfMaxY == 0.0 )
        {
            dfMinX = adfDstGeoTransform[0];
            dfMaxX = adfDstGeoTransform[0] + adfDstGeoTransform[1] * nPixels;
            dfMaxY = adfDstGeoTransform[3];
            dfMinY = adfDstGeoTransform[3] + adfDstGeoTransform[5] * nLines;
        }

        dfXRes = (dfMaxX - dfMinX) / nForcePixels;
        dfYRes = (dfMaxY - dfMinY) / nForceLines;

        adfDstGeoTransform[0] = dfMinX;
        adfDstGeoTransform[3] = dfMaxY;
        adfDstGeoTransform[1] = dfXRes;
        adfDstGeoTransform[5] = -dfYRes;

        nPixels = nForcePixels;
        nLines = nForceLines;
    }

    else if( dfMinX != 0.0 || dfMinY != 0.0 || dfMaxX != 0.0 || dfMaxY != 0.0 )
    {
        dfXRes = adfDstGeoTransform[1];
        dfYRes = fabs(adfDstGeoTransform[5]);

        nPixels = (int) ((dfMaxX - dfMinX + (dfXRes/2.0)) / dfXRes);
        nLines = (int) ((dfMaxY - dfMinY + (dfYRes/2.0)) / dfYRes);

        adfDstGeoTransform[0] = dfMinX;
        adfDstGeoTransform[3] = dfMaxY;
    }

/* -------------------------------------------------------------------- */
/*      Create the in memory dataset.                                   */
/* -------------------------------------------------------------------- */
    printf( "Creating dataset is that %dP x %dL.\n", nPixels, nLines );

    char memInitStr[255];
    snprintf(memInitStr,
                255, "MEM:::DATAPOINTER=%d,PIXELS=%i,LINES=%i,BANDS=3,DATATYPE=Byte",
                (long) dstPtr, nForcePixels, nForceLines);
    printf("%s\n", memInitStr);
    hDstDS = GDALOpen(memInitStr, GA_Update);

    if( hDstDS == NULL )
        return NULL;

/* -------------------------------------------------------------------- */
/*      Write out the projection definition.                            */
/* -------------------------------------------------------------------- */
    GDALSetProjection( hDstDS, pszTargetSRS );
    GDALSetGeoTransform( hDstDS, adfDstGeoTransform );

/* -------------------------------------------------------------------- */
/*      Copy the color table, if required.                              */
/* -------------------------------------------------------------------- */
    hCT = GDALGetRasterColorTable( GDALGetRasterBand(hSrcDS,1) );
    if( hCT != NULL )
        GDALSetRasterColorTable( GDALGetRasterBand(hDstDS,1), hCT );

    return hDstDS;
}
