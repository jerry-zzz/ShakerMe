/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    system_info.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	定义了系统的一些参数:设备名称,参数路径,界面配置,通用按钮大小配置,权限定义.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H


/*-- 软件功能 -----------------------------------------------------------------*/
// 调试模式,脱机运行
//#define DEBUG_MODE

// 日志记录
//#define LOG_EN      // 开启日志记录到本地文件功能


/*-- 软件版本 -----------------------------------------------------------------*/
#define VERSION         "V0.0.5"

/*-- 设备名称 -----------------------------------------------------------------*/
#define MACHINE_NAME    "激振器设备软件"
#define PROCESS_NAME    L"ShakerMe.exe"        // 注意不能删除L,只需更改".."内内容即可!

/*-- 参数数据路径 --------------------------------------------------------------*/
#define ROOT_PATH                   "D:/IET/ShakerMe"
#define SYSTEM_PARAM_DB_PATH        "D:/IET/ShakerMe/sys_config.db"
#define LOG_PATH                    "D:/IET/ShakerMe/log"
#define TEST_RECORD_FILE            "D:/IET/ShakerMe/DetectionRecord.db"

#define ROOT_PATH_ONE               "D:/IET/ShakerMe/product/"
#define ROOT_PATH_TWO               "D:/IET/ShakerMe/program/"

/*-- 嵌套界面配置 ----------------------------------- --------------------------*/
#define NESTED_PANEL_POS_X      100     // 嵌套界面的坐标x
#define NESTED_PANEL_POS_Y      60      // 嵌套界面的坐标y
#define NESTED_PANEL_POS_W      831    // 嵌套界面的宽度w
#define NESTED_PANEL_POS_H      671     // 嵌套界面的高度h

/*-- 通用按钮大小配置 -----------------------------------------------------------*/
#define PUSHBUTTON_SIZE         81      // 按钮size

/*-- 权限定义 -----------------------------------------------------------------*/
#define OP_LEVEL                0       // 操作员权限等级
#define ADMIN_LEVEL             1       // 管理员权限等级

#define OP_NAME                 "操作员"
#define ADMIN_NAME              "管理员"

/*-- 信号灯状态定义 -------------------------------------------------------------*/
#define LAMP_RUN                0X01    // 运行-绿灯-常亮
#define LAMP_IDLE               0x02    // 待机-黄灯-常亮
#define LAMP_ALM                0x04    // 报警-红灯-常亮

#endif // SYSTEM_INFO_H
