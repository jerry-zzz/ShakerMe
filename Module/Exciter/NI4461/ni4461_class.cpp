#include "ni4461_class.h"
#include <QDebug>


NI4461_Class::NI4461_Class(QObject *parent) : QObject(parent)
{
    // AO
//    thread_ao = new QThread;
    ao = new NI4461_AO;
//    ao->moveToThread(thread_ao);

    connect(this, &NI4461_Class::sig_to_NI4461_AO_set_param, ao, &NI4461_AO::slot_from_NI4461_Class_set_ao_param);
    connect(this, &NI4461_Class::sig_to_NI4461_AO_start_ao_task, ao, &NI4461_AO::slot_from_NI4461_Class_start_ao_task);
    connect(this, &NI4461_Class::sig_to_NI4461_AO_stop_ao_task, ao, &NI4461_AO::slot_from_NI4461_Class_stop_ao_task);
//    thread_ao->start();

    // AI 0
    thread_ai0 = new QThread;
    ai0 = new NI4461_AI;
    ai0->moveToThread(thread_ai0);
    thread_ai0->start();

    connect(this, &NI4461_Class::sig_to_NI4461_AI0_set_param, ai0, &NI4461_AI::slot_from_NI4461_Class_set_ai_param);
    connect(this, &NI4461_Class::sig_to_NI4461_AI0_start_ai_task, ai0, &NI4461_AI::slot_from_NI4461_Class_start_ai_task);
    connect(this, &NI4461_Class::sig_to_NI4461_AI0_stop_ai_task, ai0, &NI4461_AI::slot_from_NI4461_Class_stop_ai_task);
    connect(ai0, &NI4461_AI::sig_to_NI4461_Class_ai_read_data, this, &NI4461_Class::slot_from_NI4461_AI0_read_data);

//    connect(this, &NI4461_Class::sig_to_NI4461_AI0_start_ai_feedback,ai0,&NI4461_AI::slot_from_NI4461_Class_start_ai_feedback);
//    connect(ai0, &NI4461_AI::sig_to_NI4461_Class_ai_feedback_data,this,&NI4461_Class::slot_from_NI4461_AI0_feedback_data);

    // AI 1
    thread_ai1 = new QThread;
    ai1 = new NI4461_AI;
    ai1->moveToThread(thread_ai1);
    thread_ai1->start();

    connect(this, &NI4461_Class::sig_to_NI4461_AI1_set_param, ai1, &NI4461_AI::slot_from_NI4461_Class_set_ai_param);
    connect(this, &NI4461_Class::sig_to_NI4461_AI1_start_ai_task, ai1, &NI4461_AI::slot_from_NI4461_Class_start_ai_task);
    connect(this, &NI4461_Class::sig_to_NI4461_AI1_stop_ai_task, ai1, &NI4461_AI::slot_from_NI4461_Class_stop_ai_task);
    connect(ai1, &NI4461_AI::sig_to_NI4461_Class_ai_read_data, this, &NI4461_Class::slot_from_NI4461_AI1_read_data);

    qRegisterMetaType<TaskHandle>("TaskHandle");
}

NI4461_Class::~NI4461_Class()
{
    thread_ai0->quit();
    thread_ai1->quit();
    delete ai0;
    delete ai1;

}


//----------------------------------------------------------------------采集卡 AO

// 设置参数并执行任务
void NI4461_Class::set_NI4461_AO_param(double ao_output_fs, QList<double> fAnlgArray, QString dev_name, int max_voltage, double fSampleRate, unsigned int numSampsPerChan)
{
    emit sig_to_NI4461_AO_set_param(ao_output_fs, fAnlgArray, dev_name, max_voltage, fSampleRate, numSampsPerChan);
}
// 开始ao任务
void NI4461_Class::start_ao_task()
{
    emit sig_to_NI4461_AO_start_ao_task();
}
// 停止ao任务
void NI4461_Class::stop_ao_task()
{
    emit sig_to_NI4461_AO_stop_ao_task();
}
// 更新AO是否工作的状态
void NI4461_Class::slot_from_NI4461_AO_update_status(bool is_ao_running)
{
    emit sig_output_ao_running_status(is_ao_running);
}





//---------------------------------------------------------------------采集卡 AI0
// 设置参数并执行任务
void NI4461_Class::set_NI4461_AI0_param(QString dev_name, double fSampleRate)
{
    emit sig_to_NI4461_AI0_set_param(dev_name, fSampleRate);
}
// 开始ai任务
void NI4461_Class::start_ai0_task(int task_type)
{
    emit sig_to_NI4461_AI0_start_ai_task(task_type);
}

// 停止ai任务
void NI4461_Class::stop_ai0_task()
{
    emit sig_to_NI4461_AI0_stop_ai_task();
}
// 更新AI是否工作的状态
void NI4461_Class::slot_from_NI4461_AI0_update_status(bool is_ai0_running)
{
    emit sig_output_ai0_running_status(is_ai0_running);
}

// 输出AI读取的通道的值
void NI4461_Class::slot_from_NI4461_AI0_read_data(double *readArray, int read_length)
{
    emit sig_output_ai0_read_data(readArray, read_length);
}



//---------------------------------------------------------------------采集卡 AI1
// 设置参数并执行任务
void NI4461_Class::set_NI4461_AI1_param(QString dev_name, double fSampleRate)
{
    emit sig_to_NI4461_AI1_set_param(dev_name, fSampleRate);
}
// 开始ai任务
void NI4461_Class::start_ai1_task(int task_type)
{
    emit sig_to_NI4461_AI1_start_ai_task(task_type);
}
// 停止ai任务
void NI4461_Class::stop_ai1_task()
{
    emit sig_to_NI4461_AI1_stop_ai_task();
}
// 更新AI是否工作的状态
void NI4461_Class::slot_from_NI4461_AI1_update_status(bool is_ai1_running)
{
    emit sig_output_ai1_running_status(is_ai1_running);
}
// 输出AI读取的通道的值
void NI4461_Class::slot_from_NI4461_AI1_read_data(double *readArray, int read_length)
{
    emit sig_output_ai1_read_data(readArray, read_length);
}
