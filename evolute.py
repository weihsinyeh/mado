import numpy as np
import matplotlib.pyplot as plt

# 参数 t 的范围取决于起点和终点的角度
def deltoid_curve(start_angle, end_angle, rotate_angle=0):
    # 将角度转换为弧度
    start_rad = np.radians(start_angle)
    end_rad = np.radians(end_angle)
    
    # 生成 t 的范围
    t = np.linspace(start_rad, end_rad, 500)
    
    # deltoid 曲线的参数方程
    x = 2 * np.cos(t) + np.cos(2 * t)
    y = 2 * np.sin(t) - np.sin(2 * t)
    
    # 将旋转角度转换为弧度
    rotate_rad = np.radians(rotate_angle)
    
    # 应用旋转矩阵
    x_rot = x * np.cos(rotate_rad) - y * np.sin(rotate_rad)
    y_rot = x * np.sin(rotate_rad) + y * np.cos(rotate_rad)
    
    return x_rot, y_rot

# 定义起始和终点角度
start_angle = 120  # 起始角度
end_angle = 360   # 终点角度
rotate_angle = 0  # 旋转角度

# 获取曲线的部分坐标（带旋转）
x_part, y_part = deltoid_curve(start_angle, end_angle, rotate_angle)

# 绘制完整的 deltoid 曲线作为对比
t_full = np.linspace(0, 2 * np.pi, 1000)
x_full = 2 * np.cos(t_full) + np.cos(2 * t_full)
y_full = 2 * np.sin(t_full) - np.sin(2 * t_full)

# 创建图形
plt.figure(figsize=(6, 6))

# 绘制完整的 deltoid 曲线
plt.plot(x_full, y_full, label='Full Deltoid Curve', color='lightgray')

# 绘制旋转后的部分 deltoid 曲线
plt.plot(x_part, y_part, label=f'Rotated Part Deltoid ({start_angle}° to {end_angle}°, rotate {rotate_angle}°)', color='blue')

# 图形设置
plt.title('Partial Deltoid Curve with Rotation')
plt.gca().set_aspect('equal', adjustable='box')
plt.grid(True)
plt.legend()
plt.xlim(-3, 3)
plt.ylim(-3, 3)

# 显示图像
plt.show()