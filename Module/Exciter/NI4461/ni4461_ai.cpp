#include "Module/Exciter/NI4461/ni4461_ai.h"
#include <QDebug>
#include <QCoreApplication>
#include <QThread>



//NI4461_AI *g_ai;
/*float64 readArray[4096] = {0.0};  */      // 要读入样本的数组

NI4461_AI::NI4461_AI(QObject *parent) : QObject(parent)
{
    taskhandle = 0;
    is_ai_running = false;
    is_ai_real_running = false;
}

NI4461_AI::~NI4461_AI()
{
    DAQmxClearTask(taskhandle);
}

// 设置参数并执行任务
void NI4461_AI::slot_from_NI4461_Class_set_ai_param(QString dev_name, double fSampleRate)
{
    m_dev_name = dev_name;
    m_fSampleRate = fSampleRate;
}


// 开始ai任务
void NI4461_AI::slot_from_NI4461_Class_start_ai_task(int task_type)
{
    int errcode = 0;
    QString error_string;

    // 创建任务
    errcode = DAQmxCreateTask("", &taskhandle);
    if(errcode != 0)
    {
        error_string = get_error_string(errcode);
        qDebug() << "AI创建任务:" << error_string;
        return;
    }
    // qDebug() << "start ai";

    // 创建AI通道并添加到任务中
    switch(task_type)
    {
    case ACC_TASK:
        {
            errcode = DAQmxCreateAIAccelChan(taskhandle, m_dev_name.toLatin1().data(), "", DAQmx_Val_Cfg_Default, -10.0, 10.0, DAQmx_Val_AccelUnit_g, 97.40, DAQmx_Val_mVoltsPerG,DAQmx_Val_Internal, 0.004, NULL);
            qDebug()<<"ai0"<<m_dev_name;
        }
        break;
    case FORCE_TASK:
        {
            errcode = DAQmxCreateAIForceIEPEChan(taskhandle, m_dev_name.toLatin1().data(), "", DAQmx_Val_Cfg_Default , -10.0, 10.0, DAQmx_Val_Newtons, 21.90, DAQmx_Val_mVoltsPerNewton, DAQmx_Val_Internal, 0.004, NULL);
            qDebug()<<"ai1"<<m_dev_name;
        }
        break;
    }
    if(errcode != 0)
    {
        error_string = get_error_string(errcode);
        qDebug() << "AI创建AI通道并添加到任务中:" <<m_dev_name<< error_string;
        return;
    }

    // 配置采样时钟
    errcode = DAQmxCfgSampClkTiming(taskhandle, NULL, m_fSampleRate, DAQmx_Val_Rising, DAQmx_Val_ContSamps, m_fSampleRate);
    if(errcode != 0)
    {
        error_string = get_error_string(errcode);
        qDebug() << "AI配置采样时钟:" << error_string;
        return;
    }

    // 开始任务
    errcode = DAQmxStartTask(taskhandle);
    if(errcode != 0)
    {
        error_string = get_error_string(errcode);
        qDebug() << "AI开始任务:" << error_string;
        return;
    }

    // 更新AI运行状态
    is_ai_running = true;
    is_ai_real_running = true;

//    emit sig_to_NI4461_Class_ai_running_status(is_ai_running);
//    qDebug() << "slot_from_NI4461_Class_start_ai_task" << is_ai_running << QThread::currentThreadId();

    int32 sampsPerChanRead = 4096;     // 从每个通道读取的实际样本数。
    while(is_ai_running)
    {
        // 读取数据
        errcode = DAQmxReadAnalogF64(taskhandle, -1, 5, DAQmx_Val_GroupByScanNumber, readArray, 4096, &sampsPerChanRead, NULL);
        if(errcode != 0)
        {
            error_string = get_error_string(errcode);
            qDebug() << "AI 读取数据:" << error_string;
            return;
        }
        is_ai_recv_running = true;
        emit sig_to_NI4461_Class_ai_read_data(readArray, sampsPerChanRead);

        QThread::msleep(10);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    is_ai_recv_running = false;
    qDebug() << "ai_stop";
}




// 停止ai任务
void NI4461_AI::slot_from_NI4461_Class_stop_ai_task()
{
    QString error_string;
    int errcode;
    is_ai_running = false;

    errcode = DAQmxStopTask(taskhandle);
    if(errcode != 0)
    {
        error_string = get_error_string(errcode);
        qDebug() << "AI停止任务:" << error_string;
        // return;
    }

    errcode = DAQmxClearTask(taskhandle);
    if(errcode != 0)
    {
        error_string = get_error_string(errcode);
        qDebug() << "AI清除任务:" << error_string;
        // return;
    }
    // 更新AI运行状态
     is_ai_real_running = false;
    // emit sig_to_NI4461_Class_ai_running_status(is_ai_real_running);
    // qDebug() << "slot_from_NI4461_Class_stop_ai_task" << is_ai_running;
}

/*
void NI4461_AI::start_ai_and_recv_data(int task_type)
{
    // 创建任务
    int errcode = 0;
    QString error_string;

    errcode = DAQmxCreateTask("", &taskhandle);
    if(errcode != 0)
    {
        error_string = get_error_string(errcode);
        qDebug() << "AI创建任务:" << error_string;
        return;
    }
    qDebug() << "start ai";


    // 创建AI通道并添加到任务中
    switch(task_type)
    {
    case ACC_TASK:
        {
            errcode = DAQmxCreateAIAccelChan(taskhandle, m_dev_name.toLatin1().data(), "", DAQmx_Val_Cfg_Default, -10.0, 10.0, DAQmx_Val_AccelUnit_g, 1000, DAQmx_Val_mVoltsPerG,DAQmx_Val_Internal, 0.004, NULL);
        }
        break;
    case FORCE_TASK:
        {
            errcode = DAQmxCreateAIForceIEPEChan(taskhandle, m_dev_name.toLatin1().data(), "", DAQmx_Val_Cfg_Default , -10.0, 10.0, DAQmx_Val_Newtons, 21.90, DAQmx_Val_mVoltsPerNewton, DAQmx_Val_Internal, 0.004, NULL);
        }
        break;
    }
    if(errcode != 0)
    {
        error_string = get_error_string(errcode);
        qDebug() << "AI创建AI通道并添加到任务中:" << error_string;
        return;
    }

    // 配置采样时钟
    errcode = DAQmxCfgSampClkTiming(taskhandle, NULL, m_fSampleRate, DAQmx_Val_Rising, DAQmx_Val_ContSamps, m_fSampleRate);
    if(errcode != 0)
    {
        error_string = get_error_string(errcode);
        qDebug() << "AI配置采样时钟:" << error_string;
        return;
    }

    // 开始任务
    errcode = DAQmxStartTask(taskhandle);
    if(errcode != 0)
    {
        error_string = get_error_string(errcode);
        qDebug() << "AI开始任务:" << error_string;
        return;
    }

    // 更新AI运行状态
    is_ai_running = true;
    emit sig_to_NI4461_Class_ai_running_status(is_ai_running);
    qDebug() << "slot_from_NI4461_Class_start_ai_task" << is_ai_running << QThread::currentThreadId();

    int32 sampsPerChanRead = 0;     // 从每个通道读取的实际样本数。
    while(is_ai_running)
    {
        // 读取数据
        errcode = DAQmxReadAnalogF64(taskhandle, -1, 5, DAQmx_Val_GroupByScanNumber, readArray, 4096, &sampsPerChanRead, NULL);
        if(errcode != 0)
        {
            error_string = get_error_string(errcode);
            qDebug() << "AI 读取数据:" << error_string;
            return;
        }
//        emit sig_to_NI4461_Class_ai_read_data(readArray, sampsPerChanRead);
        emit sig_to_NI4461_Class_ai_feedback_data(readArray, sampsPerChanRead);
        QThread::msleep(10);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    qDebug() << "ai_stop";
}
*/


// 获取错误描述
QString NI4461_AI::get_error_string(int error_code)
{
    char errorString[4096] = {0};
    const int str_len = 4096;
    DAQmxGetErrorString(error_code, errorString, str_len);

    return QString(errorString);
}

// 获得ai运行状态
bool NI4461_AI::get_ai_status()
{
    return this->is_ai_real_running;
}

bool NI4461_AI::get_ai_recv_status()
{
    return this->is_ai_recv_running;
}
