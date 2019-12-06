#ifndef USERINFO_H
#define USERINFO_H

#include <QWidget>
#include "personalcenter.h"
#include "jobinfo.h"

namespace Ui {
class UserInfo;
}

class UserInfo : public QWidget
{
    Q_OBJECT

public:
/*- Module Function ----------------------------------------------------------*/
    explicit UserInfo(QWidget *parent = 0);
    ~UserInfo();

    // 初始化产品模块
    void init_userinfo(JobInfo *jobinfo);
    // 显示界面
    void display_panel();
    // 隐藏界面
    void hide_panel();
    // 设置用户信息:id,权限
    void set_account_info(QString id, QString permissions);
/*- Programmer Add Function --------------------------------------------------*/

signals:
/*- Module Function ----------------------------------------------------------*/
    // 日志记录
    void msg_log(int type, QString msg, bool msgbox);

    void sig_user_logout();
/*- Programmer Add Function --------------------------------------------------*/

public slots:
/*- Module Function ----------------------------------------------------------*/
    void pb_logout();
    void pb_persional_center();
    void slot_update_worker_info(QString time_start, QString time_end, QString qualified, QString unqualified, QString total);
    void slot_msg_log(int type, QString msg, bool msgbox);

/*- Programmer Add Function --------------------------------------------------*/

private:
/*- Module Variable ----------------------------------------------------------*/
    Ui::UserInfo *ui;
    PersonalCenter *pcenter;        // 个人中心面板
    JobInfo *m_jobinfo;             // 工作信息

    QString m_id = "";              // 登录用户id
    QString m_permissions = "";     // 登录用户权限

/*- Programmer Add Variable --------------------------------------------------*/

};

#endif // USERINFO_H
