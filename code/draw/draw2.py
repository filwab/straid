# coding: utf-8
import numpy as np
import matplotlib.pyplot as plt
import math

def MODtransmatrix(theta1,theta2,theta3,theta4,theta5,theta6):
    #连杆偏移
    d1 = 398
    d2 = -0.299
    d3 = 0
    d4 = 556.925
    d5 = 0
    d6 = 165

    #连杆长度
    a1 = 0
    a2 = 168.3
    a3 = 650.979
    a4 = 156.240
    a5 = 0
    a6 = 0

    #连杆扭角
    alpha1 = 0
    alpha2 = math.pi/2
    alpha3 = 0
    alpha4 = math.pi/2
    alpha5 = -math.pi/2
    alpha6 = math.pi/2

    MDH = [[theta1, d1, a1, alpha1],
    [theta2+math.pi/2, d2, a2, alpha2],
    [theta3, d3, a3, alpha3],
    [theta4, d4, a4, alpha4],
    [theta5, d5, a5, alpha5],
    [theta6, d6, a6, alpha6]]

    T01 = [[math.cos(MDH[0][0]), -math.sin(MDH[0][0]), 0, MDH[0][2]],
    [math.sin(MDH[0][0])*math.cos(MDH[0][3]), math.cos(MDH[0][0])*math.cos(MDH[0][3]), -math.sin(MDH[0][3]), -math.sin(MDH[0][3])*MDH[0][1]],
    [math.sin(MDH[0][0])*math.sin(MDH[0][3]), math.cos(MDH[0][0])*math.sin(MDH[0][3]), math.cos(MDH[0][3]), math.cos(MDH[0][3])*MDH[0][1]],
    [0, 0, 0, 1]]

    T12 = [[math.cos(MDH[1][0]), -math.sin(MDH[1][0]), 0, MDH[1][2]],
    [math.sin(MDH[1][0])*math.cos(MDH[1][3]), math.cos(MDH[1][0])*math.cos(MDH[1][3]), -math.sin(MDH[1][3]), -math.sin(MDH[1][3])*MDH[1][1]],
    [math.sin(MDH[1][0])*math.sin(MDH[1][3]), math.cos(MDH[1][0])*math.sin(MDH[1][3]), math.cos(MDH[1][3]), math.cos(MDH[1][3])*MDH[1][1]],
    [0, 0, 0, 1]]

    T23 = [[math.cos(MDH[2][0]), -math.sin(MDH[2][0]), 0, MDH[2][2]],
    [math.sin(MDH[2][0])*math.cos(MDH[2][3]), math.cos(MDH[2][0])*math.cos(MDH[2][3]), -math.sin(MDH[2][3]), -math.sin(MDH[2][3])*MDH[2][1]],
    [math.sin(MDH[2][0])*math.sin(MDH[2][3]), math.cos(MDH[2][0])*math.sin(MDH[2][3]), math.cos(MDH[2][3]), math.cos(MDH[2][3])*MDH[2][1]],
    [0, 0, 0, 1]]

    T34 = [[math.cos(MDH[3][0]), -math.sin(MDH[3][0]), 0, MDH[3][2]],
    [math.sin(MDH[3][0])*math.cos(MDH[3][3]), math.cos(MDH[3][0])*math.cos(MDH[3][3]), -math.sin(MDH[3][3]), -math.sin(MDH[3][3])*MDH[3][1]],
    [math.sin(MDH[3][0])*math.sin(MDH[3][3]), math.cos(MDH[3][0])*math.sin(MDH[3][3]), math.cos(MDH[3][3]), math.cos(MDH[3][3])*MDH[3][1]],
    [0, 0, 0, 1]]

    T45 = [[math.cos(MDH[4][0]), -math.sin(MDH[4][0]), 0, MDH[4][2]],
    [math.sin(MDH[4][0])*math.cos(MDH[4][3]), math.cos(MDH[4][0])*math.cos(MDH[4][3]), -math.sin(MDH[4][3]), -math.sin(MDH[4][3])*MDH[4][1]],
    [math.sin(MDH[4][0])*math.sin(MDH[4][3]), math.cos(MDH[4][0])*math.sin(MDH[4][3]), math.cos(MDH[4][3]), math.cos(MDH[4][3])*MDH[4][1]],
    [0, 0, 0, 1]]

    T56 = [[math.cos(MDH[5][0]), -math.sin(MDH[5][0]), 0, MDH[5][2]],
    [math.sin(MDH[5][0])*math.cos(MDH[5][3]), math.cos(MDH[5][0])*math.cos(MDH[5][3]), -math.sin(MDH[5][3]), -math.sin(MDH[5][3])*MDH[5][1]],
    [math.sin(MDH[5][0])*math.sin(MDH[5][3]), math.cos(MDH[5][0])*math.sin(MDH[5][3]), math.cos(MDH[5][3]), math.cos(MDH[5][3])*MDH[5][1]],
    [0, 0, 0, 1]]

    T06 = np.dot(np.dot(np.dot(np.dot(np.dot(T01, T12), T23), T34), T45), T56)

    return T06

# 定义关节角度的最小值和最大值
theta1_min, theta1_max = -165, 165
theta2_min, theta2_max = -95, 70
theta3_min, theta3_max = -85, 95
theta4_min, theta4_max = -180, 180
theta5_min, theta5_max = -115, 115
theta6_min, theta6_max = -360, 360

# 设置随机种子，以确保可重复性
np.random.seed(42)

# 定义采样点数量
n = 30000

# 初始化空间坐标数组
x = np.zeros(n)
y = np.zeros(n)
z = np.zeros(n)

# 循环生成随机点
for i in range(n):
    # 随机生成关节角度
    theta1 = np.deg2rad(theta1_min) + (np.deg2rad(theta1_max) - np.deg2rad(theta1_min)) * np.random.rand()
    theta2 = np.deg2rad(theta2_min) + (np.deg2rad(theta2_max) - np.deg2rad(theta2_min)) * np.random.rand()
    theta3 = np.deg2rad(theta3_min) + (np.deg2rad(theta3_max) - np.deg2rad(theta3_min)) * np.random.rand()
    theta4 = np.deg2rad(theta4_min) + (np.deg2rad(theta4_max) - np.deg2rad(theta4_min)) * np.random.rand()
    theta5 = np.deg2rad(theta5_min) + (np.deg2rad(theta5_max) - np.deg2rad(theta5_min)) * np.random.rand()
    theta6 = np.deg2rad(theta6_min) + (np.deg2rad(theta6_max) - np.deg2rad(theta6_min)) * np.random.rand()
    
    # 计算末端执行器的位姿
    Tws = MODtransmatrix(theta1, theta2, theta3, theta4, theta5, theta6)
    x[i] = Tws[0, 3]
    y[i] = Tws[1, 3]
    z[i] = Tws[2, 3]

# 绘制三维图形
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.scatter(x, y, z, s=0.5, c='b')

# 设置坐标轴标签和标题
ax.set_xlabel('x轴 (mm)', fontsize=12)
ax.set_ylabel('y轴 (mm)', fontsize=12)
ax.set_zlabel('z轴 (mm)', fontsize=12)
ax.set_title('机器人工作空间', fontsize=14)

# 显示网格线
ax.grid(True)

# 显示图形
plt.savefig('机器人工作空间.png', dpi=300)