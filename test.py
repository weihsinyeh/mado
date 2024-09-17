import numpy as np
import matplotlib.pyplot as plt
import math

class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y
plt.figure(figsize=(10, 6))

def read_points_from_file(filename):
    points_list = []
    with open(filename, 'r') as file:
        for line in file:
            x, y = map(float, line.strip().split())
            points_list.append(Point(x, y))
    points = np.array(points_list)
    return points

filename = 'test' 
points = read_points_from_file(filename)

import numpy as np
plt.figure(figsize=(10, 6))
def find_circle_center_and_radius(x1, y1, x2, y2, xm, ym):
    def midpoint(xa, ya, xb, yb):
        return (xa + xb) / 2, (ya + yb) / 2

    def line_parameters(xa, ya, xb, yb):
        mx, my = midpoint(xa, ya, xb, yb)
        if xb - xa == 0:  
            return None, mx  
        slope = -1 * (xb - xa) / (yb - ya)
        intercept = my - slope * mx
        return slope, intercept
    
    def find_center(x1, y1, x2, y2, xm, ym):
        slope1, intercept1 = line_parameters(x1, y1, x2, y2)
        slope2, intercept2 = line_parameters(x1, y1, xm, ym)
        
        if slope1 is None:  
            cx = intercept1
            cy = slope2 * cx + intercept2
        elif slope2 is None:  
            cx = intercept2
            cy = slope1 * cx + intercept1
        else:
            if slope1 == slope2:  
                return None
            cx = (intercept2 - intercept1) / (slope1 - slope2)
            cy = slope1 * cx + intercept1
        return cx, cy

    def distance(xa, ya, xb, yb):
        return np.sqrt((xb - xa) ** 2 + (yb - ya) ** 2)
    
    r_x, r_y = x2 - x1, y2-y1
    unit_dist = np.sqrt((r_x) ** 2 + (r_y) ** 2)
    r_x /= unit_dist
    r_y /= unit_dist
    
    cx, cy = find_center(x1, y1, x2, y2, xm, ym) 
    cx += -r_y * (unit_dist /(2*np.sqrt(3))) 
    cy += r_x * (unit_dist /(2*np.sqrt(3))) 
    radius = distance(cx, cy, x1, y1)

    dist = distance(x2, y2, x1, y1)
    new_radius = dist/ np.sqrt(3)

    theta = np.linspace(0, 2 * np.pi, 100) 
    x = cx + new_radius * np.cos(theta)  # 圓的x座標
    y = cy + new_radius * np.sin(theta)  # 圓的y座標
    plt.plot(x, y)
    plt.gca().set_aspect('equal', adjustable='box')  # 確保x和y軸的比例相等
    return new_radius, cx, cy

def calculate_normal_angle(x1, y1, x2, y2, xm, ym):
    radius, xc, yc = find_circle_center_and_radius(x1, y1, x2, y2, xm, ym)


    r_x, r_y = x2 - x1, y2-y1
    angle = np.arctan(r_y/r_x)

    print("angle",angle)
    angle_deg = angle * (180 / math.pi)
    
    print("angle_deg",angle_deg)
    return angle_deg, xc, yc, radius

def deltoid_curve(radius, start_angle, end_angle, rotate_angle=0, translate_x=0, translate_y=0):

    start_rad = np.radians(start_angle)
    end_rad = np.radians(end_angle)
    t = np.linspace(start_rad, end_rad, 500)
    x = 2 * np.cos(t) + np.cos(2 * t)
    y = 2 * np.sin(t) - np.sin(2 * t)
 
    x *= radius/3
    y *= radius/3
    rotate_rad = np.radians(rotate_angle)
    x_rot = x * np.cos(rotate_rad) - y * np.sin(rotate_rad)
    y_rot = x * np.sin(rotate_rad) + y * np.cos(rotate_rad)
    
    x_trans = x_rot + translate_x
    y_trans = y_rot + translate_y
    
    return x_trans, y_trans

total_points = len(points)
angle, xc, yc, radius = calculate_normal_angle(
    points[0].x, points[0].y, 
    points[total_points-1].x, points[total_points-1].y, 
    points[total_points//2].x, points[total_points//2].y
)
# 1/np.sqrt(3)
# (arc_angle - 120) / 2 = theta
start_angle =  0 
end_angle = 240 
rotate_angle = angle -30 
translate_x = xc  
translate_y = yc  

x_part, y_part = deltoid_curve(radius ,start_angle, end_angle, rotate_angle, translate_x, translate_y)

plt.plot(x_part, y_part, label=f'deltoid_curve', color='blue')
ori_list ,evolute_list = [],[]
for point in points:
    ori_list.append([point.x, point.y])
ori = np.array(ori_list)

plt.scatter(ori[:,0], ori[:,1], label='Path', color='blue')
plt.legend()
plt.title('Evolute')
plt.xlabel('x')
plt.ylabel('y')
plt.grid(True)
plt.axis('equal')
plt.show()
