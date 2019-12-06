/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    equipmentdebug.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	设备调试面板.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#include "equipmentdebug.h"
#include "ui_equipmentdebug.h"
#include <QMessageBox>
#include <QDebug>

#include "system_info.h"
#include "config_axis_io.h"
#include "../Log/loginfo.h"


EquipmentDebug::EquipmentDebug(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EquipmentDebug)
{
    ui->setupUi(this);

    //设为无边框
    setWindowFlags(Qt::FramelessWindowHint);
    this->setGeometry(NESTED_PANEL_POS_X, NESTED_PANEL_POS_Y, NESTED_PANEL_POS_W, NESTED_PANEL_POS_H);

    // 定时器
    panel_refresh_timer = new QTimer();
    connect(panel_refresh_timer, &QTimer::timeout, this, &EquipmentDebug::refresh_panel_AMP_Info);

    // io图标
    lamp_on.load(":/icon/res/lamp_signal_on.png");
    lamp_off.load(":/icon/res/lamp_signal_off.png");
    // 信号显示图片设置
    ui->limit_height_lamp->setScaledContents(true);
    ui->differ_lamp->setScaledContents(true);
    ui->cylinder_lamp_L_up->setScaledContents(true);
    ui->cylinder_lamp_L_down->setScaledContents(true);
    ui->cylinder_lamp_R_up->setScaledContents(true);
    ui->cylinder_lamp_R_down->setScaledContents(true);

    // 槽函数
    connect(ui->param_save, &QPushButton::clicked, this, &EquipmentDebug::pb_param_save);
    connect(ui->servo, &QPushButton::clicked, this, &EquipmentDebug::pb_set_servo);
    connect(ui->move_r_f, &QPushButton::clicked, this, &EquipmentDebug::pb_move_r_forward_dir);
    connect(ui->move_r_n, &QPushButton::clicked, this, &EquipmentDebug::pb_move_r_negative_dir);
    connect(ui->move_stop, &QPushButton::clicked, this, &EquipmentDebug::pb_move_stop);
    connect(ui->move_home, &QPushButton::clicked, this, &EquipmentDebug::pb_move_home);

    //-- add your code ---------------------------------------------------------

    connect(ui->read_pos_bed,&QPushButton::clicked,this,&EquipmentDebug::pb_read_pos_bed);
    connect(ui->move_pos_bed,&QPushButton::clicked,this,&EquipmentDebug::pb_move_pos_bed);
    connect(ui->read_pos_sponge,&QPushButton::clicked,this,&EquipmentDebug::pb_read_pos_sponge);
    connect(ui->move_pos_sponge,&QPushButton::clicked,this,&EquipmentDebug::pb_move_pos_sponge);
    connect(ui->read_pos_shell,&QPushButton::clicked,this,&EquipmentDebug::pb_read_pos_shell);
    connect(ui->move_pos_shell,&QPushButton::clicked,this,&EquipmentDebug::pb_move_pos_shell);
    connect(ui->read_pos_bare,&QPushButton::clicked,this,&EquipmentDebug::pb_read_pos_bare);
    connect(ui->move_pos_bare,&QPushButton::clicked,this,&EquipmentDebug::pb_move_pos_bare);
    connect(ui->read_pos_bed_y,&QPushButton::clicked,this,&EquipmentDebug::pb_read_pos_bed_y);
    connect(ui->move_pos_bed_y,&QPushButton::clicked,this,&EquipmentDebug::pb_move_pos_bed_y);
    connect(ui->read_pos_y,&QPushButton::clicked,this,&EquipmentDebug::pb_read_pos_y);
    connect(ui->move_pos_y,&QPushButton::clicked,this,&EquipmentDebug::pb_move_pos_y);
    connect(ui->read_pos_z,&QPushButton::clicked,this,&EquipmentDebug::pb_read_pos_z);
    connect(ui->move_pos_z,&QPushButton::clicked,this,&EquipmentDebug::pb_move_pos_z);
    // 槽函数

    connect(ui->read_pos_module_2, &QPushButton::clicked, this, &EquipmentDebug::pb_read_pos_shaker_test);
    connect(ui->move_pos_module_2, &QPushButton::clicked, this, &EquipmentDebug::pb_move_pos_shaker_test);
    connect(ui->ELECTRIC_VALVE_en,&QPushButton::clicked,this,&EquipmentDebug::pb_signal_ELECTRIC_en);

    //压力传感器
    connect(ui->modbus_rtu, &QPushButton::clicked, this, &EquipmentDebug::pb_modbus_command);
    connect(ui->open_ws_serialport, &QPushButton::clicked, this, &EquipmentDebug::pb_open_serialport_command);
    connect(ui->timer_switch, &QPushButton::clicked, this, &EquipmentDebug::pb_open_timer_switch);

    //激光测距传感器
    connect(ui->open_lr_serialport,&QPushButton::clicked,this,&EquipmentDebug::pb_open_lr_serialport_command);
    connect(ui->timer_switch_lr,&QPushButton::clicked,this,&EquipmentDebug::pb_open_lr_timer_switch);
    connect(ui->send_lr_command,&QPushButton::clicked,this,&EquipmentDebug::pb_lr_command);

    //功率放大器反馈调节页面
    connect(ui->btn_start_feedback,&QPushButton::clicked,this,&EquipmentDebug::slot_btn_start_feedback_clicked);
    connect(ui->btn_reset_feedback,&QPushButton::clicked,this,&EquipmentDebug::slot_btn_reset_feedback_clicked);

    // 加载参数
    load_param();
}

// 析构
EquipmentDebug::~EquipmentDebug()
{
    panel_refresh_timer->stop();
    delete panel_refresh_timer;
    delete ui;
}

/*
 *==============================================================================
 *   初始化模块/设置权限/界面显示/界面隐藏
 *==============================================================================
 */

