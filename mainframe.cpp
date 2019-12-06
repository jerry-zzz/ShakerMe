/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    mainframe.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	主界面框架,初始化各个子面板,加载各个参数,初始化系统.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */

#include "mainframe.h"
#include "ui_mainframe.h"
#include <QDesktopWidget>
#include <QMessageBox>
#include <QDebug>
#include <stdexcept>
#include "Module/Log/sql_thread.h"
#include "system_info.h"

using namespace std;

MainFrame::MainFrame(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainFrame)
{
    ui->setupUi(this);

    //界面设为无边框
    setWindowFlags(Qt::FramelessWindowHint);

    //获取主屏幕分辨率,并设置界面
    QDesktopWidget* pDesktopWidget = QApplication::desktop();
    QRect screenRect = pDesktopWidget->screenGeometry();
    int nWidth = screenRect.width();
    int nHeight = screenRect.height();
    nWidth = 1024;
    nHeight = 768;
    this->setGeometry(0, 0, nWidth, nHeight);
    ui->machine_name->setText(MACHINE_NAME);

    // init 按钮 槽函数
    init_pb_connect();

    // 工作信息
    m_jobinfo = new JobInfo;

    // 日志
    ui_log = new LogInfo(this);
    ui_log->hide_panel();
    connect(this, &MainFrame::msg_log, ui_log, &LogInfo::record_log);

    // 主界面 ui
    ui_main = new MainPanel(this);
    ui_main->display_panel();
    connect(ui_main, &MainPanel::msg_log, ui_log, &LogInfo::record_log);
//    connect(ui_main, &MainPanel::sig_mode_num, this, &MainFrame::slot_mode_num);

    //图形界面
    ui_graph = new ShakerGraph(this);
    connect(ui_graph, &ShakerGraph::msg_log, ui_log, &LogInfo::record_log);
    ui_graph->hide_panel();

    // 运动控制卡
    ui_bsp = new Bsp(this);
    connect(ui_bsp, &Bsp::msg_log, ui_log, &LogInfo::record_log);
    connect(ui_bsp, &Bsp::sig_update_device_param,this,&MainFrame::slot_update_device_param);
    ui_bsp->hide_panel();

    // 检测系统
    ui_dect = new Detection(this);
    ui_dect->hide_panel();
    connect(ui_dect, &Detection::msg_log, ui_log, &LogInfo::record_log);
    //-- add your code ---------------------------------------------------------   
    // NI采集卡唯一对象
    ni = new NI4461_Class();   //唯一的ni对象

    //-- add your code ---------------------------------------------------------
    // 功率放大器模块
    shakerFb = new ShakerFeedback(ni);
    threadShakerFb = new QThread(this);
    shakerFb->moveToThread(threadShakerFb);
    threadShakerFb->start();

    //-- add your code ---------------------------------------------------------
    // 设备调试
    ui_ed = new EquipmentDebug(this);
    ui_ed->hide_panel();

    connect(ui_ed, &EquipmentDebug::msg_log, ui_log, &LogInfo::record_log);

    connect(ui_ed,&EquipmentDebug::sig_to_reset_feedback,shakerFb,&ShakerFeedback::slot_reset);
    connect(shakerFb,&ShakerFeedback::sig_send_feedback_result,ui_ed,&EquipmentDebug::slot_recv_feedback_result);
    connect(shakerFb,&ShakerFeedback::sig_send_current_voltage,ui_ed,&EquipmentDebug::slot_recv_current_feedback_voltage);
    connect(ui_ed,&EquipmentDebug::sig_to_feedback_start,shakerFb,&ShakerFeedback::slot_start);
    connect(ui_ed,&EquipmentDebug::sig_to_feedback_stop,shakerFb,&ShakerFeedback::slot_stop);
    //-- add your code ---------------------------------------------------------

    //激振器调试
    ui_shaker = new Shaker(this,ni,shakerFb);
    ui_shaker->hide_panel();
    connect(ui_dect,&Detection::sig_send_serial_open_result,ui_shaker,&Shaker::slot_recv_serial_open_result);

    //-- add your code ---------------------------------------------------------
    connect(ui_shaker, &Shaker::sig_send_shaker_acc_buff, shakerFb,&ShakerFeedback::slot_from_ai0_feedback_data);
//    connect(ui_shaker,&Shaker::sig_to_feedback_start,shakerFb,&ShakerFeedback::slot_start); //改成动态连接
//    connect(ui_shaker,&Shaker::sig_to_feedback_stop,shakerFb,&ShakerFeedback::slot_stop);
    connect(shakerFb,&ShakerFeedback::sig_send_feedback_result,ui_shaker,&Shaker::slot_recv_feedback_result);

//    connect(ui_ed,&EquipmentDebug::sig_to_feedback_start,shakerFb,&ShakerFeedback::slot_start);
//    connect(ui_ed,&EquipmentDebug::sig_to_feedback_stop,shakerFb,&ShakerFeedback::slot_stop);
//    connect(ui_ed,&EquipmentDebug::sig_to_feedback_start,ui_shaker,&Shaker::slot_;
//    connect(ui_ed,&EquipmentDebug::sig_to_feedback_stop,ui_shaker,&Shaker::slot_stop);
    // 产品管理
    ui_product = new ProductSet(this);
    ui_product->hide_panel();
    connect(ui_product, &ProductSet::msg_log, ui_log, &LogInfo::record_log);
    connect(ui_product, &ProductSet::sig_update_product_param, ui_dect, &Detection::slot_update_product_param);
    connect(ui_product, &ProductSet::sig_get_current_procuct_name, this, &MainFrame::slot_get_current_procuct_name);
    connect(ui_product, &ProductSet::sig_update_current_product_db_path, ui_ed, &EquipmentDebug::slot_update_db_path);
    connect(ui_product, &ProductSet::sig_update_current_product_pattern, ui_main,&MainPanel::slot_update_product_pattern);


    // 系统设置
    ui_setup = new SystemSetup(this);
    ui_setup->hide_panel();
    connect(ui_setup, &SystemSetup::msg_log, ui_log, &LogInfo::record_log);

    // 用户管理
    ui_user = new UserInfo(this);
    ui_user->hide_panel();
    connect(ui_user, &UserInfo::msg_log, ui_log, &LogInfo::record_log);
    connect(ui_user, &UserInfo::sig_user_logout, this, &MainFrame::slot_user_logout);

    // 查询面板
    ui_query = new Query(this);
    connect(ui_query, &Query::msg_log, ui_log, &LogInfo::record_log);
    connect(ui_setup, &SystemSetup::sig_update_sys_param, ui_query, &Query::slot_from_SystemSetup_update_sys_param);
    ui_query->hide_panel();

    // 流程
    tprocess = new Process();


    connect(tprocess, &Process::msg_log, ui_log, &LogInfo::record_log);
    connect(tprocess, &Process::sig_reflash_axis_platform_z_status, ui_main, &MainPanel::slot_reflash_axis_platform_z_status);
    connect(tprocess, &Process::sig_reflash_axis_platform_y_status, ui_main, &MainPanel::slot_reflash_axis_platform_y_status);
    connect(tprocess, &Process::sig_reflash_axis_shaker_z_status, ui_main, &MainPanel::slot_reflash_axis_shaker_z_status);
    connect(tprocess, &Process::sig_reflash_platform_status, ui_main, &MainPanel::slot_reflash_platform_status);
    connect(ui_ed, &EquipmentDebug::sig_update_ep_param, tprocess, &Process::slot_update_ep_param);
    connect(ui_setup, &SystemSetup::sig_update_sys_param, tprocess, &Process::slot_update_sys_param);
    connect(tprocess, &Process::sig_update_signal_lamp, this, &MainFrame::update_signal_lamp);
    connect(ui_product, &ProductSet::sig_get_current_procuct_name, tprocess, &Process::slot_update_current_procuct_name);
    connect(ui_product, &ProductSet::sig_update_pd_param, tprocess, &Process::slot_update_pd_param);

    //-- add your code ---------------------------------------------------------
    connect(tprocess, &Process::sig_update_check_result, ui_main, &MainPanel::update_history_and_current_statistics);
    connect(tprocess, &Process::sig_read_pcb_info, ui_dect, &Detection::dete_query_pcb_info);
    connect(ui_dect, &Detection::sig_output_pcb_info, tprocess, &Process::slot_output_pcb_info);
    connect(ui_main, &MainPanel::sig_mode_num, tprocess, &Process::slot_mode_num);
    connect(tprocess,&Process::sig_set_ni_param_and_start_ao_ai,ui_shaker,&Shaker::slot_set_ni_param_and_start_ao_ai);
    //激振器调试
    connect(ui_shaker, &Shaker::sig_start_signal_detection, ui_dect, &Detection::slot_start_recv_sensor_data);
    connect(ui_shaker, &Shaker::sig_stop_signal_detection, ui_dect, &Detection::slot_stop_recv_sensor_data);


    /*传输数据至图形界面*/
    connect(ui_shaker,&Shaker::sig_send_wavebuff,ui_graph,&ShakerGraph::slot_receive_wavebuff);
    connect(ui_shaker,&Shaker::sig_send_shaker_acc_buff,ui_graph,&ShakerGraph::slot_receive_shaker_acc_buff);
    connect(ui_shaker,&Shaker::sig_send_shaker_force_buff,ui_graph,&ShakerGraph::slot_receive_shaker_force_buff);
    connect(ui_dect,&Detection::sig_output_com_data,ui_graph,&ShakerGraph::slot_receive_sensor_buff);
    connect(ui_dect,&Detection::sig_clear_serial_data,ui_graph,&ShakerGraph::init_sensor_data_buff);


    connect(ui_setup,&SystemSetup::sig_sensor_generation,ui_main,&MainPanel::slot_sensor_generation);
    connect(ui_setup,&SystemSetup::sig_sensor_generation,ui_dect,&Detection::slot_sensor_generation);
    connect(ui_setup,&SystemSetup::sig_sensor_generation,ui_shaker,&Shaker::slot_sensor_generation);
    connect(ui_setup,&SystemSetup::sig_sensor_generation,this,&MainFrame::slot_sensor_generation);

    //串口信息更新
    connect(ui_bsp, &Bsp::sig_update_device_param, ui_dect, &Detection::slot_update_device_param);

    // 定时器
    current_Timer = new QTimer(this);
    connect(current_Timer, SIGNAL(timeout()), this, SLOT(slot_current_time()));

    signal_Timer = new QTimer(this);
    connect(signal_Timer, SIGNAL(timeout()), this, SLOT(slot_signal_check_timer()));

    //压力、激光传感器对象movetothread
    mws = new WeighingSensor();
    connect(ui_bsp, &Bsp::sig_update_ws, mws, &WeighingSensor::slot_update_device_param);
    threadmws = new QThread(this);
    mws->moveToThread(threadmws);

    mlr = new LaserRanging();
    connect(ui_bsp, &Bsp::sig_update_lr, mlr, &LaserRanging::slot_update_device_param);
    threadmlr = new QThread(this);
    mlr->moveToThread(threadmlr);

    threadmws->start();
    threadmlr->start();

    // 各个模块参数初始化
    bsp_status = ui_bsp->init_bsp_param();
    ui_ed->init_EquipmentDebug(ui_bsp,mws,mlr,ui_shaker,shakerFb);
    ui_product->init_product(ROOT_PATH, SYSTEM_PARAM_DB_PATH, ui_bsp, mws, mlr);
    ui_setup->init_system_setup();
    ui_user->init_userinfo(m_jobinfo);
    tprocess->init_Process(ui_bsp,ui_shaker);

    // 设备调试
    connect(mws,&WeighingSensor::send_result,ui_product,&ProductSet::rece_current_value_ws);
    connect(mws,&WeighingSensor::send_result,ui_ed,&EquipmentDebug::slots_rece_result);

    connect(mlr,&LaserRanging::send_result,ui_product,&ProductSet::rece_current_value_lr);
    connect(mlr,&LaserRanging::send_result,ui_ed,&EquipmentDebug::slots_rece_lr_result);
    connect(mlr,&LaserRanging::result_id,ui_ed,&EquipmentDebug::slots_lr_id_result);


    // 流程
    tprocess->start();

    // 登录界面
    ui_login = new Login();
    connect(ui_login, &Login::msg_log, ui_log, &LogInfo::record_log);
    ui_login->setWindowFlags(ui_login->windowFlags() &~ Qt::WindowMaximizeButtonHint);//禁止最大化
    ui_login->setWindowFlags(ui_login->windowFlags() &~ Qt::WindowMinMaxButtonsHint);//禁止最大和最小化
    ui_login->setWindowFlags(ui_login->windowFlags() &~ Qt::WindowCloseButtonHint);//禁止关闭
    ui_login->display_panel();
    connect(ui_login, &Login::sig_user_login, this, &MainFrame::slot_user_login);

    // 版本号
    ui->version->setText(VERSION);
    //-- add your code ---------------------------------------------------------

}

