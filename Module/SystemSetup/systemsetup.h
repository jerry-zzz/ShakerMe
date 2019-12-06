#ifndef SYSTEMSETUP_H
#define SYSTEMSETUP_H

#include <QWidget>
#include "../Sql/sql_bindata.h"


namespace Ui {
class SystemSetup;
}


// 系统设置界面参数
typedef struct
{
    // 轴运动速率设置
    double z_ratio_G;//激振器Z轴
    double z_ratio_Z;//载台z轴
    double z_ratio_Y;//载台Y轴

    // 系统设置
    int unable_init;            // 禁用开机自动初始化
    int unable_beep;            // 禁用蜂鸣器报警
    double y_pos_min;           // 传感器在槽内运动最小值
    double y_pos_max;           // 传感器在槽内运动最大值
    double sensor_bed_max;      // 传感器伸出床板最大值
    double shaker_distance_max; // 激振器行程范围最大值
    double shaker_bed_pos;      // 激振器Z轴到床板标定位置
    double platform_z_bed_pos;  // 载台Z轴到床板位置
    double plank_thickness;     // 床板厚度
    double platform_y_pos;      // 载台Y轴到激振器位置

    int sensor_generation;  //当前传感器代数
}Sys_Setup;


class SystemSetup : public QWidget
{
    Q_OBJECT

public:
    explicit SystemSetup(QWidget *parent = 0);
    ~SystemSetup();

/*- Module Function ----------------------------------------------------------*/
    // 初始化系统设置模块
    void init_system_setup();
    // 显示界面
    void display_panel();
    // 隐藏界面
    void hide_panel();

    // 加载参数
    void load_param();

    // 保存参数
    void save_param();

    // 获取系统设置参数
    Sys_Setup get_sys_setup_param();

/*- Programmer Add Function --------------------------------------------------*/


signals:
/*- Module Function ----------------------------------------------------------*/
    // 日志记录
    void msg_log(int type, QString msg, bool msgbox);
    // 更新系统参数
    void sig_update_sys_param(Sys_Setup ss);

    void sig_sensor_generation(int sensor_generation);
/*- Programmer Add Function --------------------------------------------------*/

public slots:
/*- Module Function ----------------------------------------------------------*/
    void pb_save();

/*- Programmer Add Function --------------------------------------------------*/


private:
/*- Module Variable ----------------------------------------------------------*/
    Ui::SystemSetup *ui;
    QString path_param;         // 系统设置参数文件路径
    Sys_Setup sys_setup;        // 系统设置界面参数
    // 数据库读取,保存
    Sql_BinData m_sql_bin;

/*- Programmer Add Variable --------------------------------------------------*/

};

#endif // SYSTEMSETUP_H