// 初始化模块
void EquipmentDebug::init_EquipmentDebug(Bsp *bsp,WeighingSensor *ws,LaserRanging *lr,Shaker *shaker, ShakerFeedback *shakerFb)
{
    this->m_bsp = bsp;
    this->mws = ws;
    this->mlr = lr;
    this->shaker = shaker;
    this->shakerFb  = shakerFb;

    // 压力定时器
    m_pTimer_mws = new QTimer();
    connect(m_pTimer_mws, &QTimer::timeout, mws, &WeighingSensor::slot_read_current_numeric_command);
    // 激光定时器
    m_pTimer_mlr = new QTimer();
    connect(m_pTimer_mlr, &QTimer::timeout, mlr, &LaserRanging::channel_querl);
    load_param();
    //-- add your code ---------------------------------------------------------
}

// 显示界面
void EquipmentDebug::display_panel()
{
    // 加载参数
    load_param();
    // 更新轴列表
    ui->axis_index->clear();
    QStringList axis_list;
    for(int i=0;i<AXIS_MAX_NUM;i++)
    {
        axis_list.append(QString("%1").arg(m_bsp->axis[i].get_axis_config_param()->name));
    }
    ui->axis_index->insertItems(0, axis_list);

    show();     // 显示界面
    panel_refresh_timer->start(50);   // 开启界面刷新定时器
}

// 隐藏界面
void EquipmentDebug::hide_panel()
{
    panel_refresh_timer->stop();
    hide();
}

/*
 *==============================================================================
 *   界面模块功能函数
 *==============================================================================
 */

