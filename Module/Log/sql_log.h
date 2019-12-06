#ifndef SQL_LOG_H
#define SQL_LOG_H

#include <QObject>
#include <QReadWriteLock>

class Sql_Log : public QObject
{
    Q_OBJECT
public:
    explicit Sql_Log(QObject *parent = nullptr);

    // 设置log文件路径
    void set_file_path(QString dir, QString file);
    // 返回log文件路径
    QString get_dir_path(void);
    QString get_file_path(void);

signals:
    // 输出log信息
    void sig_bind_tablewidget(QString datetime, QString type, QString user, QString msg);


public slots:
    // 根据时间段 显示log记录
    void read_select_time(QString dbPath, QString table, QString starttime, QString endtime);

    // 获取所有log记录
    void read_all(QString dbPath, QString table);

    // 记录log信息到数据库
    void record(QString dbPath, QString table, QString datetime, QString userID, QString msg);


private:
    QString dir_path;
    QString file_path;
    QReadWriteLock log_Lock;

};

#endif // SQL_LOG_H
