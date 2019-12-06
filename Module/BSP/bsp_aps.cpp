/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    bsp_aps.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	凌华运动控制卡 AMP_204C/208C 的封装库.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#include "bsp_aps.h"
#include <QTime>
#include <stdexcept>
#include <QDebug>
#include <QThread>
#include <QCoreApplication>

Bsp_Aps::Bsp_Aps()
{
    pMoveParam = new APS_MovePara;
    pAxisInfo  = new APS_AxisInfo;
}

Bsp_Aps::~Bsp_Aps()
{
    delete pMoveParam;
    delete pAxisInfo;
}

/* 设置轴运动参数 */
void Bsp_Aps::set_axis_move_param(APS_MovePara *param)
{
    memcpy(pMoveParam, param, sizeof(APS_MovePara));
}

/* 获取轴运动参数 */
APS_MovePara *Bsp_Aps::get_axis_move_param(void)
{
    return pMoveParam;
}

/* 设置轴配置参数 */
void Bsp_Aps::set_axis_config_param(APS_AxisInfo *param)
{
    memcpy(pAxisInfo, param, sizeof(APS_AxisInfo));
}

/* 获取获取轴配置参数 */
APS_AxisInfo *Bsp_Aps::get_axis_config_param(void)
{
    return pAxisInfo;
}



/*
 *==============================================================================
 *                                  运动控制API
 *==============================================================================
 */

/* 设置板卡的轴参数 */
void Bsp_Aps::set_axis_param_for_card()
{
    // step 1
    if(pAxisInfo->error_check_level_en)
        APS_set_axis_param(pAxisInfo->axis_id, PRA_ERR_C_LEVEL, pAxisInfo->error_check_level_value);     // Error counter check level
    else
        APS_set_axis_param(pAxisInfo->axis_id, PRA_ERR_C_LEVEL, 0);

    APS_SetSoftLimit();

    APS_set_axis_param(pAxisInfo->axis_id, PRA_EL_MODE, pAxisInfo->el_stop_mode);

    // step 2
    APS_set_axis_param(pAxisInfo->axis_id, PRA_ALM_LOGIC,    pAxisInfo->almLogic);     // high active
    APS_set_axis_param(pAxisInfo->axis_id, PRA_EL_LOGIC,     pAxisInfo->elLogic);      // not inverse
    APS_set_axis_param(pAxisInfo->axis_id, PRA_ORG_LOGIC,    pAxisInfo->orgLogic);     // inverse
    APS_set_axis_param(pAxisInfo->axis_id, PRA_EZ_LOGIC,     pAxisInfo->ezLogic);      // low active
    APS_set_axis_param(pAxisInfo->axis_id, PRA_INP_LOGIC,    pAxisInfo->inpLogic);     // high active
    // step 3
    APS_set_axis_param(pAxisInfo->axis_id, PRA_PLS_IPT_MODE, pAxisInfo->encoder_mode); // 4xAB
    APS_set_axis_param(pAxisInfo->axis_id, PRA_ENCODER_DIR,  pAxisInfo->encoder_dir);  // encoder dir: negative
    // step 4
    APS_set_axis_param(pAxisInfo->axis_id, PRA_PLS_OPT_MODE, pAxisInfo->output_mode);  // pulse/dir
    // other
    APS_set_axis_param(pAxisInfo->axis_id, PRA_SERVO_LOGIC,  0);                       // low active        pAxisInfo->servoLogic

    // 回零设置
    APS_set_axis_param  (pAxisInfo->axis_id, PRA_HOME_MODE,  pAxisInfo->homeMode);     // home mode  0:ORG  1:EL   2:EZ
    APS_set_axis_param  (pAxisInfo->axis_id, PRA_HOME_DIR,   pAxisInfo->homeDir);      // Set home direction   0:p-dir   1:n-dir
    APS_set_axis_param  (pAxisInfo->axis_id, PRA_HOME_EZA,   pAxisInfo->homeEZA);      // EZ alignment enable
    APS_set_axis_param_f(pAxisInfo->axis_id, PRA_HOME_CURVE, 0);                       // homing curve parten(T or s curve)
    APS_set_axis_param_f(pAxisInfo->axis_id, PRA_HOME_ACC,   pAxisInfo->homeAcc);      // Acceleration deceleration rate
    APS_set_axis_param_f(pAxisInfo->axis_id, PRA_HOME_VS,    0);                       // homing start velocity
    APS_set_axis_param_f(pAxisInfo->axis_id, PRA_HOME_VM,    pAxisInfo->homeVm);       // homing max velocity
    APS_set_axis_param_f(pAxisInfo->axis_id, PRA_HOME_VO,    pAxisInfo->homeVo);       // Homing leave ORG velocity
    APS_set_axis_param_f(pAxisInfo->axis_id, PRA_HOME_SHIFT, pAxisInfo->homeShift);    // The shift from ORG
}

