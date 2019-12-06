#pragma once
#ifndef SQL_THREAD_H
#define SQL_THREAD_H

#include <QObject>
#include <QDateTime>
#include "sql_log.h"
#include <QThread>


class Sql_Thread_Record : public QObject
{
    Q_OBJECT
public:
    explicit Sql_Thread_Record(QObject *parent = nullptr);


signals:

public slots:
    void slot_RecordLog(QString type, QString date, QString userID, QString msg);
    void slot_read_log_from_select_time(QString type, QString starttime, QString endtime);

public:
    Sql_Log *sql_log;
};

class Sql_Thread : public QObject
{
    Q_OBJECT
public:
    explicit Sql_Thread(QObject *parent = nullptr);
    ~Sql_Thread();

signals:
    void sig_RecordLog(QString type, QString date, QString userID, QString msg);
    void sig_read_log_from_select_time(QString type, QString starttime, QString endtime);

public slots:
    void record_Info_msg(QString user_id, QString msg);
    void record_Alarm_msg(QString user_id, QString msg);
    void load_log_from_select_time(QString type, QString starttime, QString endtime);

public:
    Sql_Thread_Record *sql_t_r;
    QThread *sqlThread;
};


#endif // SQL_THREAD_H
