#ifndef DETECTION_H
#define DETECTION_H

#include <QWidget>
#include <QThread>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "../BSP/bsp_define.h"
#include "serialcommunication.h"
#include "signalprocessing.h"
#include "QCustomPlot.h"
#include "filter.h"

namespace Ui {
class Detection;
}

class Detection : public QWidget
{
    Q_OBJECT

public:
    explicit Detection(QWidget *parent = 0);
    ~Detection();
/*- Module Function ----------------------------------------------------------*/
    // 初始化
    void init_Detection();
    // 显示界面
    void display_panel();
    // 隐藏界面
    void hide_panel();
    // 日志信息
    void log_info(QString msg);

/*- Programmer Add Function --------------------------------------------------*/

signals:
/*- Module Function ----------------------------------------------------------*/
    // 日志记录
    void msg_log(int type, QString msg, bool msgbox);

    // 信号测试
    void sig_detection_sensor_signal_detection(QString com);
    void sig_detection_stop_sensor_signal_detection();

    void sig_clear_serial_data();//清除图形界面数据
/*- Programmer Add Function --------------------------------------------------*/
    // 查询pcb的id和软件版本
    void sig_read_pcb_id_and_version(QString com);
    void sig_read_pcb_id_and_version_second(QString com);

    void sig_output_pcb_info(QString id, QString version);

    // 检测测试串口
    void sig_output_com_data(double hr, double br);

public slots:
/*- Module Function ----------------------------------------------------------*/
    void slot_sensor_generation(int sensor_generation);
    //测试按钮
    void pb_test();

    void setLineEditValue(int value);
    // 查询pcb信息
    void dete_query_pcb_info(int en);

    // 返回pcb的id和软件版本
    void slot_pcb_info(QString id, QString version, QString error_msg);

    // 检测测试串口左
//    void slot_read_com_data(QCPGraph* index_h, QCPGraph* index_b, double hr, double br);
    void slot_update_graph();

    // 刷新界面显示图形
    void slot_update_graph_replot(bool en);

    // 刷新检测模块参数
    void slot_update_product_param(Full_Product_Param dp);
    // 刷新串口参数
    void slot_update_device_param(QVector<Device_Attribute> device_list);

    void start_signal_detection();
    void stop_signal_detection();
/*- Programmer Add Function --------------------------------------------------*/

private:
/*- Module Variable ----------------------------------------------------------*/
    Ui::Detection *ui;

/*- Programmer Add Variable --------------------------------------------------*/

    SignalProcessing *sp;
    QThread *thread_sp;

    bool is_com_L_open = false;   // 判断串口是否打开.

    // 数据采集标志位
    int data_sample_L = 0;
    QFile fileL;

    SerialCommunication *pcb_com;
    bool is_info_received = false;
    QString tmp_id, tmp_version, tmp_msg;
    QString m_info_id, m_info_version;

    Full_Product_Param m_dp;   //主要为了设置产品文件名称，用于保存数据
    Device_Attribute m_com;
    Device_Attribute m_com_second;
    // 串口接收数据后显示到graph标志位
    bool show_graph_EN = false;

    // 串口接收到数据标志位
    bool is_serial_read_data = false;

    // 检测标志位(防止重复检测)
    bool is_detection = false;

    bool is_data_save = false;
    //滤波
    bool is_filter_flag = false;
    bool is_move_filter = false;
    int filter_index = 0;
    QList<double> L_ai_hr;
    QList<double> L_ai_br;
    double w1 = 0.0;
    double w2 = 0.0;
    double w3 = 0.0;
    double w11 = 0.0;
    double w22 = 0.0;
    double w33 = 0.0;

//---------------------------重构代码-----------------------------

public:
    // 打开串口
    bool open_com(QSerialPort *serial, QString com, int baud);

    // 关闭串口
    bool close_com(QSerialPort *serial);

    // 给串口发送命令
    void send_start_command(QSerialPort *serial);
public:

    // 清空存放串口数据的buff
    void init_sensor_data_buff();

    // 校验数据
    int check_byte(QByteArray checkData,int count);

    // 数据归一化
    double signal_normalization(QByteArray recvData);

    // 解析二代传感器数据
    void parseSensorDataForSecondGeneration(QByteArray &recvData, QByteArray& heartData, QByteArray &breathData);

    // 解析一代传感器数据
    void parseSensorDataForFirstGeneration(QByteArray &recvData, QByteArray& heartData, QByteArray &breathData);

    bool save_data_into_file(QString fileName);

    // 往graph上添加新数据
    void set_data_to_graph(QCPGraph* index_h,QCPGraph* index_b, double hr, double br);

public slots:
    // 串口打开/关闭按钮 响应函数
    void btn_serialport_clicked();
    void filter_set();

private:

    QString mSerialStartTime;  // 串口打开时间
    QString mSerialStopTime;   // 串口关闭时间
    QByteArray requestData;    // 存放串口原始数据
    QByteArray heartData;      // 存放心跳原始数据
    QByteArray breathData;     // 存放呼吸原始数据
    QSerialPort *mSerialport;
    int mSensorGeneration = 0; // 传感器代数：0第一代，1第二代
    QString mFileSaveName = "tmp";  // 串口数据保存文件名
    QTimer *timer;      // 定时刷新界面图形
    //心跳波形图
    QCPGraph* heart_graph = nullptr;
    //呼吸波形图
    QCPGraph* breath_graph = nullptr;

    int remove_data_index = 15000;//10分钟

    //graph 时间标记
    unsigned long long graph_index = 0;

private:
    bool mSerialPortStatus = false;

private:
    // 根据界面上的选择 设置当前的传感器是哪一代(0,1
    void set_sensor_generation(int generation);

    // 获得当前的传感器属于哪一代
    int get_sensor_generation();

    // 设置当前串口运行状态
    void set_serialport_status(bool status);

    // 获得当前串口运行状态
    bool get_serialport_status();

public slots:

    void serial_read_all();

    //供外部消息调用
    void slot_start_recv_sensor_data(QString fileSaveName,int generation);
    void slot_stop_recv_sensor_data();

signals:
    sig_save_data_into_file(QString fileName,QByteArray heartData,QByteArray breathData,QString startTime, QString stopTime);
    sig_send_serial_open_result(bool result);
};

#endif // DETECTION_H