/*add your code*/
//-- 槽函数 ---------------------------------------------------------------------
//modbus指令
void EquipmentDebug::pb_modbus_command()
{
    wsp.farmar_weight = ui->nominal_weight->value();
    wsp.sensor_sensitivity = ui->sensitivity->value();
    wsp.sensor_range = ui->sensor_range->value();
    wsp.decimal_point = ui->decimal_point->value();
    wsp.zero_trigger_threshold = ui->zero_trigger_threshold->value();
    wsp.auto_zero = ui->auto_zero_set->value();
    wsp.range_coefficient = ui->range_coefficient->value();
    wsp.zero_reset_range = ui->zero_reset_range->value();
    wsp.digital_filter = ui->digital_filter->value();
    wsp.rated_range = ui->rated_range->value();
    wsp.auto_calibration = ui->auto_calibration->value();
    wsp.sentenced_stabilizing_range = ui->sentenced_stabilizing_range->value();
    wsp.sentenced_stabilizing_t = ui->sentenced_stabilizing_t->value();
    wsp.machine_code = ui->machine_code->value();
    wsp.send_interval = ui->send_interval->value();
    wsp.zero_tracking_range = ui->zero_tracking_range->value();
    wsp.auto_zero_delay = ui->auto_zero_delay->value();
    wsp.sample_rate = ui->sample_rate->value();
    wsp.communication_protocol = ui->communication_protocol->value();
    wsp.data_format = ui->data_format->value();
    wsp.baud_rate = ui->baud_rate->value();

    if(ui->choose_modbus_command->currentText() == "读取当前数值命令"){
        if(mws->read_current_numeric_command())
            qDebug()<< "crc16校验码成功";
        else
        {
            QMessageBox::information(NULL, "提示", "当前数值crc16校验码错误!");
            // 日志记录
            emit msg_log(MSG_INFO, QString("当前数值crc16校验码错误!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "当前重量清零"){
        if(mws->clear_current_weight())
            emit msg_log(MSG_INFO, QString("清零成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示", "清零失败!");
            // 日志记录
            emit msg_log(MSG_INFO, QString("清零失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "砝码校准方法"){
        if(mws->weight_calibration_method(wsp.farmar_weight))
            emit msg_log(MSG_INFO, QString("砝码校验成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","砝码校验失败!");
            emit msg_log(MSG_INFO, QString("砝码校验失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "数字校准方法"){
        if(mws->digital_calibration_method(wsp.sensor_sensitivity,wsp.sensor_range))
            emit msg_log(MSG_INFO, QString("数字校准成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","数字校验失败!");
            emit msg_log(MSG_INFO, QString("数字校验失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "修改小数点"){
        if(mws->change_float(wsp.decimal_point))
            emit msg_log(MSG_INFO, QString("修改小数点成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","修改小数点失败!");
            emit msg_log(MSG_INFO, QString("修改小数点失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "清零触发门限设置"){
        if(mws->zero_trigger_threshold_set(wsp.zero_trigger_threshold))
            emit msg_log(MSG_INFO, QString("清零触发门限修改成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","清零触发门限修改失败!");
            emit msg_log(MSG_INFO, QString("清零触发门限修改失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "自动清零设置"){
        if(mws->auto_zero_set(wsp.auto_zero))
            emit msg_log(MSG_INFO, QString("自动清零设置修改成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","自动清零设置修改失败!");
            emit msg_log(MSG_INFO, QString("自动清零设置修改失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "量程系数设置"){
        if(mws->range_coefficient_set(wsp.range_coefficient))
            emit msg_log(MSG_INFO, QString("量程系数修改成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","量程系数修改失败!");
            emit msg_log(MSG_INFO, QString("量程系数修改失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "上电清零范围设置"){
        if(mws->zero_reset_range_set(wsp.zero_reset_range))
            emit msg_log(MSG_INFO, QString("上电清零范围修改成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","上电清零范围修改失败!");
            emit msg_log(MSG_INFO, QString("上电清零范围修改失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "数字滤波设置"){
        if(mws->digital_filter_set(wsp.digital_filter))
            emit msg_log(MSG_INFO, QString("数字滤波修改成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","数字滤波修改失败!");
            emit msg_log(MSG_INFO, QString("数字滤波修改失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "额定量程设置"){
        if(mws->rated_range_set(wsp.rated_range))
            emit msg_log(MSG_INFO, QString("额定量程修改成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","额定量程修改失败!");
            emit msg_log(MSG_INFO, QString("额定量程修改失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "自动校准设置"){
        if(mws->auto_calibration_set(wsp.auto_calibration))
            emit msg_log(MSG_INFO, QString("自动校准修改成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","自动校准修改失败!");
            emit msg_log(MSG_INFO, QString("自动校准修改失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "判稳范围设置"){
        if(mws->sentenced_stabilizing_range_set(wsp.sentenced_stabilizing_range))
            emit msg_log(MSG_INFO, QString("判稳范围修改成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","判稳范围修改失败!");
            emit msg_log(MSG_INFO, QString("判稳范围修改失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "判稳周期设置"){
        if(mws->sentenced_stabilizing_t_set(wsp.sentenced_stabilizing_t))
            emit msg_log(MSG_INFO, QString("判稳周期修改成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","判稳周期修改失败!");
            emit msg_log(MSG_INFO, QString("判稳周期修改失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "通讯机码设置"){
        if(mws->machine_code_set(wsp.machine_code))
            emit msg_log(MSG_INFO, QString("通讯机码修改成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","通讯机码修改失败!");
            emit msg_log(MSG_INFO, QString("通讯机码修改失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "发送包间隔设置"){
        if(mws->send_interval_set(wsp.send_interval))
            emit msg_log(MSG_INFO, QString("发送包间隔修改成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","发送包间隔修改失败!");
            emit msg_log(MSG_INFO, QString("发送包间隔修改失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "零位跟踪范围设置"){
        if(mws->zero_tracking_range_set(wsp.zero_tracking_range))
            emit msg_log(MSG_INFO, QString("零位跟踪范围修改成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","零位跟踪范围修改失败!");
            emit msg_log(MSG_INFO, QString("零位跟踪范围修改失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "清零延时设置"){
        if(mws->auto_zero_delay_set(wsp.auto_zero_delay))
            emit msg_log(MSG_INFO, QString("清零延时修改成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","清零延时修改失败!");
            emit msg_log(MSG_INFO, QString("清零延时修改失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "采样速率设置"){
        if(mws->sample_rate_set(wsp.sample_rate))
            emit msg_log(MSG_INFO, QString("采样速率修成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","采样速率修改失败!");
            emit msg_log(MSG_INFO, QString("采样速率修改失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "通讯协议设置"){
        if(mws->communication_protocol_set(wsp.communication_protocol))
            emit msg_log(MSG_INFO, QString("通讯协议修改成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","通讯协议修改失败!");
            emit msg_log(MSG_INFO, QString("通讯协议修改失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "数据格式设置"){
        if(mws->data_format_set(wsp.data_format))
            emit msg_log(MSG_INFO, QString("数据格式修成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","数据格式修改失败!");
            emit msg_log(MSG_INFO, QString("数据格式修改失败!\n"), false);
        }
    }
    else if(ui->choose_modbus_command->currentText() == "波特率设置"){
        if(mws->baud_rate_set(wsp.baud_rate))
            emit msg_log(MSG_INFO, QString("波特率修改成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","波特率修改失败!");
            emit msg_log(MSG_INFO, QString("波特率修改失败!\n"), false);
        }
    }
}

//显示当前压力值
void EquipmentDebug::slots_rece_result(double result)
{
    ui->current_value->setText(QString::number(result,10,5));
}

//打开压力传感器串口
void EquipmentDebug::pb_open_serialport_command()
{
    QString cur_str = ui->open_ws_serialport->text();
    if(cur_str == "打开串口")
    {
        if(mws->open_serialport()==true)
        {
            ui->open_ws_serialport->setText("关闭串口");
        }
        else
            QMessageBox::information(NULL, "提示","串口打开失败!");
    }
    else if(cur_str == "关闭串口")
    {
        mws->close_serialport();
        ui->open_ws_serialport->setText("打开串口");
    }
}

//压力传感器定时器开关
void EquipmentDebug::pb_open_timer_switch()
{
    QString cur_str = ui->timer_switch->text();
    if(cur_str == "打开定时器")
    {
        if(ui->open_ws_serialport->text() == "关闭串口")
        {
            m_pTimer_mws->start(1000);
            ui->timer_switch->setText("关闭定时器");
        }
    }
    else if(cur_str == "关闭定时器")
    {
        m_pTimer_mws->stop();
        ui->timer_switch->setText("打开定时器");
    }
}

//打开激光测距传感器串口
void EquipmentDebug::pb_open_lr_serialport_command()
{
    QString cur_str = ui->open_lr_serialport->text();
    if(cur_str == "打开串口")
    {
        if(mlr->open_serialport()==true)
        {
            ui->open_lr_serialport->setText("关闭串口");
        }
        else
            QMessageBox::information(NULL, "提示","串口打开失败!");
    }
    else if(cur_str == "关闭串口")
    {
        mlr->close_serialport();
        ui->open_lr_serialport->setText("打开串口");
    }
}

//激光测距传感器定时器开关
void EquipmentDebug::pb_open_lr_timer_switch()
{
    QString cur_str = ui->timer_switch_lr->text();
    if(cur_str == "打开定时器")
    {
        if(ui->open_lr_serialport->text() == "关闭串口")
        {
            m_pTimer_mlr->start(1000);
            ui->timer_switch_lr->setText("关闭定时器");
        }
    }
    else if(cur_str == "关闭定时器")
    {
        m_pTimer_mlr->stop();
        ui->timer_switch_lr->setText("打开定时器");
    }
}

//发送激光测距传感器指令
void EquipmentDebug::pb_lr_command()
{
    lrp.filter_samples = ui->filter_samples->currentText();
    lrp.gain_pga = ui->gain_pga->currentText();
    lrp.collector_ID = ui->collector_ID->value();
    lrp.choose_singel_channel = ui->choose_singel_channel->currentText();
    mlr->update_lr_prarm(lrp.filter_samples,lrp.gain_pga,lrp.collector_ID,lrp.choose_singel_channel);
    if(ui->choose_lr_command->currentText() == "双通道查询"){
        mlr->dual_channel_querl();
    }
    else if(ui->choose_lr_command->currentText() == "双通道触发模式"){
        mlr->dual_channel_trigger_mode();
    }
    else if(ui->choose_lr_command->currentText() == "单通道查询"){
        mlr->channel_querl();
    }
    else if(ui->choose_lr_command->currentText() == "单通道触发模式"){
        mlr->channel_trigger_mode();
    }
    else if(ui->choose_lr_command->currentText() == "写采集器ID"){
        if(mlr->write_collector_ID(lrp.collector_ID))
            emit msg_log(MSG_INFO, QString("ID改写成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","ID改写失败!");
            emit msg_log(MSG_INFO, QString("ID改写失败!\n"), false);
        }
    }
    else if(ui->choose_lr_command->currentText() == "读采集器ID"){
        mlr->read_collector_ID();
    }
    else if(ui->choose_lr_command->currentText() == "写采集器波特率"){
        QString baudrate = ui->collector_baudrate->currentText();
        if(mlr->write_collector_baudrate(baudrate))
            emit msg_log(MSG_INFO, QString("激光传感器波特率改写成功!\n"), true);
        else
        {
            QMessageBox::information(NULL, "提示","激光传感器波特率改写失败!");
            emit msg_log(MSG_INFO, QString("激光传感器波特率改写失败!\n"), false);
        }
    }
}

//显示当前距离
void EquipmentDebug::slots_rece_lr_result(double out_1,double out_2)
{
   /* if(out_1<2)
        out_1=5;
    if(out_2<2)
        out_2=5;*/
    ui->channel_one->setText(QString::number(out_1,10,5));
    ui->channel_two->setText(QString::number(out_2,10,5));
    ui->channel_one_distance->setText(QString::number((7.5-out_1)*80,10,5));
    ui->channel_two_distance->setText(QString::number((7.5-out_2)*80,10,5));
}

void EquipmentDebug::slots_lr_id_result(int id)
{
    ui->collector_ID->setValue(id);
}



// 功率放大器开始标定
void EquipmentDebug::slot_btn_start_feedback_clicked()
{
    if(ui->btn_start_feedback->text() == QString("开始标定"))
    {
        if(shaker->is_shaker_running() || shakerFb->status())
        {
            QMessageBox::warning(NULL,"警告","激振器正在运行中，当前不能进行功放调节，请稍候再试");
            return;
        }
        ui->btn_start_feedback->setText(QString("结束标定"));
        emit sig_to_feedback_start();
    }
    else if(ui->btn_start_feedback->text() == QString("结束标定"))
    {
        emit sig_to_feedback_stop();
        ui->btn_start_feedback->setText(QString("开始标定"));
    }
}

// 重新初始化功率放大器的基准
void EquipmentDebug::slot_btn_reset_feedback_clicked()
{
    double voltage = ui->box_current_feedback_voltage->value();
    if(voltage<= 0 || voltage >=5)
    {
        QMessageBox::warning(NULL,"警告","当前电压值不可作为功率放大器的基准电压，请再次确认后重新测试");
        return;
    }
    emit sig_to_reset_feedback(voltage);
}



//
void EquipmentDebug::slot_recv_feedback_result(bool result, double voltage)
{
    qDebug()<<"slot_recv_feedback_result:"<<result<<","<<voltage;
    if(result == true)
    {
        QMessageBox::information(NULL,"提示","功率放大器调节完成 ！");
    }
    else
    {
        QMessageBox::information(NULL,"提示","功率放大器调节失败，请将功放旋钮设置到红线附近 ！");
    }
    ui->btn_start_feedback->setText(QString("开始标定"));
}

void EquipmentDebug::slot_recv_current_feedback_voltage(double voltage)
{
    ui->box_current_feedback_voltage->setValue(voltage);
}

/*
// 使能 控制信号
void EquipmentDebug::pb_signal_PCB_en()
{
    int status = m_bsp->io[O_POWER_EN].GetBit();
    m_bsp->io[O_POWER_EN].SetBit(status?0:1);

}
*/
//电磁阀使能
void EquipmentDebug::pb_signal_ELECTRIC_en()
{
    int status_1 = m_bsp->io[O_ELECTRIC_VALVE].GetBit();
    m_bsp->io[O_ELECTRIC_VALVE].SetBit(status_1?0:1);
}

// 读取 激振器检测位置
void EquipmentDebug::pb_read_pos_shaker_test()
{
    ep.module_pos = m_bsp->axis[0].APS_GetFbPos();
    ui->cmd_pos_2->setValue(ep.module_pos);
}
// 运动到 激振器检测位置
void EquipmentDebug::pb_move_pos_shaker_test()
{
    int button= QMessageBox::question(NULL, "提示", "确定要 激振器检测位置 吗?\n移动前,请注意安全!", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if(button == QMessageBox::Yes)
    {
        emit msg_log(MSG_INFO, QString("确定要 激振器检测位置 吗?用户选择:YES"), false);
        double move_ratio = ui->move_ratio->value();
        if(m_bsp->axis[0].APS_GetSVON())
        {
            m_bsp->axis[0].APS_AMove(ep.module_pos, move_ratio);

            try
            {
                m_bsp->axis[0].APS_WaitMoveDone();
            }
            catch(const std::runtime_error& e)
            {
                emit msg_log(MSG_ALM, QString("%1").arg(e.what()), true);
            }

            emit msg_log(MSG_INFO, "轴 运动到激振器检测位置 完成!", true);
        }
        else
            emit msg_log(MSG_INFO, "请先励磁轴Z!", true);
    }
}

//4种模式，3个轴的测试点位
void EquipmentDebug::pb_read_pos_bed()
{
    ep.pos_bed = m_bsp->axis[AXIS_Z_SHAKER].APS_GetFbPos();
    ui->pos_bed->setValue(ep.pos_bed);
}

void EquipmentDebug::pb_move_pos_bed()
{
    int button= QMessageBox::question(NULL, "提示", "确定要 运动到类床激振器Z轴测试点位 吗?\n移动前,请注意安全!", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if(button == QMessageBox::Yes)
        {
            emit msg_log(MSG_INFO, QString("确定要 激振器Z轴一步运动到测试位置 吗?用户选择:YES"), false);
            double move_ratio = ui->move_ratio->value();
            if(m_bsp->axis[AXIS_Z_SHAKER].APS_GetSVON())
            {
                m_bsp->axis[AXIS_Z_SHAKER].APS_AMove(ep.pos_bed - ep.move_distance, move_ratio);
                try
                {
                    m_bsp->axis[AXIS_Z_SHAKER].APS_WaitMoveDone();
                }
                catch(const std::runtime_error& e)
                {
                    emit msg_log(MSG_ALM, QString("激振器Z轴:%1").arg(e.what()), true);
                    return;
                }

                m_bsp->axis[AXIS_Z_SHAKER].APS_AMove(ep.move_distance, ep.ratio_slowly);
                try
                {
                    m_bsp->axis[AXIS_Z_SHAKER].APS_WaitMoveDone();
                }
                catch(const std::runtime_error& e)
                {
                    emit msg_log(MSG_ALM, QString("激振器Z轴:%1").arg(e.what()), true);
                    return;
                }

                emit msg_log(MSG_INFO, "激振器Z轴一步运动到测试位置 完成!", true);
            }
            else
                emit msg_log(MSG_INFO, "请先励磁激振器Z轴!", true);
        }
}

void EquipmentDebug::pb_read_pos_sponge()
{
    ep.pos_sponge = m_bsp->axis[AXIS_Z_SHAKER].APS_GetFbPos();
    ui->pos_sponge->setValue(ep.pos_sponge);
}

void EquipmentDebug::pb_move_pos_sponge()
{
    int button= QMessageBox::question(NULL, "提示", "确定要 运动到海绵激振器Z轴测试点位 吗?\n移动前,请注意安全!", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if(button == QMessageBox::Yes)
        {
            emit msg_log(MSG_INFO, QString("确定要 激振器Z轴一步运动到测试位置 吗?用户选择:YES"), false);
            double move_ratio = ui->move_ratio->value();
            if(m_bsp->axis[AXIS_Z_SHAKER].APS_GetSVON())
            {
                m_bsp->axis[AXIS_Z_SHAKER].APS_AMove(ep.pos_sponge - ep.move_distance, move_ratio);
                try
                {
                    m_bsp->axis[AXIS_Z_SHAKER].APS_WaitMoveDone();
                }
                catch(const std::runtime_error& e)
                {
                    emit msg_log(MSG_ALM, QString("激振器Z轴:%1").arg(e.what()), true);
                    return;
                }

                m_bsp->axis[AXIS_Z_SHAKER].APS_AMove(ep.move_distance, ep.ratio_slowly);
                try
                {
                    m_bsp->axis[AXIS_Z_SHAKER].APS_WaitMoveDone();
                }
                catch(const std::runtime_error& e)
                {
                    emit msg_log(MSG_ALM, QString("激振器Z轴:%1").arg(e.what()), true);
                    return;
                }

                emit msg_log(MSG_INFO, "激振器Z轴一步运动到测试位置 完成!", true);
            }
            else
                emit msg_log(MSG_INFO, "请先励磁激振器Z轴!", true);
        }
}

void EquipmentDebug::pb_read_pos_shell()
{
    ep.pos_shell = m_bsp->axis[AXIS_Z_SHAKER].APS_GetFbPos();
    ui->pos_shell->setValue(ep.pos_shell);
}

void EquipmentDebug::pb_move_pos_shell()
{
    int button= QMessageBox::question(NULL, "提示", "确定要 运动到带壳裸板激振器Z轴测试点位 吗?\n移动前,请注意安全!", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if(button == QMessageBox::Yes)
        {
            emit msg_log(MSG_INFO, QString("确定要 激振器Z轴一步运动到测试位置 吗?用户选择:YES"), false);
            double move_ratio = ui->move_ratio->value();
            if(m_bsp->axis[AXIS_Z_SHAKER].APS_GetSVON())
            {
                m_bsp->axis[AXIS_Z_SHAKER].APS_AMove(ep.pos_shell - ep.move_distance, move_ratio);
                try
                {
                    m_bsp->axis[AXIS_Z_SHAKER].APS_WaitMoveDone();
                }
                catch(const std::runtime_error& e)
                {
                    emit msg_log(MSG_ALM, QString("激振器Z轴:%1").arg(e.what()), true);
                    return;
                }

                m_bsp->axis[AXIS_Z_SHAKER].APS_AMove(ep.move_distance, ep.ratio_slowly);
                try
                {
                    m_bsp->axis[AXIS_Z_SHAKER].APS_WaitMoveDone();
                }
                catch(const std::runtime_error& e)
                {
                    emit msg_log(MSG_ALM, QString("激振器Z轴:%1").arg(e.what()), true);
                    return;
                }

                emit msg_log(MSG_INFO, "激振器Z轴一步运动到测试位置 完成!", true);
            }
            else
                emit msg_log(MSG_INFO, "请先励磁激振器Z轴!", true);
        }
}

void EquipmentDebug::pb_read_pos_bare()
{
    ep.pos_bare = m_bsp->axis[AXIS_Z_SHAKER].APS_GetFbPos();
    ui->pos_bare->setValue(ep.pos_bare);
}

void EquipmentDebug::pb_move_pos_bare()
{
    int button= QMessageBox::question(NULL, "提示", "确定要 运动到裸板激振器Z轴测试点位 吗?\n移动前,请注意安全!", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if(button == QMessageBox::Yes)
        {
            emit msg_log(MSG_INFO, QString("确定要 激振器Z轴一步运动到测试位置 吗?用户选择:YES"), false);
            double move_ratio = ui->move_ratio->value();
            if(m_bsp->axis[AXIS_Z_SHAKER].APS_GetSVON())
            {
                m_bsp->axis[AXIS_Z_SHAKER].APS_AMove(ep.pos_bare - ep.move_distance, move_ratio);
                try
                {
                    m_bsp->axis[AXIS_Z_SHAKER].APS_WaitMoveDone();
                }
                catch(const std::runtime_error& e)
                {
                    emit msg_log(MSG_ALM, QString("激振器Z轴:%1").arg(e.what()), true);
                    return;
                }

                m_bsp->axis[AXIS_Z_SHAKER].APS_AMove(ep.move_distance, ep.ratio_slowly);
                try
                {
                    m_bsp->axis[AXIS_Z_SHAKER].APS_WaitMoveDone();
                }
                catch(const std::runtime_error& e)
                {
                    emit msg_log(MSG_ALM, QString("激振器Z轴:%1").arg(e.what()), true);
                    return;
                }

                emit msg_log(MSG_INFO, "激振器Z轴一步运动到测试位置 完成!", true);
            }
            else
                emit msg_log(MSG_INFO, "请先励磁激振器Z轴!", true);
        }
}

void EquipmentDebug::pb_read_pos_safe()
{
    ep.pos_safe = m_bsp->axis[AXIS_Z_SHAKER].APS_GetFbPos();
    ui->pos_safe->setValue(ep.pos_safe);
}

void EquipmentDebug::pb_move_pos_safe()
{
    int button= QMessageBox::question(NULL, "提示", "确定要 激振器Z轴运动到安全位置 吗?\n移动前,请注意安全!", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if(button == QMessageBox::Yes)
        {
            emit msg_log(MSG_INFO, QString("确定要 激振器Z轴运动到安全位置 吗?用户选择:YES"), false);
            double move_ratio = ui->move_ratio->value();

            if(m_bsp->axis[AXIS_Z_SHAKER].APS_GetSVON())
            {
                m_bsp->axis[AXIS_Z_SHAKER].APS_AMove(ep.pos_safe, move_ratio);
                try
                {
                    m_bsp->axis[AXIS_Z_SHAKER].APS_WaitMoveDone();
                }
                catch(const std::runtime_error& e)
                {
                    emit msg_log(MSG_ALM, QString("激振器Z轴:%1").arg(e.what()), true);
                    return;
                }
                emit msg_log(MSG_INFO, "激振器Z轴运动到安全位置 完成!", true);
            }
            else
                emit msg_log(MSG_INFO, "请先励磁激振器Z轴!", true);
        }
}

void EquipmentDebug::pb_read_pos_bed_y()
{
    ep.pos_bed_y = m_bsp->axis[AXIS_Y_PLATFORM].APS_GetFbPos();
    ui->pos_bed_y->setValue(ep.pos_bed_y);
}

void EquipmentDebug::pb_move_pos_bed_y()
{
    int button= QMessageBox::question(NULL, "提示", "确定要 运动到载台Y轴类床测试点位 吗?\n移动前,请注意安全!", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if(button == QMessageBox::Yes)
    {
        emit msg_log(MSG_INFO, QString("确定要 载台Y轴一步运动到类床测试位置 吗?用户选择:YES"), false);
        double move_ratio = ui->move_ratio->value();
        if(m_bsp->axis[AXIS_Y_PLATFORM].APS_GetSVON())
        {
            m_bsp->axis[AXIS_Y_PLATFORM].APS_AMove(ep.pos_bed_y - ep.move_distance, move_ratio);
            try
            {
                m_bsp->axis[AXIS_Y_PLATFORM].APS_WaitMoveDone();
            }
            catch(const std::runtime_error& e)
            {
                emit msg_log(MSG_ALM, QString("载台Y轴:%1").arg(e.what()), true);
                return;
            }

            m_bsp->axis[AXIS_Y_PLATFORM].APS_AMove(ep.move_distance, ep.ratio_slowly);
            try
            {
                m_bsp->axis[AXIS_Y_PLATFORM].APS_WaitMoveDone();
            }
            catch(const std::runtime_error& e)
            {
                emit msg_log(MSG_ALM, QString("载台Y轴:%1").arg(e.what()), true);
                return;
            }

            emit msg_log(MSG_INFO, "载台Y轴一步运动到测试位置 完成!", true);
        }
        else
            emit msg_log(MSG_INFO, "请先励磁载台Y轴!", true);
    }
}

void EquipmentDebug::pb_read_pos_y()
{
    ep.pos_y = m_bsp->axis[AXIS_Y_PLATFORM].APS_GetFbPos();
    ui->pos_y->setValue(ep.pos_y);
}

void EquipmentDebug::pb_move_pos_y()
{
    int button= QMessageBox::question(NULL, "提示", "确定要 运动到载台Y轴测试点位 吗?\n移动前,请注意安全!", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if(button == QMessageBox::Yes)
    {
        emit msg_log(MSG_INFO, QString("确定要 载台Y轴一步运动到类床测试位置 吗?用户选择:YES"), false);
        double move_ratio = ui->move_ratio->value();
        if(m_bsp->axis[AXIS_Y_PLATFORM].APS_GetSVON())
        {
            m_bsp->axis[AXIS_Y_PLATFORM].APS_AMove(ep.pos_y - ep.move_distance, move_ratio);
            try
            {
                m_bsp->axis[AXIS_Y_PLATFORM].APS_WaitMoveDone();
            }
            catch(const std::runtime_error& e)
            {
                emit msg_log(MSG_ALM, QString("载台Y轴:%1").arg(e.what()), true);
                return;
            }

            m_bsp->axis[AXIS_Y_PLATFORM].APS_AMove(ep.move_distance, ep.ratio_slowly);
            try
            {
                m_bsp->axis[AXIS_Y_PLATFORM].APS_WaitMoveDone();
            }
            catch(const std::runtime_error& e)
            {
                emit msg_log(MSG_ALM, QString("载台Y轴:%1").arg(e.what()), true);
                return;
            }

            emit msg_log(MSG_INFO, "载台Y轴一步运动到测试位置 完成!", true);
        }
        else
            emit msg_log(MSG_INFO, "请先励磁载台Y轴!", true);
    }
}

void EquipmentDebug::pb_read_pos_z()
{
    ep.pos_z = m_bsp->axis[AXIS_Z_PLATFORM].APS_GetFbPos();
    ui->pos_z->setValue(ep.pos_z);
}

void EquipmentDebug::pb_move_pos_z()
{
    int button= QMessageBox::question(NULL, "提示", "确定要 运动到载台Z轴测试点位 吗?\n移动前,请注意安全!", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if(button == QMessageBox::Yes)
    {
        emit msg_log(MSG_INFO, QString("确定要 载台Z轴一步运动到类床测试位置 吗?用户选择:YES"), false);
        double move_ratio = ui->move_ratio->value();
        if(m_bsp->axis[AXIS_Y_PLATFORM].APS_GetFbPos() == ep.pos_y || m_bsp->axis[AXIS_Y_PLATFORM].APS_GetFbPos() == ep.pos_bed_y)
        {
            if(m_bsp->axis[AXIS_Z_PLATFORM].APS_GetSVON())
            {
                m_bsp->axis[AXIS_Z_PLATFORM].APS_AMove(ep.pos_z - ep.move_distance, move_ratio);
                try
                {
                    m_bsp->axis[AXIS_Z_PLATFORM].APS_WaitMoveDone();
                }
                catch(const std::runtime_error& e)
                {
                    emit msg_log(MSG_ALM, QString("载台Z轴:%1").arg(e.what()), true);
                    return;
                }

                m_bsp->axis[AXIS_Z_PLATFORM].APS_AMove(ep.move_distance, ep.ratio_slowly);
                try
                {
                    m_bsp->axis[AXIS_Z_PLATFORM].APS_WaitMoveDone();
                }
                catch(const std::runtime_error& e)
                {
                    emit msg_log(MSG_ALM, QString("载台Z轴:%1").arg(e.what()), true);
                    return;
                }

                emit msg_log(MSG_INFO, "载台Z轴一步运动到测试位置 完成!", true);
            }
            else
                emit msg_log(MSG_INFO, "请先励磁载台Z轴!", true);
        }
        else
            emit msg_log(MSG_INFO, "请先移动载台Y轴!", true);
    }
}


// 定时器更新界面io 电机位置信息
void EquipmentDebug::refresh_panel_AMP_Info(void)
{
    int index = ui->axis_index->currentIndex();

// 更新轴信息
    // 读取反馈位置
    double fbpos = m_bsp->axis[index].APS_GetFbPos();

    // 读取励磁状态
    int servo_status = m_bsp->axis[index].APS_GetSVON();

    // 更新励磁状态
    if(servo_status)
    {
        ui->servo->setText("Servo on");
        ui->servo->setStyleSheet("QPushButton{border-radius:10px;background-color:rgb(92, 163, 99);font:75 12pt \"黑体\";color: white;}");
    }
    else
    {
        ui->servo->setText("Servo off");
        ui->servo->setStyleSheet("QPushButton{border-radius:10px;background-color:rgb(250, 50, 50);font:75 12pt \"黑体\";color: white;}");
    }

    // 更新反馈位置
    ui->fb_pos->setValue(fbpos);

    // 更新 界面IO输入信号
    //-- add your code ---------------------------------------------------------

    //更新限高传感器检测io
    if(m_bsp->io[I_LIMIT_HEIGHT].GetBit())
        ui->limit_height_lamp->setPixmap(lamp_on);
    else
        ui->limit_height_lamp->setPixmap(lamp_off);

    //更新产品区分传感器检测io
    if(m_bsp->io[I_DIFFER].GetBit())
        ui->differ_lamp->setPixmap(lamp_on);
    else
        ui->differ_lamp->setPixmap(lamp_off);

    //更新左气缸上传感器状态io
    if(m_bsp->io[I_CYLINDER_L_UP].GetBit())
        ui->cylinder_lamp_L_up->setPixmap(lamp_on);
    else
        ui->cylinder_lamp_L_up->setPixmap(lamp_off);

    //更新左气缸下传感器状态io
    if(m_bsp->io[I_CYLINDER_L_DOWN].GetBit())
        ui->cylinder_lamp_L_down->setPixmap(lamp_on);
    else
        ui->cylinder_lamp_L_down->setPixmap(lamp_off);

    //更新右气缸上传感器状态io
    if(m_bsp->io[I_CYLINDER_R_UP].GetBit())
        ui->cylinder_lamp_R_up->setPixmap(lamp_on);
    else
        ui->cylinder_lamp_R_up->setPixmap(lamp_off);

    //更新右气缸下传感器状态io
    if(m_bsp->io[I_CYLINDER_R_DOWN].GetBit())
        ui->cylinder_lamp_R_down->setPixmap(lamp_on);
    else
        ui->cylinder_lamp_R_down->setPixmap(lamp_off);

    //PCB电源使能
   /* if(m_bsp->io[O_POWER_EN].GetBit())
        ui->power_en->setStyleSheet("QPushButton{border-radius:5px;background-color:rgb(92, 163, 99);font:75 9pt \"黑体\";color: white;}");
    else
        ui->power_en->setStyleSheet("QPushButton{border-radius:5px;background-color:rgb(250, 50, 50);font:75 9pt \"黑体\";color: white;}");
*/
    //气缸电磁阀使能
    if(m_bsp->io[O_ELECTRIC_VALVE].GetBit())
        ui->ELECTRIC_VALVE_en->setStyleSheet("QPushButton{border-radius:5px;background-color:rgb(92, 163, 99);font:75 9pt \"黑体\";color: white;}");
    else
        ui->ELECTRIC_VALVE_en->setStyleSheet("QPushButton{border-radius:5px;background-color:rgb(250, 50, 50);font:75 9pt \"黑体\";color: white;}");
}

/*
 *==============================================================================
 *   读取/保存参数
 *==============================================================================
 */

// 加载界面参数
void EquipmentDebug::load_param()
{
    try
    {
        m_sql_bin.read_db(param_db_path, "PointPosition", &ep, sizeof(Equipment_Param));
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("读取设备点位信息错误\n%1").arg(e.what()), true);
        return;
    }
    ui->pos_bed->setValue(ep.pos_bed);
    ui->pos_sponge->setValue(ep.pos_sponge);
    ui->pos_shell->setValue(ep.pos_shell);
    ui->pos_bare->setValue(ep.pos_bare);
    ui->pos_safe->setValue(ep.pos_safe);
    ui->pos_bed_y->setValue(ep.pos_bed_y);
    ui->pos_y->setValue(ep.pos_y);
    ui->ratio_slowly->setValue(ep.ratio_slowly * 100);
    ui->move_distance->setValue(ep.move_distance);
    ui->pos_z->setValue(ep.pos_z);
    ui->type_en->setChecked(ep.type_en?Qt::Checked : Qt::Unchecked);
    //-- add your code ---------------------------------------------------------
    emit sig_update_ep_param(ep);
}

// 保存界面参数
void EquipmentDebug::save_param()
{
    //-- add your code ---------------------------------------------------------

    ep.pos_bed = ui->pos_bed->value();
    ep.pos_sponge = ui->pos_sponge->value();
    ep.pos_shell = ui->pos_shell->value();
    ep.pos_bare = ui->pos_bare->value();
    ep.pos_safe = ui->pos_safe->value();
    ep.pos_bed_y = ui->pos_bed_y->value();
    ep.pos_y = ui->pos_y->value();
    ep.ratio_slowly = ui->ratio_slowly->value()/100;
    ep.move_distance = ui->move_distance->value();
    ep.pos_z = ui->pos_z->value();
    ep.type_en = ui->type_en->checkState() == Qt::Checked ? 1:0;

    wsp.farmar_weight = ui->nominal_weight->value();
    wsp.sensor_sensitivity = ui->sensitivity->value();
    wsp.sensor_range = ui->sensor_range->value();
    wsp.decimal_point = ui->decimal_point->value();
    wsp.zero_trigger_threshold = ui->zero_trigger_threshold->value();
    wsp.auto_zero = ui->auto_zero_set->value();
    wsp.range_coefficient = ui->range_coefficient->value();
    wsp.zero_reset_range = ui->zero_reset_range->value();
    wsp.digital_filter = ui->digital_filter->value();
    wsp.rated_range = ui->rated_range->value();
    wsp.auto_calibration = ui->auto_calibration->value();
    wsp.sentenced_stabilizing_range = ui->sentenced_stabilizing_range->value();
    wsp.sentenced_stabilizing_t = ui->sentenced_stabilizing_t->value();
    wsp.machine_code = ui->machine_code->value();
    wsp.send_interval = ui->send_interval->value();
    wsp.zero_tracking_range = ui->zero_tracking_range->value();
    wsp.auto_zero_delay = ui->auto_zero_delay->value();
    wsp.sample_rate = ui->sample_rate->value();
    wsp.communication_protocol = ui->communication_protocol->value();
    wsp.data_format = ui->data_format->value();
    wsp.baud_rate = ui->baud_rate->value();

    lrp.filter_samples = ui->filter_samples->currentText();
    lrp.gain_pga = ui->gain_pga->currentText();
    lrp.collector_ID = ui->collector_ID->value();
    lrp.choose_singel_channel = ui->choose_singel_channel->currentText();

    // bin数据形式保存到数据库
    QByteArray param((char*)&ep, sizeof(Equipment_Param));
    try
    {
        m_sql_bin.save_db(param_db_path, "PointPosition", param);
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("保存设备点位信息错误\n%1").arg(e.what()), true);
        return;
    }

    QByteArray param_ws((char*)&wsp, sizeof(WeighingSensor_Param));
    try
    {
        m_sql_bin.save_db(param_db_path, "PointPosition_ws", param_ws);
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("保存称重传感器点位信息错误\n%1").arg(e.what()), true);
        return;
    }

    QByteArray param_lr((char*)&lrp, sizeof(LaserRanging_Param));
    try
    {
        m_sql_bin.save_db(param_db_path, "PointPosition_lr", param_lr);
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("保存激光测距传感器点位信息错误\n%1").arg(e.what()), true);
        return;
    }

    // 发送信号函数,刷新参数
    emit sig_update_ep_param(ep);
    emit msg_log(MSG_INFO, "保存设备点位信息成功!", true);
}


/*
 *==============================================================================
 *   模块固有功能函数
 *==============================================================================
 */
//-------------------------------------------------------------------------槽函数
void EquipmentDebug::slot_update_db_path(QString path)
{
    param_db_path = path;
    load_param();
}

// 参数保存
void EquipmentDebug::pb_param_save(void)
{
    save_param();
}

// 轴调试 励磁
void EquipmentDebug::pb_set_servo()
{
    int index = ui->axis_index->currentIndex();
    int status = m_bsp->axis[index].APS_GetSVON();
    m_bsp->axis[index].APS_SetSVON(!status);

    // 更新励磁状态
    if(status)
    {
        ui->servo->setText("Servo on");
        ui->servo->setStyleSheet("QPushButton{border-radius:5px;background-color:rgb(92, 163, 99);font:75 9pt \"黑体\";color: white;}");
    }
    else
    {
        ui->servo->setText("Servo off");
        ui->servo->setStyleSheet("QPushButton{border-radius:5px;background-color:rgb(250, 50, 50);font:75 9pt \"黑体\";color: white;}");
    }
}

// 轴调试 相对正向运动
void EquipmentDebug::pb_move_r_forward_dir(void)
{
    double cmd_pos = ui->cmd_pos->value();
    double move_ratio = ui->move_ratio->value();

    int index = ui->axis_index->currentIndex();
    m_bsp->axis[index].APS_RMove( cmd_pos, move_ratio);
}

// 轴调试 相对负向运动
void EquipmentDebug::pb_move_r_negative_dir(void)
{
    double cmd_pos = ui->cmd_pos->value();
    double move_ratio = ui->move_ratio->value();

    int index = ui->axis_index->currentIndex();
    m_bsp->axis[index].APS_RMove(-cmd_pos, move_ratio);
}

// 轴调试 停止运动
void EquipmentDebug::pb_move_stop(void)
{
    int index = ui->axis_index->currentIndex();
    m_bsp->axis[index].APS_dStop();
}

// 轴调试 回零
void EquipmentDebug::pb_move_home(void)
{
    int index = ui->axis_index->currentIndex();
    m_bsp->axis[index].APS_Home();
}

