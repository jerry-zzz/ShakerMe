#ifndef BSP_H
#define BSP_H

#include <QWidget>
#include <QTimer>
#include <QCheckBox>
#include "sql_axisconfig.h"
#include "bsp_aps.h"
#include "bsp_io.h"
#include "bsp_device.h"

#include "config_axis_io.h"


namespace Ui {
class BSP;
}


class Bsp : public QWidget
{
    Q_OBJECT
public:
    explicit Bsp(QWidget *parent = 0);
    ~Bsp();

    void display_panel();
    void hide_panel();

    // 初始化参数
    int init_bsp_param();

    // 加载所有轴参数，IO参数
    void load_all_axis_param();
    void load_all_io_param();
    void load_all_device_param();

    void load_axis_config_panel(int index);
    void save_axis_config_panel(int index);

    void load_io_config_panel();
    void save_io_config_panel();

    void load_device_config_panel(int index);
    void save_device_config_panel(int index);

    // 轴,io
    Bsp_Aps *axis;
    Bsp_IO *io;
    Bsp_Device *device;

/*--------------------------------------------------------------- APS板卡初始化 */
    void Init_APS_Card();



signals:
    // 日志记录
    void msg_log(int type, QString msg, bool msgbox);

    // 输出更新串口,网口等设备信息
    void sig_update_device_param(QVector<Device_Attribute> device_list);
    void sig_update_ws(QVector<Device_Attribute> device_list);
    void sig_update_lr(QVector<Device_Attribute> device_list);

public slots:
    void time_Polling_AMP_Info(void);
    void pb_axis_save(void);
    void pb_axis_select(void);
    void pb_reset_encoder_pos(void);
    void pb_reset_output_pos(void);
    void pb_set_servo(void);
    void pb_move_forward_dir_start(void);
    void pb_move_negative_dir_start(void);
    void pb_move_forward_dir_stop(void);
    void pb_move_negative_dir_stop(void);
    void pb_io_save();
    void pb_io_test(); //(int row, int column);
    void pb_device_save();
    void pb_device_select();



private:
    Ui::BSP *ui;

    int current_axis_index = 0;     // 轴配置面板轴选择的序号.
    int current_device_index = 0;   // 设备配置面板选择的序号.

    // 配置界面状态轮询定时器
    QTimer *timer;

    // 轴配置参数数据库存储对象
    Sql_AxisConfig sql_axisconfig;

    // 给轴配置面板调试
    Bsp_Aps temp_aps;

    // 图像
    QPixmap lamp_on;
    QPixmap lamp_off;

    // 保存图片
    QIcon ico_save;

    int axis_max_num;
    int io_max_num;
    int device_max_num;
};

#endif // BSP_H
