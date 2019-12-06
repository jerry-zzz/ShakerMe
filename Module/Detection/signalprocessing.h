#ifndef SIGNALPROCESSING_H
#define SIGNALPROCESSING_H

#include <QObject>
#include <QMediaPlayer>
#include "../ProductSet/productset.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>


#define CHECK_NOISE_NG          0x01        // 噪音测试不通过
#define CHECK_SINE_HZ_NG        0x02        // 正弦信号频率不通过
#define CHECK_SINE_PTP_NG       0x04        // 正弦信号峰峰值不通过
#define CHECK_HEART_RATE_NG     0x08        // 心率计算不通过
#define CHECK_COEF_NG           0x10        // 比值系数计算不通过
#define CHECK_MIN_NG            0x20        // 峰值不通过

typedef struct{
    double noise;
    QList<double> hz;
    QList<double> ptp;
    QList<int> hr;
    QList<double> coef;
    QList<double> min_threshold;
}TestResult;




class SignalProcessing : public QObject
{
    Q_OBJECT
public:
    explicit SignalProcessing(QObject *parent = nullptr);

    // crc8校验
    unsigned char crc8_chk_value(unsigned char *message, int len);

    // 日志信息
    void log_info(QString msg);

    void set_product_param(Full_Product_Param dp);
    // 判断噪声
    double judgment_noise_signal(QList<double> array, int len, double distance);
    // 计算心率
    void calc_heart_rate(QList<double> array, int len, double distance, int *mid_dist, double *ratio, double *hr_peak);

    // 计算周期和峰峰值
    void calc_fs_and_ptp(QList<double> array, int len, double distance, double *fs, double *ptp);

    // findPeaks
    void findPeaks(QList<double> src, double src_lenth, double distance, int *indMax, int *indMax_len, int *indMin, int *indMin_len);

    //解析传感器接收到数据
    void parseSensorData(QByteArray recvData);
signals:
    // 刷新界面显示图形
    void sig_update_graph_replot(bool en);

    // 输出检测结果
    void sig_output_detection_result(int result_L, int result_R, TestResult L_tr, TestResult R_tr);

    // 输出噪声检测结果
    void sig_output_noise_result(int result_L, double noise_L, int result_R, double noise_R);


public slots:
    void read_com_data_L(double hr, double br);
    void read_com_data_R(double hr, double br);
    // 信号测试
    void sensor_signal_detection(QString com);
    void stop_sensor_signal_detection();
    void serial_readup1();
    void serial_readup();

    bool save_data_into_file(QString fileName,QByteArray heartData,QByteArray breathData,QString startTime, QString stopTime); // 在线程中写数据到文件

private:
    QMediaPlayer aduio;



    QList<double> com_data_L;
    int sample_points_L = 0;
    bool is_sample_finish_L = false;

    QList<double> com_data_R;
    int sample_points_R = 0;
    bool is_sample_finish_R = false;

    Full_Product_Param m_dp;
    Device_Attribute d_com;

    int time_index = 0;
    QFile serial_txt;
    QSerialPort *my_serialport = nullptr;
    QByteArray requestData;
    QByteArray heartData;
    QByteArray breathData;
//    QByteArray timeData;
    QStringList timeData;
};

#endif // SIGNALPROCESSING_H
