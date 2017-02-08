#include "text.h"
#include "style.h"
#include "util.h"
#include "bounds.h"
#include "memory.h"
#include <math.h>

#define GTimer GTimer_GTK
#include <glib.h>
#include <glib-object.h>
#undef GTimer

// A storage structure that holds the current state of the layout.
typedef struct {
  PangoLayout *layout;
  int placed;
  simplet_bounds_t *bounds;
} placement_t;

// Create and return a new lithograph, returns NULL on failure.
simplet_lithograph_t *simplet_lithograph_new(cairo_t *ctx) {
  simplet_lithograph_t *litho;
  if (!(litho = malloc(sizeof(*litho)))) return NULL;

  memset(litho, 0, sizeof(*litho));

  if (!(litho->placements = simplet_list_new(litho))) {
    free(litho);
    return NULL;
  }

  litho->ctx = ctx;
  litho->pango_ctx = pango_cairo_create_context(ctx);

  cairo_reference(ctx);
  simplet_retain((simplet_retainable_t *)litho);
  return litho;
}

// Free a placement.
void placement_vfree(void *placement) {
  placement_t *plc = placement;
  simplet_bounds_free(plc->bounds);
  g_object_unref(plc->layout);
  free(plc);
}

// Free a lithograph and unref the stored ctx.
void simplet_lithograph_free(simplet_lithograph_t *litho) {
  if (simplet_release((simplet_retainable_t *)litho) > 0) return;

  cairo_destroy(litho->ctx);
  simplet_list_set_item_free(litho->placements, placement_vfree);
  g_object_unref(litho->pango_ctx);
  simplet_list_free(litho->placements);
  free(litho);
}

// Create and return a new placement.
placement_t *placement_new(PangoLayout *layout, simplet_bounds_t *bounds) {
  placement_t *placement;
  if (!(placement = malloc(sizeof(*placement)))) return NULL;

  memset(placement, 0, sizeof(*placement));

  placement->layout = layout;
  placement->bounds = bounds;
  placement->placed = FALSE;

  return placement;
}

// Before placing a new label we need to see if the label overlaps over
// previously placed labels. This algorithm will be refactored a bit to try
// NE SE SW NW placements in the future.
void try_and_insert_placement(simplet_lithograph_t *litho, PangoLayout *layout,
                              double x, double y) {
  int width, height;
  // Find the computed width and height of a layout in image pixels
  pango_layout_get_pixel_size(layout, &width, &height);
  simplet_bounds_t *bounds = simplet_bounds_new();
  if (!bounds) return;
  // Create a bounds to test for intersection
  simplet_bounds_extend(bounds, floor(x - width / 2), floor(y - height / 2));
  simplet_bounds_extend(bounds, floor(x + width / 2), floor(y + height / 2));

  // Iterate through the list of already placed labels and check for overlaps.
  simplet_listiter_t *iter = simplet_get_list_iter(litho->placements);
  placement_t *placement;
  while ((placement = (placement_t *)simplet_list_next(iter))) {
    if (simplet_bounds_intersects(placement->bounds, bounds)) {
      simplet_bounds_free(bounds);
      g_object_unref(layout);
      simplet_list_iter_free(iter);
      return;
    }
  }

  // If we get here we can create and insert a new placement.
  placement_t *plc = placement_new(layout, bounds);
  if (!plc) {
    simplet_bounds_free(bounds);
    g_object_unref(layout);
    return;
  }

  simplet_list_push(litho->placements, (void *)plc);
}

// Apply the labels to the map.
void simplet_lithograph_apply(simplet_lithograph_t *litho,
                              simplet_list_t *styles) {
  simplet_listiter_t *iter = simplet_get_list_iter(litho->placements);
  placement_t *placement;
  cairo_save(litho->ctx);
  while ((placement = (placement_t *)simplet_list_next(iter))) {
    if (placement->placed == TRUE) continue;
    cairo_move_to(litho->ctx, placement->bounds->nw.x, placement->bounds->se.y);

    // Draw the placement
    pango_cairo_layout_path(litho->ctx, placement->layout);

    placement->placed = TRUE;
  }
  simplet_apply_styles(litho->ctx, styles, "text-stroke-weight",
                       "text-stroke-color", "color", NULL);

  // Apply and draw various outline options.
  cairo_restore(litho->ctx);
}

// Create and add a placement to the current lithograph if it doesn't overlap
// with current labels.
void simplet_lithograph_add_placement(simplet_lithograph_t *litho,
                                      OGRFeatureH feature,
                                      simplet_list_t *styles,
                                      cairo_t *proj_ctx) {
  simplet_style_t *field = simplet_lookup_style(styles, "text-field");
  if (!field) return;

  OGRFeatureDefnH defn;
  if (!(defn = OGR_F_GetDefnRef(feature))) return;

  int idx = OGR_FD_GetFieldIndex(defn, (const char *)field->arg);
  if (idx < 0) return;

  // Find the largest sub geometry of a particular multi-geometry.
  OGRGeometryH super = OGR_F_GetGeometryRef(feature);
  OGRGeometryH geom = super;
  double area = 0.0;
  switch (wkbFlatten(OGR_G_GetGeometryType(super))) {
    case wkbMultiPolygon:
    case wkbGeometryCollection:
      for (int i = 0; i < OGR_G_GetGeometryCount(super); i++) {
        OGRGeometryH subgeom = OGR_G_GetGeometryRef(super, i);
        if (subgeom == NULL) continue;
        double ar = OGR_G_Area(subgeom);
        if (ar > area) {
          geom = subgeom;
          area = ar;
        }
      }
      break;
    default:;
  }

  // Find the center of our geometry. This sometimes throws an invalid geometry
  // error, so there is a slight bug here somehow.
  OGRGeometryH center;
  if (!(center = OGR_G_CreateGeometry(wkbPoint))) return;
  if (OGR_G_Centroid(geom, center) == OGRERR_FAILURE) {
    OGR_G_DestroyGeometry(center);
    return;
  }

  // Turn font hinting off
  cairo_font_options_t *opts;
  if (!(opts = cairo_font_options_create())) {
    OGR_G_DestroyGeometry(center);
    return;
  }

  cairo_font_options_set_hint_style(opts, CAIRO_HINT_STYLE_NONE);
  cairo_font_options_set_hint_metrics(opts, CAIRO_HINT_METRICS_OFF);
  pango_cairo_context_set_font_options(litho->pango_ctx, opts);
  cairo_font_options_destroy(opts);

  // Get the field containing the text for the label.
  char *txt = simplet_copy_string(OGR_F_GetFieldAsString(feature, idx));
  PangoLayout *layout = pango_layout_new(litho->pango_ctx);
  pango_layout_set_text(layout, txt, -1);
  free(txt);

  // Grab the font to use and apply tracking.
  simplet_style_t *font = simplet_lookup_style(styles, "font");
  simplet_apply_styles(layout, styles, "letter-spacing", NULL);

  const char *font_family;

  if (!font)
    font_family = "helvetica 12px";
  else
    font_family = font->arg;

  PangoFontDescription *desc = pango_font_description_from_string(font_family);
  pango_layout_set_font_description(layout, desc);
  pango_font_description_free(desc);

  double x = OGR_G_GetX(center, 0), y = OGR_G_GetY(center, 0);
  cairo_user_to_device(proj_ctx, &x, &y);

  // Finally try the placement and test for overlaps.
  try_and_insert_placement(litho, layout, x, y);
  OGR_G_DestroyGeometry(center);
}