/*配置板卡参数 */
void Bsp_Aps::APS_config_board()
{


}
/*
 * @ brief  获得当前轴的逻辑位置
 * @ param  .
 * @ retval pos
 */
double Bsp_Aps::APS_GetCmPos(void)
{
    double pos = 0.0;
    APS_get_command_f(pAxisInfo->axis_id, &pos);

    pos = pos / pAxisInfo->pulseFactor;
    return pos;
}

/*
 * @ brief  设置当前轴的逻辑位置
 * @ param  cmpos   指令位置值
 * @ retval None
 */
void Bsp_Aps::APS_SetCmPos (double cmpos)
{
    APS_set_command_f (pAxisInfo->axis_id, cmpos * pAxisInfo->pulseFactor);
}

/*
 * @ brief  获得当前轴的实际位置
 * @ param  .
 * @ retval pos
 */
double Bsp_Aps::APS_GetFbPos (void)
{
    double pos = 0.0;
    if(pAxisInfo->extEncode == ENCODER_ENABLE)    // 有编码器返回值
    {
        APS_get_position_f (pAxisInfo->axis_id, &pos);
        pos = pos / pAxisInfo->pulseFactor;
    }
    else    // 无编码器返回值
    {
        pos = APS_GetCmPos();
    }
    return pos;
}

/*
 * @ brief  设置当前轴的实际位置
 * @ param  fbpos   编码器反馈位置.
 * @ retval .
 */
void Bsp_Aps::APS_SetFbPos (double fbpos)
{
    APS_set_position_f (pAxisInfo->axis_id, fbpos * pAxisInfo->pulseFactor);
}

/*
 * @ brief  获得当前轴的IO状态
 * @ param  Name        Type        Description
 *          bit         int         IO bit, zero-based
 *                                  MIO_ALM(0)：报警
 *                                  MIO_PEL(1)：正限
 *                                  MIO_MEL(2)：负限
 *                                  MIO_ORG(3)：原点
 *                                  MIO_INP(6)：到位
 *                                  MIO_SVON(7)：励磁
 *                                  MIO_SPEL(11)：软正限
 *                                  MIO_SMEL(12)：软负限
 * @ retval Name        Type        Description
 *          io          int         Return motion IO bit status
 *                                  0 ： low
 *                                  1 ： high
 */
int Bsp_Aps::APS_GetIOBit(int bit)
{
    int io = 0;
    io = APS_motion_io_status (pAxisInfo->axis_id);
    io = (io >> bit) & 0x01;
    return io;
}

/* 获取运动轴INP */
int Bsp_Aps::APS_GetINP (void)
{
    return APS_GetIOBit(MIO_INP);
}

/* 获取运动轴正限位IO */
int Bsp_Aps::APS_GetPEL (void)
{
    return APS_GetIOBit(MIO_PEL);
}

/* 获取运动轴负限位IO */
int Bsp_Aps::APS_GetMEL (void)
{
    return APS_GetIOBit(MIO_MEL);
}

/* 获取运动轴原点IO */
int Bsp_Aps::APS_GetORG (void)
{
    return APS_GetIOBit(MIO_ORG);
}

/* 获取运动轴报警IO */
int Bsp_Aps::APS_GetALM (void)
{
    return APS_GetIOBit(MIO_ALM);
}

