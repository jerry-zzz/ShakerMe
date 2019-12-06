/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    loginfo.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	日志面板.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#include "loginfo.h"
#include "ui_loginfo.h"
#include "system_info.h"
#include <qDebug>
#include <QMessageBox>
#include <stdexcept>




LogInfo::LogInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LogInfo)
{
    ui->setupUi(this);

    //设为无边框
    setWindowFlags(Qt::FramelessWindowHint);

    this->setGeometry(NESTED_PANEL_POS_X, NESTED_PANEL_POS_Y, NESTED_PANEL_POS_W, NESTED_PANEL_POS_H);

    // 设置tableWidget的列宽
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    //禁止单元格编辑
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //数据库访问.
    connect(m_sql_thread.sql_t_r->sql_log, &Sql_Log::sig_bind_tablewidget, this, &LogInfo::slot_update_tablewidget);

    // 更新log
    connect(ui->calendarWidget, &QCalendarWidget::clicked, this, &LogInfo::slot_calendar_clicked);

    // radio
    connect(ui->operate_log, &QRadioButton::clicked, this, &LogInfo::slot_calendar_clicked);
    connect(ui->alarm_log, &QRadioButton::clicked, this, &LogInfo::slot_calendar_clicked);
    connect(ui->all_user, &QRadioButton::clicked, this, &LogInfo::slot_calendar_clicked);
    connect(ui->current_user, &QRadioButton::clicked, this, &LogInfo::slot_calendar_clicked);

    connect(ui->en, &QCheckBox::stateChanged, this, &LogInfo::pb_single_persional_en);
    connect(ui->query, &QPushButton::clicked, this, &LogInfo::pb_query_log);

    pb_single_persional_en(Qt::Unchecked);

    //-- add your code ---------------------------------------------------------

}

LogInfo::~LogInfo()
{
    delete ui;
}


/*
 *==============================================================================
 *   设置权限/界面显示/界面隐藏
 *==============================================================================
 */

// 设置面板权限
void LogInfo::set_current_level(int level)
{
    current_level = level;
}

// 显示界面
void LogInfo::display_panel()
{
    ui->en->setChecked(false);

    if(current_level == OP_LEVEL)
    {
        ui->groupBox_2->setVisible(false);
        ui->groupBox_3->setVisible(false);
    }
    else
    {
        ui->groupBox_2->setVisible(true);
        ui->groupBox_3->setVisible(true);
    }

    // 获取用户列表
    ui->user_list->clear();    
    QStringList user_list = sql_user.get_user_list();
    ui->user_list->insertItems(0, user_list);


    //设置日期为当天
    QDate date=QDate::currentDate();
    ui->calendarWidget->setSelectedDate(date);

    QDate date_s = date.addMonths(-1);
    QDate date_e = date;
    ui->dateStart->setDate(date_s);
    ui->dateEnd->setDate(date_e);

    //加载日志
    slot_calendar_clicked();

    //显示
    show();
}

// 隐藏界面
void LogInfo::hide_panel()
{
    hide();
}

/*
 *==============================================================================
 *   模块固有功能函数
 *==============================================================================
 */

