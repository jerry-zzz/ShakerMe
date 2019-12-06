#ifndef SQL_BINDATA_H
#define SQL_BINDATA_H

#include <QObject>
#include <QtSql>
#include <QReadWriteLock>

class Sql_BinData : public QObject
{
    Q_OBJECT
public:
    explicit Sql_BinData(QObject *parent = nullptr);

    // 保存到数据库
    void save_db(QString dbPath, QString table, QByteArray bin_data);

    // 从数据库读取
    void read_db(QString dbPath, QString table, void* bin_data, int len);

signals:

public slots:

private:
    QReadWriteLock data_Lock;
};

#endif // SQL_BINDATA_H