/* 获取运动轴励磁IO */
int Bsp_Aps::APS_GetSVON (void)
{
    return APS_GetIOBit(MIO_SVON);
}

/* 获取运动轴Z相IO */
int Bsp_Aps::APS_GetEZ (void)
{
    return APS_GetIOBit(MIO_EZ);
}

/* 获取EMG IO */
int Bsp_Aps::APS_GetEMG (void)
{
    return APS_GetIOBit(MIO_EMG);
}


/*
 * @ brief  设置当前轴的励磁状态
 * @ param  Name        Type        Description
 *          on_off      int         励磁状态，0-不励磁，1-励磁
 * @ retval .
 */
void Bsp_Aps::APS_SetSVON (int on_off)
{
    APS_set_servo_on (pAxisInfo->axis_id, on_off);
}

/*
 * @ brief  使能并设置轴软限位
 * @ param  Name        Type        Description
 *          pel_en      int         正限位是否使能
 *          spel        double      软正限位位置
 *          pel_en      int         负限位是否使能
 *          smel        double      软负限位位置
 * @ retval .
 */
void Bsp_Aps::APS_SetSoftLimitEx (int pel_en, double spel, int mel_en, double smel)
{
    if(pel_en)  // 软限位使能
    {
        APS_set_axis_param_f (pAxisInfo->axis_id, PRA_SPEL_POS, spel);
        APS_set_axis_param (pAxisInfo->axis_id, PRA_SPEL_EN, 2);
    }
    else
    {
        APS_set_axis_param (pAxisInfo->axis_id, PRA_SPEL_EN, 0);
    }

    if(mel_en)  // 软限位使能
    {
        APS_set_axis_param_f (pAxisInfo->axis_id, PRA_SMEL_POS, smel);
        APS_set_axis_param (pAxisInfo->axis_id, PRA_SMEL_EN, 2);
    }
    else
    {
        APS_set_axis_param (pAxisInfo->axis_id, PRA_SMEL_EN, 0);
    }
}

/*
 * @ brief  设置轴软限位
 * @ param  .
 * @ retval .
 */
void Bsp_Aps::APS_SetSoftLimit(void)
{
    APS_SetSoftLimitEx(pAxisInfo->soft_pel_en, pAxisInfo->soft_pel_value, pAxisInfo->soft_mel_en, pAxisInfo->soft_mel_value);
}

/*
 * @ brief  通用轴点到点运动函数, 不检测到位
 * @ param  Name        Type        Description
 *          pos         double      如果是相对运动，该值为distance，如果是绝对运动，该值为position
 *          ratio       double      运动速率，当该值为负数时使用内部速率
 *          mode        int         Bit-mapped parameter that can include one or more of the following
 *									flags:
 *                                      APS_AM:     绝对运动
 *                                      APS_RM:     相对运动
 * @ retval .
 */
void Bsp_Aps::APS_MoveEx(double pos, double ratio, int mode, const APS_MovePara *mp)
{
    APS_MovePara *tmp = (APS_MovePara *)mp;
    if (mp == nullptr)
    {
        tmp = pMoveParam;
    }
    ratio = (ratio > 1.0) ? 1.0 : ( (ratio < 0) ? 0.0 : ratio);

    pos = pos * pAxisInfo->pulseFactor;

    APS_set_axis_param_f (pAxisInfo->axis_id, PRA_SF,  tmp->sfac); //Set S-factor
    APS_set_axis_param_f (pAxisInfo->axis_id, PRA_VS,  tmp->vs * ratio);
    APS_set_axis_param_f (pAxisInfo->axis_id, PRA_ACC, tmp->acc * ratio * ratio); //Set acceleration
    APS_set_axis_param_f (pAxisInfo->axis_id, PRA_DEC, tmp->dec * ratio * ratio); //Set deceleration

    if (mode == APS_AM)
    {
        APS_absolute_move (pAxisInfo->axis_id, pos, tmp->vm * ratio);
    }
    else
    {
        APS_relative_move (pAxisInfo->axis_id, pos, tmp->vm * ratio);
    }
}

