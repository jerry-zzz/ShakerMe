#ifndef SHAKER_H
#define SHAKER_H

#include <QWidget>
#include "NI4461/ni4461_class.h"
#include <QTimer>
#include "shakerfeedback.h"

struct DataPrivate;


namespace Ui {
class Shaker;
}

typedef struct AoParam
{
    QString NiDevName;    // NI的设备名称
    QString AoDevName;
    int AoSample;         // AO输出采样频率
    int RecvTime;         // AO输出时间
    QString WavePath;     // 波形文件路径
    QList<double> waveBuf;// 当前波形
    double WaveFre;       // 波形频率
    double WaveMaxVoltage;// 波形幅值最大值（
}Ao_Param;


typedef struct AiParam
{
    QString NiDevName;    // NI的设备名称
    QString AiDevName;    // Ai名称
    int AiSample;         // Ai1采样频率
}Ai_Param;




class Shaker : public QWidget
{
    Q_OBJECT

public:
    //explicit Shaker(QWidget *parent = 0);
    Shaker(QWidget *parent = 0,NI4461_Class* niObject = NULL,ShakerFeedback *shakerFb = NULL);
    ~Shaker();

    void myzoom_config();
    // 显示界面
    void display_panel();
    // 隐藏界面
    void hide_panel();

    //NI错误码查询
    QString get_error_string(int error_code);

    // 设置NI对象


public:
    bool is_shaker_running();  //外部判断激振器是否在工作

    void start_setting();
    bool is_setting_finished();
    bool is_started_success();

private:
    void init_graph();

    bool read_wave_from_file(QString fileName,int requestLength, QList<double> &waveBuff);//读取波形文件
    bool set_ao_param(QString wavePath, int aoSample, int recvTime);   //设置Ao参数
    bool set_ai_param(int aiNo, int aiSample);                         //设置Ai参数

    // 从界面上获得ao和ai的参数
    bool get_ao_ai_param_from_ui();

    bool get_ao_ai_param_for_sine_model_from_ui();

    // 根据界面上的勾选，决定开启那个ai
    bool start_ai_task();

    bool stop_ai_task();

    // 开启定时器
    void start_my_timer();

    // 关闭定时器
    void stop_my_timer();

    // 波形由代码生成，并输出波形
    bool pb_start();

    void pb_stop();

    // 波形从文件中读取，并输出波形
    bool pb_start_wav();

    // 获取串口打开结果
    bool get_serial_opened_result();

    void init_serial_wait_flag();

    void wait_serial_open_complete(QString wavePath);

    void set_shaker_running_flag(bool flag);

    void start_shaker_for_sine();

    void start_shaker_for_file();

    // 波形缩放
    bool scale(QList<double> &waveBuff);
public:
    // 用于外部参数设置
    bool set_ni_param(QString wavePath, int aoSample, int recvTime);
    // 从外部启动定时器
    bool pb_start_wav_for_process();

    void wait_for_wave_finish();

//public:
//    //激振器启动是否成功的标志
//    bool is_shaker_start_success = false;
//    bool is_shaker_set_finished = false;


private:
    Ui::Shaker *ui;

private:
    NI4461_Class *ni;

    DataPrivate * mSelfData;

    ShakerFeedback *shakerFb;         // 激振器反馈调节对象组件


    // ao param
    Ao_Param mAoParam;
    // ai param
    Ai_Param mAiParam[2];

    // graph
    unsigned long long acc_org_index = 0;

    QTimer *timer;      //定时器，采样时间
    int time_count;     //定时器计数值（秒）
    bool mRunningFlag = false;   //激振器是否运行

    // 等待串口打开完成（无论成功与否）
    bool mWaitSerialOpenFlag = false;

    bool mIsSerialOpenSuccess = false;

    QString dir_name;     //文件名称

    int mSensorGeneration; //传感器代数


//    QThread *threadShakerFb;


private slots:
    void on_pushButton_feedback_start_clicked();
    void on_pushButton_feedback_stop_clicked();

signals:
    void sig_to_feedback_start();
    void sig_to_feedback_stop();
    void sig_test_send_data(double *readArray, int read_length);

signals:
    void stop_txt_serial();
    // 信号测试
    void sig_start_signal_detection(QString filepath,int generation);
    void sig_stop_signal_detection();

    //发送波形buff给图形界面
    void sig_send_wavebuff(QList<double> &waveBuff);
    void sig_send_shaker_acc_buff(double *readArray,int read_length);
    void sig_send_shaker_force_buff(double *readArray,int read_length);


public slots:
    void slot_sensor_generation(int sensor_generation);

    // 开始按钮
    void pb_start_clicked();

    // 文件选择按钮
    void pb_sine_dir_set();

    // 选择播放文件还是sine
    void radio_sine_file_clicked();

    // 获取加速度计传感器通道的数据
    void get_acc_data(double *readArray, int read_length);

    // 获取力传感器通道的数据
    void get_force_data(double *readArray, int read_length);

    // 获取麒盛传感器通道数据
    void get_sensor_data(double heartRate, double breathRate);

    // 定时器刷新
    void timeUpDate();

    void slot_set_ni_param_and_start_ao_ai(QString wavePath, int aoSample, int recvTime , QString product_name);

    void slot_recv_serial_open_result(bool serialFlag);

    void slot_recv_feedback_result(bool result, double voltageScale);


 };

#endif // SHAKER_H
