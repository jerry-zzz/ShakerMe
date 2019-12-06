#ifndef NI4461_AO_H
#define NI4461_AO_H

#include <QObject>
#include"Module/Exciter/NIDAQmx.h"

class NI4461_AO : public QObject
{
    Q_OBJECT
public:
    explicit NI4461_AO(QObject *parent = nullptr);
    ~NI4461_AO();

    // 获取错误描述
    QString get_error_string(int error_code);
    bool get_ao_status();
signals:
    // 输出AO是否工作的状态
    void sig_to_NI4461_Class_ao_running_status(bool is_ao_running);

public slots:
    // 设置参数并执行任务
    void slot_from_NI4461_Class_set_ao_param(double ao_output_fs, QList<double> fAnlgArray, QString dev_name, int max_voltage, double fSampleRate, unsigned int numSampsPerChan);
    // 开始ao任务
    void slot_from_NI4461_Class_start_ao_task();
    // 停止ao任务
    void slot_from_NI4461_Class_stop_ao_task();

private:
    //--- 设定参数 ---------------------------------------------------------------
    double m_ao_output_fs;              // 输出频率
    QList<double> m_fAnlgArray;         // 波形数据
    QString m_dev_name;                 // 设备名称(到NI MAX内查看)
    int m_max_voltage;                  // 输出最大电压
    double m_fSampleRate;               // 采样率
    unsigned int m_numSampsPerChan;     // 通道写入的样本数(可与AO波形缓存大小一致)

    //--- 类变量 ----------------------------------------------------------------
    TaskHandle taskhandle = 0;          // 任务句柄
    bool is_ao_running = false;         // ao是否运行状态



};

#endif // NI4461_AO_H
