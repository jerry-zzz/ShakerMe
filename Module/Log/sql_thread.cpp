/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    sql_thread.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	此类与 sql_log 共用，用于将日志记录放在线程内操作.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#include "sql_thread.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <stdexcept>
#include <QDebug>



//-- class Sql_Thread ----------------------------------------------------------
Sql_Thread::Sql_Thread(QObject *parent) : QObject(parent)
{
    sql_t_r = new Sql_Thread_Record;
    connect(this, &Sql_Thread::sig_RecordLog, sql_t_r, &Sql_Thread_Record::slot_RecordLog);
    connect(this, &Sql_Thread::sig_read_log_from_select_time, sql_t_r, &Sql_Thread_Record::slot_read_log_from_select_time);

    sqlThread = new QThread;
    sql_t_r->moveToThread(sqlThread);
    sqlThread->start();

}

Sql_Thread::~Sql_Thread()
{
    sqlThread->quit();
    sqlThread->wait();
    delete sql_t_r;
    delete sqlThread;
}


void Sql_Thread::record_Info_msg(QString user_id, QString msg)
{
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString current_date = QString("%1").arg(current_date_time);
    emit sig_RecordLog("Info", current_date, user_id, msg);
}

void Sql_Thread::record_Alarm_msg(QString user_id, QString msg)
{
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString current_date = QString("%1").arg(current_date_time);
    emit sig_RecordLog("Alarm", current_date, user_id, msg);
}

void Sql_Thread::load_log_from_select_time(QString type, QString starttime, QString endtime)
{
    emit sig_read_log_from_select_time(type, starttime, endtime);
}



//-- class Sql_Thread_Record ----------------------------------------------------------
Sql_Thread_Record::Sql_Thread_Record(QObject *parent) : QObject(parent)
{
    sql_log = new Sql_Log;
}



void Sql_Thread_Record::slot_RecordLog(QString type, QString date, QString userID, QString msg)
{
    QString path;
    path = sql_log->get_dir_path();
    QDir dir_r(path);
    if(!dir_r.exists())
        dir_r.mkpath(path);//创建多级目录
    try
    {
        sql_log->record(sql_log->get_file_path(), type, date, userID, msg);
    }
    catch(const std::runtime_error& e)
    {
        QFile file("c:/runtime_error.txt");
        file.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream text_stream(&file);
        text_stream << date << "    " << type << "    "<< msg << "\r\n";
        text_stream << date << "    " << "error" << "    "<< e.what() << "\r\n";
        file.flush();
        file.close();
    }
}

void Sql_Thread_Record::slot_read_log_from_select_time(QString type, QString starttime, QString endtime)
{
    sql_log->read_select_time(sql_log->get_file_path(), type, starttime, endtime);
}
