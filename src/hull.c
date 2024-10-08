/*
 * Twin - A Tiny Window System
 * Copyright (c) 2004 Carl Worth <cworth@cworth.org>
 * All rights reserved.
 */

#include <stdbool.h>
#include <stdlib.h>

#include "twin_private.h"

typedef struct twin_slope {
    twin_sfixed_t dx;
    twin_sfixed_t dy;
} twin_slope_t, twin_distance_t;

typedef struct _twin_hull {
    twin_spoint_t point;
    twin_slope_t slope;
    bool discard;
} twin_hull_t;

static void _twin_slope_init(twin_slope_t *slope,
                             const twin_spoint_t *a,
                             const twin_spoint_t *b)
{
    slope->dx = b->x - a->x;
    slope->dy = b->y - a->y;
}

static twin_hull_t *_twin_hull_create(const twin_path_t *path, int *nhull)
{
    int n = path->npoints;
    const twin_spoint_t *p = path->points;
    twin_hull_t *hull;

    int e = 0;
    for (int i = 1; i < n; i++)
        if (p[i].y < p[e].y || (p[i].y == p[e].y && p[i].x < p[e].x))
            e = i;

    hull = malloc(n * sizeof(twin_hull_t));
    if (!hull)
        return NULL;
    *nhull = n;

    for (int i = 0, j; i < n; i++) {
        /* place extremum first in array */
        if (i == 0)
            j = e;
        else if (i == e)
            j = 0;
        else
            j = i;

        hull[i].point = p[j];
        _twin_slope_init(&hull[i].slope, &hull[0].point, &hull[i].point);

        /* Discard all points coincident with the extremal point */
        if (i != 0 && hull[i].slope.dx == 0 && hull[i].slope.dy == 0)
            hull[i].discard = true;
        else
            hull[i].discard = false;
    }

    return hull;
}

/* Compare two slopes. Slope angles begin at 0 in the direction of the
   positive X axis and increase in the direction of the positive Y
   axis.

   WARNING: This function only gives correct results if the angular
   difference between a and b is less than PI.

   <  0 => a less positive than b
   == 0 => a equal to be
   >  0 => a more positive than b
*/
static int _twin_slope_compare(const twin_slope_t *a, const twin_slope_t *b)
{
    twin_dfixed_t diff = ((twin_dfixed_t) a->dy * (twin_dfixed_t) b->dx -
                          (twin_dfixed_t) b->dy * (twin_dfixed_t) a->dx);

    if (diff > 0)
        return 1;
    if (diff < 0)
        return -1;

    if (a->dx == 0 && a->dy == 0)
        return 1;
    if (b->dx == 0 && b->dy == 0)
        return -1;

    return 0;
}

static int _twin_hull_vertex_compare(const void *av, const void *bv)
{
    twin_hull_t *a = (twin_hull_t *) av;
    twin_hull_t *b = (twin_hull_t *) bv;

    int ret = _twin_slope_compare(&a->slope, &b->slope);

    /* In the case of two vertices with identical slope from the
       extremal point discard the nearer point. */

    if (ret == 0) {
        twin_dfixed_t a_dist, b_dist;
        a_dist = ((twin_dfixed_t) a->slope.dx * a->slope.dx +
                  (twin_dfixed_t) a->slope.dy * a->slope.dy);
        b_dist = ((twin_dfixed_t) b->slope.dx * b->slope.dx +
                  (twin_dfixed_t) b->slope.dy * b->slope.dy);
        if (a_dist < b_dist) {
            a->discard = true;
            ret = -1;
        } else {
            b->discard = true;
            ret = 1;
        }
    }

    return ret;
}

static int _twin_hull_prev_valid(const twin_hull_t *hull,
                                 int maybe_unused num_hull,
                                 int index)
{
    do {
        /* hull[0] is always valid, so don't test and wraparound */
        index--;
    } while (hull[index].discard);

    return index;
}

static int _twin_hull_next_valid(const twin_hull_t *hull,
                                 int num_hull,
                                 int index)
{
    do {
        index = (index + 1) % num_hull;
    } while (hull[index].discard);

    return index;
}

/*
 * Graham scan to compute convex hull
 */

static void _twin_hull_eliminate_concave(twin_hull_t *hull, int num_hull)
{
    twin_slope_t slope_ij, slope_jk;

    int i = 0;
    int j = _twin_hull_next_valid(hull, num_hull, i);
    int k = _twin_hull_next_valid(hull, num_hull, j);

    do {
        _twin_slope_init(&slope_ij, &hull[i].point, &hull[j].point);
        _twin_slope_init(&slope_jk, &hull[j].point, &hull[k].point);

        /* Is the angle formed by ij and jk concave? */
        if (_twin_slope_compare(&slope_ij, &slope_jk) >= 0) {
            if (i == k)
                break;
            hull[j].discard = true;
            j = i;
            i = _twin_hull_prev_valid(hull, num_hull, j);
        } else {
            i = j;
            j = k;
            k = _twin_hull_next_valid(hull, num_hull, j);
        }
    } while (j != 0);
}

/*
 * Convert the hull structure back to a simple path
 */
static twin_path_t *_twin_hull_to_path(const twin_hull_t *hull, int num_hull)
{
    twin_path_t *path = twin_path_create();

    for (int i = 0; i < num_hull; i++) {
        if (hull[i].discard)
            continue;
        _twin_path_sdraw(path, hull[i].point.x, hull[i].point.y);
    }

    return path;
}

/*
 * Given a path, return the convex hull using the Graham scan algorithm.
 */

twin_path_t *twin_path_convex_hull(twin_path_t *path)
{
    twin_hull_t *hull;
    int num_hull;
    twin_path_t *convex_path;

    hull = _twin_hull_create(path, &num_hull);
    if (!hull)
        return NULL;

    qsort(hull + 1, num_hull - 1, sizeof(twin_hull_t),
          _twin_hull_vertex_compare);

    _twin_hull_eliminate_concave(hull, num_hull);

    convex_path = _twin_hull_to_path(hull, num_hull);

    free(hull);

    return convex_path;
}
