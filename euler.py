import numpy as np
import matplotlib.pyplot as plt
def fresnel_integral(t, N=10000):
    """
    自行實現Fresnel積分，分別返回C(t)和S(t)
    t: 弧長參數
    N: 積分的離散化區間數
    返回 C(t), S(t)
    """
    # 定義積分範圍
    s = np.linspace(0, t, N)
    ds = t / (N - 1)  # 步長是個標量
    
    # 計算cos和sin函數
    cos_vals = np.cos(np.pi / 2 * s**2)
    sin_vals = np.sin(np.pi / 2 * s**2)
    
    # 使用梯形積分法累積和
    C_integral = np.cumsum(cos_vals) * ds
    S_integral = np.cumsum(sin_vals) * ds
    
    return C_integral[-1], S_integral[-1]

# Euler Spiral的參數
def euler_spiral(t, A=1):
    """
    計算Euler Spiral的座標，使用自實現的Fresnel積分
    t: 是弧長參數
    A: 是曲率變化的常數
    返回x, y座標
    """
    # 計算Fresnel積分
    C, S = fresnel_integral(A * t)
    
    # 計算x和y座標
    x = C * np.sqrt(np.pi / 2)
    y = S * np.sqrt(np.pi / 2)
    
    return x, y
# 給定的一段path上的點
points = np.array([
    [0, 0], [1, 1.5], [2, 3], [3, 5], [4, 3.5], [5, 2], [6, 1]
])

# 計算兩點之間的距離
def distance(p1, p2):
    return np.sqrt((p2[0] - p1[0])**2 + (p2[1] - p1[1])**2)

# 計算兩點之間的角度（弧度）
def angle_between(p1, p2):
    return np.arctan2(p2[1] - p1[1], p2[0] - p1[0])

# 給定初始與終止點來生成 Euler Spiral
def euler_spiral_path(start_point, end_point, num_points=100):
    A = 1  # Euler Spiral的常數
    t = np.linspace(0, 1, num_points)  # 步長，用於生成曲線
    x_vals, y_vals = euler_spiral(t, A)  # 使用Fresnel積分計算Euler Spiral
    # 將生成的Euler Spiral的點轉換到給定的起點和終點之間
    x_vals = np.interp(x_vals, (x_vals.min(), x_vals.max()), (start_point[0], end_point[0]))
    y_vals = np.interp(y_vals, (y_vals.min(), y_vals.max()), (start_point[1], end_point[1]))
    return x_vals, y_vals

# 計算現有路徑上的曲率並插入Euler Spiral
def modify_path_with_spiral(points, start_idx, end_idx):
    start_point = points[start_idx]
    end_point = points[end_idx]
    
    # 生成Euler Spiral的點
    x_vals, y_vals = euler_spiral_path(start_point, end_point)
    
    # 替換原始路徑中的點
    new_path = np.vstack((points[:start_idx], np.column_stack((x_vals, y_vals)), points[end_idx+1:]))
    
    return new_path

# 繪製結果
plt.figure(figsize=(10, 6))
# 原始路徑
plt.plot(points[:, 0], points[:, 1], 'o-', label='Original Path', color='blue')

# 修改路徑，假設我們要用Euler Spiral替換第2個點到第5個點
new_path = modify_path_with_spiral(points, 2, 5)

# 新的路徑
plt.plot(new_path[:, 0], new_path[:, 1], 's--', label='Path with Euler Spiral', color='red')

plt.xlabel('X')
plt.ylabel('Y')
plt.title('Path with Euler Spiral')
plt.legend()
plt.grid(True)
plt.show()
