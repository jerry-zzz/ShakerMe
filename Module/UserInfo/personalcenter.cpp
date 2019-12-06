/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    personalcenter.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	个人中心面板.主要是用于权限更改以及密码更改.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */



#include "personalcenter.h"
#include "ui_personalcenter.h"
#include "../Log/loginfo.h"
#include <QDebug>


PersonalCenter::PersonalCenter(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PersonalCenter)
{
    ui->setupUi(this);
    setFixedSize(260, 300);

    connect(ui->confirm, &QPushButton::clicked, this, &PersonalCenter::pb_change_permissions);
    connect(ui->change_pw, &QPushButton::clicked, this, &PersonalCenter::pb_change_password);

}

PersonalCenter::~PersonalCenter()
{
    delete ui;
}

// 显示界面
void PersonalCenter::display_panel()
{
    ui->tabWidget->setCurrentIndex(0);
    show();
}

// 设置当前用户名称
void PersonalCenter::set_current_user_id(QString id, QString level_name)
{
    user_id = id;
    ui->current_permissions->setText(level_name);
}



// 更改权限
void PersonalCenter::pb_change_permissions()
{
    int level = ui->permissions->currentIndex();
    QString admin_id = ui->admin_id->text();
    QString admin_pw = ui->admin_pw->text();


    try{
        sql_user.change_permissions(admin_id, admin_pw, user_id, level);
    }
    catch(const std::runtime_error& e)
    {
        QString msg = QString("更改权限失败:%1").arg(e.what());
        emit msg_log(MSG_INFO, msg, true);
    }
    emit msg_log(MSG_INFO, "更改权限成功!\n新权限在重新登录后生效!", true);
}

// 更改密码
void PersonalCenter::pb_change_password()
{
    QString old_pw = ui->old_pw->text();
    QString new_pw = ui->new_pw->text();
    QString new_pw_2 = ui->new_pw_2->text();

    if(new_pw != new_pw_2)
    {
        emit msg_log(MSG_INFO, "两次输入的新密码不一致!", true);
        return;
    }


    try{
        sql_user.change_password(user_id, old_pw, new_pw);
        emit msg_log(MSG_INFO,"更改密码成功",true);
    }
    catch(const std::runtime_error& e)
    {
        QString msg = QString("更改密码失败:%1").arg(e.what());
        emit msg_log(MSG_INFO, msg, true);
    }

}
