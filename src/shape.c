/*
 * Twin - A Tiny Window System
 * Copyright (c) 2024 National Cheng Kung University, Taiwan
 * All rights reserved.
 */

#include "twin_private.h"

typedef struct _twin_spline {
    twin_spoint_t a, b, c;
} twin_triangle_t;

static twin_triangle_t * triangle(twin_coord_t x_start,
                                  twin_coord_t x_end,
                                  twin_coord_t y_start,
                                  twin_coord_t y_end)
{
    int y_number = (y_end - y_start) / 60;
    int x_number = (x_end - x_start) / 60;
    int number = 2 * y_number * x_number;
    twin_triangle_t *triangles = calloc(number, sizeof(twin_triangle_t));

    for(int j = 0;j < y_number; j++)
    {
        for(int i = 0 ;i < x_number; i++){
            triangles[j*2*x_number + i].a.x = x_start + 60*i;
            triangles[j*2*x_number + i].b.x = x_start + 60*(i+1);
            triangles[j*2*x_number + i].c.x = x_start + 60*i;

            triangles[j*2*x_number + i].a.y = y_start + 60*j;
            triangles[j*2*x_number + i].b.y = y_start + 60*j;
            triangles[j*2*x_number + i].c.y = y_start + 60*(j+1);
        }
        for(int i = 0 ;i < x_number; i++){
            triangles[(j*2+1)*x_number + i].a.x = x_start + 60*(i+1);
            triangles[(j*2+1)*x_number + i].b.x = x_start + 60*(i+1);
            triangles[(j*2+1)*x_number + i].c.x = x_start + 60*i;

            triangles[(j*2+1)*x_number + i].a.y = y_start + 60*j;
            triangles[(j*2+1)*x_number + i].b.y = y_start + 60*(j+1);
            triangles[(j*2+1)*x_number + i].c.y = y_start + 60*(j+1);
        }
    }
    for(int i=0;i<number;i++){
        printf("a : (%d, %d), b : (%d, %d), c : (%d, %d)\n",triangles[i].a.x,
                                                          triangles[i].a.y,
                                                          triangles[i].b.x,
                                                          triangles[i].b.y,
                                                          triangles[i].c.x,
                                                          triangles[i].c.y);
    }
    return triangles;
}

void twin_shape(twin_pixmap_t *pixmap,
                twin_point_t *points,
                twin_point_t *oldpoints,
                int n_points)
{
    twin_coord_t x_start = 60;
    twin_coord_t x_end = (pixmap->width - pixmap->width %60) - x_start;
    twin_coord_t y_start = 60;
    twin_coord_t y_end = (pixmap->height - pixmap->height %60) - x_start;

    twin_triangle_t * triangles = triangle(x_start, x_end, y_start, y_end);
    return;
}
