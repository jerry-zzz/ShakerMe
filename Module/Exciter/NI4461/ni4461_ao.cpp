#include "Module/Exciter/NI4461/ni4461_ao.h"
#include <QDebug>
#include <QThread>
#include <windows.h>

NI4461_AO::NI4461_AO(QObject *parent) : QObject(parent)
{
    taskhandle = 0;
    is_ao_running = false;
}

NI4461_AO::~NI4461_AO()
{
    DAQmxClearTask(taskhandle);
}



// 设置参数并执行任务
void NI4461_AO::slot_from_NI4461_Class_set_ao_param(double ao_output_fs, QList<double> fAnlgArray, QString dev_name, int max_voltage, double fSampleRate, unsigned int numSampsPerChan)
{
    m_ao_output_fs = ao_output_fs;      //无效
    m_fAnlgArray = fAnlgArray;
    m_dev_name = dev_name;
    m_max_voltage = max_voltage;         //无效
    m_fSampleRate = fSampleRate;
    m_numSampsPerChan = numSampsPerChan; //无效
}

// 开始ao任务
void NI4461_AO::slot_from_NI4461_Class_start_ao_task()
{
    int errcode = 0;
    QString error_string;


    // 创建任务
    errcode = DAQmxCreateTask("", &taskhandle);
    if(errcode != 0)
    {
        error_string = get_error_string(errcode);
        qDebug() << "AO创建任务:" << error_string;
        return;
    }
    qDebug() << "ao handle" << taskhandle;


    // 创建AO通道并添加到任务中
    errcode = DAQmxCreateAOVoltageChan(taskhandle, m_dev_name.toLatin1().data(), "", -10, 10, DAQmx_Val_Volts, NULL);
    if(errcode != 0)
    {
        error_string = get_error_string(errcode);
        qDebug() << "AO创建AO通道并添加到任务中:" << error_string;
//        return;
    }

    // 配置采样时钟
    qDebug()<<m_fSampleRate;
//    errcode = DAQmxCfgSampClkTiming(taskhandle, NULL, m_fSampleRate, DAQmx_Val_Rising , DAQmx_Val_ContSamps, m_fSampleRate);
    errcode = DAQmxCfgSampClkTiming(taskhandle, NULL, m_fSampleRate, DAQmx_Val_Rising , DAQmx_Val_ContSamps, m_fSampleRate);
    if(errcode != 0)
    {
        error_string = get_error_string(errcode);
        qDebug() << "AO配置采样时钟:" << error_string;
//        return;
    }

    // 设置缓存区
    errcode = DAQmxCfgOutputBuffer(taskhandle, m_fAnlgArray.size());
    if(errcode != 0)
    {
        error_string = get_error_string(errcode);
        qDebug() << "AO设置缓存区:" << error_string << "code=" << errcode;
//        return;
    }

    // 写入波形
    float64 *fAnlgArray = new float64[m_fAnlgArray.size()];
    for(uInt32 nIndex=0; nIndex<m_fAnlgArray.size(); nIndex++)
    {
        fAnlgArray[nIndex] = m_fAnlgArray[nIndex];
    }
    int32 sampsPerChanWritten = 0;
    errcode = DAQmxWriteAnalogF64(taskhandle, m_fAnlgArray.size(), 0, -1, DAQmx_Val_GroupByChannel, fAnlgArray, &sampsPerChanWritten, NULL);
    if(errcode != 0)
    {
        error_string = get_error_string(errcode);
        qDebug() << "AO创建波形:" << error_string << "code=" << errcode;
        return;
    }

    // 开始任务
    errcode = DAQmxStartTask(taskhandle);
    if(errcode != 0)
    {
        error_string = get_error_string(errcode);
        qDebug() << "AO开始任务:" << error_string;
        return;
    }
    // 更新AO运行状态
    is_ao_running = true;
    emit sig_to_NI4461_Class_ao_running_status(is_ao_running);
    qDebug() << "slot_from_NI4461_Class_start_ao_task" << is_ao_running;
}

// 停止ao任务
void NI4461_AO::slot_from_NI4461_Class_stop_ao_task()
{
    QString error_string;
    int errcode;

    errcode = DAQmxStopTask(taskhandle);
    if(errcode != 0)
    {
        error_string = get_error_string(errcode);
        qDebug() << "AO停止任务:" << error_string;
//        return;
    }

    errcode = DAQmxClearTask(taskhandle);
    if(errcode != 0)
    {
        error_string = get_error_string(errcode);
        qDebug() << "AO清除任务:" << error_string;
//        return;
    }
    qDebug() << "stop_ao_task:: DAQmxClearTask ao handle" << taskhandle;
    taskhandle = 0;

    // 更新AO运行状态
    is_ao_running = false;
    emit sig_to_NI4461_Class_ao_running_status(is_ao_running);

    qDebug() << "slot_from_NI4461_Class_stop_ao_task" << is_ao_running;
}

// 获取错误描述
QString NI4461_AO::get_error_string(int error_code)
{
    char errorString[512] = {0};
    const int str_len = 512;
    DAQmxGetErrorString(error_code, errorString, str_len);
    return QString(errorString);
}

// 获取NI-AO运行状态
bool NI4461_AO::get_ao_status()
{
    return this->is_ao_running;
}
