/*
 * Twin - A Tiny Window System
 * Copyright (c) 2004 Keith Packard <keithp@keithp.com>
 * All rights reserved.
 */

#include "twin_private.h"

/*
 * Find the point in path which is furthest left of the line
 */
static int _twin_path_leftpoint(twin_path_t *path,
                                const twin_spoint_t *p1,
                                const twin_spoint_t *p2)
{
    twin_spoint_t *points = path->points;
    int best = 0;
    /*
     * Normal form of the line is Ax + By + C = 0,
     * these are the A and B factors.  As we're just comparing
     * across x and y, the value of C isn't relevant
     */
    twin_dfixed_t Ap = p2->y - p1->y;
    twin_dfixed_t Bp = p1->x - p2->x;

    twin_dfixed_t max = -0x7fffffff;

    for (int p = 0; p < path->npoints; p++) {
        twin_dfixed_t vp = Ap * points[p].x + Bp * points[p].y;

        if (vp > max) {
            max = vp;
            best = p;
        }
    }
    return best;
}

static int _around_order(const twin_spoint_t *a1,
                         const twin_spoint_t *a2,
                         const twin_spoint_t *b1,
                         const twin_spoint_t *b2)
{
    twin_dfixed_t adx = (a2->x - a1->x);
    twin_dfixed_t ady = (a2->y - a1->y);
    twin_dfixed_t bdx = (b2->x - b1->x);
    twin_dfixed_t bdy = (b2->y - b1->y);
    twin_dfixed_t diff = (ady * bdx - bdy * adx);

    if (diff < 0)
        return -1;
    if (diff > 0)
        return 1;
    return 0;
}

#define F(x) twin_sfixed_to_double(x)

#define A(a) ((a) < 0 ? -(a) : (a))

bool trainge(twin_spoint_t *prev, twin_spoint_t *cur, twin_spoint_t *next) {
    // Get squared lengths of sides of the triangle
    twin_sfixed_t a2 = twin_sfixed_mul(prev->x - cur->x, prev->x - cur->x) + 
                        twin_sfixed_mul(prev->y - cur->y, prev->y - cur->y);
    twin_sfixed_t b2 = twin_sfixed_mul(cur->x - next->x, cur->x - next->x) + 
                        twin_sfixed_mul(cur->y - next->y, cur->y - next->y);
    twin_sfixed_t c2 =  twin_sfixed_mul(next->x - prev->x, next->x - prev->x) + 
                        twin_sfixed_mul(next->y - prev->y, next->y - prev->y);
    
    // Check for acute triangle:
    // For an acute triangle, the sum of the squares of any two sides must be greater than the square of the remaining side.
    return (a2 + b2 > c2) && (b2 + c2 > a2) && (c2 + a2 > b2);
}
/*
 * Convolve one subpath with a convex pen.  The result is
 * a closed path.
 */