MainFrame::~MainFrame()
{
    delete ui;
}


/*
 *==============================================================================
 *   模块固有功能函数
 *==============================================================================
 */
//com刷新
void MainFrame::slot_update_device_param(QVector<Device_Attribute> device_list)
{
    m_com_ws = device_list.at(2);
    m_com_lr = device_list.at(3);

    mws->update_com_imformation(m_com_ws.com);
    mlr->update_com_imformation(m_com_lr.com);
}

//传感器代数选择
void MainFrame::slot_sensor_generation(int sensor_generation)
{
    if(sensor_generation == 1)
        ui->generation->setText("一代传感器");
    else if(sensor_generation == 2)
        ui->generation->setText("二代传感器");
}
// 实时时间刷新
void MainFrame::slot_current_time()
{
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy/MM/dd hh:mm:ss ddd");
    ui->currenttime->setText(current_date);

    // 每59秒保存一次下机时间 此处更新周期可根据实际项目更改
    if((current_date_time.time().second() % 59 == 0))
    {
        if(can_record_logout_time)
        {
            m_jobinfo->record_logout_time();
            can_record_logout_time = false;
        }
    }
    else
        can_record_logout_time = true;

}

void MainFrame::slot_signal_check_timer()
{
    if(!tprocess->is_running())
    {
        // 传感器
        QString info;
    //    int exist_L = ui_bsp->io[I_EXIST_L].GetBit();
        int cylinder_L,cylinder_R,i_high_limit,i_differ;
        if(ui_bsp->io[I_CYLINDER_L_UP].GetBit() == 0 && ui_bsp->io[I_CYLINDER_L_DOWN].GetBit() == 0)
            cylinder_L = 0;
        else
            cylinder_L = 1;
        if(ui_bsp->io[I_CYLINDER_R_UP].GetBit() == 0 && ui_bsp->io[I_CYLINDER_R_DOWN].GetBit() == 0)
            cylinder_R = 0;
        else
            cylinder_R = 1;
        if(ui_bsp->io[I_LIMIT_HEIGHT].GetBit() == 0)
            i_high_limit = 0;
        else
            i_high_limit = 1;
        if(ui_bsp->io[I_DIFFER].GetBit() == 0)
            i_differ = 0;
        else
            i_differ = 1;
        if(!cylinder_L)
            info = "等待\n上料";
        else if(!cylinder_R)
            info = "等待\n上料";
        else if(!i_high_limit)
            info = "限制\n高度";
        else if(!i_differ)
            info = "产品\n区分";
        else
            info = "上料\n完成";

        ui_main->update_gongwei_status_runtime(info);
    }
}

