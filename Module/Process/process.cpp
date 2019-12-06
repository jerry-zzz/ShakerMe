/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    process.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	设备流程线程设计文件.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#include "process.h"
#include <QDebug>
#include <QMessageBox>
#include "Module/BSP/bsp.h"
#include "../EquipmentDebug/equipmentdebug.h"
#include "../SystemSetup/systemsetup.h"
#include "system_info.h"
#include "config_axis_io.h"
#include "../Log/loginfo.h"

//extern Bsp *m_bsp;

Process::Process()
{
    isrunning = false;
    run_next = 0;

    //-- add your code ---------------------------------------------------------
}

Process::~Process()
{

}

// 初始化流程模块
void Process::init_Process(Bsp *bsp,Shaker *skr)
{
    m_bsp = bsp;
    m_skr = skr;
}

// 日志记录
void Process::log_info(QString msg)
{
#ifdef LOG_EN
    qInfo() << msg;
#endif
}

void Process::slot_mode_num(int mode_num)
{
    choose_mode_num=mode_num;
}


/*
 *==============================================================================
 *   模块固有功能函数
 *==============================================================================
 */

// 查询pcb的id和软件版本
void Process::slot_output_pcb_info(QString id, QString version)
{
    m_info_id = id;
    m_info_version = version;
    is_info_received = true;
    log_info(QString("读取到硬件信息:id-%1;ver-%2").arg(id).arg(version));
}


//--  public 函数  --------------------------------------------------------------
// 线程是否运行
bool Process::is_running()
{
    return isrunning;
}

// 双按钮按下一次
void Process::set_run_next()
{
    run_next = 1;
}
// 退料(将检测工位的产品退出来)
void Process::return_of_product()
{
    if(run_next == 0)
    {
        emit sig_update_signal_lamp(LAMP_RUN);

        // 激振器z轴到安全位置
        shaker_z_move_to_safe_state(m_ep.pos_safe);;
        try
        {
            wait_shaker_z_in_place();
        }
        catch(const std::runtime_error& e)
        {
            emit sig_update_signal_lamp(LAMP_ALM);
            QString errInfo = QString("退料失败: %1").arg(e.what());
            emit msg_log(MSG_ALM, errInfo, true);
            return;
        }

        // 载台z轴回零

        m_bsp->axis[AXIS_Z_PLATFORM].APS_Home();

        try
        {
            m_bsp->axis[AXIS_Z_PLATFORM].APS_WaitHomeDone();
        }
        catch(const std::runtime_error& e)
        {
            emit sig_update_signal_lamp(LAMP_ALM);
            m_bsp->axis[AXIS_Z_PLATFORM].APS_SetSVON(0);
            emit msg_log(MSG_ALM, QString("系统初始化:\n%1\n请手动励磁回零!").arg(e.what()), true);
        }

        // 载台y轴回零
        m_bsp->axis[AXIS_Y_PLATFORM].APS_Home();
        try
        {
            m_bsp->axis[AXIS_Y_PLATFORM].APS_WaitHomeDone();
        }
        catch(const std::runtime_error& e)
        {
            emit sig_update_signal_lamp(LAMP_ALM);
            m_bsp->axis[AXIS_Y_PLATFORM].APS_SetSVON(0);
            emit msg_log(MSG_ALM, QString("系统初始化:\n%1\n请手动励磁回零!").arg(e.what()), true);
        }

        emit sig_update_signal_lamp(LAMP_IDLE);
    }
}

