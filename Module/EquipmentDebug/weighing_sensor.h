#ifndef WeighingSensor_H
#define WeighingSensor_H
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "../BSP/bsp_define.h"
#include "modbus.h"
#include <qthread.h>
#include <QCoreApplication>
class WeighingSensor : public QObject
{
    Q_OBJECT
public:
    explicit WeighingSensor(QObject *parent = nullptr);
    //等待接收应答
    void waitfor_reply();
    //读取当前数值命令
    bool read_current_numeric_command();
    //当前重量清零
    bool clear_current_weight();
    //砝码校准方法
    bool weight_calibration_method(int farmar_weight);
    //数字校准方法
    bool digital_calibration_method(int sensor_sensitivity,int sensor_range);
    //修改小数点
    bool change_float(int decimal_point);
    //清零触发门限设置
    bool zero_trigger_threshold_set(int zero_trigger_threshold);
    //自动清零设置
    bool auto_zero_set(int auto_zero);
    //量程系数设置
    bool range_coefficient_set(int range_coefficient);
    //上电清零范围设置
    bool zero_reset_range_set(int zero_reset_range);
    //数字滤波设置
    bool digital_filter_set(int digital_filter);
    //额定量程设置
    bool rated_range_set(int rated_range);
    //自动校准设置
    bool auto_calibration_set(int auto_calibration);
    //判稳范围设置
    bool sentenced_stabilizing_range_set(int sentenced_stabilizing_range);
    //判稳周期设置
    bool sentenced_stabilizing_t_set(int sentenced_stabilizing_t);
    //通讯机码设置
    bool machine_code_set(int machine_code);
    //发送包间隔设置
    bool send_interval_set(int send_interval);
    //零位跟踪范围设置
    bool zero_tracking_range_set(int zero_tracking_range);
    //清零延时设置
    bool auto_zero_delay_set(int auto_zero_delay);
    //采样速率设置
    bool sample_rate_set(int sample_rate);
    //通讯协议设置
    bool communication_protocol_set(int communication_protocol);
    //数据格式设置
    bool data_format_set(int data_format);
    //波特率设置
    bool baud_rate_set(int baud_rate);

    //更新当前串口信息
    void update_com_imformation(QString com);
    //打开当前串口
    bool open_serialport();
    //关闭当前串口
    void close_serialport();
    //打开定时器
    void open_timer();
    //关闭定时器
    void close_timer();

public slots:
    void serial_read_all();
    void slot_update_device_param(QVector<Device_Attribute> device_list);
    void slot_read_current_numeric_command();
signals:
    void send_result(double result);
private:
    ModBus *mb;
    QSerialPort *serial;
    QString serial_com;
    QByteArray reply_all;
    bool send_flag =false;
    QTimer *timer;
    double index=0;
};

#endif // WeighingSensor_H