// 按钮槽函数初始化
void MainFrame::init_pb_connect()
{
    connect(ui->b_close,&QPushButton::clicked, this, &MainFrame::pb_close);
    connect(ui->b_minimize,&QPushButton::clicked, this, &MainFrame::pb_minimize);
    connect(ui->p_main,&QPushButton::clicked, this, &MainFrame::pb_main);
    connect(ui->p_product,&QPushButton::clicked, this, &MainFrame::pb_product_set);
    connect(ui->p_sysparam,&QPushButton::clicked, this, &MainFrame::pb_sysparam);
    connect(ui->p_test,&QPushButton::clicked, this, &MainFrame::pb_test);
    connect(ui->p_debug,&QPushButton::clicked, this, &MainFrame::pb_debug);
    connect(ui->p_config,&QPushButton::clicked, this, &MainFrame::pb_config);
    connect(ui->p_user,&QPushButton::clicked, this, &MainFrame::pb_user);
    connect(ui->b_account,&QPushButton::clicked, this, &MainFrame::pb_user);
    connect(ui->p_log,&QPushButton::clicked, this, &MainFrame::pb_loginfo);
    connect(ui->b_start, &QPushButton::clicked, this, &MainFrame::pb_start);
    connect(ui->b_reset, &QPushButton::clicked, this, &MainFrame::pb_reset);
    connect(ui->p_exciter, &QPushButton::clicked, this,&MainFrame::pb_exciter);
//    connect(ui->p_query,&QPushButton::clicked, this, &MainFrame::pb_query);
    connect(ui->p_graphies,&QPushButton::clicked,this,&MainFrame::pb_shakergraph);
    connect(ui->b_emg,&QPushButton::clicked,this,&MainFrame::pb_emg);
}

