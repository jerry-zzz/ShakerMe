#ifndef MODBUS_H
#define MODBUS_H

#include <QObject>

class ModBus : public QObject
{
    Q_OBJECT
public:  
    explicit ModBus(QObject *parent = nullptr);
    //CRC16校验
    unsigned int GetCRC16(unsigned char *ptr,  unsigned char len , char &h_crch ,char &l_crcl);
    //QString字符串转16进制
    QByteArray QString2Hex(QString str);
    char ConvertHexChar(char ch);
    //上位机 发送命令解析
    QByteArray ModBus_command_analyze(QByteArray command);
    //下位机 接收命令解析
    bool ModBus_reply_analyze(QByteArray reply);
    //解析传感器返回指令
    double parseSensorReply(QByteArray reply);
private:
    //上位机
    QByteArray Device_Address_S =nullptr;//设备地址
    QByteArray Function_Code_S;//功能码
    QByteArray Register_Address_S;//寄存器起始地址[高8位][低8位]
    QByteArray Read_Data_Length_S;//读取数据长度
    QByteArray Command_CRC;//CRC16校验码

    //下位机
    QByteArray Device_Address_R;//设备地址
    QByteArray Function_Code_R;//功能码
    int Data_Bytes_Number;//数据字节数
    QByteArray Data_Reply;//返回数据
    QByteArray Reply_CRC;//CRC16校验码
};

#endif // MODBUS_H