// 绝对运动
void Bsp_Aps::APS_AMove (double pos, double ratio)
{
    APS_MoveEx(pos, ratio, APS_AM, nullptr);
}

// 相对运动
void Bsp_Aps::APS_RMove (double pos, double ratio)
{
    APS_MoveEx(pos, ratio, APS_RM, nullptr);
}

/*
 * @ brief  轴停止函数
 * @ param  Name        Type        Description
 *          mode        int         停止模式: APS_STOP_EMG:立即停，APS_STOP_DEC:减速停
 * @ retval .
 */
void Bsp_Aps::APS_Stop (int mode)
{
    if (mode == APS_STOP_MODE_EMG)
    {
        APS_emg_stop (pAxisInfo->axis_id);
    }
    else
    {
        APS_stop_move (pAxisInfo->axis_id);
    }
}

void Bsp_Aps::APS_dStop (void)
{
    APS_Stop(APS_STOP_MODE_DEC);
}

void Bsp_Aps::APS_eStop (void)
{
    APS_Stop(APS_STOP_MODE_EMG);
}


/*
 * @ brief  获取轴运动停止代码信息
 * @ param  .
 * @ retval 返回代码说明信息.
 */
QString Bsp_Aps::APS_GetStopCode(void)
{
    int stopcode = 0;
    I32  status;
    QString errInfo;

    APS_get_stop_code (pAxisInfo->axis_id, &status);
    switch (status)
    {
        case 0:  stopcode = APS_STOP_NORMAL; errInfo = "正常停止"; break;
        case 1:  stopcode = APS_STOP_EMG; errInfo = "紧急信号停止"; break;
        case 2:  stopcode = APS_STOP_ALM; errInfo = "报警信号停止"; break;
        case 4:  stopcode = APS_STOP_PEL; errInfo = "正限位信号停止"; break;
        case 5:  stopcode = APS_STOP_MEL; errInfo = "负限位信号停止"; break;
        case 6:  stopcode = APS_STOP_SPEL; errInfo = "软正限位信号停止"; break;
        case 7:  stopcode = APS_STOP_SMEL; errInfo = "软负限位信号停止"; break;
        case 13: stopcode = APS_STOP_ERROR_LEVEL; errInfo = "错误位置检查级别信号停止"; break;
        default: stopcode = APS_STOP_OTHER_CODE; errInfo = QString("其他信号停止,code=%1").arg(status); break;
    }
    return errInfo;
}


/*
 * @ brief  查询运动是否停止(非阻塞)
 * @ param  .
 * @ retval 0:未停止   1:停止    throw 异常停止信息
 */
int Bsp_Aps::APS_isMoveDone (void)
{
    int  amp_status = 0;
    int  amp_inp = 1;

    amp_status = APS_motion_status (pAxisInfo->axis_id);
    if ((amp_status >> MTS_ASTP) & 0x01)
    {
        APS_eStop();
        QString stopInfo = APS_GetStopCode();
//        qWarning(stopInfo.toStdString().data());
        throw std::runtime_error(stopInfo.toStdString().data());
    }
    amp_status = (amp_status >> MTS_NSTP) & 0x01;

    // 注:伺服有INP信号,步进是没有INP信号的.
    amp_inp = APS_motion_io_status (pAxisInfo->axis_id);
    amp_inp = (amp_inp >> MIO_INP) & 0x01;

    if (amp_status && amp_inp)
    {
        return 1;
    }
    return 0;
}


/*
 * @ brief  等待运动停止(阻塞)
 * @ param  .
 * @ retval throw 异常停止信息
 */
void Bsp_Aps::APS_WaitMoveDone(void)
{
    QTime startTime = QTime::currentTime();
    QTime stopTime = QTime::currentTime();
    int elapsed = 0;
    int  stopcode = 0;

    // 若轴未励磁,则报警
    int svob_status = APS_GetSVON();
    if(!svob_status)
    {
        QString name(pAxisInfo->name);
        QString errInfo = QString("轴:%1 未励磁!运动失败!").arg(name);
        throw std::runtime_error(errInfo.toStdString().data());
    }

    do
    {
        QThread::msleep(10);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        try
        {
            stopcode = APS_isMoveDone();
            if (stopcode == 1)
            {
                break;
            }
        }
        catch(const std::runtime_error& e)
        {
            throw std::runtime_error(e.what());
        }

        stopTime = QTime::currentTime();
        elapsed = startTime.secsTo(stopTime);
        if (elapsed > pMoveParam->timeout)
        {
            APS_eStop();
            QString name(pAxisInfo->name);
            QString errInfo = QString("轴:%1 运动超时.").arg(name);
            throw std::runtime_error(errInfo.toStdString().data());
        }
    } while (1);
}