// 刷新主界面的状态信息
// 刷新信号灯状态
void MainFrame::update_signal_lamp(int lamp_status)
{
    ui_bsp->io[O_GLAMP].SetBit((lamp_status>>0)&0x01);
    ui_bsp->io[O_YLAMP].SetBit((lamp_status>>1)&0x01);
    ui_bsp->io[O_RLAMP].SetBit((lamp_status>>2)&0x01);

    switch(lamp_status)
    {
    case LAMP_RUN:
        ui->m_state->setStyleSheet("image: url(:/icon/res/lamp_run.png)");
        ui->m_state_str->setText("运行中");
        break;
    case LAMP_IDLE:
        ui->m_state->setStyleSheet("image: url(:/icon/res/lamp_idle.png)");
        ui->m_state_str->setText("待机中");
        break;
    case LAMP_ALM:
        ui->m_state->setStyleSheet("image: url(:/icon/res/lamp_alarm.png)");
        ui->m_state_str->setText("报警");
        break;
    default:
        ui->m_state->setStyleSheet("image: url(:/icon/res/lamp_white.png)");
    }
}

// 用户登录函数, 更新用户id, 各面板权限信息
void MainFrame::slot_user_login(QString id, int permissions)
{
    // 记录用户上机时间
    m_jobinfo->record_login_time(id);
    m_jobinfo->record_logout_time();

    // 用户id和权限
    m_id = id;
    m_permissions_level = permissions;

    QString name;
    ui->id->setText(id);
    switch(permissions)
    {
    case OP_LEVEL:
        name = OP_NAME;
        break;
    case ADMIN_LEVEL:
        name = ADMIN_NAME;
        break;
    }
    ui->permission->setText(name);
    ui_user->set_account_info(id, name);
    ui_log->set_user_id(id);

    //更新各面板的权限信息
    ui_product->set_current_level(m_permissions_level);
    ui_log->set_current_level(m_permissions_level);
    ui_main->init_panel(m_jobinfo);

    // 开启时间刷新定时器
    signal_Timer->start(50);
    current_Timer->start(900);
    slot_current_time();

    pb_main();      // 显示主界面
    this->show();
}

