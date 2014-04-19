// modified from gdalwarpsimple.c
// https://gist.github.com/thejefflarson/a690c76145b7d36d55f5
#include <gdal.h>
#include "gdal_alg.h"
#include "cpl_string.h"
#include "ogr_srs_api.h"

// dfMinX, dfMinY, dfMaxX, dfMaxY
//    georeferenced extents of output file to be created

// dfXRes, dfYRes
//    output file resolution (in target georeferenced units)

// forcePixels, forceLines
//    output file size in pixels and lines

// *dstPtr
//    ptr in which to store the in mem dataset
//    e.g. malloc(sizeof(char) * width * height * 4);

static GDALDatasetH 
GDALWarpCreateOutput( GDALDatasetH hSrcDS, char *dstPtr,
                      const char *pszFormat, const char *pszSourceSRS, 
                      const char *pszTargetSRS, int nOrder, 
                      char **papszCreateOptions, 
                      double dfMinX, double dfMinY, double dfMaxX, double dfMaxY,
                      double dfXRes, double dfYRes,
                      int nForcePixels, int nForceLines
                    );

// static double        dfMinX=0.0, dfMinY=0.0, dfMaxX=0.0, dfMaxY=0.0;
// static double        dfXRes=0.0, dfYRes=0.0;
// static int           nForcePixels=0, nForceLines=0;