/*
 * @ brief  回零,不检测回零完成
 * @ param  .
 * @ retval .
 */
void Bsp_Aps::APS_Home (void)
{
    // 关闭软限位使能
    APS_SetSoftLimitEx(0, 0, 0, 0);

    APS_set_axis_param  (pAxisInfo->axis_id, PRA_HOME_MODE, pAxisInfo->homeMode);   // home mode  0:ORG  1:EL   2:EZ
    APS_set_axis_param  (pAxisInfo->axis_id, PRA_HOME_DIR,  pAxisInfo->homeDir);    // Set home direction   0:p-dir   1:n-dir
    APS_set_axis_param  (pAxisInfo->axis_id, PRA_HOME_EZA,  pAxisInfo->homeEZA);    // EZ alignment enable
    APS_set_axis_param_f(pAxisInfo->axis_id, PRA_HOME_CURVE,0.5);                   // homing curve parten(T or s curve)
    APS_set_axis_param_f(pAxisInfo->axis_id, PRA_HOME_ACC,  pAxisInfo->homeAcc);    // Acceleration deceleration rate
    APS_set_axis_param_f(pAxisInfo->axis_id, PRA_HOME_VS,   0);                     // homing start velocity
    APS_set_axis_param_f(pAxisInfo->axis_id, PRA_HOME_VM,   pAxisInfo->homeVm);     // homing max velocity
    APS_set_axis_param_f(pAxisInfo->axis_id, PRA_HOME_VO,   pAxisInfo->homeVo);     // Homing leave ORG velocity
    APS_set_axis_param_f(pAxisInfo->axis_id, PRA_HOME_SHIFT,pAxisInfo->homeShift);  // The shift from ORG
    APS_home_move(pAxisInfo->axis_id);
}

/*
 * @ brief  等待回零完成
 * @ param  .
 * @ retval throw 异常停止信息
 */
void Bsp_Aps::APS_WaitHomeDone (void)
{
    QTime startTime = QTime::currentTime();
    QTime stopTime = QTime::currentTime();
    int elapsed = 0;

    int  amp_status = 0;

    // 若轴未励磁,则报警
    int svob_status = APS_GetSVON();
    if(!svob_status)
    {
        QString name(pAxisInfo->name);
        QString errInfo = QString("轴:%1 未励磁!回零失败!").arg(name);
        throw std::runtime_error(errInfo.toStdString().data());
    }

    while (1)
    {
        QThread::msleep(10);
        QCoreApplication::processEvents(QEventLoop::AllEvents);

        amp_status = APS_motion_status (pAxisInfo->axis_id);
        if ((amp_status >> MTS_ASTP) & 1)
        {
            APS_eStop();
            QString name(pAxisInfo->name);
            QString stopInfo = QString("轴:%1 回零异常\n异常信息:%2").arg(name).arg(APS_GetStopCode());
//            qWarning(stopInfo.toStdString().data());
            throw std::runtime_error(stopInfo.toStdString().data());
        }
        if (!((amp_status >> MTS_HMV) & 1))
        {
            // 回零完成后,重新设置软限位
            APS_SetSoftLimit();
            break;
        }

        stopTime = QTime::currentTime();
        elapsed = startTime.secsTo(stopTime);
        if (elapsed > pMoveParam->timeout)
        {
            APS_eStop();
            QString name(pAxisInfo->name);
            QString errInfo = QString("轴:%1 回零超时.").arg(name);
//            qWarning(errInfo.toStdString().data());
            throw std::runtime_error(errInfo.toStdString().data());
        }
    }
}
