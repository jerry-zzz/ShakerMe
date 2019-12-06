/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    systemsetup.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	系统设置面板.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#include "systemsetup.h"
#include "ui_systemsetup.h"
#include "system_info.h"
#include "../Log/loginfo.h"
#include <QFileDialog>
#include <QDebug>


SystemSetup::SystemSetup(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SystemSetup)
{
    ui->setupUi(this);

    //设为无边框
    setWindowFlags(Qt::FramelessWindowHint);
    this->setGeometry(NESTED_PANEL_POS_X, NESTED_PANEL_POS_Y, NESTED_PANEL_POS_W, NESTED_PANEL_POS_H);

    // 信号槽
    connect(ui->save, &QPushButton::clicked, this, &SystemSetup::pb_save);
    //connect(ui->search_dirset, &QToolButton::clicked, this, &SystemSetup::pb_search_dir_set);
    //connect(ui->record_dirset, &QToolButton::clicked, this, &SystemSetup::pb_record_dir_set);

    //-- add your code ---------------------------------------------------------

    // 信号槽

    // 其他

}

SystemSetup::~SystemSetup()
{
    delete ui;
}



/*
 *==============================================================================
 *   初始化模块/设置权限/界面显示/界面隐藏
 *==============================================================================
 */

// 初始化系统设置模块
void SystemSetup::init_system_setup()
{
    // 初始化系统设置参数文件路径
    path_param = QString("%1/sys_config.db").arg(ROOT_PATH);

    // 加载,并向其他模块更新参数
    load_param();
}

// 显示界面
void SystemSetup::display_panel()
{
    // 加载,并向其他模块更新参数
    load_param();
    show();

    //-- add your code -----------------------------------------------------

}

// 隐藏界面
void SystemSetup::hide_panel()
{
    hide();
}

// 获取系统设置参数
Sys_Setup SystemSetup::get_sys_setup_param()
{
    return sys_setup;
}



/*
 *==============================================================================
 *   界面模块功能函数
 *==============================================================================
 */

/*add your code*/


/*
 *==============================================================================
 *   读取/保存参数
 *==============================================================================
 */


// load参数
void SystemSetup::load_param()
{
    try
    {
        m_sql_bin.read_db(path_param, "SysSetup", &sys_setup, sizeof(Sys_Setup));
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("读取设备点位信息错误\n%1").arg(e.what()), true);
        return;
    }

    /* add your code */

    // 系统设置
    if(sys_setup.unable_init)
        ui->checkBox->setCheckState(Qt::Checked);
    else
        ui->checkBox->setCheckState(Qt::Unchecked);
    if(sys_setup.unable_beep)
        ui->checkBox_2->setCheckState(Qt::Checked);
    else
        ui->checkBox_2->setCheckState(Qt::Unchecked);

    // 轴运动速率设置
    ui->z_ratio_G->setValue(sys_setup.z_ratio_G * 100.0);
    ui->z_ratio_Z->setValue(sys_setup.z_ratio_Z * 100.0);
    ui->z_ratio_Y->setValue(sys_setup.z_ratio_Y * 100.0);

    ui->y_pos_min->setValue(sys_setup.y_pos_min);
    ui->y_pos_max->setValue(sys_setup.y_pos_max);
    ui->sensor_bed_max->setValue(sys_setup.sensor_bed_max);
    ui->shaker_distance_max->setValue(sys_setup.shaker_distance_max);
    ui->shaker_bed_pos->setValue(sys_setup.shaker_bed_pos);
    ui->platform_z_bed_pos->setValue(sys_setup.platform_z_bed_pos);
    ui->plank_thickness->setValue(sys_setup.plank_thickness);
    ui->platform_y_pos->setValue(sys_setup.platform_y_pos);

//    ui->output_search_dir->setText(QString(sys_setup.search_dir));
//    ui->output_record_dir->setText(QString(sys_setup.record_path));
    int generation = sys_setup.sensor_generation;
    ui->sensor_generation->setValue(generation);
    emit sig_sensor_generation(generation);
    // 更新参数
    emit sig_update_sys_param(sys_setup);
}

void SystemSetup::save_param()
{
    /* add your code */

    // 系统设置
    if(ui->checkBox->checkState() == Qt::Checked)
        sys_setup.unable_init = 1;
    else
        sys_setup.unable_init = 0;
    if(ui->checkBox_2->checkState() == Qt::Checked)
        sys_setup.unable_beep = 1;
    else
        sys_setup.unable_beep = 0;

    // 轴运动速率设置
    sys_setup.z_ratio_G = ui->z_ratio_G->value() / 100.0;
    sys_setup.z_ratio_Z = ui->z_ratio_Z->value() / 100.0;
    sys_setup.z_ratio_Y = ui->z_ratio_Y->value() / 100.0;


    sys_setup.y_pos_min = ui->y_pos_min->value();
    sys_setup.y_pos_max = ui->y_pos_max->value();
    sys_setup.sensor_bed_max = ui->sensor_bed_max->value();
    sys_setup.shaker_distance_max = ui->shaker_distance_max->value();
    sys_setup.shaker_bed_pos  = ui->shaker_bed_pos->value();
    sys_setup.platform_z_bed_pos = ui->platform_z_bed_pos->value();
    sys_setup.plank_thickness = ui->plank_thickness->value();
    sys_setup.platform_y_pos = ui->platform_y_pos->value();

    if(ui->sensor_generation->value()<1)
    {
        ui->sensor_generation->setValue(2);
    }
    sys_setup.sensor_generation = ui->sensor_generation->value();

    // 保存到数据库
    QByteArray param((char*)&sys_setup, sizeof(Sys_Setup));
    try
    {
        m_sql_bin.save_db(path_param, "SysSetup", param);
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("保存设备点位信息错误\n%1").arg(e.what()), true);
        return;
    }


    emit sig_sensor_generation(ui->sensor_generation->value());
    // 更新参数
    emit sig_update_sys_param(sys_setup);
    emit msg_log(MSG_INFO, "保存设备点位信息成功!", true);
}

/*
 *==============================================================================
 *   模块固有功能函数
 *==============================================================================
 */

//-------------------------------------------------------------------------槽函数
void SystemSetup::pb_save()
{
    save_param();
}

