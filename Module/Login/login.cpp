/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    login.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	登录面板.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */



#include "login.h"
#include "ui_login.h"
#include "../Log/loginfo.h"
#include "system_info.h"






Login::Login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
    setFixedSize(400, 300);

    ui->machine_name->setText(MACHINE_NAME);

    ui_register = new RegisterUser();
    connect(ui_register, &RegisterUser::sig_update_user_list, this, &Login::slot_update_user_list);

    connect(ui->login, &QPushButton::clicked, this, &Login::pb_login);
    connect(ui->register_2, &QPushButton::clicked, this, &Login::pb_register);
    connect(ui->password, &QLineEdit::returnPressed, this, &Login::pb_login);
}

Login::~Login()
{
    delete ui_register;
    delete ui;
}

void Login::display_panel()
{
    // 更新登录界面用户名列表
    slot_update_user_list();

    ui->id->setCurrentText("");
    ui->password->setText("");
    ui->id->setFocus();

//#ifndef DEBUG_MODE
    ui->id->setCurrentText("admin");
    ui->password->setText("admin");
    ui->password->setFocus();
//#endif

    show();
}

// 更新登录界面用户名列表
void Login::slot_update_user_list()
{
    ui->id->clear();
    QStringList user_list = sql_user.get_user_list();
    ui->id->insertItems(0, user_list);
}

void Login::pb_login()
{
    int isok = false;
    int permissions = 0;
    QString id = ui->id->currentText();
    QString password = ui->password->text();

    try
    {
        isok = sql_user.comparison_id_password(id, password, &permissions);
        current_user_id = id;
        current_user_permission = permissions;
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("登录错误：%1").arg(e.what()), true);
        return;
    }

    if(isok)
    {
        emit sig_user_login(id, permissions);
        this->hide();
    }
    else
        emit msg_log(MSG_INFO, "您输入的工号或密码有错误!", true);
}




void Login::pb_register()
{
    ui_register->display_panel();
}


// 获取当前用户的名称
QString Login::get_current_user_id()
{
    return current_user_id;
}

// 获取当前用户的权限
int Login::get_current_user_permission()
{
    return current_user_permission;
}
