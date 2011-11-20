#include "text.h"
#include "style.h"
#include "util.h"
#include "bounds.h"

typedef struct {
  PangoLayout *layout;
  double x;
  double y;
  simplet_bounds_t *bounds;
} placement_t;

simplet_lithograph_t *
simplet_lithograph_new(cairo_t *ctx, simplet_list_t *styles){
  simplet_lithograph_t *litho;
  if(!(litho = malloc(sizeof(*litho))))
    return NULL;

  memset(litho, 0, sizeof(*litho));

  if(!(litho->placements = simplet_list_new(litho))){
    free(litho);
    return NULL;
  }
  litho->styles = styles;
  litho->ctx = ctx;
  cairo_reference(ctx);

  return litho;
}

void
placement_vfree(void *placement){
  placement_t *plc = placement;
  simplet_bounds_free(plc->bounds);
  g_object_unref(plc->layout);
  free(plc);
}

void
simplet_lithograph_free(simplet_lithograph_t *litho){
  cairo_destroy(litho->ctx);
  simplet_list_set_item_free(litho->placements, placement_vfree);
  simplet_list_free(litho->placements);
  // style is not owned by the lithos, TODO: Copy it.
  free(litho);
}

placement_t *
placement_new(PangoLayout *layout, simplet_bounds_t *bounds){
  placement_t *placement;
  if(!(placement = malloc(sizeof(*placement))))
    return NULL;

  memset(placement, 0, sizeof(*placement));

  placement->layout = layout;
  placement->bounds = bounds;

  return placement;
}


void
try_placement(simplet_lithograph_t *litho, PangoLayout *layout, double x, double y){
  int width, height;
  pango_layout_get_pixel_size(layout, &width, &height);
  simplet_bounds_t *bounds = simplet_bounds_new();
  if(!bounds) return;
  simplet_bounds_extend(bounds, x - width / 2, y - height / 2);
  simplet_bounds_extend(bounds, x + width / 2 , y + height / 2);

  simplet_listiter_t *iter = simplet_get_list_iter(litho->placements);
  placement_t *placement;
  while((placement = (placement_t *) simplet_list_next(iter))){
    if(simplet_bounds_intersects(placement->bounds, bounds)){
      simplet_bounds_free(bounds);
      g_object_unref(layout);
      simplet_list_iter_free(iter);
      return;
    }
  }

  // if we get here we can create and insert a new placement;
  placement_t *plc = placement_new(layout, bounds);
  if(!plc) {
    simplet_bounds_free(bounds);
    g_object_unref(layout);
    return;
  }

  simplet_list_push(litho->placements, (void *)plc);
}

void
simplet_lithograph_apply(simplet_lithograph_t *litho){
  simplet_listiter_t *iter = simplet_get_list_iter(litho->placements);
  placement_t *placement;
  cairo_save(litho->ctx);
  while((placement = (placement_t *) simplet_list_next(iter))){
    cairo_move_to(litho->ctx, placement->bounds->nw.x, placement->bounds->se.y);
    pango_cairo_layout_path(litho->ctx, placement->layout);
  }
  simplet_apply_styles(litho->ctx, litho->styles, "text-halo-weight", "text-halo-color", "color",  NULL);
  cairo_restore(litho->ctx);
}

void
simplet_lithograph_add_placement(simplet_lithograph_t *litho,
  OGRFeatureH feature, cairo_t *proj_ctx) {

  simplet_style_t *field = simplet_lookup_style(litho->styles, "text-field");
  if(!field) return;

  OGRFeatureDefnH defn;
  if(!(defn = OGR_F_GetDefnRef(feature))) return;

  int idx = OGR_FD_GetFieldIndex(defn, (const char*) field->arg);
  if(idx < 0) return;


  OGRGeometryH super = OGR_F_GetGeometryRef(feature);
  OGRGeometryH geom = super;
  double area = 0.0;
  switch(wkbFlatten(OGR_G_GetGeometryType(super))) {
    case wkbMultiPolygon:
    case wkbGeometryCollection:
      for(int i = 0; i < OGR_G_GetGeometryCount(super); i++) {
        OGRGeometryH subgeom = OGR_G_GetGeometryRef(super, i);
        if(subgeom == NULL) continue;
        double ar = OGR_G_Area(subgeom);
        if(ar > area) {
          geom = subgeom;
          area = ar;
        }
      }
      break;
    default:
      ;
  }

  OGRGeometryH center;
  if(!(center = OGR_G_CreateGeometry(wkbPoint))) return;
  if(OGR_G_Centroid(geom, center) == OGRERR_FAILURE) {
    OGR_G_DestroyGeometry(center);
    return;
  }

  char *txt = simplet_copy_string(OGR_F_GetFieldAsString(feature, idx));
  PangoLayout *layout = pango_cairo_create_layout(litho->ctx);
  pango_layout_set_text(layout, txt, -1);
  free(txt);

  simplet_style_t *font = simplet_lookup_style(litho->styles, "font");
  const char *font_family;

  if(!font)
    font_family = "helvetica 12px";
  else
    font_family = font->arg;

  PangoFontDescription *desc = pango_font_description_from_string(font_family);
  pango_layout_set_font_description(layout, desc);
  pango_font_description_free(desc);

  double x = OGR_G_GetX(center, 0), y = OGR_G_GetY(center, 0);
  cairo_user_to_device(proj_ctx, &x, &y);
  try_placement(litho, layout, x, y);
  OGR_G_DestroyGeometry(center);
}
