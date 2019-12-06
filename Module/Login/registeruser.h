#ifndef REGISTERUSER_H
#define REGISTERUSER_H

#include <QWidget>
#include "../UserInfo/sql_account.h"




namespace Ui {
class RegisterUser;
}

class RegisterUser : public QWidget
{
    Q_OBJECT

public:
/*- Module Function ----------------------------------------------------------*/
    explicit RegisterUser(QWidget *parent = 0);
    ~RegisterUser();

    void display_panel();

/*- Programmer Add Function --------------------------------------------------*/

signals:
/*- Module Function ----------------------------------------------------------*/
    // 注册后更新登录界面用户名列表
    void sig_update_user_list();

/*- Programmer Add Function --------------------------------------------------*/

public slots:
/*- Module Function ----------------------------------------------------------*/
    void pb_register();

/*- Programmer Add Function --------------------------------------------------*/

private:
/*- Module Variable ----------------------------------------------------------*/
    Ui::RegisterUser *ui;
    Sql_Account sql_user;

/*- Programmer Add Variable --------------------------------------------------*/

};

#endif // REGISTERUSER_H
