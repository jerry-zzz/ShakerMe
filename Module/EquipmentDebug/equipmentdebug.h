#ifndef EQUIPMENTDEBUG_H
#define EQUIPMENTDEBUG_H

#include <QWidget>
#include <QTimer>
#include "../Sql/sql_bindata.h"
#include "../BSP/bsp.h"
#include "weighing_sensor.h"
#include "laserranging.h"
#include "Module/Exciter/shaker.h"
#include "Module/Exciter/shakerfeedback.h"
#include <QThread>


namespace Ui {
class EquipmentDebug;
}


// 设备调试界面参数
typedef struct
{
    double module_pos;                 // 模板点位
    double pos_bed;
    double pos_sponge;
    double pos_shell;
    double pos_bare;
    double pos_safe;
    double pos_bed_y;
    double pos_y;
    double pos_z;
    double ratio_slowly;
    double move_distance;
    int type_en;
}Equipment_Param;

//压力传感器界面参数
typedef struct
{
    int farmar_weight;              //砝码重量
    int sensor_sensitivity;         //数字校准
    int sensor_range;               //标定量程
    int decimal_point;              //修改小数点
    int zero_trigger_threshold;     //清零触发门限
    int auto_zero;                  //自动清零
    int range_coefficient;          //量程系数
    int zero_reset_range;           //上电清零范围
    int digital_filter;             //数字滤波
    int rated_range;                //额定量程
    int auto_calibration;           //自动校准
    int sentenced_stabilizing_range;//判稳范围
    int sentenced_stabilizing_t;    //判稳周期
    int machine_code;               //通讯机码
    int send_interval;              //发送包间隔
    int zero_tracking_range;        //零位跟踪范围
    int auto_zero_delay;            //清零延时
    int sample_rate;                //采样速率
    int communication_protocol;     //通讯协议
    int data_format;                //数据格式
    int baud_rate;                  //波特率

}WeighingSensor_Param;

//激光测距传感器界面参数
typedef struct
{
    QString filter_samples ;//滤波个数
    QString gain_pga ;//增益
    int collector_ID ;//采集器ID
    QString choose_singel_channel;//单通道选择
}LaserRanging_Param;


class EquipmentDebug : public QWidget
{
    Q_OBJECT
public:
/*- Module Function ----------------------------------------------------------*/
    explicit EquipmentDebug(QWidget *parent = nullptr);
    ~EquipmentDebug();

    // 初始化模块
    void init_EquipmentDebug(Bsp *bsp,WeighingSensor *ws,LaserRanging *lr,Shaker *shaker, ShakerFeedback *shakerFb);
    // 显示界面
    void display_panel();
    // 隐藏界面
    void hide_panel();

    void load_param();
    void save_param();
/*- Programmer Add Function --------------------------------------------------*/

signals:
/*- Module Function ----------------------------------------------------------*/
    // 日志记录
    void msg_log(int type, QString msg, bool msgbox);
    // 更新设备调试参数
    void sig_update_ep_param(Equipment_Param ep);
/*- Programmer Add Function --------------------------------------------------*/
signals:
    void sig_to_feedback_start();
    void sig_to_feedback_stop();
    void sig_to_reset_feedback(double voltage);

public slots:
/*- Module Function ----------------------------------------------------------*/
    void slot_update_db_path(QString path);
    void pb_param_save(void);
    void refresh_panel_AMP_Info(void);
    void pb_set_servo();
    void pb_move_r_forward_dir(void);
    void pb_move_r_negative_dir(void);
    void pb_move_stop(void);
    void pb_move_home(void);

/*- Programmer Add Function --------------------------------------------------*/
    //pcb使能
    //void pb_signal_PCB_en();
    //电磁阀使能
    void pb_signal_ELECTRIC_en();
    /***************************************/
    void pb_read_pos_shaker_test();
    void pb_move_pos_shaker_test();
//4种模式，3个轴的测试点位
    void pb_read_pos_bed();
    void pb_move_pos_bed();
    void pb_read_pos_sponge();
    void pb_move_pos_sponge();
    void pb_read_pos_shell();
    void pb_move_pos_shell();
    void pb_read_pos_bare();
    void pb_move_pos_bare();
    void pb_read_pos_safe();
    void pb_move_pos_safe();
    void pb_read_pos_bed_y();
    void pb_move_pos_bed_y();
    void pb_read_pos_y();
    void pb_move_pos_y();
    void pb_read_pos_z();
    void pb_move_pos_z();

    /***************************************/
    //压力传感器
    void pb_modbus_command();//发送modbus指令
    void pb_open_serialport_command();//打开压力传感器串口
    void pb_open_timer_switch();//压力传感器定时器开关;
    void slots_rece_result(double result);
    //激光测距传感器
    void pb_open_lr_serialport_command();//打开激光测距传感器串口
    void pb_open_lr_timer_switch();//激光测距传感器定时器开关
    void pb_lr_command();//发送激光测距传感器指令
    void slots_rece_lr_result(double out_1,double out_2);
    void slots_lr_id_result(int id);

    // 功率放大器
    void slot_btn_start_feedback_clicked();
    void slot_btn_reset_feedback_clicked();
    void slot_recv_feedback_result(bool result, double voltage);
    void slot_recv_current_feedback_voltage(double voltage);
private:
/*- Module Function ----------------------------------------------------------*/
    Ui::EquipmentDebug *ui;
    Bsp *m_bsp;             // 运动控制库
    Equipment_Param ep;     // 设备调试界面参数
    QTimer *panel_refresh_timer;          // 界面状态轮询定时器
    QTimer *m_pTimer_mws;// 压力传感器定时器
    QTimer *m_pTimer_mlr;// 激光传感器定时器
    Sql_BinData m_sql_bin;  // 数据库读取,保存
    QPixmap lamp_on;        // io图标 on
    QPixmap lamp_off;       // io图标 off
    QString param_db_path;  // 参数存储地址.

/*- Programmer Add Function --------------------------------------------------*/
public:
    WeighingSensor *mws;    //压力传感器
    LaserRanging *mlr;      //激光测距传感器
    LaserRanging_Param lrp; //激光测距传感器界面参数
    WeighingSensor_Param wsp;//压力传感器界面参数

private:
    Shaker *shaker;
    ShakerFeedback *shakerFb;

};


#endif // EQUIPMENTDEBUG_H
