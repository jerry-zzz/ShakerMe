#ifndef LASERRANGING_H
#define LASERRANGING_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTimer>
#include <qthread.h>
#include <QCoreApplication>
#include "../BSP/bsp_define.h"
#include "modbus.h"

class LaserRanging : public QObject
{
    Q_OBJECT
public:
    explicit LaserRanging(QObject *parent = nullptr);
    //更新当前串口信息
    void update_com_imformation(QString com);
    //更新面板参数
    QByteArray update_lr_prarm(QString filter_samples,QString gain_pga,int collector_ID,QString choose_singel_channel);
    //通道实际电压值计算
    double voltage_calculation(QByteArray reply);
    //双通道查询
    void dual_channel_querl();
    //双通道触发模式
    void dual_channel_trigger_mode();
    //单通道查询
    void channel_querl();
    //单通道触发模式
    void channel_trigger_mode();
    //写采集器ID
    bool write_collector_ID(int collector_ID);
    //读采集器ID
    void read_collector_ID();
    //写采集器波特率
    bool write_collector_baudrate(QString baudrate);
    //中断指令
    void interrupt_instruction();
    //打开当前串口
    bool open_serialport();
    //关闭当前串口
    void close_serialport();
    //打开定时器
    void open_timer();
    //关闭定时器
    void close_timer();
    //等待接收应答
    void waitfor_reply();

public slots:
    void serial_read_all();
    void slot_update_device_param(QVector<Device_Attribute> device_list);
signals:
    void send_result(double out_1,double out_2);
    void result_id(int id);
private:
    ModBus *mb;
    QSerialPort *serial;
    QString serial_com;
    QByteArray reply;
    bool send_flag =false;
    QTimer *timer;
    //面板参数
    QString filtersamples_str;
    QString gainpga_str;
    QString channel_mdoe;
    int collectorID;

    int AD_RESOLUTION = 16777215;//24位AD分辨率
    int AD_REF;//增益
};

#endif // LASERRANGING_H
