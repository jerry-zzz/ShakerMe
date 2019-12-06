#ifndef SQL_ACCOUNT_H
#define SQL_ACCOUNT_H

#include <QObject>
#include <QReadWriteLock>
#include <QtSql>

class Sql_Account : public QObject
{
    Q_OBJECT
public:
    explicit Sql_Account(QObject *parent = nullptr);

    QString get_db_path(void);

    QStringList get_user_list();

signals:

public slots:
    // 注册用户
    void registered_db(QString userid, QString password, int level);

    // 查询工号是否被注册
    bool query_id_is_registered(QString userid);

    // 比对登录账户密码
    bool comparison_id_password(QString userid, QString password, int *permissions);

    // 更改权限
    void change_permissions(QString admin_id, QString admin_pw, QString user_id, int level);

    // 更改密码
    void change_password(QString user_id, QString old_pw, QString new_pw);


private:
    QReadWriteLock user_Lock;

    QString db_path;
    QString table;
};

#endif // SQL_ACCOUNT_H
