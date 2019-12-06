#ifndef SQL_JOBINFO_H
#define SQL_JOBINFO_H

#include <QObject>
#include <QReadWriteLock>
#include <QtSql>


typedef struct
{
    QString userid;
    QString logintime;
    QString logouttime;
    unsigned int good;
    unsigned int bad;
    unsigned int total;
}JobData;


class Sql_JobInfo : public QObject
{
    Q_OBJECT
public:
    explicit Sql_JobInfo(QObject *parent = nullptr);

    QString get_db_path(void);

signals:

public slots:
    // 保存到数据库
    void save_db(QString userid, QString logintime, QString logouttime,
                              unsigned int good, unsigned int bad, unsigned int total);

    // 更新工人工作信息
    void update_job_info(QString userid, QString logintime, QString logouttime,
                         unsigned int good, unsigned int bad, unsigned int total);

    // 从数据库读取30条信息记录
    void read_db_some_record(QString userid, JobData *jobdata, int *readNum);

    // 根据时间段搜索记录
    QStringList read_db_from_select_time(QDateTime starttime, QDateTime endtime);


private:
    QReadWriteLock jobLock;

    QString db_path;
    QString table;


};

#endif // SQL_JOBINFO_H
