#ifndef BSP_APS_H
#define BSP_APS_H

#include <QString>
#include "AMP_208C/APS168.h"
#include "AMP_208C/APS_Define.h"
#include "AMP_208C/ErrorCodeDef.h"
#include "AMP_208C/type_def.h"
#include "bsp_define.h"


class Bsp_Aps
{
public:
    Bsp_Aps();
	~Bsp_Aps();


    /* 设置/获取轴运动参数 */
    void set_axis_move_param(APS_MovePara *param);
    APS_MovePara *get_axis_move_param(void);

    /* 设置/获取轴配置参数 */
    void set_axis_config_param(APS_AxisInfo *param);
    APS_AxisInfo *get_axis_config_param(void);

    /* 设置板卡的轴参数 */
    void set_axis_param_for_card();
    /*配置板卡参数 */
    void APS_config_board();

    /*
     *  获取/设置运动位置/命令位置函数
     */
    double APS_GetCmPos (void);
    void   APS_SetCmPos (double cmpos);
    double APS_GetFbPos (void);
    void   APS_SetFbPos (double fbpos);

    /*
     *  获取运动板轴IO逻辑，使能函数
     */
    int APS_GetIOBit(int bit);
    /* 获取运动状态 */
//    QString APS_GetState(void);
    /* 获取运动轴INP */
    int APS_GetINP (void);
    /* 获取运动轴正限位IO */
    int APS_GetPEL (void);
    /* 获取运动轴负限位IO */
    int APS_GetMEL (void);
    /* 获取运动轴原点IO */
    int APS_GetORG (void);
    /* 获取运动轴报警IO */
    int APS_GetALM (void);
    /* 获取运动轴励磁IO */
    int APS_GetSVON (void);
    /* 设置运动轴励磁 */
    void APS_SetSVON (int on_off);
    /* 获取运动轴Z相IO */
    int APS_GetEZ (void);
    /* 获取EMG IO */
    int APS_GetEMG (void);

	/*
	 *  设置软限位函数
	 */
    void APS_SetSoftLimit   (void);
    void APS_SetSoftLimitEx (int pel_en, double spel, int mel_en, double smel);


    /*
     *  单轴运动函数
     */
    void APS_AMove (double pos, double ratio);
    void APS_RMove (double pos, double ratio);
    void APS_MoveEx(double pos, double ratio, int mode, const APS_MovePara *mp);


    /*
     *  停止运动函数
     */
    void APS_Stop  (int mode);
    void APS_dStop (void);
    void APS_eStop (void);

    /*
     *  获取停止代码信息
     */
    QString APS_GetStopCode(void);

    /*
     *  检测运动完成函数（非阻塞函数）
     */
    int APS_isMoveDone (void);

    /*
     *  检测运动完成函数（阻塞函数）
     */
    void APS_WaitMoveDone    (void);


    /*
     *  轴回零函数
     */
    void APS_Home (void);
    void APS_WaitHomeDone (void);


private:
    APS_MovePara *pMoveParam;
    APS_AxisInfo *pAxisInfo;
};

#endif // BSP_APS_H
