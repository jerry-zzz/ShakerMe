#ifndef LOGINFO_H
#define LOGINFO_H

#include <QWidget>
#include "sql_thread.h"
#include "../UserInfo/sql_account.h"


namespace Ui {
class LogInfo;
}

/*--- log消息类型 ---------*/
#define  MSG_INFO				0  //非生产消息记录
#define  MSG_ALM				1  //生产报警记录

class LogInfo : public QWidget
{
    Q_OBJECT
public:
/*- Module Function ----------------------------------------------------------*/
    explicit LogInfo(QWidget *parent = 0);
    ~LogInfo();

    // 设置面板权限
    void set_current_level(int level);
    // 显示界面
    void display_panel();
    // 隐藏界面
    void hide_panel();
    // 根据时间段选择日志
    void select_log_from_date(QString date_s, QString date_e, QString log_type);
    // 设置登录用户名称
    void set_user_id(QString id);

/*- Programmer Add Function --------------------------------------------------*/

signals:
/*- Module Function ----------------------------------------------------------*/

/*- Programmer Add Function --------------------------------------------------*/

public slots:
/*- Module Function ----------------------------------------------------------*/
    // 日志记录
    void record_log(int type, QString msg, bool msgbox);
    // 刷新界面表格的信息
    void slot_update_tablewidget(QString datetime, QString type, QString user, QString msg);
    // 日历控件点击
    void slot_calendar_clicked();

    // 指定人查询使能
    void pb_single_persional_en(int status);
    // 指定人查询
    void pb_user_list(const QString &user);
    // 区间日志查询
    void pb_query_log();

/*- Programmer Add Function --------------------------------------------------*/

private:
/*- Module Variable ----------------------------------------------------------*/
    Ui::LogInfo *ui;
    Sql_Thread m_sql_thread;
    Sql_Account sql_user;
    int current_level = 0;

    QString user_id;
    QString log_type;
    QString user_type;

/*- Programmer Add Variable --------------------------------------------------*/

};

#endif // LOGINFO_H