void Process::run()
{
    /* 定义变量参数 */    

    //检测结果
    int result=0;

    // 初始化设备信号灯
    emit sig_update_signal_lamp(LAMP_IDLE);
/*
    is_test = 1;
    emit sig_read_pcb_info(is_test);*/

    // 关闭开始按钮的灯
//    m_bsp->io[O_START_LAMP_L].SetBit(0);
//    m_bsp->io[O_START_LAMP_R].SetBit(0);

//    // 测试代码 点亮设备信号灯
//    m_bsp->io[O_RLAMP].SetBit(1);
//    m_bsp->io[O_GLAMP].SetBit(1);
//    m_bsp->io[O_YLAMP].SetBit(1);

//    m_bsp->axis[AXIS_Z_SHAKER].APS_Home();

    // 初始化检测工位信息
    emit sig_reflash_platform_status("等待");
    emit sig_reflash_axis_platform_z_status("停止", 0);
    emit sig_reflash_axis_platform_y_status("停止", 0);
    emit sig_reflash_axis_shaker_z_status("停止", 0);


#ifndef DEBUG_MODE
    // 设备开机初始化
    if(!m_ss.unable_init)
    {
        /* add your code */
        // 轴回零初始化
        m_bsp->axis[AXIS_Z_SHAKER].APS_Home();

        try
        {
            m_bsp->axis[AXIS_Z_SHAKER].APS_WaitHomeDone();
        }
        catch(const std::runtime_error& e)
        {
            emit sig_update_signal_lamp(LAMP_ALM);
            m_bsp->axis[AXIS_Z_SHAKER].APS_SetSVON(0);
            emit msg_log(MSG_ALM, QString("系统初始化:\n%1\n请手动励磁回零!").arg(e.what()), true);
        }
        log_info("激振器Z轴初始化完成.");

        m_bsp->axis[AXIS_Z_PLATFORM].APS_Home();
        try
        {
            m_bsp->axis[AXIS_Z_PLATFORM].APS_WaitHomeDone();
        }
        catch(const std::runtime_error& e)
        {
            emit sig_update_signal_lamp(LAMP_ALM);
            m_bsp->axis[AXIS_Z_PLATFORM].APS_SetSVON(0);
            emit msg_log(MSG_ALM, QString("系统初始化:\n%1\n请手动励磁回零!").arg(e.what()), true);
        }
        log_info("载台Z轴初始化完成.");

        m_bsp->axis[AXIS_Y_PLATFORM].APS_Home();
        try
        {
            m_bsp->axis[AXIS_Y_PLATFORM].APS_WaitHomeDone();
        }
        catch(const std::runtime_error& e)
        {
            emit sig_update_signal_lamp(LAMP_ALM);
            m_bsp->axis[AXIS_Y_PLATFORM].APS_SetSVON(0);
            emit msg_log(MSG_ALM, QString("系统初始化:\n%1\n请手动励磁回零!").arg(e.what()), true);
        }
        log_info("载台Y轴初始化完成.");
    }

    // 初始化IO
    //m_bsp->io[O_POWER_EN].SetBit(1);
    m_bsp->io[O_ELECTRIC_VALVE].SetBit(1);

    log_info("设备初始化完成.");
#endif

    while(!isInterruptionRequested())
    {
        isrunning = false;
        msleep(10);
        //开始按钮
        if(run_next)
        {
            run_next = 0;
            isrunning = true;
            // 产品区分检测
            if(m_bsp->io[I_DIFFER].GetBit() != 1)
            {
                is_test = 0;
                log_info("产品型号不一致!退出生产");
                emit sig_update_signal_lamp(LAMP_ALM);
                emit msg_log(MSG_ALM, "产品型号不一致，请区分产品高度！", true);
                continue;
            }
            else
            {
                is_test = 1;
            }
            //限高传感器
            if(m_bsp->io[I_LIMIT_HEIGHT].GetBit() == 0 )
            {
                is_test = 0;
                log_info("产品限制高度！退出生产.");
                emit sig_update_signal_lamp(LAMP_ALM);
                emit msg_log(MSG_ALM, "产品超过限制高度，停止生产！", true);
                continue;
            }
            else
            {
                is_test = 1;
            }

            // 设置信号灯为绿灯
            emit sig_update_signal_lamp(LAMP_RUN);

            /* add your code */
            // 更新检测结果信息
            result = -1;
            emit sig_update_check_result(result, "", "");
            log_info("主界面检测结果信息清空.");

            if(choose_mode_num==1){
                class_bed_mode();
            }
            else if(choose_mode_num==2){
                sponge_mode();
            }
            else if(choose_mode_num==3){
                shelled_mode();
            }
            else if(choose_mode_num==4){
                bare_board_mode();
            }

            if(is_finished_flag == true)
            {
                emit sig_update_check_result(2, m_info_version, m_info_id);//不合格
                emit sig_update_signal_lamp(LAMP_ALM);
                QString errInfo = QString("测试代码,流程报警错误!");
                emit msg_log(MSG_ALM, errInfo, true);
                log_info(errInfo);
                run_next = 0;
                continue;
            }
            else
            {
                emit sig_update_check_result(1, m_info_version, m_info_id);//合格
            }
            msleep(2000);

            run_next = 0;
            isrunning = false;
            emit sig_update_signal_lamp(LAMP_IDLE);
            log_info("测试完成.");
        }
    }
}


/*
 *==============================================================================
 *   界面模块功能函数
 *==============================================================================
 */
/*add your code*/

//--  流程动作函数  --------------------------------------------------------------