// 用户退出登录槽函数
void MainFrame::slot_user_logout()
{
    // 停止时间刷新定时器
    signal_Timer->stop();
    current_Timer->stop();
    // 隐藏界面    
    hide();
    // 显示登陆界面
    ui_login->display_panel();
    // 记录用户下机时间
    m_jobinfo->record_logout_time();
}

// 刷新产品名称,并显示
void MainFrame::slot_get_current_procuct_name(QString product_name)
{
    ui->product_name->setText(product_name);
}

//按钮 停止
void MainFrame::pb_emg()
{
    tprocess->return_of_product();
}

// 按钮 开始
void MainFrame::pb_start()
{
    ui_main->choose_mode();
    tprocess->set_run_next();
}

// 按钮 清除报警
void MainFrame::pb_reset()
{
    // 关闭蜂鸣器
    ui_bsp->io[O_BEEP].SetBit(0);
    // 将红灯变为黄灯
    ui_bsp->io[O_GLAMP].SetBit(0);
    ui_bsp->io[O_YLAMP].SetBit(1);
    ui_bsp->io[O_RLAMP].SetBit(0);
    //-- add your code 此处应该根据实际情况清除报警,使设备能正常运行---------------------

}

// 按钮 关闭
void MainFrame::pb_close()
{
    if(tprocess->is_running())
    {
        QMessageBox::warning(NULL, "warning", "请等检测完成后再关闭");
    }
    else
    {
        int button= QMessageBox::question(NULL, "提示", "确定要关闭程序吗?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if(button == QMessageBox::Yes)
        {
            //关闭线程
            threadShakerFb->quit();       

            // 记录下机时间
            m_jobinfo->record_logout_time();
            current_Timer->stop();
            signal_Timer->stop();

            threadmws->requestInterruption();
            threadmws->quit();
            threadmws->wait();

            threadmlr->requestInterruption();
            threadmlr->quit();
            threadmlr->wait();

            // 关闭线程
            tprocess->requestInterruption();
            tprocess->quit();
            tprocess->wait();

            // delete 各个模块
            delete current_Timer;
            delete signal_Timer;
            delete tprocess;
            delete ui_main;
            delete ui_product;
            delete ui_dect;
            delete ui_setup;
            delete ui_ed;
            delete ui_login;
            delete ui_query;
            delete ui_user;            
            delete ui_log;
            delete ui_bsp;
            delete ui_shaker;
            delete ui_graph;
            // delete 组件
            delete ni;
            delete shakerFb;

            this->close();
        }
    }
}

// 按钮 最小化
void MainFrame::pb_minimize()
{
    this->setWindowState(Qt::WindowMinimized);
}

// 选择模块界面
void MainFrame::switch_panel(int last_panel)
{
    switch(last_panel)
    {
    case PANLE_MAIN: ui_main->hide_panel();
        ui->p_main->setStyleSheet("QPushButton{\
                                    background-image: url(:/icon/res/ico_btn_bg.png);\
                                    color:white;}");
        break;
    case PANEL_PRODUCTSET: ui_product->hide_panel();
        ui->p_product->setStyleSheet("QPushButton{\
                                    background-image: url(:/icon/res/ico_btn_bg.png);\
                                    color:white;}");
        break;
    case PANLE_TIME:
        break;
    case PANLE_USER: ui_user->hide_panel();
        ui->p_user->setStyleSheet("QPushButton{\
                                    background-image: url(:/icon/res/ico_btn_bg.png);\
                                    color:white;}");
        break;
    case PANLE_LOGINFO: ui_log->hide_panel();
        ui->p_log->setStyleSheet("QPushButton{\
                                    background-image: url(:/icon/res/ico_btn_bg.png);\
                                    color:white;}");
        break;
    case PANLE_SYSPARAM: ui_setup->hide_panel();
        ui->p_sysparam->setStyleSheet("QPushButton{\
                                    background-image: url(:/icon/res/ico_btn_bg.png);\
                                    color:white;}");
        break;
    case PANLE_DEBUG: ui_ed->hide_panel();
        ui->p_debug->setStyleSheet("QPushButton{\
                                    background-image: url(:/icon/res/ico_btn_bg.png);\
                                    color:white;}");
        break;
    case PANLE_CONFIG: ui_bsp->hide_panel();
        ui->p_config->setStyleSheet("QPushButton{\
                                    background-image: url(:/icon/res/ico_btn_bg.png);\
                                    color:white;}");
        break;
    case PANLE_TEST: ui_dect->hide_panel();
        ui->p_test->setStyleSheet("QPushButton{\
                                    background-image: url(:/icon/res/ico_btn_bg.png);\
                                    color:white;}");
        break;
    case PANLE_EXCITER:ui_shaker->hide_panel();
        ui->p_exciter->setStyleSheet("QPushButton{\
                                    background-image: url(:/icon/res/ico_btn_bg.png);\
                                    color:white;}");
        break;
    case PANLE_GRAPH:ui_graph->hide_panel();
        ui->p_graphies->setStyleSheet("QPushButton{\
                                    background-image: url(:/icon/res/ico_btn_bg.png);\
                                    color:white;}");

    }
}

// 按钮 主界面
void MainFrame::pb_main()
{   
    emit msg_log(MSG_INFO, "进入主界面.", false);
    switch_panel(current_panel_index);
    ui_main->display_panel();
    current_panel_index = PANLE_MAIN;
    ui->p_main->setStyleSheet("QPushButton{\
                                background-image: url(:/icon/res/ico_btn_bg.png);\
                                color:yellow;}");
}

// 按钮 产品设置
void MainFrame::pb_product_set()
{
    emit msg_log(MSG_INFO, "进入产品设置界面.", false);
    switch_panel(current_panel_index);
    current_panel_index = PANEL_PRODUCTSET;
    ui_product->display_panel();
    ui->p_product->setStyleSheet("QPushButton{\
                                background-image: url(:/icon/res/ico_btn_bg.png);\
                                color:yellow;}");
}

// 按钮 系统设置
void MainFrame::pb_sysparam()
{
    if(m_permissions_level == OP_LEVEL)
    {
        emit msg_log(MSG_INFO, "系统设置面板需要管理员权限才能进入\n请联系管理员!", true);
        return;
    }
    emit msg_log(MSG_INFO, "进入系统设置界面.", false);
    switch_panel(current_panel_index);
    current_panel_index = PANLE_SYSPARAM;
    ui_setup->display_panel();
    ui->p_sysparam->setStyleSheet("QPushButton{\
                                background-image: url(:/icon/res/ico_btn_bg.png);\
                                color:yellow;}");
}

// 按钮 检测系统
void MainFrame::pb_test()
{
    emit msg_log(MSG_INFO, "进入检测系统界面.", false);
    switch_panel(current_panel_index);
    current_panel_index = PANLE_TEST;
    ui_dect->display_panel();
    ui->p_test->setStyleSheet("QPushButton{\
                                background-image: url(:/icon/res/ico_btn_bg.png);\
                                color:yellow;}");
}

// 按钮 设备调试
void MainFrame::pb_debug()
{   
    if(m_permissions_level == OP_LEVEL)
    {
        emit msg_log(MSG_INFO, "设备调试面板需要管理员权限才能进入\n请联系管理员!", true);
        return;
    }
    emit msg_log(MSG_INFO, "进入设备调试界面.", false);
    switch_panel(current_panel_index);
    current_panel_index = PANLE_DEBUG;
    ui_ed->display_panel();
    ui->p_debug->setStyleSheet("QPushButton{\
                                background-image: url(:/icon/res/ico_btn_bg.png);\
                                color:yellow;}");
}

//按钮 激振器调试
void MainFrame::pb_exciter()
{
    if(m_permissions_level == OP_LEVEL)
    {
        emit msg_log(MSG_INFO, "设备调试面板需要管理员权限才能进入\n请联系管理员!", true);
        return;
    }
    emit msg_log(MSG_INFO, "进入设备调试界面.", false);
    switch_panel(current_panel_index);
    current_panel_index = PANLE_EXCITER;
    ui_shaker->display_panel();
    ui->p_exciter->setStyleSheet("QPushButton{\
                                background-image: url(:/icon/res/ico_btn_bg.png);\
                                color:yellow;}");
}

//按钮 图形显示
void MainFrame::pb_shakergraph()
{
    emit msg_log(MSG_INFO, "进入图形显示界面.", false);
    switch_panel(current_panel_index);
    current_panel_index = PANLE_GRAPH;
    ui_graph->display_panel();
    ui->p_graphies->setStyleSheet("QPushButton{\
                                background-image: url(:/icon/res/ico_btn_bg.png);\
                                color:yellow;}");
}

// 按钮 设备配置
void MainFrame::pb_config()
{    
    if(m_permissions_level == OP_LEVEL)
    {
        emit msg_log(MSG_INFO, "设备配置面板需要管理员权限才能进入\n请联系管理员!", true);
        return;
    }
    emit msg_log(MSG_INFO, "进入设备配置界面.", false);
    switch_panel(current_panel_index);
    current_panel_index = PANLE_CONFIG;
    ui_bsp->display_panel();
    ui->p_config->setStyleSheet("QPushButton{\
                                background-image: url(:/icon/res/ico_btn_bg.png);\
                                color:yellow;}");
}

// 按钮 用户管理
void MainFrame::pb_user()
{
    emit msg_log(MSG_INFO, "进入用户管理界面.", false);
    switch_panel(current_panel_index);
    current_panel_index = PANLE_USER;
    ui_user->display_panel();
    ui->p_user->setStyleSheet("QPushButton{\
                                background-image: url(:/icon/res/ico_btn_bg.png);\
                                color:yellow;}");
}
/*
// 按钮 记录查询
void MainFrame::pb_query()
{
    switch_panel(current_panel_index);
    ui_query->display_panel();
    current_panel_index = PANLE_QUERY;
    ui->p_query->setStyleSheet("QPushButton{\
                                background-image: url(:/icon/res/ico_btn_bg.png);\
                                color:yellow;}");
}
*/
// 按钮 日志
void MainFrame::pb_loginfo()
{
    switch_panel(current_panel_index);
    ui_log->display_panel();
    current_panel_index = PANLE_LOGINFO;
    ui->p_log->setStyleSheet("QPushButton{\
                                background-image: url(:/icon/res/ico_btn_bg.png);\
                                color:yellow;}");
}
