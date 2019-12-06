#ifndef NI4461_AI_H
#define NI4461_AI_H

#include <QObject>
#include "Module/Exciter/NIDAQmx.h"



#define ACC_TASK        0       // 加速度传感器采集
#define FORCE_TASK      1       // 力传感器采集


class NI4461_AI : public QObject
{
    Q_OBJECT
public:
    explicit NI4461_AI(QObject *parent = nullptr);
    ~NI4461_AI();

    // 获取错误描述
    QString get_error_string(int error_code);
    bool get_ai_status();
    bool get_ai_recv_status();



signals:
    // 输出AI是否工作的状态
    // void sig_to_NI4461_Class_ai_running_status(bool is_ai_running);
    // 输出AI读取的通道的值
    void sig_to_NI4461_Class_ai_read_data(double *readArray, int read_length);

public slots:
    // 设置参数并执行任务
    void slot_from_NI4461_Class_set_ai_param(QString dev_name, double fSampleRate);
    // 开始ai任务
    void slot_from_NI4461_Class_start_ai_task(int task_type);
    // 停止ai任务
    void slot_from_NI4461_Class_stop_ai_task();




private:
    //--- 设定参数 ---------------------------------------------------------------
    QString m_dev_name;                 // 设备名称(到NI MAX内查看)
    double m_fSampleRate;               // 采样率

    //--- 类变量 ----------------------------------------------------------------
    TaskHandle taskhandle = 0;          // 任务句柄
    bool is_ai_running = false;         // ai是否运行状态,主要控制内部的数据采集逻辑
    bool is_ai_real_running = false;    // ai当前状态，用于外部访问
    bool is_ai_recv_running = false;    // ai当前的采集状态
    float64 readArray[4096] = {0.0};
};

#endif // NI4461_AI_H
