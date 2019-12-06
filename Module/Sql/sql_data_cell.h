#ifndef SQL_DATA_CELL_H
#define SQL_DATA_CELL_H

#include <QObject>
#include <QReadWriteLock>
#include <QtSql>

class Sql_data_cell : public QObject
{
    Q_OBJECT
public:
    explicit Sql_data_cell(QObject *parent = nullptr);

    void set_db_path(QString path);
    QString get_db_path(void);

    // 保存到数据库
    void save_db(QString dbPath, QString table, QString Column, QString data);

    // 从数据库读取
    QString read_db(QString dbPath, QString table, QString Column);

signals:

public slots:


private:
    QReadWriteLock cellLock;

    QString db_path;

};

#endif // SQL_DATA_CELL_H
