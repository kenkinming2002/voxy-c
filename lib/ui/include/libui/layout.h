#ifndef LIBUI_LAYOUT_H
#define LIBUI_LAYOUT_H

#include <libmath/vector.h>

enum ui_anchor
{
  UI_ANCHOR_LOW,
  UI_ANCHOR_HIGH,
  UI_ANCHOR_CENTER,
};

struct ui_rect
{
  fvec2_t position;
  fvec2_t dimension;
  float rounding;
};

static inline float ui_rect_left(struct ui_rect rect);
static inline float ui_rect_right(struct ui_rect rect);

static inline float ui_rect_bottom(struct ui_rect rect);
static inline float ui_rect_top(struct ui_rect rect);

static inline float ui_rect_width(struct ui_rect rect);
static inline float ui_rect_height(struct ui_rect rect);

static inline void ui_rect_set_position_x(struct ui_rect *rect, enum ui_anchor anchor, float x);
static inline void ui_rect_set_position_y(struct ui_rect *rect, enum ui_anchor anchor, float y);
static inline void ui_rect_set_position(struct ui_rect *rect, enum ui_anchor anchor_x, enum ui_anchor anchor_y, fvec2_t position);

struct ui_grid
{
  unsigned width;
  unsigned height;

  fvec2_t position;
  fvec2_t spacing;

  fvec2_t dimension;
  float rounding;
};

static inline void ui_grid_set_position_x(struct ui_grid *grid, enum ui_anchor anchor, float x);
static inline void ui_grid_set_position_y(struct ui_grid *grid, enum ui_anchor anchor, float y);
static inline void ui_grid_set_position(struct ui_grid *grid, enum ui_anchor anchor_x, enum ui_anchor anchor_y, fvec2_t position);

static inline struct ui_rect ui_grid_get_rect_total(struct ui_grid grid);
static inline struct ui_rect ui_grid_get_rect_at(struct ui_grid grid, unsigned x, unsigned y);


static inline float ui_rect_left(struct ui_rect rect)
{
  return rect.position.x;
}

static inline float ui_rect_right(struct ui_rect rect)
{
  return rect.position.x + rect.dimension.x;
}

static inline float ui_rect_bottom(struct ui_rect rect)
{
  return rect.position.y;
}

static inline float ui_rect_top(struct ui_rect rect)
{
  return rect.position.y + rect.dimension.y;
}

static inline float ui_rect_width(struct ui_rect rect)
{
  return rect.dimension.x;
}

static inline float ui_rect_height(struct ui_rect rect)
{
  return rect.dimension.y;
}

static inline void ui_rect_set_position_x(struct ui_rect *rect, enum ui_anchor anchor, float x)
{
  rect->position.x = x;
  switch(anchor)
  {
  case UI_ANCHOR_LOW:
    break;
  case UI_ANCHOR_CENTER:
    rect->position.x -= rect->dimension.x * 0.5f;
    break;
  case UI_ANCHOR_HIGH:
    rect->position.x -= rect->dimension.x;
    break;
  }
}

static inline void ui_rect_set_position_y(struct ui_rect *rect, enum ui_anchor anchor, float y)
{
  rect->position.y = y;
  switch(anchor)
  {
  case UI_ANCHOR_LOW:
    break;
  case UI_ANCHOR_CENTER:
    rect->position.y -= rect->dimension.y * 0.5f;
    break;
  case UI_ANCHOR_HIGH:
    rect->position.y -= rect->dimension.y;
    break;
  }
}

static inline void ui_rect_set_position(struct ui_rect *rect, enum ui_anchor anchor_x, enum ui_anchor anchor_y, fvec2_t position)
{
  ui_rect_set_position_x(rect, anchor_x, position.x);
  ui_rect_set_position_y(rect, anchor_y, position.y);
}

static inline void ui_grid_set_position_x(struct ui_grid *grid, enum ui_anchor anchor, float x)
{
  grid->position.x = x;
  switch(anchor)
  {
  case UI_ANCHOR_LOW:
    break;
  case UI_ANCHOR_CENTER:
    grid->position.x -= ui_grid_get_rect_total(*grid).dimension.x * 0.5f;
    break;
  case UI_ANCHOR_HIGH:
    grid->position.x -= ui_grid_get_rect_total(*grid).dimension.x;
    break;
  }
}

static inline void ui_grid_set_position_y(struct ui_grid *grid, enum ui_anchor anchor, float y)
{
  grid->position.y = y;
  switch(anchor)
  {
  case UI_ANCHOR_LOW:
    break;
  case UI_ANCHOR_CENTER:
    grid->position.y -= ui_grid_get_rect_total(*grid).dimension.y * 0.5f;
    break;
  case UI_ANCHOR_HIGH:
    grid->position.y -= ui_grid_get_rect_total(*grid).dimension.y;
    break;
  }
}

static inline void ui_grid_set_position(struct ui_grid *grid, enum ui_anchor anchor_x, enum ui_anchor anchor_y, fvec2_t position)
{
  ui_grid_set_position_x(grid, anchor_x, position.x);
  ui_grid_set_position_y(grid, anchor_y, position.y);
}

static inline struct ui_rect ui_grid_get_rect_total(struct ui_grid grid)
{
  struct ui_rect rect;
  rect.position = grid.position;
  rect.dimension.x = grid.width  * grid.dimension.x + (grid.width  + 1) * grid.spacing.x;
  rect.dimension.y = grid.height * grid.dimension.y + (grid.height + 1) * grid.spacing.y;
  rect.rounding = grid.rounding;
  return rect;
}

static inline struct ui_rect ui_grid_get_rect_at(struct ui_grid grid, unsigned x, unsigned y)
{
  struct ui_rect rect;
  rect.position.x = grid.position.x + x * grid.dimension.x + (x + 1) * grid.spacing.x;
  rect.position.y = grid.position.y + y * grid.dimension.y + (y + 1) * grid.spacing.y;
  rect.dimension = grid.dimension;
  rect.rounding = grid.rounding;
  return rect;
}


#endif // LIBUI_LAYOUT_H
