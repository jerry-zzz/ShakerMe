#ifndef PERSONALCENTER_H
#define PERSONALCENTER_H

#include <QWidget>
#include "../UserInfo/sql_account.h"


namespace Ui {
class PersonalCenter;
}

class PersonalCenter : public QWidget
{
    Q_OBJECT

public:
    explicit PersonalCenter(QWidget *parent = 0);
    ~PersonalCenter();

    // 显示界面
    void display_panel();

    // 设置当前用户名称
    void set_current_user_id(QString id, QString level_name);

signals:
    // 日志记录
    void msg_log(int type, QString msg, bool msgbox);


public slots:
    // 更改权限
    void pb_change_permissions();
    // 更改密码
    void pb_change_password();

private:
    Ui::PersonalCenter *ui;
    Sql_Account sql_user;
    QString user_id;
    QString user_level;
};

#endif // PERSONALCENTER_H
