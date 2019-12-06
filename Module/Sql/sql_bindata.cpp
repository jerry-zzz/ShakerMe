/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    sql_bindata.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	以blob方式存储于数据库的封装.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */

#include "sql_bindata.h"

Sql_BinData::Sql_BinData(QObject *parent) : QObject(parent)
{

}


/*
 * @ brief  数据库二进制保存
 * @ param  dbPaht      db文件路径
 * @ param  table       table
 * @ param  bin_data    要保存的数据
 * @ retval None
 */
void Sql_BinData::save_db(QString dbPath, QString table, QByteArray bin_data)
{
    QWriteLocker Locker(&data_Lock);

    QSqlDatabase db;
    if(QSqlDatabase::contains("sql_bin_data"))
        db = QSqlDatabase::database("sql_bin_data");
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE", "sql_bin_data");
    }
    db.setDatabaseName(dbPath);

    if(!db.open())
    {
        QString estr;
        estr = "sql_bin_data_save open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }


    QSqlQuery qry(db);

    qry.prepare("CREATE TABLE IF NOT EXISTS " + table + " (id INTEGER PRIMARY KEY, data BLOB)");
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_bin_data_save:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //更新
    qry.prepare( "SELECT * FROM " + table);
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_bin_data_save:SELECT error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        if(qry.next() == false)
        {
            qry.prepare("INSERT INTO " + table + " VALUES(?, ?)");
            qry.bindValue(0, 1);
            qry.bindValue(1, bin_data, QSql::Binary);
            if( !qry.exec() )
            {
                QString estr;
                estr = "sql_bin_data_save:INSERT error " + qry.lastError().text();
                throw std::runtime_error(estr.toLatin1().data());
            }
        }
        else
        {
            qry.prepare("REPLACE INTO " + table + " VALUES(?, ?)");
            qry.bindValue(0,1);
            qry.bindValue(1, bin_data, QSql::Binary);
            if( !qry.exec() )
            {
                QString estr;
                estr = "sql_bin_data_save:REPLACE error " + qry.lastError().text();
                throw std::runtime_error(estr.toLatin1().data());
            }
        }
    }
    db.close();
}

/*
 * @ brief  数据库二进制读取
 * @ param  dbPaht      db文件路径
 * @ param  table       table
 * @ param  bin_data    要读取的数据的指针
 * @ param  len         data的长度
 * @ retval None
 */
void Sql_BinData::read_db(QString dbPath, QString table, void* bin_data, int len)
{
    QReadLocker Locker(&data_Lock);
    QSqlDatabase db;
    if(QSqlDatabase::contains("sql_bin_data"))
      db = QSqlDatabase::database("sql_bin_data");
    else
      db = QSqlDatabase::addDatabase("QSQLITE", "sql_bin_data");

    db.setDatabaseName( dbPath );// 设置数据库名与路径, 此时是放在上一个目录

    if( !db.open() )
    {
        QString estr;
        estr = "sql_bin_data_read open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    QSqlQuery qry(db);
    // 若不存在,则创建
    qry.prepare("CREATE TABLE IF NOT EXISTS " + table + " (id INTEGER PRIMARY KEY, data BLOB)");
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_bin_data_read:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //查询
    qry.prepare( "SELECT * FROM " + table);
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_bin_data_read:SELECT error! " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        if (qry.next())
        {
            QByteArray buf;
            buf = qry.value(1).toByteArray();
            memcpy(bin_data, (char *)buf.data(), len);
        }
        else
        {
            QByteArray buf;
            for(int i=0;i<len;i++)
                buf.append('\0');
            memcpy(bin_data, (char *)buf.data(), len);
        }
    }
    db.close();
}
