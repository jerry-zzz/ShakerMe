#ifndef NI4461_CLASS_H
#define NI4461_CLASS_H

#include <QObject>
#include <QThread>
#include "ni4461_ao.h"
#include "ni4461_ai.h"
#include "Module/Exciter/NIDAQmx.h"

class NI4461_Class : public QObject
{
    Q_OBJECT
public:
    explicit NI4461_Class(QObject *parent = nullptr);
    ~ NI4461_Class();

    //------------------------------------------------------------------采集卡 AO
    NI4461_AO *ao;
    // 设置参数并执行任务
    void set_NI4461_AO_param(double ao_output_fs, QList<double> fAnlgArray, QString dev_name, int max_voltage, double fSampleRate, unsigned int numSampsPerChan);
    // 开始ao任务
    void start_ao_task();
    // 停止ao任务
    void stop_ao_task();


    //-----------------------------------------------------------------采集卡 AI0
    NI4461_AI *ai0;
    // 设置参数并执行任务
    void set_NI4461_AI0_param(QString dev_name, double fSampleRate);
    // 开始ai任务
    void start_ai0_task(int task_type);
    // 停止ai任务
    void stop_ai0_task();

    void start_ai0_task_for_feedback(int task_type);

    //-----------------------------------------------------------------采集卡 AI1
    NI4461_AI *ai1;
    // 设置参数并执行任务
    void set_NI4461_AI1_param(QString dev_name, double fSampleRate);
    // 开始ai任务
    void start_ai1_task(int task_type);
    // 停止ai任务
    void stop_ai1_task();

signals:
    //------------------------------------------------------------------采集卡 AO
    // 设置采集卡参数
    void sig_to_NI4461_AO_set_param(double ao_output_fs, QList<double> fAnlgArray, QString dev_name, int max_voltage, double fSampleRate, unsigned int numSampsPerChan);
    // 开始ao任务
    void sig_to_NI4461_AO_start_ao_task();
    // 停止ao任务
    void sig_to_NI4461_AO_stop_ao_task();
    // 更新AO是否工作的状态
    void sig_output_ao_running_status(bool is_ao_running);


    //-----------------------------------------------------------------采集卡 AI0
    // 设置采集卡参数
    void sig_to_NI4461_AI0_set_param(QString dev_name, double fSampleRate);
    // 开始ai任务
    void sig_to_NI4461_AI0_start_ai_task(int task_type);
    // 停止ai任务
    void sig_to_NI4461_AI0_stop_ai_task();
    // 更新AI是否工作的状态
    void sig_output_ai0_running_status(bool is_ai0_running);
    // 输出AI读取的通道的值
    void sig_output_ai0_read_data(double *readArray, int read_length);

    // 开启ai0的任务，用于feedback
//    void sig_to_NI4461_AI0_start_ai_feedback(int task_type);

//    void sig_output_ai0_feedback_data(double *readArray, int read_length);

    //-----------------------------------------------------------------采集卡 AI1
    // 设置采集卡参数
    void sig_to_NI4461_AI1_set_param(QString dev_name, double fSampleRate);
    // 开始ai任务
    void sig_to_NI4461_AI1_start_ai_task(int task_type);
    // 停止ai任务
    void sig_to_NI4461_AI1_stop_ai_task();
    // 更新AI是否工作的状态
    void sig_output_ai1_running_status(bool is_ai1_running);
    // 输出AI读取的通道的值
    void sig_output_ai1_read_data(double *readArray, int read_length);

public slots:
    //------------------------------------------------------------------采集卡 AO
    // 更新AO是否工作的状态
    void slot_from_NI4461_AO_update_status(bool is_ao_running);

    //-----------------------------------------------------------------采集卡 AI0
    // 更新AI是否工作的状态
    void slot_from_NI4461_AI0_update_status(bool is_ai0_running);
    // 输出AI读取的通道的值
    void slot_from_NI4461_AI0_read_data(double *readArray, int read_length);

    // 输出AI读取的通道的值-for feedback
//    void slot_from_NI4461_AI0_feedback_data(double *readArray, int read_length);

    //-----------------------------------------------------------------采集卡 AI1
    // 更新AI是否工作的状态
    void slot_from_NI4461_AI1_update_status(bool is_ai1_running);
    // 输出AI读取的通道的值
    void slot_from_NI4461_AI1_read_data(double *readArray, int read_length);

private:
    QThread *thread_ai0;
    QThread *thread_ai1;
    QThread *thread_ao;
};

#endif // NI4461_CLASS_H
