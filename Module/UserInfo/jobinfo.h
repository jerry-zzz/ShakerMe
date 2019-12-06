#ifndef JOBINFO_H
#define JOBINFO_H

#include <QObject>
#include "sql_jobinfo.h"


class JobInfo : public QObject
{
    Q_OBJECT
public:
    explicit JobInfo(QObject *parent = nullptr);

    // 记录上机时间
    void record_login_time(QString user_id);

    // 记录下机时间
    void record_logout_time();

    // 记录当前用户生产数量
    void record_current_user_work_num(int ok_num, int false_num);

    // 获取上机时间
    QString get_login_time();

    // 获取下机时间
    QString get_logout_time();

    // 获取合格品数目
    unsigned int get_qualified_num();

    // 获取不合格品数目
    unsigned int get_unqualified_num();

    // 获取总数目
    unsigned int get_total_num();

    void get_jobinfo_history(JobData *jobdata, int *readNum);

    QStringList get_user_id_history(QDate date);



signals:  

public slots:

private:
    Sql_JobInfo sql_job;

    QString login_time = "";            // 登录时间
    QString logout_time = "";           // 登出时间
    unsigned int qualified_num = 0;     // 合格品数量
    unsigned int unqualified_num = 0;   // 不合格品数量
    unsigned int total_num = 0;         // 总数量

    QString temp_user_id;



};

#endif // JOBINFO_H
