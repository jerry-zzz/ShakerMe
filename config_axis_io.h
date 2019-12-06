/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    config_axis_io.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	定义了轴,io,串口,网口等信息,直接在此文件进行配置.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#ifndef CONFIG_AXIS_IO_H
#define CONFIG_AXIS_IO_H

/* 陈磊的设备
 *
 *
 */


//-- 轴配置 -----------------------------------------------

/* 最大轴数 */
#define AXIS_MAX_NUM        3

/* 轴信息 */
#define AXIS_Z_SHAKER       0       // 激振器Z轴
#define AXIS_Y_PLATFORM     1       // 载台Y轴
#define AXIS_Z_PLATFORM     2       // 载台Z轴



//-- IO配置 -----------------------------------------------

/* 最大IO数 */
#define IO_MAX_NUM          32

/* IO信息 */
// 输入1-8


#define I_CYLINDER_L_UP       0       // 左气缸上状态传感器
#define I_CYLINDER_L_DOWN     1       // 左气缸下状态传感器
#define I_CYLINDER_R_UP       2       // 右气缸上状态传感器
#define I_CYLINDER_R_DOWN     3       // 右气缸下状态传感器
#define I_LIMIT_HEIGHT        4       // 限高传感器
//#define RESERVE               5       //
//#define RESERVE               6       //
//#define RESERVE               7       //
// 输入9-16
#define I_DIFFER              8       // 产品区分检测传感器(对射)
//#define RESERVE              9       // 产品区分检测传感器(对射)
//#define RESERVE               10      //
//#define RESERVE               11      //
//#define RESERVE               12      //
//#define RESERVE               13      //
//#define RESERVE               14      //
//#define RESERVE               15      //
// 输出1-8
#define O_RLAMP				  16      // 三色灯--红灯
#define O_YLAMP				  17      // 三色灯--黄灯
#define O_GLAMP				  18      // 三色灯--绿灯
#define O_BEEP				  19      // 三色灯--蜂鸣器
//#define RESERVE             20      //
//#define RESERVE             21      //
//#define RESERVE             22      //
//#define RESERVE             23      //
// 输出9-16
#define O_ELECTRIC_VALVE      24      // 气缸电磁阀开关
//#define RESERVE             25      //
//#define RESERVE             26      //
//#define RESERVE             27      //
//#define RESERVE             28      //
//#define RESERVE             29      //
//#define RESERVE             30      //
//#define RESERVE             31      //

//-- 其他设备配置 -----------------------------------------------

/* 最大设备数 */
#define DEVICE_MAX_NUM      4

/* 设备信息 */
#define COM_MODULE          0       // 模板串口

#endif // CONFIG_AXIS_IO_H
