/*
 * Twin - A Tiny Window System
 * Copyright (c) 2004 Keith Packard <keithp@keithp.com>
 * All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>

#include "twin_private.h"

#include "apps_shape.h"
#define ASSET_PATH "assets/"
#define D(x) twin_double_to_fixed(x)

#define _apps_shape_pixmap(control) ((control)->widget.window->pixmap)

typedef struct _apps_control {
    twin_widget_t widget;
    twin_pixmap_t *pixmap;
    int n_points;
    twin_point_t *points;
    twin_point_t *oldpoints;
    int which;
    twin_fixed_t width;
    twin_matrix_t transition;
    twin_matrix_t inverse_transition;
} apps_control_t;

static void _init_control_point(apps_control_t *control)
{
    const int init_point_quad[3][2] = {
        {100, 100},
        {200, 100},
        {300, 100},
    };
    const int init_point_cubic[4][2] = {
        {100, 100},
        {300, 300},
        {100, 300},
        {300, 100},
    };
    const int(*init_point)[2];
    if (control->n_points == 4) {
        init_point = init_point_cubic;
    } else if (control->n_points == 3) {
        init_point = init_point_quad;
    }
    for (int i = 0; i < control->n_points; i++) {
        control->points[i].x = twin_int_to_fixed(init_point[i][0]);
        control->points[i].y = twin_int_to_fixed(init_point[i][1]);
    }
}

static void _apps_shape_paint(apps_control_t *control)
{
    twin_shape(control->pixmap, control->points, control->oldpoints,
               control->n_points);

    twin_path_t *path = twin_path_create();
    twin_pixmap_t *pixmap = control->widget.window->pixmap;
    twin_pixmap_t *background_pixmap = control->pixmap;
    twin_operand_t srcop = {
        .source_kind = TWIN_PIXMAP,
        .u.pixmap = background_pixmap,
    };
    twin_composite(pixmap, 0, 0, &srcop, 0, 0, 0, 0, 0, TWIN_SOURCE,
                   pixmap->width, pixmap->height);
    twin_path_set_matrix(path, control->transition);

    for (int i = 0; i < control->n_points; i++) {
        twin_path_empty(path);
        twin_path_circle(path, control->points[i].x, control->points[i].y,
                         twin_int_to_fixed(10));
        twin_paint_path(_apps_shape_pixmap(control), 0xffff0000, path);
    }
    twin_path_destroy(path);
}

static void _apps_shape_button_signal(maybe_unused twin_button_t *button,
                                      twin_button_signal_t signal,
                                      void *closure)
{
    if (signal != TwinButtonSignalDown)
        return;

    apps_control_t *control = closure;
    control->n_points = (control->n_points == 3) ? 4 : 3;
    _init_control_point(control);
    _twin_widget_queue_paint(&control->widget);
}

static twin_dispatch_result_t _apps_shape_update_pos(apps_control_t *control,
                                                     twin_event_t *event)
{
    if (control->which < 0)
        return TwinDispatchContinue;
    twin_fixed_t x = twin_int_to_fixed(event->u.pointer.x);
    twin_fixed_t y = twin_int_to_fixed(event->u.pointer.y);

    control->oldpoints[control->which].x = control->points[control->which].x;
    control->oldpoints[control->which].y = control->points[control->which].y;

    control->points[control->which].x = twin_sfixed_to_fixed(
        _twin_matrix_x(&(control->inverse_transition), x, y));
    control->points[control->which].y = twin_sfixed_to_fixed(
        _twin_matrix_y(&(control->inverse_transition), x, y));
    _twin_widget_queue_paint(&control->widget);
    twin_widget_children_paint((control->widget).parent);
    return TwinDispatchDone;
}

static int _apps_shape_hit(apps_control_t *control,
                           twin_fixed_t x,
                           twin_fixed_t y)
{
    int i;
    for (i = 0; i < control->n_points; i++) {
        twin_fixed_t px = twin_sfixed_to_fixed(
            _twin_matrix_x(&(control->transition), control->points[i].x,
                           control->points[i].y));
        twin_fixed_t py = twin_sfixed_to_fixed(
            _twin_matrix_y(&(control->transition), control->points[i].x,
                           control->points[i].y));
        if (twin_fixed_abs(x - px) < twin_int_to_fixed(10) &&
            twin_fixed_abs(y - py) < twin_int_to_fixed(10))
            return i;
    }
    return -1;
}

static twin_dispatch_result_t _apps_shape_dispatch(twin_widget_t *widget,
                                                   twin_event_t *event)
{
    apps_control_t *control = (apps_control_t *) widget;

    if (_twin_widget_dispatch(widget, event) == TwinDispatchDone)
        return TwinDispatchDone;
    switch (event->kind) {
    case TwinEventPaint:
        _apps_shape_paint(control);
        break;
    case TwinEventButtonDown:
        control->which =
            _apps_shape_hit(control, twin_int_to_fixed(event->u.pointer.x),
                            twin_int_to_fixed(event->u.pointer.y));
        return _apps_shape_update_pos(control, event);
        break;
    case TwinEventMotion:
        return _apps_shape_update_pos(control, event);
        break;
    case TwinEventButtonUp:
        if (control->which < 0)
            return TwinDispatchContinue;
        _apps_shape_update_pos(control, event);
        control->which = -1;
        return TwinDispatchDone;
        break;
    default:
        break;
    }
    return TwinDispatchContinue;
}


static void _apps_shape_init(apps_control_t *control,
                             twin_box_t *parent,
                             twin_dispatch_proc_t dispatch,
                             int n_points)
{
    static twin_widget_layout_t preferred = {0, 0, 1, 1};
    preferred.height = parent->widget.window->screen->height * 2 / 3;
    _twin_widget_init(&control->widget, parent, 0, preferred, dispatch);
    twin_matrix_identity(&control->transition);
    twin_matrix_rotate(&control->transition, TWIN_ANGLE_11_25);
    twin_matrix_identity(&control->inverse_transition);
    twin_matrix_rotate(&control->inverse_transition, -TWIN_ANGLE_11_25);
    control->points = calloc(n_points, sizeof(twin_point_t));
    control->oldpoints = calloc(n_points, sizeof(twin_point_t));
    control->n_points = n_points;
    control->width = twin_int_to_fixed(100);
    _init_control_point(control);
    twin_button_t *button =
        twin_button_create(parent, "Switch curve", 0x33ae0000, D(10),
                           TwinStyleBold | TwinStyleOblique);
    twin_widget_set(&button->label.widget, 0xc0808080);
    button->signal = _apps_shape_button_signal;
    button->closure = control;
    button->label.widget.shape = TwinShapeRectangle;


    twin_pixmap_t *raw_background = NULL;
#if defined(CONFIG_LOADER_PNG)
    raw_background = twin_pixmap_from_file(ASSET_PATH "tux.png", TWIN_ARGB32);
#endif
    if (!raw_background)
        return;

    twin_pixmap_t *scaled_background =
        twin_pixmap_create(TWIN_ARGB32, parent->widget.window->pixmap->width,
                           parent->widget.window->pixmap->height);
    twin_fixed_t sx, sy;
    sx =
        twin_fixed_div(twin_int_to_fixed(raw_background->width),
                       twin_int_to_fixed(parent->widget.window->pixmap->width));
    sy = twin_fixed_div(
        twin_int_to_fixed(raw_background->height),
        twin_int_to_fixed(parent->widget.window->pixmap->height));

    twin_matrix_scale(&raw_background->transform, sx, sy);
    twin_operand_t srcop = {
        .source_kind = TWIN_PIXMAP,
        .u.pixmap = raw_background,
    };

    twin_composite(scaled_background, 0, 0, &srcop, 0, 0, 0, 0, 0, TWIN_SOURCE,
                   scaled_background->width, scaled_background->height);
    control->pixmap = scaled_background;
    twin_pixmap_destroy(raw_background);
}

static apps_control_t *apps_shape_create(twin_box_t *parent, int n_points)
{
    apps_control_t *control = malloc(sizeof(apps_control_t));
    _apps_shape_init(control, parent, _apps_shape_dispatch, n_points);
    return control;
}

void apps_shape_start(twin_screen_t *screen,
                      const char *name,
                      int x,
                      int y,
                      int w,
                      int h)
{
    twin_toplevel_t *toplevel = twin_toplevel_create(
        screen, TWIN_ARGB32, TwinWindowApplication, x, y, w, h, name);
    apps_control_t *control = apps_shape_create(&toplevel->box, 4);
    (void) control;
    twin_toplevel_show(toplevel);
}
