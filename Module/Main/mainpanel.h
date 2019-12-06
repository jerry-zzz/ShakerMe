#ifndef MAINPANEL_H
#define MAINPANEL_H

#include <QWidget>
#include <stdexcept>
#include "../Log/loginfo.h"
#include "../Sql/sql_bindata.h"
#include "../UserInfo/jobinfo.h"

namespace Ui {
class MainPanel;
}

// 历史数据结构
typedef struct
{
    int qualified_num;
    int unqualified_num;
}HistoryInfo;


class MainPanel : public QWidget
{
    Q_OBJECT
public:
    explicit MainPanel(QWidget *parent = nullptr);
    ~MainPanel();    

/*- Module Function ----------------------------------------------------------*/
    void init_panel(JobInfo *info);
    void display_panel();
    void hide_panel();

    // 读取历史数据
    HistoryInfo read_history();
    // 保存历史数据
    void save_history(HistoryInfo hi);

    void update_gongwei_status_runtime(QString info);
/*- Programmer Add Function --------------------------------------------------*/

//选择模式
    void choose_mode();

signals:
/*- Module Function ----------------------------------------------------------*/
    // 日志记录
    void msg_log(int type, QString msg, bool msgbox);

/*- Programmer Add Function --------------------------------------------------*/
    //模式选择
    void sig_mode_num(int mode_num);

public slots:
/*- Module Function ----------------------------------------------------------*/

    void slot_sensor_generation(int sensor_generation);
    // 刷新历史与当前检测结果记录
    void update_history_and_current_statistics(int result, QString eInfo, QString pcb_info);
/*- Programmer Add Function --------------------------------------------------*/
    void slot_update_product_pattern(int pattern);
    void slot_cur_model_change();

    void slot_reflash_platform_status( QString status);
    void slot_reflash_axis_platform_z_status(QString status, int ismove);
    void slot_reflash_axis_platform_y_status(QString status, int ismove);
    void slot_reflash_axis_shaker_z_status(QString status, int ismove);

private:
/*- Module Variable ----------------------------------------------------------*/
    Ui::MainPanel *ui;
    Sql_BinData m_sql_bin;
    JobInfo *m_jobinfo;

/*- Programmer Add Variable --------------------------------------------------*/

    int current_product_pattern  = -1;                   //当前运行的产品模式
    int generation ;
    int m_test_count = 0;              // 当前模式产品计数
    int m_c_all;//当前总计

};


#endif // MAINPANEL_H
