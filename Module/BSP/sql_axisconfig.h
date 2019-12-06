#ifndef SQL_AXIS_CONFIG_H
#define SQL_AXIS_CONFIG_H

#include <QObject>
#include <QReadWriteLock>
#include <QtSql>

class Sql_AxisConfig : public QObject
{
    Q_OBJECT
public:
    explicit Sql_AxisConfig(QObject *parent = nullptr);

    QString get_db_path(void);

    // 保存到数据库
    void save_db(QString dbPath, QString table, int index, QByteArray buf);

    // 从数据库读取
    void read_db(QString dbPath, QString table, int index, void* data, int len);

signals:

public slots:

private:
    QReadWriteLock data_Lock;

    QString db_path;
};

#endif // SQL_AXIS_CONFIG_H
