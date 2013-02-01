#include <ogr_api.h>
#include <ogr_srs_api.h>
#include <assert.h>

int main(void) {
  OGRGeometryH tmpLine;
  tmpLine = OGR_G_CreateGeometry(wkbLineString);
  assert(tmpLine);

  double nwx = -17532819.797499999;
  double nwy = 10018754.17;
  double sx  = -15028131.254999999;
  double sy  = 7514065.6274999995;

  // Add all the points defining the bounds to the geometry
  OGR_G_AddPoint_2D(tmpLine, nwx, nwy);
  OGR_G_AddPoint_2D(tmpLine, sx, sy);
  OGR_G_AddPoint_2D(tmpLine, nwx, sy);
  OGR_G_AddPoint_2D(tmpLine, sx, nwy);

  // Calculate the Convex Hull
  OGRGeometryH ogrBounds;
  if(!(ogrBounds = OGR_G_ConvexHull(tmpLine))){
    OGR_G_DestroyGeometry(tmpLine);
    puts("ohno");
  }

  OGR_G_DestroyGeometry(tmpLine);
  OGR_G_DestroyGeometry(ogrBounds);

  puts("yay!");

  return 0;
}
