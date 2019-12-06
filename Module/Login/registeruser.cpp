/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    registeruser.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	用户注册面板.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#include "registeruser.h"
#include "ui_registeruser.h"
#include "system_info.h"
#include <QMessageBox>



RegisterUser::RegisterUser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegisterUser)
{
    ui->setupUi(this);
    setFixedSize(271, 328);
    // 槽函数
    connect(ui->register_2, &QPushButton::clicked, this, &RegisterUser::pb_register);
    connect(ui->password_2, &QLineEdit::returnPressed, this, &RegisterUser::pb_register);

    //-- add your code ---------------------------------------------------------

}

RegisterUser::~RegisterUser()
{
    delete ui;
}

/*
 *==============================================================================
 *   界面显示/界面隐藏
 *==============================================================================
 */

// 界面显示
void RegisterUser::display_panel()
{
    ui->id->setText("");
    ui->password_1->setText("");
    ui->password_2->setText("");
    show();
}

// 注册
void RegisterUser::pb_register()
{
    QString id = ui->id->text();
    QString pw1 = ui->password_1->text();
    QString pw2 = ui->password_2->text();

    if(id.isEmpty())
    {
        QMessageBox::about(NULL, "错误", "请输入工号!");
        return;
    }

    if(pw1.isEmpty())
    {
        QMessageBox::about(NULL, "错误", "请输入密码!");
        return;
    }


    if(pw1 != pw2)
    {
        QMessageBox::about(NULL, "错误", "两次填写的密码不一致!");
    }
    else
    {
        bool isregist = false;
        // 查询工号是否已经被注册
        isregist = sql_user.query_id_is_registered(id);

        if(isregist)
            QMessageBox::about(NULL, "错误", "您输入的工号已经被注册!");
        else
        {
            // 保存数据库
            try
            {
                sql_user.registered_db(id, pw1, OP_LEVEL);
            }
            catch(const std::runtime_error& e)
            {
                QMessageBox::about(NULL, "Title", e.what());
                return;
            }

            emit sig_update_user_list();
            QMessageBox::about(NULL, "成功", "注册成功!");
            hide();
        }
    }

}