// 刷新界面表格的信息
void LogInfo::slot_update_tablewidget(QString datetime, QString type, QString user, QString msg)
{
    if(ui->en->checkState() != Qt::Checked)
    {
        if(user_type != "All")
            if(user != user_id)
                return;
    }
    else
    {
        if(user_type != user)
            return;
    }

    int row = 0;
    ui->tableWidget->insertRow(row);
    ui->tableWidget->setItem(row,0,new QTableWidgetItem(datetime));
    ui->tableWidget->setItem(row,1,new QTableWidgetItem(type));
    ui->tableWidget->setItem(row,2,new QTableWidgetItem(user));
    ui->tableWidget->setItem(row,3,new QTableWidgetItem(msg));
    //居中
    ui->tableWidget->item(row,0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    ui->tableWidget->item(row,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    ui->tableWidget->item(row,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
//    ui->tableWidget->item(row,3)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
}

// 日历控件点击
void LogInfo::slot_calendar_clicked()
{
    // 清空table
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->clearContents();

    QDate date = ui->calendarWidget->selectedDate();
    QString date_s = date.toString("yyyy-MM-dd") + " 00:00:00";
    QString date_e = date.addDays(1).toString("yyyy-MM-dd") + " 00:00:00";

    // 日志选择
    if(ui->operate_log->isChecked() == true)
        log_type = "Info";
    else
        log_type = "Alarm";

    if(ui->en->checkState() != Qt::Checked)
    {
        // 用户选择
        if(ui->all_user->isChecked() == true)
            user_type = "All";
        else
            user_type = user_id;
    }
    else
    {
        // 指定用户
        user_type = ui->user_list->currentText();
    }


    select_log_from_date(date_s, date_e, log_type);
}

// 区间日志查询
void LogInfo::pb_query_log()
{
    // 清空table
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->clearContents();

    QString date_s = ui->dateStart->date().toString("yyyy-MM-dd") + " 00:00:00";
    QString date_e = ui->dateEnd->date().addDays(1).toString("yyyy-MM-dd") + " 00:00:00";

    // 日志选择
    if(ui->operate_log->isChecked() == true)
        log_type = "Info";
    else
        log_type = "Alarm";

    if(ui->en->checkState() != Qt::Checked)
    {
        // 用户选择
        if(ui->all_user->isChecked() == true)
            user_type = "All";
        else
            user_type = user_id;
    }
    else
    {
        // 指定用户
        user_type = ui->user_list->currentText();
    }

    select_log_from_date(date_s, date_e, log_type);
}

// 根据时间段选择日志
void LogInfo::select_log_from_date(QString date_s, QString date_e, QString log_type)
{
    try
    {
        m_sql_thread.load_log_from_select_time(log_type, date_s, date_e);
    }
    catch(const std::runtime_error& e)
    {
        QMessageBox::about(NULL, "通知", QString("加载log信息错误:%1").arg(e.what()));
    }
}

// 设置登录用户名称
void LogInfo::set_user_id(QString id)
{
    user_id = id;
}

// 指定人查询使能
void LogInfo::pb_single_persional_en(int status)
{
    if(status == Qt::Checked)
    {
        ui->all_user->setEnabled(false);
        ui->current_user->setEnabled(false);
        ui->user_list->setEnabled(true);
        connect(ui->user_list, &QComboBox::currentTextChanged, this, &LogInfo::pb_user_list);
        pb_user_list(ui->user_list->currentText());
    }
    else
    {
        ui->all_user->setEnabled(true);
        ui->current_user->setEnabled(true);
        ui->user_list->setEnabled(false);
        disconnect(ui->user_list, &QComboBox::currentTextChanged, this, &LogInfo::pb_user_list);
    }
}
// 指定人查询
void LogInfo::pb_user_list(const QString &user)
{
    // 清空table
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->clearContents();

    QDate date = ui->calendarWidget->selectedDate();
    QString date_s = date.toString("yyyy-MM-dd") + " 00:00:00";
    QString date_e = date.addDays(1).toString("yyyy-MM-dd") + " 00:00:00";

    // 日志选择
    if(ui->operate_log->isChecked() == true)
        log_type = "Info";
    else
        log_type = "Alarm";

    // 指定用户
    user_type = user;

    select_log_from_date(date_s, date_e, log_type);
}

// 日志记录
void LogInfo::record_log(int type, QString msg, bool msgbox)
{
    if(type == MSG_INFO)
    {
        m_sql_thread.record_Info_msg(user_id, msg);
        if(msgbox)
            QMessageBox::about(0, "通知", msg);
    }
    else if(type == MSG_ALM)
    {
        m_sql_thread.record_Alarm_msg(user_id, msg);
        if(msgbox)
            QMessageBox::about(0, "报警", msg);
    }
}
