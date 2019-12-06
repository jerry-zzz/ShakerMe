/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    Sql_JobInfo.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	用于员工上班下班时间，检测产品数目的数据库操作.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#include "Sql_JobInfo.h"
#include "system_info.h"
#include <QSqlQueryModel>





Sql_JobInfo::Sql_JobInfo(QObject *parent) : QObject(parent)
{
    db_path = QString("%1/sys_config.db").arg(ROOT_PATH);
    table = "JobInfo";
}

QString Sql_JobInfo::get_db_path(void)
{
    return db_path;
}


// 保存到数据库
void Sql_JobInfo::save_db(QString userid, QString logintime, QString logouttime,
                          unsigned int good, unsigned int bad, unsigned int total)
{
    QWriteLocker Locker(&jobLock);

    QSqlDatabase db;
    if(QSqlDatabase::contains("my_sql_jobinfo_save"))
        db = QSqlDatabase::database("my_sql_jobinfo_save");
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE", "my_sql_jobinfo_save");
    }
    db.setDatabaseName(db_path);

    if(!db.open())
    {
        QString estr;
        estr = "my_sql_jobinfo_save open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);
    qry.prepare("CREATE TABLE IF NOT EXISTS " + table + " (id TEXT PRIMARY KEY, UserID TEXT, LoginTime TEXT, LogoutTime TEXT, Good NUMERIC, Bad NUMERIC, Total NUMERIC)");
    if( !qry.exec() )
    {
        QString estr;
        estr = "my_sql_jobinfo_save:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //更新
    qry.prepare( "SELECT * FROM " + table);
    if( !qry.exec() )
    {
        QString estr;
        estr = "my_sql_jobinfo_save:SELECT error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        qry.last();
        int id = qry.at();
        if(id < 0)
            id = 0;
        else
            id++;
        qry.prepare("INSERT INTO " + table + " (id, userid, LoginTime, LogoutTime, Good, Bad, Total) VALUES (?, ?, ?, ?, ?, ?, ?)");
        qry.bindValue(0, id);
        qry.bindValue(1, userid);
        qry.bindValue(2, logintime);
        qry.bindValue(3, logouttime);
        qry.bindValue(4, good);
        qry.bindValue(5, bad);
        qry.bindValue(6, total);
        if( !qry.exec() )
        {
            QString estr;
            estr = "my_sql_jobinfo_save:INSERT error " + qry.lastError().text();
            throw std::runtime_error(estr.toLatin1().data());
        }
    }
    db.close();
}

// 从数据库读取30条信息记录
void Sql_JobInfo::read_db_some_record(QString userid, JobData *jobdata, int *readNum)
{
    QWriteLocker Locker(&jobLock);

    QSqlDatabase db;
    if(QSqlDatabase::contains("my_sql_jobinfo_read2"))
        db = QSqlDatabase::database("my_sql_jobinfo_read2");
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE", "my_sql_jobinfo_read2");
    }
    db.setDatabaseName(db_path);

    if(!db.open())
    {
        QString estr;
        estr = "my_sql_jobinfo_read2 open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);
    qry.prepare("CREATE TABLE IF NOT EXISTS " + table + " (id TEXT PRIMARY KEY, UserID TEXT, LoginTime TEXT, LogoutTime TEXT, Good NUMERIC, Bad NUMERIC, Total NUMERIC)");
    if( !qry.exec() )
    {
        QString estr;
        estr = "my_sql_jobinfo_read2:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //更新
    QString cmd = QString("SELECT * FROM %1 WHERE UserID = '%2'").arg(table).arg(userid);
    qry.prepare(cmd);
    if( !qry.exec() )
    {
        QString estr;
        estr = "my_sql_jobinfo_read2:SELECT error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        int num = 0;
        if(qry.last())
        {
            do
            {
                jobdata[num].userid = qry.value(1).toString();
                jobdata[num].logintime = qry.value(2).toString();
                jobdata[num].logouttime = qry.value(3).toString();
                jobdata[num].good = qry.value(4).toInt();
                jobdata[num].bad = qry.value(5).toInt();
                jobdata[num].total = qry.value(6).toInt();
                num++;
                if(num >= 30)
                    break;
            }
            while(qry.previous());
            jobdata[0].logouttime = "";
        }
        *readNum = num;
    }
    db.close();
}

// 更新工人工作信息
void Sql_JobInfo::update_job_info(QString userid, QString logintime, QString logouttime,
                                  unsigned int good, unsigned int bad, unsigned int total)
{
    QWriteLocker Locker(&jobLock);

    QSqlDatabase db;
    if(QSqlDatabase::contains("my_sql_jobinfo_update"))
        db = QSqlDatabase::database("my_sql_jobinfo_update");
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE", "my_sql_jobinfo_update");
    }
    db.setDatabaseName(db_path);

    if(!db.open())
    {
        QString estr;
        estr = "my_sql_jobinfo_update open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);
    qry.prepare("CREATE TABLE IF NOT EXISTS " + table + " (id TEXT PRIMARY KEY, UserID TEXT, LoginTime TEXT, LogoutTime TEXT, Good NUMERIC, Bad NUMERIC, Total NUMERIC)");
    if( !qry.exec() )
    {
        QString estr;
        estr = "my_sql_jobinfo_update:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //更新
    qry.prepare( "SELECT * FROM " + table);
    if( !qry.exec() )
    {
        QString estr;
        estr = "my_sql_jobinfo_update:SELECT error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        qry.last();
        int id = qry.at();
        qry.prepare("REPLACE INTO " + table + " (id, userid, LoginTime, LogoutTime, Good, Bad, Total) VALUES (?, ?, ?, ?, ?, ?, ?)");
        qry.bindValue(0, id);
        qry.bindValue(1, userid);
        qry.bindValue(2, logintime);
        qry.bindValue(3, logouttime);
        qry.bindValue(4, good);
        qry.bindValue(5, bad);
        qry.bindValue(6, total);
        if( !qry.exec() )
        {
            QString estr;
            estr = "my_sql_jobinfo_update:REPLACE error " + qry.lastError().text();
            throw std::runtime_error(estr.toLatin1().data());
        }
    }
    db.close();
}


// 根据时间段搜索记录
QStringList Sql_JobInfo::read_db_from_select_time(QDateTime starttime, QDateTime endtime)
{
    QStringList user_list;
    for(int i=0;i<12;i++)
        user_list.append("");

    QWriteLocker Locker(&jobLock);

    QSqlDatabase db;
    if(QSqlDatabase::contains("my_sql_read_db_from_select_time"))
        db = QSqlDatabase::database("my_sql_read_db_from_select_time");
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE", "my_sql_read_db_from_select_time");
    }
    db.setDatabaseName(db_path);

    if(!db.open())
    {
        QString estr;
        estr = "my_sql_read_db_from_select_time open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);
    qry.prepare("CREATE TABLE IF NOT EXISTS " + table + " (id TEXT PRIMARY KEY, UserID TEXT, LoginTime TEXT, LogoutTime TEXT, Good NUMERIC, Bad NUMERIC, Total NUMERIC)");
    if( !qry.exec() )
    {
        QString estr;
        estr = "my_sql_read_db_from_select_time:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //查询
    QString cmd = QString("SELECT * FROM " + table + " where datetime(LoginTime)>=datetime('%1') and datetime(LoginTime)<datetime('%2')").arg(starttime.toString("yyyy-MM-dd hh:mm:ss")).arg(endtime.toString("yyyy-MM-dd hh:mm:ss"));
    qry.prepare(cmd);
    if(!qry.exec())
    {
        QString estr;
        estr = "my_sql_read_db_from_select_time:SELECT error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        QDateTime login_time;
        QDateTime logout_time;
        QString user;

        while (qry.next())
        {
            user = qry.value(1).toString();
            login_time = QDateTime::fromString(qry.value(2).toString(), "yyyy-MM-dd hh:mm:ss");
            logout_time = QDateTime::fromString(qry.value(3).toString(), "yyyy-MM-dd hh:mm:ss");

            int s_min_diff = starttime.time().secsTo(login_time.time()) / 3600; // 转为小时
            int e_min_diff = starttime.time().secsTo(logout_time.time()) / 3600; // 转为小时

            if(s_min_diff <= 0)     // 开机时间早于8点
                s_min_diff = 0;
            if(e_min_diff >= 12)    // 结束时间晚于8点，当8点处理
                e_min_diff = 11;
            if(e_min_diff == 0)     // 结束时间不定，则当开机时间处理。
                e_min_diff = s_min_diff;

            for(int i=s_min_diff;i<=e_min_diff;i++)
            {
                QString tmp = user_list.at(i);
                if(tmp.isEmpty())   // 此处只取一个小时内第一个上机的员工，并不是很合理，但是跟麒盛李成沟通过。
                    user_list.replace(i, user);
            }
        }
    }
    db.close();

    return user_list;
}
