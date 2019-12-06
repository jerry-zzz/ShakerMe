/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    mainpanel.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	设备主界面信息面板
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#include "mainpanel.h"
#include "ui_mainpanel.h"
#include "system_info.h"


#include <QDebug>


MainPanel::MainPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainPanel)
{
    ui->setupUi(this);

    //设为无边框
    setWindowFlags(Qt::FramelessWindowHint);
    this->setGeometry(NESTED_PANEL_POS_X, NESTED_PANEL_POS_Y, NESTED_PANEL_POS_W, NESTED_PANEL_POS_H);
    //-- add your code ---------------------------------------------------------   
    connect(ui->mode_comboBox,&QComboBox::currentTextChanged,this,&MainPanel::slot_cur_model_change);
}

MainPanel::~MainPanel()
{
    delete ui;
}

/*
 *==============================================================================
 *   初始化/界面显示/界面隐藏
 *==============================================================================
 */

void MainPanel::init_panel(JobInfo *info)
{
    m_jobinfo = info;
    //-- add your code ---------------------------------------------------------
    update_history_and_current_statistics(-1, "", "");
}

void MainPanel::display_panel()
{    
    show();     //显示
}

void MainPanel::hide_panel()
{
    hide();
}

/*
 *==============================================================================
 *   界面模块功能函数
 *==============================================================================
 */

void MainPanel::slot_sensor_generation(int sensor_generation)
{
    generation = sensor_generation;
//    ui->product_count->setText("0");
}

/*- 槽函数 --------------------------------------------------------------------*/
void MainPanel::slot_update_product_pattern(int pattern)
{
    this->current_product_pattern = pattern;
    this->ui->mode_comboBox->setCurrentIndex(pattern);
    emit sig_mode_num(pattern);
}

void MainPanel::slot_cur_model_change()
{
    this->ui->mode_comboBox->setCurrentIndex(this->current_product_pattern);
}
/*- 功能函数 -------------------------------------------------------------------*/
void MainPanel::choose_mode()
{
//   int mode_num = 0,generation_num = 0;
   int mode_num = 0;
   QString mode = ui->mode_comboBox->currentText();
   if(mode=="类床模式")
   {
       mode_num=1;      
   }
   else if(mode=="海绵模式"){
        mode_num=2;
   }
   else if(mode=="带壳模式"){
        mode_num=3;
   }
   else if(mode=="裸板模式"){
        mode_num=4;
   }
   emit sig_mode_num(mode_num);   

}

void MainPanel::update_gongwei_status_runtime(QString info)
{
    ui->status->setText(info);
}


void MainPanel::slot_reflash_platform_status( QString status)
{
    if(status == "检测中")
    {
        ui->lamp_check->setStyleSheet("image: url(:/icon/res/lamp_status_green.png)");
    }
    else
    {
        ui->lamp_check->setStyleSheet("image: url(:/icon/res/lamp_status_yellow.png)");
    }
    ui->status_check->setText(status);
}
void MainPanel::slot_reflash_axis_shaker_z_status(QString status, int ismove)
{
    if(status == "报警")
    {
        ui->status_shaker_z->setText(status);
        ui->lamp_shaker_z->setStyleSheet("image: url(:/icon/res/lamp_status_red.png)");
    }
    else
    {
        ui->status_shaker_z->setText(status);
        if(ismove)
            ui->lamp_shaker_z->setStyleSheet("image: url(:/icon/res/lamp_status_green.png)");
        else
            ui->lamp_shaker_z->setStyleSheet("image: url(:/icon/res/lamp_status_yellow.png)");
    }
}
void MainPanel::slot_reflash_axis_platform_y_status(QString status, int ismove)
{
    if(status == "报警")
    {
        ui->status_platform_y->setText(status);
        ui->lamp_platform_y->setStyleSheet("image: url(:/icon/res/lamp_status_red.png)");
    }
    else
    {
        ui->status_platform_y->setText(status);
        if(ismove)
            ui->lamp_platform_y->setStyleSheet("image: url(:/icon/res/lamp_status_green.png)");
        else
            ui->lamp_platform_y->setStyleSheet("image: url(:/icon/res/lamp_status_yellow.png)");
    }
}
void MainPanel::slot_reflash_axis_platform_z_status(QString status, int ismove)
{
    if(status == "报警")
    {
        ui->status_platform_z->setText(status);
        ui->lamp_platform_z->setStyleSheet("image: url(:/icon/res/lamp_status_red.png)");
    }
    else
    {
        ui->status_platform_z->setText(status);
        if(ismove)
            ui->lamp_platform_z->setStyleSheet("image: url(:/icon/res/lamp_status_green.png)");
        else
            ui->lamp_platform_z->setStyleSheet("image: url(:/icon/res/lamp_status_yellow.png)");
    }
}
/*
 *==============================================================================
 *   模块固有功能函数
 *==============================================================================
 */

void MainPanel::update_history_and_current_statistics(int result, QString eInfo, QString pcb_info)
{
    int ok_num = 0;
    int false_num = 0;

    ui->eInfo->setText(eInfo);
    ui->pcb_info->setText(pcb_info);

    switch(result)
    {
    case -1:
        ui->result->setText("");
        ui->result->setStyleSheet("background-color: rgb(222, 222, 222);");
        break;
    case 1:
        ok_num++;
        ui->result->setText("合格");
        ui->result->setStyleSheet("background-color: rgb(0, 255, 0);");
        break;
    case 0:
        false_num++;
        ui->result->setText("不合格");
        ui->result->setStyleSheet("background-color: rgb(255, 0, 0);");
        break;
    default:
        ui->result->setText("异常\n错误");
        ui->result->setStyleSheet("background-color: rgb(255, 0, 0);");
        break;
    }
    m_jobinfo->record_current_user_work_num(ok_num, false_num);

    HistoryInfo hi;
    // 当前合格数量
    hi.qualified_num += ok_num;
    // 当前不合格数量
    hi.unqualified_num += false_num;
    // 当前总计
    m_c_all = hi.qualified_num + hi.unqualified_num;
    qDebug()<<hi.qualified_num<<hi.unqualified_num;
    ui->product_count->setText(QString::number(m_c_all, 10));
}

/*
 *==============================================================================
 *   读取/保存参数
 *==============================================================================
 */

// 读取历史数据
HistoryInfo MainPanel::read_history()
{
    QString path_param = QString("%1/sys_config.db").arg(ROOT_PATH);
    HistoryInfo hi;
    hi.qualified_num = 0;
    hi.unqualified_num = 0;

    try
    {
        m_sql_bin.read_db(path_param, "HistoryInfo", &hi, sizeof(HistoryInfo));
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("历史检测记录信息读取错误\n%1").arg(e.what()), true);
    }
    return hi;
}

// 保存历史数据
void MainPanel::save_history(HistoryInfo hi)
{
    QString path_param = QString("%1/sys_config.db").arg(ROOT_PATH);

    QByteArray param((char*)&hi, sizeof(HistoryInfo));
    try
    {
        m_sql_bin.save_db(path_param, "HistoryInfo", param);
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("历史记录保存错误!\n历史合格数:%1\n历史不合格数:%2").arg(hi.qualified_num).arg(hi.unqualified_num), false);
        return;
    }
}





