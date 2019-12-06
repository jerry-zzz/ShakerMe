#ifndef SHAKERFEEDBACK_H
#define SHAKERFEEDBACK_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include "NI4461/ni4461_class.h"

struct FeedbackDataPrivate;
struct FeedbackObjectPrivate;

class ShakerFeedback:public QObject
{
    Q_OBJECT
public:
   ShakerFeedback(NI4461_Class *ni);
    ~ShakerFeedback();

//public:
private:
    bool start_shaker_feedback();
    bool start_shaker_sensor_feedback();


private:
    // 当前类存放数据的结构体
    FeedbackDataPrivate *mSelfData;
    // 当前类存放对象的结构体
    FeedbackObjectPrivate *mSelfObj;


private:
    QList<double> create_ao_wave(double ao_output_fs,double maxVoltage,double fSampleRate_ao,unsigned int numSampsPerChan_ao);

    double get_wave_amplitude(QList<double> waveBuff,int beginInx, int endInx);

    bool get_ni_status();

    void set_acc_feedback_status(bool flag);

    bool get_acc_feedback_status();

    bool wait_for_feedback_data();

    // 设置sensor数据的标记位
    void set_sensor_feedback_status(bool flag);

    // 获取sensor数据的标记位
    bool get_sensor_feedback_status();

    bool wait_for_sensor_feedback_data();


    bool get_next_voltage_by_acc_limits(double curAmplitude,double curVoltage, double &nextVoltage);

    bool get_next_voltage_by_sensor_limits(double curAmplitude,double curVoltage, double &nextVoltage);


    // 保存ni最大输出电压到数据库
    bool save_voltage_to_sql(double voltage);

    // 获取上一次ni最大输出电压
    double get_voltage_from_sql();

    // 将一个波形缩放到合适的尺度
    QList<double> scale(QList<double> oriWave,int multiples=1);



    // 根据Acc传感器，修正激振器的输出（递归）
    bool reviseAoOutputByAccRecursive(double voltage,double &bestVoltage);

    // 根据Acc传感器，修正激振器的输出（需要在外层套用循环）
    bool reviseAoOutputByAcc(double curVoltage,double &bestVoltage);

    // 根据麒盛传感器是否满量程，寻找NI最佳输出电压
    bool reviseAoOutputBySensor(double curVoltage, double &bestVoltage);


    bool reviseAoOutputByAiCell(double voltage,bool &isContinue,double &nextVoltage);

    bool reviseAoOutputBySensorCell(double voltage,bool &isContinue,double &nextVoltage);


public:
    bool status();

    double get_voltage_scale();

signals:

    void sig_send_feedback_result(bool result, double voltageScale);

    void sig_send_current_voltage(double voltage);


public slots:
    void slot_from_ai0_feedback_data(double *readArray, int readLength);

    void slot_from_detection_sensor_data(double heartRate, double breathRate);

    void slot_start();

    void slot_stop();

    void slot_reset(double voltage);




};

#endif // SHAKERFEEDBACK_H
