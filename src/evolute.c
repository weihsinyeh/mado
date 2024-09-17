#include "twin_private.h"

void midpoint(twin_spoint_t *a, twin_spoint_t *b, twin_spoint_t* mid) 
{
    mid->x = twin_dfixed_div(twin_sfixed_to_dfixed(a->x + b->x),twin_int_to_dfixed(2));
    mid->y = twin_dfixed_div(twin_sfixed_to_dfixed(a->y + b->y),twin_int_to_dfixed(2));
}

void line_parameters(twin_spoint_t *a, 
                    twin_spoint_t *b,
                    twin_dfixed_t *slope,
                    twin_dfixed_t *intercept) {
    twin_spoint_t mid;
    midpoint(a, b, &mid);
    bool vertical_slope = true;
    if (b->x - a->x == 0) 
        vertical_slope = false;
        *slope = NULL;
        *intercept = mid.x;
    
    *slope = -1 * twin_dfixed_div(twin_sfixed_to_dfixed(b->x - a->x),twin_sfixed_to_dfixed(b->y - a->y));
    *intercept = mid.y - twin_dfixed_mul(slope,twin_sfixed_to_dfixed(mid.x));
}

twin_spoint_t* find_center(twin_spoint_t *a, 
                twin_spoint_t *b, 
                twin_spoint_t *mid) {
    
    twin_dfixed_t slope1, intercept1, slope2, intercept2;
    twin_spoint_t center;
    line_parameters(a, b, &slope1, &intercept1);
    line_parameters(b, mid, &slope2, &intercept2);

    if (slope1 == NULL) {
        center.x = twin_dfixed_to_sfixed(intercept1);
        center.y = twin_dfixed_to_sfixed(
                        twin_dfixed_mul(slope2,twin_sfixed_to_dfixed(center.x)) 
                        + intercept2);
    } else if (slope2 == NULL) {
        center.x = twin_dfixed_to_sfixed(intercept2);
        center.y = twin_dfixed_to_sfixed(
                        twin_dfixed_mul(slope1,twin_sfixed_to_dfixed(center.y))
                        + intercept1);
    } else {        
        center.x = twin_dfixed_to_sfixed(
                        twin_dfixed_div(twin_sfixed_to_dfixed(intercept2 - intercept1),
                                        twin_sfixed_to_dfixed(slope1 - slope2)));
        center.y = twin_dfixed_to_sfixed(   
                        twin_dfixed_mul(slope1,twin_sfixed_to_dfixed(center.x))
                        + intercept1);
    }
    return &center;
}
twin_sfixed_t distance(twin_spoint_t *a, twin_spoint_t *b) {
    return  _twin_sfixed_sqrt(
            twin_dfixed_to_sfixed(twin_dfixed_mul(twin_sfixed_to_dfixed(b->x - a->x),twin_sfixed_to_dfixed(b->x - a->x))) + 
            twin_dfixed_to_sfixed(twin_dfixed_mul(twin_sfixed_to_dfixed(b->y - a->y),twin_sfixed_to_dfixed(b->y - a->x))));
}
void find_circle_center_and_radius( twin_spoint_t *a, 
                                    twin_spoint_t *b, 
                                    twin_spoint_t *mid,
                                    twin_spoint_t *center,
                                    twin_dfixed_t *radius){
    double pi = 3.14159265358979323846;
    twin_dfixed_t PI = twin_double_to_dfixed(pi);
    twin_dfixed_t r_x = b->x-a->x;
    twin_dfixed_t r_y = b->y-a->y;
    twin_spoint_t *center = find_center(a, b, mid);
    // check a b c counter-clockwise
    // check a c b clockwise

    center->x += -twin_dfixed_div(r_y,(2 * _twin_sfixed_sqrt(twin_int_to_sfixed(3))));
    center->y += twin_dfixed_div(r_x,(2 * _twin_sfixed_sqrt(twin_int_to_sfixed(3))));

    // Calculate radius
    twin_sfixed_t dist = distance(a, b);
    *radius =  twin_dfixed_div(twin_sfixed_to_dfixed(dist), 
                                twin_sfixed_to_dfixed(_twin_sfixed_sqrt(twin_int_to_sfixed(3))));
}
void calculate_normal_angle(  twin_spoint_t *a, twin_spoint_t *b, 
                                        twin_dfixed_t *cos, twin_dfixed_t *sin){
    twin_sfixed_t dx = b->x - a->x;
    twin_sfixed_t dy = b->y - a->y;
    twin_dfixed_t dist = twin_sfixed_to_dfixed(
                            _twin_sfixed_sqrt(twin_dfixed_to_sfixed(twin_dfixed_mul(twin_sfixed_to_dfixed(dx),twin_sfixed_to_dfixed(dx)) + 
                                              twin_dfixed_to_sfixed(twin_dfixed_mul(twin_sfixed_to_dfixed(dy),twin_sfixed_to_dfixed(dy))))));
    *cos = twin_dfixed_div(twin_sfixed_to_dfixed(dx),dist);
    *sin = twin_dfixed_div(twin_sfixed_to_dfixed(dy),dist);
}
void evolute(twin_path_t* path){
    twin_spoint_t a = path->points[0];
    twin_spoint_t b = path->points[path->npoints-1];
    twin_spoint_t mid = path->points[path->npoints>>1];

    twin_spoint_t center;
    twin_dfixed_t radius;
    find_circle_center_and_radius(&a,&b,&mid,&center,&radius);
    twin_dfixed_t cos, sin;
    calculate_normal_angle(&a,&b,&cos,&sin);

    twin_angle_t start_angle =  twin_degrees_to_angle(0);
    twin_angle_t end_angle = twin_degrees_to_angle(240);    

    twin_path_t *evolute = twin_create_path();
    for (twin_angle_t i = start_angle; i< end_angle;i+=20){
        twin_sfixed_t x = twin_fixed_to_sfixed(twin_cos(i)) << 1 + twin_fixed_to_sfixed(twin_cos(i<<1));
        twin_sfixed_t y = twin_fixed_to_sfixed(twin_sin(i)) << 1 - twin_fixed_to_sfixed(twin_sin(i<<1));
    
        twin_dfixed_t x1 = twin_dfixed_div(twin_dfixed_mul(twin_sfixed_to_dfixed(x),radius),twin_int_to_dfixed(3));
        twin_dfixed_t y1 = twin_dfixed_div(twin_dfixed_mul(twin_sfixed_to_dfixed(y),radius),twin_int_to_dfixed(3));
        twin_sfixed_t x_rot = twin_dfixed_to_sfixed(twin_dfixed_mul(x1,cos) - twin_dfixed_mul(y,sin));
        twin_sfixed_t y_rot = twin_dfixed_to_sfixed(twin_dfixed_mul(x1,sin) + twin_dfixed_mul(y,cos));
        
        _twin_path_sdraw( evolute,  x_rot + center.x, y_rot + center.y);
                        
    }
}