//类床模式
void Process::class_bed_mode()
{
    qDebug()<<"类床模式！";
    is_finished_flag = false;
    //判断左气缸是否压紧
    if(is_cylinder_tight_L())
    {
        log_info("左侧气缸已经压紧.");
        //判断右气缸是否压紧
        if(is_cylinder_tight_R())
        {
            log_info("右侧气缸已经压紧.");
            if(is_differ())
            {
                is_test = 1;
                log_info("被测传感器与产品一致,可以测试.");
            }
            else
            {
                is_test = 0;
                emit sig_update_signal_lamp(LAMP_ALM);
                emit msg_log(MSG_ALM, "当前摆放传感器与当前载入的产品不一致.\n请检查传感器产品或重新设定产品参数!", true);
                log_info("传感器与产品不一致,退出生产.");
                run_next = 0;
            }
        }
        else
        {
            is_test = 0;
            emit sig_update_signal_lamp(LAMP_ALM);
            emit msg_log(MSG_ALM, "当前右气缸未压紧.\n请检查传感器是否摆放规整!", true);
            log_info("右侧气缸未压紧,退出生产.");
            run_next = 0;
        }
    }
    else
    {
        is_test = 0;
        emit sig_update_signal_lamp(LAMP_ALM);
        emit msg_log(MSG_ALM, "当前左气缸未压紧.\n请检查传感器是否摆放规整!", true);
        log_info("左侧气缸未压紧,退出生产.");
        run_next = 0;
    }
    log_info(QString("本次测试工位:%1").arg(is_test?"工位":""));


    // 读取传感器ID和软件版本
    m_info_id.clear();
    m_info_version.clear();
    if(is_test)
    {
        log_info("开始读取传感器的硬件信息.");
        is_info_received = false;
        emit sig_read_pcb_info(is_test);
        while(!is_info_received)
        {
            QThread::msleep(1);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
        log_info("传感器的硬件信息读取结束.");
    }
    /* 若没有读到ID和软件版本,可能是旧版本程序,无通讯协议导致读取超时.也可能是串口连续读取错误,导致读取失败(要考虑到这种情况) */
    /* TODO:是否要加读取为空的消息提示框? */

    emit sig_reflash_platform_status("检测中");

    for(Product_Param val : m_pp.mProduct_Param_List)
    {

        //载台Z轴到回零位置
        if(is_test)
        {
            m_bsp->axis[AXIS_Z_PLATFORM].APS_Home();
            try
            {
                m_bsp->axis[AXIS_Z_PLATFORM].APS_WaitMoveDone();
            }
            catch(const std::runtime_error& e)
            {
                emit sig_update_signal_lamp(LAMP_ALM);
                emit msg_log(MSG_ALM, QString("类床测试:\n%1\n载台Z轴回零错误!").arg(e.what()), true);
            }
        }
        //载台Y轴到检测位置
        if(is_test)
            platform_y_move_to_check_pos(val.y_axis_position);
        if(is_test)
        {
            try
            {
                wait_platform_y_in_place();
                log_info("载台Y轴运动到检测位置.");
            }
            catch(const std::runtime_error& e)
            {
                emit sig_update_signal_lamp(LAMP_ALM);
                QString errInfo = QString("%1").arg(e.what());
                emit msg_log(MSG_ALM, errInfo, true);
                log_info(QString("载台Y轴运动到检测位置错误:") + errInfo);
                run_next = 0;
            }
        }

        if(is_test)
        {
            //载台z轴移动距离 = 载台Z轴到床板距离 - 传感器高度 +传感器伸出床板高度 +床板厚度;
            double platform_z_check_pos = m_ss.platform_z_bed_pos - m_pp.sensor_height + m_pp.sensor_out_board_height + m_ss.plank_thickness ;
            platform_z_move_to_check_pos(platform_z_check_pos);
        }
        if(is_test)
        {
            try
            {
                wait_platform_z_in_place();
                log_info("载台Z轴运动到检测位置.");
            }
            catch(const std::runtime_error& e)
            {
                emit sig_update_signal_lamp(LAMP_ALM);
                QString errInfo = QString("%1").arg(e.what());
                emit msg_log(MSG_ALM, errInfo, true);
                log_info(QString("载台Z轴运动到检测位置错误:") + errInfo);
                run_next = 0;
            }
        }

        // 激振器z轴到检测位置
        if(is_test)
        {
            //激振器z轴移动距离 = 激振器到床板距离-加装部件长度-床垫厚度+激振器预压深度+微调距离;
            double shaker_z_check_pos = m_ss.shaker_bed_pos - m_pp.shaker_part_length - m_pp.mattress_height + m_pp.shaker_press_depth + m_pp.little_distance ;
            shaker_z_move_to_check_pos(shaker_z_check_pos);
        }

        // 等待激振器z轴到位
        if(is_test)
        {
            try
            {
                wait_shaker_z_in_place();
                log_info("激振器Z轴运动到检测位置.");
            }
            catch(const std::runtime_error& e)
            {
                emit sig_update_signal_lamp(LAMP_ALM);
                QString errInfo = QString("%1").arg(e.what());
                emit msg_log(MSG_ALM, errInfo, true);
                log_info(QString("激振器Z轴运动到检测位置:") + errInfo);
                run_next = 0;
            }
        }

        // 信号检测系统
        if(is_test)
        {
            log_info("开始信号检测.");

            QString file_path= ROOT_PATH_ONE + m_pp.product_name+"/"+val.waveform_path;
            this->m_skr->start_setting();
            emit sig_set_ni_param_and_start_ao_ai(file_path, val.sampling_rate, val.acquisition_time,m_pp.product_name);

            //无论设置成功与否，都要等待设置完成
//            if(this->m_skr->is_shaker_running()) // 判断激振器是否在工作
//            {
//                ;
//            }
            while(!this->m_skr->is_setting_finished())
            {
                QThread::msleep(20);
            }

            //判断激振器是否启动成功
            if(!this->m_skr->is_started_success())
            {
                continue;
            }

            //等待当前这条流程的波形输出完成
            while(m_skr->is_shaker_running())
            {
                QThread::msleep(10);
                QCoreApplication::processEvents(QEventLoop::AllEvents);
            }
            is_finished_flag = true;
            log_info("信号检测完成.");
        }

        // 激振器z轴到安全位置
        if(is_test)
            shaker_z_move_to_safe_state(m_ep.pos_safe);

        // 等待激振器z轴到位
        if(is_test)
        {
            try
            {
                wait_shaker_z_in_place();
                log_info("激振器Z轴运动到安全位置.");
            }
            catch(const std::runtime_error& e)
            {
                emit sig_update_signal_lamp(LAMP_ALM);
                QString errInfo = QString("%1").arg(e.what());
                emit msg_log(MSG_ALM, errInfo, true);
                log_info(QString("激振器Z轴运动到安全位置错误:") + errInfo);
                run_next = 0;
            }
        }
        //载台Z轴回零运动
        if(is_test)
            m_bsp->axis[AXIS_Z_SHAKER].APS_Home();

        try
        {
            m_bsp->axis[AXIS_Z_SHAKER].APS_WaitHomeDone();
        }
        catch(const std::runtime_error& e)
        {
            m_bsp->axis[AXIS_Z_SHAKER].APS_SetSVON(0);
            emit sig_update_signal_lamp(LAMP_ALM);
            emit msg_log(MSG_ALM, QString("系统初始化:\n%1\n请手动励磁回零!").arg(e.what()), true);
        }
        log_info("激振器Z轴回零完成.");

    }
    emit sig_reflash_platform_status("等待");

}

//海绵模式
void Process::sponge_mode()
{
    qDebug()<<"sponge_mode!";
    is_finished_flag = false;
    //判断左气缸是否压紧
    if(is_cylinder_tight_L())
    {
        log_info("左侧气缸已经压紧.");
        //判断右气缸是否压紧
        if(is_cylinder_tight_R())
        {
            log_info("右侧气缸已经压紧.");
            if(is_differ())
            {
                is_test = 1;
                log_info("被测传感器与产品一致,可以测试.");
            }
            else
            {
                is_test = 0;
                emit sig_update_signal_lamp(LAMP_ALM);
                emit msg_log(MSG_ALM, "当前摆放传感器与当前载入的产品不一致.\n请检查传感器产品或重新设定产品参数!", true);
                log_info("传感器与产品不一致,退出生产.");
                run_next = 0;
            }
        }
        else
        {
            is_test = 0;
            emit sig_update_signal_lamp(LAMP_ALM);
            emit msg_log(MSG_ALM, "当前右气缸未压紧.\n请检查传感器是否摆放规整!", true);
            log_info("右侧气缸未压紧,退出生产.");
            run_next = 0;
        }
    }
    else
    {
        is_test = 0;
        emit sig_update_signal_lamp(LAMP_ALM);
        emit msg_log(MSG_ALM, "当前左气缸未压紧.\n请检查传感器是否摆放规整!", true);
        log_info("左侧气缸未压紧,退出生产.");
        run_next = 0;
    }
    log_info(QString("本次测试工位:%1").arg(is_test?"工位":""));


    // 读取传感器ID和软件版本
    m_info_id.clear();
    m_info_version.clear();
    if(is_test)
    {
        log_info("开始读取传感器的硬件信息.");
        is_info_received = false;
        emit sig_read_pcb_info(is_test);
        while(!is_info_received)
        {
            QThread::msleep(1);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
        log_info("传感器的硬件信息读取结束.");
    }
    /* 若没有读到ID和软件版本,可能是旧版本程序,无通讯协议导致读取超时.也可能是串口连续读取错误,导致读取失败(要考虑到这种情况) */
    /* TODO:是否要加读取为空的消息提示框? */

    emit sig_reflash_platform_status("检测中");

    for(Product_Param val : m_pp.mProduct_Param_List)
    {

        //载台Z轴到回零位置
        if(is_test)
        {
            m_bsp->axis[AXIS_Z_PLATFORM].APS_Home();
            try
            {
                m_bsp->axis[AXIS_Z_PLATFORM].APS_WaitMoveDone();
            }
            catch(const std::runtime_error& e)
            {
                emit sig_update_signal_lamp(LAMP_ALM);
                emit msg_log(MSG_ALM, QString("类床测试:\n%1\n载台Z轴回零错误!").arg(e.what()), true);
            }
        }
        //载台Y轴到检测位置
        if(is_test)
            platform_y_move_to_check_pos(val.y_axis_position);
        if(is_test)
        {
            try
            {
                wait_platform_y_in_place();
                log_info("载台Y轴运动到检测位置.");
            }
            catch(const std::runtime_error& e)
            {
                emit sig_update_signal_lamp(LAMP_ALM);
                QString errInfo = QString("%1").arg(e.what());
                emit msg_log(MSG_ALM, errInfo, true);
                log_info(QString("载台Y轴运动到检测位置错误:") + errInfo);
                run_next = 0;
            }
        }

        if(is_test)
        {
            //载台z轴移动距离 = 载台Z轴到床板距离 - 传感器高度 +传感器伸出床板高度 +床板厚度;
            double platform_z_check_pos = m_ss.platform_z_bed_pos - m_pp.sensor_height + m_pp.sensor_out_board_height + m_ss.plank_thickness ;
            platform_z_move_to_check_pos(platform_z_check_pos);
        }
        if(is_test)
        {
            try
            {
                wait_platform_z_in_place();
                log_info("载台Z轴运动到检测位置.");
            }
            catch(const std::runtime_error& e)
            {
                emit sig_update_signal_lamp(LAMP_ALM);
                QString errInfo = QString("%1").arg(e.what());
                emit msg_log(MSG_ALM, errInfo, true);
                log_info(QString("载台Z轴运动到检测位置错误:") + errInfo);
                run_next = 0;
            }
        }

        // 激振器z轴到检测位置
        if(is_test)
        {
            //激振器z轴移动距离 = 激振器到床板距离-加装部件长度+激振器预压深度+微调距离;
            double shaker_z_check_pos = m_ss.shaker_bed_pos - m_pp.shaker_part_length + m_pp.shaker_press_depth + m_pp.little_distance ;
            shaker_z_move_to_check_pos(shaker_z_check_pos);
        }

        // 等待激振器z轴到位
        if(is_test)
        {
            try
            {
                wait_shaker_z_in_place();
                log_info("激振器Z轴运动到检测位置.");
            }
            catch(const std::runtime_error& e)
            {
                emit sig_update_signal_lamp(LAMP_ALM);
                QString errInfo = QString("%1").arg(e.what());
                emit msg_log(MSG_ALM, errInfo, true);
                log_info(QString("激振器Z轴运动到检测位置:") + errInfo);
                run_next = 0;
            }
        }

        // 信号检测系统
        if(is_test)
        {
            log_info("开始信号检测.");

            QString file_path= ROOT_PATH_ONE + m_pp.product_name+"/"+val.waveform_path;
            this->m_skr->start_setting();// 开始设置
            emit sig_set_ni_param_and_start_ao_ai(file_path, val.sampling_rate, val.acquisition_time,m_pp.product_name);

            //无论设置成功与否，都要等待设置完成
            while(!this->m_skr->is_setting_finished())
            {
                QThread::msleep(20);
            }

            //判断激振器是否启动成功
            if(!this->m_skr->is_started_success())
            {
                continue;
            }

            //等待当前这条流程的波形输出完成
            while(m_skr->is_shaker_running())
            {
                QThread::msleep(10);
                QCoreApplication::processEvents(QEventLoop::AllEvents);
            }

            is_finished_flag = true;
            log_info("信号检测完成.");
        }

        // 激振器z轴到安全位置
        if(is_test)
            shaker_z_move_to_safe_state(m_ep.pos_safe);

        // 等待激振器z轴到位
        if(is_test)
        {
            try
            {
                wait_shaker_z_in_place();
                log_info("激振器Z轴运动到安全位置.");
            }
            catch(const std::runtime_error& e)
            {
                emit sig_update_signal_lamp(LAMP_ALM);
                QString errInfo = QString("%1").arg(e.what());
                emit msg_log(MSG_ALM, errInfo, true);
                log_info(QString("激振器Z轴运动到安全位置错误:") + errInfo);
                run_next = 0;
            }
        }
    }
    emit sig_reflash_platform_status("等待");
}

//带壳模式
void Process::shelled_mode()
{
    qDebug()<<"带壳模式！";
    is_finished_flag = false;
    emit sig_reflash_platform_status("检测中");

    // 读取传感器ID和软件版本
    m_info_id.clear();
    m_info_version.clear();
    if(is_test)
    {
        log_info("开始读取传感器的硬件信息.");
        is_info_received = false;
        emit sig_read_pcb_info(is_test);
        while(!is_info_received)
        {
            QThread::msleep(1);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
        log_info("传感器的硬件信息读取结束.");
    }

    for(Product_Param val : m_pp.mProduct_Param_List)
    {

        if(is_test)
        {
            // 激振器z轴到检测位置 = 激振器到床板距离 - 加装部件长度 - 带壳模式工装厚度
            double shaker_z_check_pos = m_ss.shaker_bed_pos - m_pp.shaker_part_length - m_pp.mattress_height;
            shaker_z_move_to_check_pos(shaker_z_check_pos);
        }
        // 等待激振器z轴到位
        if(is_test)
        {
            try
            {
                wait_shaker_z_in_place();
                log_info("激振器Z轴运动到检测位置.");
            }
            catch(const std::runtime_error& e)
            {
                emit sig_update_signal_lamp(LAMP_ALM);
                QString errInfo = QString("%1").arg(e.what());
                emit msg_log(MSG_ALM, errInfo, true);
                log_info(QString("激振器Z轴运动到检测位置:") + errInfo);
                run_next = 0;
            }
        }
        if(is_test)
        {
            log_info("开始信号检测.");


            QString file_path= ROOT_PATH_ONE + m_pp.product_name+"/"+val.waveform_path;
            this->m_skr->start_setting();
            emit sig_set_ni_param_and_start_ao_ai(file_path, val.sampling_rate, val.acquisition_time,m_pp.product_name);

            //无论设置成功与否，都要等待设置完成
            while(!this->m_skr->is_setting_finished())
            {
                QThread::msleep(20);
            }

            //判断激振器是否启动成功W
            if(!this->m_skr->is_started_success())
            {
                continue;
            }

            //等待当前这条流程的波形输出完成
            while(m_skr->is_shaker_running())
            {
                QThread::msleep(10);
                QCoreApplication::processEvents(QEventLoop::AllEvents);
            }
            is_finished_flag = true;
            log_info("信号检测完成.");
        }

        // 激振器z轴到安全位置
        if(is_test)
            shaker_z_move_to_safe_state(m_ep.pos_safe);

        // 等待激振器z轴到位
        if(is_test)
        {
            try
            {
                wait_shaker_z_in_place();
                log_info("激振器Z轴运动到安全位置.");
            }
            catch(const std::runtime_error& e)
            {
                emit sig_update_signal_lamp(LAMP_ALM);
                QString errInfo = QString("%1").arg(e.what());
                emit msg_log(MSG_ALM, errInfo, true);
                log_info(QString("激振器Z轴运动到安全位置错误:") + errInfo);
                run_next = 0;
            }
        }
    }

    emit sig_reflash_platform_status("等待");
}

//裸板模式
void Process::bare_board_mode()
{
    qDebug()<<"裸板模式！";
    is_finished_flag = false;
    emit sig_reflash_platform_status("检测中");

    // 读取传感器ID和软件版本
    m_info_id.clear();
    m_info_version.clear();
    if(is_test)
    {
        log_info("开始读取传感器的硬件信息.");
        is_info_received = false;
        emit sig_read_pcb_info(is_test);
        while(!is_info_received)
        {
            QThread::msleep(1);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
        log_info("传感器的硬件信息读取结束.");
    }

/*    for(Product_Param val : m_pp.mProduct_Param_List)
    {
        log_info("开始信号检测.");

        QString file_path= ROOT_PATH_ONE + m_pp.product_name+"/"+val.waveform_path;
        this->m_skr->is_shaker_set_finished = false;
        emit sig_set_ni_param_and_start_ao_ai(file_path, val.sampling_rate, val.acquisition_time ,m_pp.product_name);

        //无论设置成功与否，都要等待设置完成
        while(!this->m_skr->is_shaker_set_finished)
        {
            QThread::msleep(20);
        }

        //判断激振器是否启动成功W
        if(!this->m_skr->is_shaker_start_success)
        {
            continue;
        }

        //等待当前这条流程的波形输出完成
        while(m_skr->is_shaker_running())
        {
            QThread::msleep(10);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
        is_finished_flag = true;
        log_info("信号检测完成.");
    }

    emit sig_reflash_platform_status("等待");*/

    for(Product_Param val : m_pp.mProduct_Param_List)
    {

        if(is_test)
        {
            // 激振器z轴到检测位置 = 激振器到床板距离 - 加装部件长度 - 裸板模式工装厚度
            double shaker_z_check_pos = m_ss.shaker_bed_pos - m_pp.shaker_part_length - m_pp.mattress_height;
            shaker_z_move_to_check_pos(shaker_z_check_pos);
        }
        // 等待激振器z轴到位
        if(is_test)
        {
            try
            {
                wait_shaker_z_in_place();
                log_info("激振器Z轴运动到检测位置.");
            }
            catch(const std::runtime_error& e)
            {
                emit sig_update_signal_lamp(LAMP_ALM);
                QString errInfo = QString("%1").arg(e.what());
                emit msg_log(MSG_ALM, errInfo, true);
                log_info(QString("激振器Z轴运动到检测位置:") + errInfo);
                run_next = 0;
            }
        }
        if(is_test)
        {
            log_info("开始信号检测.");

            QString file_path= ROOT_PATH_ONE + m_pp.product_name+"/"+val.waveform_path;
            this->m_skr->start_setting();
            emit sig_set_ni_param_and_start_ao_ai(file_path, val.sampling_rate, val.acquisition_time, m_pp.product_name);

            //无论设置成功与否，都要等待设置完成
            while(!this->m_skr->is_setting_finished())
            {
                QThread::msleep(20);
            }

            //判断激振器是否启动成功W
            if(!this->m_skr->is_started_success())
            {
                continue;
            }

            //等待当前这条流程的波形输出完成
            while(m_skr->is_shaker_running())
            {
                QThread::msleep(10);
                QCoreApplication::processEvents(QEventLoop::AllEvents);
            }
            is_finished_flag = true;
            log_info("信号检测完成.");
        }

        // 激振器z轴到安全位置
        if(is_test)
            shaker_z_move_to_safe_state(m_ep.pos_safe);

        // 等待激振器z轴到位
        if(is_test)
        {
            try
            {
                wait_shaker_z_in_place();
                log_info("激振器Z轴运动到安全位置.");
            }
            catch(const std::runtime_error& e)
            {
                emit sig_update_signal_lamp(LAMP_ALM);
                QString errInfo = QString("%1").arg(e.what());
                emit msg_log(MSG_ALM, errInfo, true);
                log_info(QString("激振器Z轴运动到安全位置错误:") + errInfo);
                run_next = 0;
            }
        }
    }
        emit sig_reflash_platform_status("等待");


}

/*
 *==============================================================================
 *   流程处理模块功能函数
 *==============================================================================
 */
/*
 * 判断左侧气缸是否压紧
 * true:压紧
 * false:未压紧
*/
bool Process::is_cylinder_tight_L(void)
{
    qDebug()<<"左侧气缸压紧";
    if(m_bsp->io[I_CYLINDER_L_UP].GetBit() == 1&&m_bsp->io[I_CYLINDER_L_DOWN].GetBit() == 1)
    {
        return true;
    }
    else
    {
        return false;
    }

}

/*
 * 判断右侧气缸是否压紧
 * true:压紧
 * false:未压紧
*/
bool Process::is_cylinder_tight_R(void)
{
    qDebug()<<"右侧气缸压紧";
    if(m_bsp->io[I_CYLINDER_R_UP].GetBit() == 1&&m_bsp->io[I_CYLINDER_R_DOWN].GetBit() == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*add your code*/

//--  槽函数  -------------------------------------------------------------------
// 更新当前产品名称
void Process::slot_update_current_procuct_name(QString product_name)
{
    current_product_name = product_name;
}

// 更新设备调试参数
void Process::slot_update_ep_param(Equipment_Param ep)
{
    m_ep = ep;
}
// 更新系统参数
void Process::slot_update_sys_param(Sys_Setup ss)
{
    m_ss = ss;
}
// 更新产品参数
void Process::slot_update_pd_param(Full_Product_Param pp)
{
    m_pp = pp;
}

/*
 * @ brief  获取传感器区分状态
 * @ retval true:   是当前载入的产品
 *          false:  不是当前载入的产品
 */
bool Process::is_differ(void)
{
    int status = 0;
#ifndef DEBUG_MODE
    if(m_bsp->io[I_DIFFER].GetBit() == m_ep.type_en)
        status = 1;
#else
    status = 1;
    msleep(100);
    log("产品类型正确.");
#endif
    return status?true:false;
}

/*
 * @ brief  载台Y轴到检测位置
 * @ retval .
 */
void Process::platform_y_move_to_check_pos(double val)
{
#ifndef DEBUG_MODE
    m_bsp->axis[AXIS_Y_PLATFORM].APS_AMove(val, m_ss.z_ratio_Y);
#else
    msleep(100);
    log("载台Y轴到检测位置");
#endif
    emit sig_reflash_axis_platform_y_status("运行中", 1);
}

/*
 * @ brief  等待载台Y轴到位
 * @ retval .
 */
void Process::wait_platform_y_in_place(void)
{
#ifndef DEBUG_MODE
    try
    {
        m_bsp->axis[AXIS_Y_PLATFORM].APS_WaitMoveDone();
    }
    catch(const std::runtime_error& e)
    {
        emit sig_reflash_axis_platform_y_status("报警", 1);
        QString errInfo = QString("载台Y轴:\n%1").arg(e.what());
        throw std::runtime_error(errInfo.toStdString().data());
    }
#else
    msleep(100);
    log("等待载台Y轴到位");
#endif
    emit sig_reflash_axis_platform_y_status("停止", 1);
}

/*
 * @ brief  载台Z轴到检测位置
 * @ retval .
 */
void Process::platform_z_move_to_check_pos(double val)
{
#ifndef DEBUG_MODE
    m_bsp->axis[AXIS_Z_PLATFORM].APS_AMove(val, m_ss.z_ratio_Z);
#else
    msleep(100);
    log("载台Z轴到检测位置");
#endif
    emit sig_reflash_axis_platform_z_status("运行中", 1);
}

/*
 * @ brief  等待载台Z轴到位
 * @ retval .
 */
void Process::wait_platform_z_in_place(void)
{
#ifndef DEBUG_MODE
    try
    {
        m_bsp->axis[AXIS_Z_PLATFORM].APS_WaitMoveDone();
    }
    catch(const std::runtime_error& e)
    {
        emit sig_reflash_axis_platform_z_status("报警", 1);
        QString errInfo = QString("载台Z轴:\n%1").arg(e.what());
        throw std::runtime_error(errInfo.toStdString().data());
    }
#else
    msleep(100);
    log("等待载台Z轴到位");
#endif
    emit sig_reflash_axis_platform_z_status("停止", 1);
}




/*
 * @ brief  激振器z轴到检测位置
 * @ retval .
 */
void Process::shaker_z_move_to_check_pos(double val)
{
#ifndef DEBUG_MODE
    m_bsp->axis[AXIS_Z_SHAKER].APS_AMove(val, m_ss.z_ratio_G);
#else
    msleep(100);
    log("激振器z轴到检测位置");
#endif
    emit sig_reflash_axis_shaker_z_status("运行中", 1);
}

/*
 * @ brief  等待激振器Z轴到位
 * @ retval .
 */
void Process::wait_shaker_z_in_place(void)
{
#ifndef DEBUG_MODE
    try
    {
        m_bsp->axis[AXIS_Z_SHAKER].APS_WaitMoveDone();
    }
    catch(const std::runtime_error& e)
    {
        emit sig_reflash_axis_shaker_z_status("报警", 1);
        QString errInfo = QString("激振器z轴:\n%1").arg(e.what());
        throw std::runtime_error(errInfo.toStdString().data());
    }
#else
    msleep(100);
    log("等待激振器z轴到位");
#endif
    emit sig_reflash_axis_shaker_z_status("停止", 1);
}


/*
 * @ brief  激振器z轴到安全位置
 * @ retval .
 */
void Process::shaker_z_move_to_safe_state(double val)
{
#ifndef DEBUG_MODE
    m_bsp->axis[AXIS_Z_SHAKER].APS_AMove(val, m_ss.z_ratio_G);
#else
    msleep(100);
    log("激振器z轴到安全位置");
#endif
    emit sig_reflash_axis_shaker_z_status("到安全位置", 1);
}
