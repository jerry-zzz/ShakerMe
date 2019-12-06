/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    sql_log.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	此类与 sql_thread 共用，用于将日志记录放在线程内操作.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#include "sql_log.h"
#include "system_info.h"
#include <QtSql>
#include <stdexcept>
#include <QDebug>

Sql_Log::Sql_Log(QObject *parent) : QObject(parent)
{
    dir_path = LOG_PATH;
    file_path = dir_path + "/log.db";
}


/*
 * @ brief  设置log文件路径
 * @ param  logpath:    log路径
 * @ retval .
 */
void Sql_Log::set_file_path(QString dir, QString file)
{
    dir_path = dir;
    file_path = file;
}

/*
 * @ brief  获取log文件夹路径
 * @ param  .
 * @ retval log文件夹路径
 */
QString Sql_Log::get_dir_path(void)
{
    return dir_path;
}

/*
 * @ brief  获取log文件路径
 * @ param  .
 * @ retval log路径
 */
QString Sql_Log::get_file_path(void)
{
    return file_path;
}

/*
 * @ brief  根据起始结束时间调取日志内容
 * @ param  dbPaht      db文件路径
 * @ param  table       table
 * @ param  starttime   起始时间
 * @ param  endtime     结束时间
 * @ retval None
 */
void Sql_Log::read_select_time(QString dbPath, QString table, QString starttime, QString endtime)
{
    QReadLocker Locker(&log_Lock);
    QSqlDatabase db;
    if(QSqlDatabase::contains("qt_sql_default_connection"))
      db = QSqlDatabase::database("qt_sql_default_connection");
    else
      db = QSqlDatabase::addDatabase("QSQLITE");

    db.setDatabaseName(dbPath);

    if(!db.open())
    {
        QString estr;
        estr = "sql_log_read_select_time open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);

    qry.prepare( "CREATE TABLE IF NOT EXISTS " + table + " (id INTEGER PRIMARY KEY, datetime TEXT, userID TEXT, message TEXT)"  );
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_log_read_select_time:CREATE error " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //查询
    QString cmd = QString("SELECT * FROM %1 where \
                            datetime(datetime)>=datetime('%2') and \
                            datetime(datetime)<datetime('%3')").arg(table).arg(starttime).arg(endtime);
    qry.prepare(cmd);
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_log_read_select_time:SELECT error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        while (qry.next())
        {
            QString datetime = qry.value(1).toString();
            QString type = table;
            QString user = qry.value(2).toString();
            QString msg = qry.value(3).toString();
            emit sig_bind_tablewidget(datetime, type, user, msg);
        }
    }
    db.close();
}

/*
 * @ brief  调取所有日志信息 ,并输出到log panel
 * @ param  dbPaht      db文件路径
 * @ param  table       table
 * @ retval None
 */
void Sql_Log::read_all(QString dbPath, QString table)
{
    QReadLocker Locker(&log_Lock);
    QSqlDatabase db;
    if(QSqlDatabase::contains("qt_sql_default_connection"))
      db = QSqlDatabase::database("qt_sql_default_connection");
    else
      db = QSqlDatabase::addDatabase("QSQLITE");

    db.setDatabaseName(dbPath);

    if(!db.open())
    {
        QString estr;
        estr = "sql_log_read_all open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry;

    qry.prepare( "CREATE TABLE IF NOT EXISTS " + table + " (id INTEGER PRIMARY KEY, datetime TEXT, userID TEXT, message TEXT)"  );
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_log_read_all:CREATE error " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //查询
    qry.prepare( "SELECT * FROM " + table);
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_log_read_all:SELECT error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        while (qry.next())
        {
            QString datetime = qry.value(1).toString();
            QString type = table;
            QString user = qry.value(2).toString();
            QString msg = qry.value(3).toString();
            emit sig_bind_tablewidget(datetime, type, user, msg);
        }
    }
    db.close();
}

/*
 * @ brief  日志保存到数据库
 * @ param  dbPaht      db文件路径
 * @ param  table       table
 * @ param  datetime    时间
 * @ param  msg         消息
 * @ retval None
 */
void Sql_Log::record(QString dbPath, QString table, QString datetime, QString userID, QString msg)
{
    QWriteLocker Locker(&log_Lock);
    QSqlDatabase db;
    if(QSqlDatabase::contains("qt_sql_default_connection"))
      db = QSqlDatabase::database("qt_sql_default_connection");
    else
      db = QSqlDatabase::addDatabase("QSQLITE");

    db.setDatabaseName(dbPath);

    if(!db.open())
    {
        QString estr;
        estr = "sql_log_record open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry;
    qry.prepare( "CREATE TABLE IF NOT EXISTS " + table + " (id INTEGER PRIMARY KEY, datetime TEXT, userID TEXT, message TEXT)"  );
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_log_record:CREATE error " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    qry.prepare("INSERT INTO " + table + " VALUES(?, ?, ?, ?)");
//    qry.bindValue(0, 1);
    qry.bindValue(1, datetime);
    qry.bindValue(2, userID);
    qry.bindValue(3, msg);
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_log_record:INSERT error " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    db.close();
}
