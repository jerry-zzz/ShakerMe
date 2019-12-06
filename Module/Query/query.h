#ifndef QUERY_H
#define QUERY_H

#include <QWidget>
#include <QReadWriteLock>
#include <QtSql>
#include "../SystemSetup/systemsetup.h"

#define BURN_NG             0x01        // 烧录程序失败
#define ID_WRITE_NG         0x02        // id写入失败
#define ID_READ_NG          0x04        // id读取失败
#define VERSION_READ_NG     0x08        // 软件版本读取失败
//#define RESERVE             0x10        //
//#define RESERVE             0x20        //
//#define RESERVE             0x40        //
//#define RESERVE             0x80        //

#define CHECK_HR_NG         0x100       // 心跳通道NG
#define CHECK_BR_NG         0X200       // 呼吸通道NG
#define CHECK_3_3_NG        0X400       // 3.3V通道NG
#define CHECK_1_024_NG      0X800       // 1.024V通道NG
//#define RESERVE             0x1000      //
//#define RESERVE             0x2000      //
//#define RESERVE             0x4000      //
//#define RESERVE             0x8000      //

namespace Ui {
class Query;
}

class Query : public QWidget
{
    Q_OBJECT

public:
    explicit Query(QWidget *parent = 0);
    ~Query();

    void display_panel();
    void hide_panel();

    // 清空表格
    void table_remove_rows();

    // 显示查询结果
    void table_show_query_result(QStringList rlist, bool is_hr_ng, bool is_br_ng, bool is_burn_ng, bool is_id_write_ng, bool is_id_read_ng, bool is_ver_read_ng);

    // 根据时间段获取检测记录
    QHash<QString, QList<int>> query_from_select_time(QString dbPath, QString starttime, QString endtime);
    // 根据时间段且条件信息获取检测记录
    void query_from_select_time_and_conditional(QString dbPath, QString starttime, QString endtime, int conditional, int is_OK);

    bool excel_write_data(QString file, QDate date, QStringList op_list);
    bool export_excel_based_on_search(QString file);

signals:
    // 日志记录
    void msg_log(int type, QString msg, bool msgbox);

public slots:
    // 更新系统设置参数
    void slot_from_SystemSetup_update_sys_param(Sys_Setup ss);
    // 按照条件查找
    void pb_conditional_search();
    // 导出按照条件查找的记录
    //void pb_output_conditional_search();
    // 导出报表
   // void pb_output_report_form();


private:
    Ui::Query *ui;

    //------数据库
    QReadWriteLock readLock;

    // 系统参数
    Sys_Setup m_ss;
};

#endif // QUERY_H
