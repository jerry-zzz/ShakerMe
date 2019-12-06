#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include "registeruser.h"
#include "../UserInfo/sql_account.h"


namespace Ui {
class Login;
}

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();

    void display_panel();

    // 获取当前用户的名称
    QString get_current_user_id();

    // 获取当前用户的权限
    int get_current_user_permission();

signals:
    // 日志记录
    void msg_log(int type, QString msg, bool msgbox);

    void sig_user_login(QString id, int permissions);

public slots:
    void pb_login();
    void pb_register();
    // 更新登录界面用户名列表
    void slot_update_user_list();

private:
    Ui::Login *ui;
    RegisterUser *ui_register;
    Sql_Account sql_user;

    QString current_user_id;
    int current_user_permission;
};

#endif // LOGIN_H