static void _twin_subpath_convolve(twin_path_t *path,
                                   twin_path_t *stroke,
                                   twin_path_t *pen)
{
    twin_spoint_t *sp = stroke->points;
    twin_spoint_t *pp = pen->points;
    int ns = stroke->npoints;
    int np = pen->npoints;
    twin_spoint_t *sp0 = &sp[0];
    twin_spoint_t *sp1 = &sp[1];
    int start = _twin_path_leftpoint(pen, sp0, sp1);
    twin_spoint_t *spn1 = &sp[ns - 1];
    twin_spoint_t *spn2 = &sp[ns - 2];
    int ret = _twin_path_leftpoint(pen, spn1, spn2);
    int p;
    int s;
    int starget;
    int ptarget;
    int inc;
    int first;

    s = 0;
    p = start;
    _twin_path_smove(path, sp[s].x + pp[p].x, sp[s].y + pp[p].y);
    // printf("%d\t%d\n",  sp[s].x + pp[p].x, sp[s].y + pp[p].y);
    first = path->npoints - 1;

    /* step along the path first */
    inc = 1;
    starget = ns - 1;
    ptarget = ret;
    for (;;) {
        /*
         * Convolve the edges
         */
        do {
            int sn = s + inc;
            int pn = (p == np - 1) ? 0 : p + 1;
            int pm = (p == 0) ? np - 1 : p - 1;

            /*
             * step along pen (forwards or backwards) or stroke as appropriate
             */
            if (_around_order(&sp[s], &sp[sn], &pp[p], &pp[pn]) > 0) {
                p = pn;
            } else if (_around_order(&sp[s], &sp[sn], &pp[pm], &pp[p]) < 0) {
                p = pm;
            } else {
                s = sn;
            }
            _twin_path_sdraw(path, sp[s].x + pp[p].x, sp[s].y + pp[p].y);
            // printf("%d\t%d\n",  sp[s].x + pp[p].x, sp[s].y + pp[p].y);
        } while (s != starget);

        /*
         * Finish this edge
         */

        /* draw a cap */
        switch (path->state.cap_style) {
            int pm;
        case TwinCapProjecting:
            /*
             * This draws a rough projecting cap using the
             * pen.
             *
             * First, project the line forward one pen radius
             * by finding the pen location halfway between the
             * two normals.
             *
             * Then, just add that to the normals themselves to
             * find the corners of the projecting cap.
             *
             * The result may have significant error, so overwrite
             * the existing corners with the new coordinates to
             * avoid a kink.
             */
            if (p <= ptarget)
                pm = (ptarget + p) >> 1;
            else {
                pm = (ptarget + np + p) >> 1;
                if (pm >= np)
                    pm -= np;
            }

            /* replace last point with corner of cap */
            path->npoints--;
            _twin_path_sdraw(path, sp[s].x + pp[pm].x + pp[p].x,
                             sp[s].y + pp[pm].y + pp[p].y);
            // printf("%d\t%d\n", sp[s].x + pp[pm].x + pp[p].x,
            //                  sp[s].y + pp[pm].y + pp[p].y);
            p = ptarget;
            if (inc == 1) {
                /* start next line at cap corner */
                _twin_path_sdraw(path, sp[s].x + pp[pm].x + pp[p].x,
                                 sp[s].y + pp[pm].y + pp[p].y);
                // printf("%d\t%d\n",sp[s].x + pp[pm].x + pp[p].x,
                //                 sp[s].y + pp[pm].y + pp[p].y);
            } else {
                /* overwrite initial point */
                path->points[first].x = sp[s].x + pp[pm].x + pp[p].x;
                path->points[first].y = sp[s].y + pp[pm].y + pp[p].y;
            }
            break;
        case TwinCapButt:
            p = ptarget - 1;
            fallthrough;
        case TwinCapRound:
            while (p != ptarget) {
                if (++p == np)
                    p = 0;
                _twin_path_sdraw(path, sp[s].x + pp[p].x, sp[s].y + pp[p].y);
                // printf("%d\t%d\n",sp[s].x + pp[p].x, sp[s].y + pp[p].y);
            }
            break;
        }

        if (inc == -1)
            break;

        /* reach the end of the path?  Go back the other way now */
        inc = -1;
        ptarget = start;
        starget = 0;
    }
    int start = 0, end;
    twin_path_t * evolute = twin_path_create();
    for(int i=1;i<path->npoints-1;i++){
        if(trainge(&path->points[i-1],&path->points[i],&path->points[i+1])){
            for(int j=start;j<i;j++){
                twin_sfixed_t x = twin_fixed_to_sfixed(twin_cos(j)) << 1 + twin_fixed_to_sfixed(twin_cos(j<<1));
                twin_sfixed_t y = twin_fixed_to_sfixed(twin_sin(j)) << 1 - twin_fixed_to_sfixed(twin_sin(j<<1));
                twin_dfixed_t x1 = twin_dfixed_div(twin_dfixed_mul(twin_sfixed_to_dfixed(x),radius),twin_int_to_dfixed(3));
                twin_dfixed_t y1 = twin_dfixed_div(twin_dfixed_mul(twin_sfixed_to_dfixed(y),radius),twin_int_to_dfixed(3));
                twin_sfixed_t x_rot = twin_dfixed_to_sfixed(twin_dfixed_mul(x1,cos) - twin_dfixed_mul(y,sin));
                twin_sfixed_t y_rot = twin_dfixed_to_sfixed(twin_dfixed_mul(x1,sin) + twin_dfixed_mul(y,cos));
                _twin_path_sdraw(evolute,  x_rot + center.x, y_rot + center.y);
            }
        }
        
    }
    twin_path_close(path);
}

void twin_path_convolve(twin_path_t *path,
                        twin_path_t *stroke,
                        twin_path_t *pen)
{
    int p;
    int s;
    twin_path_t *hull = twin_path_convex_hull(pen);

    p = 0;
    // printf("#######################################\n");
    for (s = 0; s <= stroke->nsublen; s++) {
        int sublen;
        int npoints;

        if (s == stroke->nsublen)
            sublen = stroke->npoints;
        else
            sublen = stroke->sublen[s];
        npoints = sublen - p;
        if (npoints > 1) {
            twin_path_t subpath = {
                .points = stroke->points + p,
                .npoints = npoints,
                .sublen = 0,
                .nsublen = 0,
            };
            _twin_subpath_convolve(path, &subpath, hull);
            p = sublen;
        }
    }
    twin_path_destroy(hull);
}
