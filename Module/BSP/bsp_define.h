/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    bsp_define.h
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	凌华运动控制卡 宏定义, 结构体参数文件.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#ifndef BSP_DEFINE_H
#define BSP_DEFINE_H

#include <QString>


/*------------------------------------------------------ 轴部分 ---------*/
/* 运动控制卡 */
#define CARD_APS_204C           0       // ADLINK 204/8C

/* 轴类型 */
#define AXIS_TYPE_SERVO         0       // 伺服电机
#define AXIS_TYPE_STEP          1       // 步进电机

/* 有无编码器 */
#define ENCODER_ENABLE          0       // 有编码器
#define ENCODER_DISABLE         1       // 无编码器

/* 运动模式 */
#define APS_RM                  0       // 相对运动
#define APS_AM                  1       // 绝对运动

/* 运动停止模式 */
#define APS_STOP_MODE_EMG       0       // 立即停止
#define APS_STOP_MODE_DEC       1       // 减速停止

/* 运动停止代码 */
#define APS_STOP_NORMAL			0       // 正常停止
#define APS_STOP_EMG			1       // 紧急信号停止
#define APS_STOP_ALM         	2       // 报警信号停止
#define APS_STOP_SVNO        	3       // 励磁信号停止
#define APS_STOP_PEL         	4       // 正限位信号停止
#define APS_STOP_MEL         	5       // 负限位信号停止
#define APS_STOP_SPEL        	6       // 软正限位信号停止
#define APS_STOP_SMEL        	7       // 软负限位信号停止
#define APS_STOP_USER_EMG    	8
#define APS_STOP_USER        	9
#define APS_STOP_GAN_L1      	10
#define APS_STOP_GAN_L2      	11
#define APS_STOP_GEAR_SLAVE		12
#define APS_STOP_ERROR_LEVEL	13
#define APS_STOP_DI          	14
#define APS_STOP_OTHER_CODE		15


/* 运动参数设置 */
typedef struct
{
    double    vs;		// 起始速度
    double    vm;		// 最大速度
    double    ve;		// 结束速度
    double    acc; 		// 加速度
    double    dec;		// 减速度
    double    sfac;		// s曲线因子
    double    timeout;  // 超时
} APS_MovePara ;

/* 轴配置界面参数 */
typedef struct
{
    // 基本参数
    char name[32];      // axis name
    int card_type;      // 运动控制卡类型
    int axis_type;      // 轴类型
    int card_id;		// card/bus id
    int axis_id;		// axis/node id
    int pulseFactor;	// 脉冲当量
    int extEncode;      // 是否有编码器反馈


    /* AMP204/8C */
    // Step 1: Satety Protection
    int error_check_level_en;	// 位移偏差检测  	0:不使能  1:使能
    int error_check_level_value;// 位移偏差检测值
    int soft_pel_en;			// 软正限位			0:不使能  1:使能
    int soft_pel_value;			// 软正限位值
    int soft_mel_en;			// 软负限位			0:不使能  1:使能
    int soft_mel_value;			// 软负限位值
    int el_stop_mode;			// 触发限位停止模式	0:不使能  1:使能

    // Step 2: Motion IO Check
    int    almLogic;        // 报警IO
    int    ezLogic;         // Z相IO
    int    elLogic;         // 限位IO
    int    inpLogic;        // 到位IO
    int    orgLogic;        // 原点
    int	   emgLogic;		// 紧急停止输入信号     // 板卡参数,在初始化时统一设置.

    // Step 3: Encoder Check
    int encoder_mode;		// 编码器反馈模式
    int encoder_dir;		// 编码器反馈方向

    // Step 4: Output Test
    int output_mode;		// 脉冲输出模式

    // 回零设置
    int    homeMode;        // 回零模式
    int    homeDir;         // 回零方向
    int    homeEZA;         // 回零EZ使能
    double homeAcc;         // 回零加速度
    double homeVm;          // 回零最大速度
    double homeVo;          // 零点搜索速度
    double homeShift;       // 回零偏移
    double homeTimeout;     // 回零超时
} APS_AxisInfo;

/*------------------------------------------------------ IO部分 ---------*/
typedef struct
{
    int card_id;            // 板卡号
    int group_id;           // 组号
    int channel;            // 通道号
    int dir;                // IO方向     0:输入  1:输出
    int isReversal;         // 极性取反   0:不取反  1:取反
    char description[256];  // 备注
}IO_Attribute;




/*------------------------------------------------------ 设备部分 ---------*/
/*-- 设备类型 */
#define D_COM       0       // 串口
#define D_NET       1       // 网口

typedef struct
{
    int type;               // 设备类型
    int baud;               // 波特率
    int port;               // 端口
    char name[256];          // 设备名称
    char com[8];            // 串口名字
    char ip[20];            // ip地址
}Device_Attribute;




#endif // BSP_DEFINE_H
