/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    sql_axisconfig.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	轴配置参数相关操作的封装库.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#include "sql_axisconfig.h"
#include "system_info.h"
#include <stdexcept>
#include <QDebug>

Sql_AxisConfig::Sql_AxisConfig(QObject *parent) : QObject(parent)
{
    db_path = QString("%1/sys_config.db").arg(ROOT_PATH);
}

QString Sql_AxisConfig::get_db_path(void)
{
    return db_path;
}


/*
 * @ brief  数据库二进制保存
 * @ param  dbPaht      db文件路径
 * @ param  table       table
 * @ param  index       序号
 * @ param  data        要保存的数据
 * @ param  len         data的长度
 * @ retval None
 */
void Sql_AxisConfig::save_db(QString dbPath, QString table, int index, QByteArray buf)
{
    QWriteLocker Locker(&data_Lock);

    QSqlDatabase db;
    if(QSqlDatabase::contains("my_sql_axisconfig_read"))
        db = QSqlDatabase::database("my_sql_axisconfig_read");
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE", "my_sql_axisconfig_read");
    }
    db.setDatabaseName(dbPath);

    if(!db.open())
    {
        QString estr;
        estr = "sql_axisconfig_save open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }


    QSqlQuery qry(db);
    qry.prepare("CREATE TABLE IF NOT EXISTS " + table + " (id INTEGER PRIMARY KEY, data BLOB)");
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_axisconfig_save:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //更新
    qry.prepare( "SELECT * FROM " + table);
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_axisconfig_save:SELECT error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        qry.prepare("REPLACE INTO " + table + " VALUES(?, ?)");
        qry.bindValue(0, index);
        qry.bindValue(1, buf, QSql::Binary);
        if( !qry.exec() )
        {
            QString estr;
            estr = "sql_axisconfig_save:REPLACE error " + qry.lastError().text();
            throw std::runtime_error(estr.toLatin1().data());
        }
    }
    db.close();
}

/*
 * @ brief  数据库二进制读取
 * @ param  dbPaht      db文件路径
 * @ param  table       table
 * @ param  data        要读取的数据的指针
 * @ param  len         data的长度
 * @ retval None
 */
void Sql_AxisConfig::read_db(QString dbPath, QString table, int index, void* data, int len)
{
    QReadLocker Locker(&data_Lock);
    QSqlDatabase db;
    if(QSqlDatabase::contains("my_sql_axisconfig_read"))
      db = QSqlDatabase::database("my_sql_axisconfig_read");
    else
      db = QSqlDatabase::addDatabase("QSQLITE", "my_sql_axisconfig_read");
    db.setDatabaseName( dbPath );       // 设置数据库名与路径, 此时是放在上一个目录

    if( !db.open() )
    {
        QString estr;
        estr = "sql_axisconfig_read open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }


    //查询
    QSqlQuery qry(db);

    // 若不存在,则创建
    qry.prepare("CREATE TABLE IF NOT EXISTS " + table + " (id INTEGER PRIMARY KEY, data BLOB)");
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_axisconfig_read:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QString cmd = QString("SELECT * FROM %1 WHERE id = '%2'").arg(table).arg(index);
    qry.prepare(cmd);

    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_axisconfig_read:SELECT error! " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        if(qry.next())
        {
            QByteArray buf;
            buf = qry.value("data").toByteArray();
            if(buf.size() == len)
                memcpy(data, (char *)buf.data(), len);
        }
        else
        {
            QString estr;
            estr = "sql_axisconfig_read:seek error! " + qry.lastError().text();
            throw std::runtime_error(estr.toLatin1().data());
        }
    }
    db.close();
}
