#ifndef SERIALCOMMUNICATION_H
#define SERIALCOMMUNICATION_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

class SerialCommunication : public QObject
{
    Q_OBJECT
public:
    explicit SerialCommunication(QObject *parent = nullptr);
    ~SerialCommunication();

    // crc8校验
    unsigned char crc8_chk_value(unsigned char *message, int len);
    // 读取PCB-ID
    QString read_pcb_id(QString com);
    // 读取PCB-ID 二代传感器
    QString read_pcb_id_second(QString com);
    // 读取软件版本
    QString read_pcb_sw_version(QString com);
    // 读取软件版本 二代传感器
    QString read_pcb_sw_version_second(QString com);
    // pcb串口通信函数
    QString pcb_serial_WriteRead(QString com, QString cmd);
    QString pcb_serial_WriteRead_second(QString com, QByteArray cmd);

signals:
    // 返回pcb的id和软件版本
    void sig_pcb_info(QString id, QString version, QString error_msg);

public slots:
    // 查询pcb的id和软件版本
    void read_pcb_id_and_version(QString com);
    //查询二代pcb的id和软件版本
    void read_pcb_id_and_version_second(QString com);

private:
    QSerialPort *my_serialport;
    QByteArray  requestData;
};

#endif // SERIALCOMMUNICATION_H
