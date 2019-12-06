/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    sql_account.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	账户信息相关操作的封装库.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */



#include "sql_account.h"
#include "system_info.h"


Sql_Account::Sql_Account(QObject *parent) : QObject(parent)
{
    db_path = QString("%1/sys_config.db").arg(ROOT_PATH);
    table = "AccountInfo";
}

QString Sql_Account::get_db_path(void)
{
    return db_path;
}

// 读取已注册的用户列表
QStringList Sql_Account::get_user_list()
{
    QStringList user_list;
    QReadLocker Locker(&user_Lock);
    QSqlDatabase db;
    if(QSqlDatabase::contains("get_user_list"))
      db = QSqlDatabase::database("get_user_list");
    else
      db = QSqlDatabase::addDatabase("QSQLITE", "get_user_list");

    db.setDatabaseName(db_path);

    if(!db.open())
    {
        QString estr;
        estr = "get_user_list open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);

    // 若不存在,则创建
    qry.prepare("CREATE TABLE IF NOT EXISTS " + table + " (id NUMERIC PRIMARY KEY, UserID TEXT, Password TEXT, Level NUMERIC)");
    if( !qry.exec() )
    {
        QString estr;
        estr = "get_user_list:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    //查询
    QString cmd = QString("SELECT * FROM %1").arg(table);
    qry.prepare(cmd);
    if( !qry.exec() )
    {
        QString estr;
        estr = "get_user_list:SELECT error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        while (qry.next())
        {
            QString user_id = qry.value(1).toString();
            user_list.append(user_id);
        }
    }
    db.close();
    return user_list;
}


// 保存到数据库
void Sql_Account::registered_db(QString userid, QString password, int level)
{
    QWriteLocker Locker(&user_Lock);

    QSqlDatabase db;
    if(QSqlDatabase::contains("my_sql_account_save"))
        db = QSqlDatabase::database("my_sql_account_save");
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE", "my_sql_account_save");
    }
    db.setDatabaseName(db_path);

    if(!db.open())
    {
        QString estr;
        estr = "sql_account_save open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);
    qry.prepare("CREATE TABLE IF NOT EXISTS " + table + " (id NUMERIC PRIMARY KEY, UserID TEXT, Password TEXT, Level NUMERIC)");
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_account_save:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //更新
    qry.prepare( "SELECT * FROM " + table);
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_account_save:SELECT error " + qry.lastError().text();
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
        qry.prepare("INSERT INTO " + table + " (id, UserID, Password, Level) VALUES (?, ?, ?, ?)");
        qry.bindValue(0, id);
        qry.bindValue(1, userid);
        qry.bindValue(2, password);
        qry.bindValue(3, level);
        if( !qry.exec() )
        {
            QString estr;
            estr = "sql_account_save:REPLACE error " + qry.lastError().text();
            throw std::runtime_error(estr.toLatin1().data());
        }
    }
    db.close();
}

// 查询工号是否被注册
bool Sql_Account::query_id_is_registered(QString userid)
{
    int isok = false;
    QReadLocker Locker(&user_Lock);
    QSqlDatabase db;
    if(QSqlDatabase::contains("my_sql_account_read"))
      db = QSqlDatabase::database("my_sql_account_read");
    else
      db = QSqlDatabase::addDatabase("QSQLITE", "my_sql_account_read");

    db.setDatabaseName(db_path);

    if(!db.open())
    {
        QString estr;
        estr = "sql_query_id_is_registered open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);

    // 若不存在,则创建
    qry.prepare("CREATE TABLE IF NOT EXISTS " + table + " (id NUMERIC PRIMARY KEY, UserID TEXT, Password TEXT, Level NUMERIC)");
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_query_id_is_registered:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    //查询
    QString cmd = QString("SELECT * FROM %1 WHERE UserID = '%2'").arg(table).arg(userid);
    qry.prepare(cmd);
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_query_id_is_registered:SELECT error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        if (qry.next())
        {
            isok = true;
        }
    }
    db.close();
    return isok;
}

// 比对登录账户密码
bool Sql_Account::comparison_id_password(QString userid, QString password, int *permissions)
{
    int isok = false;
    QReadLocker Locker(&user_Lock);
    QSqlDatabase db;
    if(QSqlDatabase::contains("my_sql_account_read"))
      db = QSqlDatabase::database("my_sql_account_read");
    else
      db = QSqlDatabase::addDatabase("QSQLITE", "my_sql_account_read");

    db.setDatabaseName(db_path);

    if(!db.open())
    {
        QString estr;
        estr = "sql_comparison_id_password open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);

    // 若不存在,则创建
    qry.prepare("CREATE TABLE IF NOT EXISTS " + table + " (id NUMERIC PRIMARY KEY, UserID TEXT, Password TEXT, Level NUMERIC)");
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_comparison_id_password:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    //查询
    QString cmd = QString("SELECT * FROM %1 WHERE UserID = '%2'").arg(table).arg(userid);
    qry.prepare(cmd);
    if( !qry.exec() )
    {
        QString estr;
        estr = "sql_comparison_id_password:SELECT error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        if (qry.next())
        {
            QString pw = qry.value(2).toString();
            int mp = qry.value(3).toInt();
            if(pw == password)
            {
                *permissions = mp;
                isok = true;
            }
            else
                *permissions = 0;
        }
    }
    db.close();
    return isok;
}


// 更改权限
void Sql_Account::change_permissions(QString admin_id, QString admin_pw, QString user_id, int level)
{
    QReadLocker Locker(&user_Lock);
    QSqlDatabase db;
    if(QSqlDatabase::contains("change_permissions"))
      db = QSqlDatabase::database("change_permissions");
    else
      db = QSqlDatabase::addDatabase("QSQLITE", "change_permissions");

    db.setDatabaseName(db_path);

    if(!db.open())
    {
        QString estr;
        estr = "change_permissions open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);

    // 若不存在,则创建
    qry.prepare("CREATE TABLE IF NOT EXISTS " + table + " (id NUMERIC PRIMARY KEY, UserID TEXT, Password TEXT, Level NUMERIC)");
    if( !qry.exec() )
    {
        QString estr;
        estr = "change_permissions:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    //查询管理员
    QString cmd = QString("SELECT * FROM %1 WHERE UserID = '%2'").arg(table).arg(admin_id);
    qry.prepare(cmd);
    if( !qry.exec() )
    {
        QString estr;
        estr = "change_permissions:SELECT error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        if (qry.next())
        {
            QString pw = qry.value(2).toString();
            int mp = qry.value(3).toInt();
            if(pw != admin_pw)
                throw std::runtime_error("管理员密码错误!");
            if(mp <= OP_LEVEL)
                throw std::runtime_error("该账户非管理员账户!");
        }
        else
        {
            throw std::runtime_error("管理员账号错误!");
        }
    }

    // 更改当前用户权限
    qry.prepare(QString("UPDATE %1 SET Level = %2 WHERE UserID = '%3'").arg(table).arg(level).arg(user_id));
    if( !qry.exec() )
    {
        QString estr;
        estr = "change_permissions:UPDATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    db.close();
}

// 更改密码
void Sql_Account::change_password(QString user_id, QString old_pw, QString new_pw)
{
    QReadLocker Locker(&user_Lock);
    QSqlDatabase db;
    if(QSqlDatabase::contains("change_password"))
      db = QSqlDatabase::database("change_password");
    else
      db = QSqlDatabase::addDatabase("QSQLITE", "change_password");

    db.setDatabaseName(db_path);

    if(!db.open())
    {
        QString estr;
        estr = "change_password open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);

    // 若不存在,则创建
    qry.prepare("CREATE TABLE IF NOT EXISTS " + table + " (id NUMERIC PRIMARY KEY, UserID TEXT, Password TEXT, Level NUMERIC)");
    if( !qry.exec() )
    {
        QString estr;
        estr = "change_password:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    //查询管理员
    QString cmd = QString("SELECT * FROM %1 WHERE UserID = '%2'").arg(table).arg(user_id);
    qry.prepare(cmd);
    if( !qry.exec() )
    {
        QString estr;
        estr = "change_password:SELECT error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        if (qry.next())
        {
            QString pw = qry.value(2).toString();
            if(pw != old_pw)
                throw std::runtime_error("用户旧密码错误!");
        }
        else
        {
            throw std::runtime_error("用户账号错误!");
        }
    }

    // 更改当前用户密码
    qry.prepare(QString("UPDATE %1 SET Password = '%2' WHERE UserID = '%3'").arg(table).arg(new_pw).arg(user_id));
    if( !qry.exec() )
    {
        QString estr;
        estr = "change_password:UPDATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    db.close();
}
