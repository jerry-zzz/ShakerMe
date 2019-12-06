/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    sql_data_cell.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	数据库单表单单元格操作封装.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#include "sql_data_cell.h"

Sql_data_cell::Sql_data_cell(QObject *parent) : QObject(parent)
{

}


void Sql_data_cell::set_db_path(QString path)
{
    db_path = path;
}

QString Sql_data_cell::get_db_path(void)
{
    return db_path;
}

// 保存到数据库
void Sql_data_cell::save_db(QString dbPath, QString table, QString Column, QString data)
{
    QWriteLocker Locker(&cellLock);

    QSqlDatabase db;
    if(QSqlDatabase::contains("my_sql_data_cell_save"))
        db = QSqlDatabase::database("my_sql_data_cell_save");
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE", "my_sql_data_cell_save");
    }
    db.setDatabaseName(dbPath);

    if(!db.open())
    {
        QString estr;
        estr = "my_sql_data_cell_save open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);
    qry.prepare(QString("CREATE TABLE IF NOT EXISTS %1 (id INTEGER PRIMARY KEY, %2 TEXT)").arg(table).arg(Column));
    if( !qry.exec() )
    {
        QString estr;
        estr = "my_sql_data_cell_save:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //查询
    qry.prepare(QString("SELECT %1 FROM %2").arg(Column).arg(table));
    if( !qry.exec() )
    {
        QString estr;
        estr = "my_sql_data_cell_save:SELECT error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        if(qry.next() == false)
        {
            qry.prepare(QString("INSERT INTO %1 VALUES (?, ?)").arg(table));
            qry.bindValue(0, 0);
            qry.bindValue(1, data);
            if( !qry.exec() )
            {
                QString estr;
                estr = "my_sql_data_cell_save:INSERT error " + qry.lastError().text();
                throw std::runtime_error(estr.toLatin1().data());
            }
        }
        else
        {
            qry.prepare(QString("UPDATE %1 SET %2 = '%3' WHERE id = 0").arg(table).arg(Column).arg(data));
            if( !qry.exec() )
            {
                QString estr;
                estr = "my_sql_data_cell_save:REPLACE error " + qry.lastError().text();
                throw std::runtime_error(estr.toLatin1().data());
            }
        }
    }
    db.close();
}


// 从数据库读取
QString Sql_data_cell::read_db(QString dbPath, QString table, QString Column)
{
    QReadLocker Locker(&cellLock);
    QString data;
    QSqlDatabase db;
    if(QSqlDatabase::contains("my_sql_data_cell_read"))
      db = QSqlDatabase::database("my_sql_data_cell_read");
    else
      db = QSqlDatabase::addDatabase("QSQLITE", "my_sql_data_cell_read");

    db.setDatabaseName( dbPath );// 设置数据库名与路径, 此时是放在上一个目录

    if( !db.open() )
    {
        QString estr;
        estr = "my_sql_data_cell_read open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    QSqlQuery qry(db);
    // 若不存在,则创建
    QString cmd = QString("CREATE TABLE IF NOT EXISTS %1 (id INTEGER PRIMARY KEY, %2 TEXT)").arg(table).arg(Column);
    qry.prepare(cmd);
    if( !qry.exec() )
    {
        QString estr;
        estr = "my_sql_data_cell_read:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //查询
    qry.prepare(QString("SELECT %1 FROM %2").arg(Column).arg(table));
    if( !qry.exec() )
    {
        QString estr;
        estr = "my_sql_data_cell_read:SELECT error! " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        if(qry.next())
        {
            data = qry.value(0).toString();
        }
        else
            data = "default";
    }
    db.close();

    return data;
}
