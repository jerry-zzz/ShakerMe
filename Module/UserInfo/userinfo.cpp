/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    userinfo.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	用户管理面板.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#include "userinfo.h"
#include "ui_userinfo.h"
#include "system_info.h"
#include <QDebug>


UserInfo::UserInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserInfo)
{
    ui->setupUi(this);
    //设为无边框
    setWindowFlags(Qt::FramelessWindowHint);
    this->setGeometry(NESTED_PANEL_POS_X, NESTED_PANEL_POS_Y, NESTED_PANEL_POS_W, NESTED_PANEL_POS_H);

    // 个人中心面板
    pcenter = new PersonalCenter();

    // init tablewidget
    ui->history_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->history_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->history_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->history_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui->history_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);

    //禁止单元格编辑
    ui->history_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 信号槽
    connect(ui->logout, &QPushButton::clicked, this, &UserInfo::pb_logout);
    connect(ui->p_center, &QPushButton::clicked, this, &UserInfo::pb_persional_center);
    connect(pcenter, &PersonalCenter::msg_log, this, &UserInfo::slot_msg_log);

    //-- add your code ---------------------------------------------------------

}

UserInfo::~UserInfo()
{
    delete pcenter;
    delete ui;
}



/*
 *==============================================================================
 *   初始化产品模块/界面显示/界面隐藏
 *==============================================================================
 */

// 初始化产品模块
void UserInfo::init_userinfo(JobInfo *jobinfo)
{
    m_jobinfo = jobinfo;
}

// 显示界面
void UserInfo::display_panel()
{
    // 隐藏 个人中心面板
    pcenter->hide();

    // 清除table
    ui->history_table->setRowCount(0);
    ui->history_table->clearContents();

    // 加载历史记录
    int read_num = 0;
    JobData jobdata[30];
    m_jobinfo->get_jobinfo_history(jobdata, &read_num);
    for(int i=0;i<read_num;i++)
        slot_update_worker_info(jobdata[i].logintime,
                                jobdata[i].logouttime,
                                QString("%1").arg(jobdata[i].good),
                                QString("%1").arg(jobdata[i].bad),
                                QString("%1").arg(jobdata[i].total)
                                );


    //显示
    show();
}

// 隐藏界面
void UserInfo::hide_panel()
{
    hide();
}



/*
 *==============================================================================
 *   界面模块功能函数
 *==============================================================================
 */

/*add your code*/






/*
 *==============================================================================
 *   模块固有功能函数
 *==============================================================================
 */

// 设置用户信息:id,权限
void UserInfo::set_account_info(QString id, QString permissions)
{
    m_id = id;
    m_permissions = permissions;

    ui->id->setText(id);
    ui->permission->setText(m_permissions);

    pcenter->set_current_user_id(m_id, m_permissions);
}

// 退出登录按钮
void UserInfo::pb_logout()
{
    emit sig_user_logout();
}

// 个人中心按钮
void UserInfo::pb_persional_center()
{
    pcenter->display_panel();
}

// 刷新界面用户工作信息
void UserInfo::slot_update_worker_info(QString time_start, QString time_end, QString qualified, QString unqualified, QString total)
{
    int row = ui->history_table->rowCount();
    ui->history_table->insertRow(row);
    ui->history_table->setItem(row, 0, new QTableWidgetItem(time_start));
    ui->history_table->setItem(row, 1, new QTableWidgetItem(time_end));
    ui->history_table->setItem(row, 2, new QTableWidgetItem(qualified));
    ui->history_table->setItem(row, 3, new QTableWidgetItem(unqualified));
    ui->history_table->setItem(row, 4, new QTableWidgetItem(total));

    ui->history_table->item(row, 0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    ui->history_table->item(row, 1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    ui->history_table->item(row, 2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    ui->history_table->item(row, 3)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    ui->history_table->item(row, 4)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
}

// 个人中心面板的消息记录
void UserInfo::slot_msg_log(int type, QString msg, bool msgbox)
{
    emit msg_log(type, msg, msgbox);
}
