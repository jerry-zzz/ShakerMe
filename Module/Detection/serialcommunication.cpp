#include "serialcommunication.h"
#include <QDebug>
#include <QTime>
#include <QThread>

SerialCommunication::SerialCommunication(QObject *parent) : QObject(parent)
{
    my_serialport = nullptr;
}

SerialCommunication::~SerialCommunication()
{
    delete my_serialport;
}
//查询二代pcb的id和软件版本
void SerialCommunication::read_pcb_id_and_version_second(QString com)
{
    QString emsg;
    bool is_time_out = true;        // 超时.
    QString read_id;
    QString read_sw_version;

    // 读取PCB-ID
    is_time_out = true;
    QThread::msleep(250);
    for(int i=0;i<2;i++)
    {
        read_id = read_pcb_id_second(com);
        if( read_id.isEmpty())
            continue;
        else if(read_id.size() == 20)
        {
            is_time_out = false;
            break;
        }
    }
    if(is_time_out)
    {
        read_id.clear();
        emsg = QString("读取PCB-ID错误!");
    }
    // 读取软件版本
    is_time_out = true;
    QThread::msleep(250);
    for(int i=0;i<2;i++)
    {
        read_sw_version = read_pcb_sw_version_second(com);        
       if(read_sw_version.isEmpty())
           continue;
       else if(read_sw_version.indexOf("PCBA")>=0)
           continue;
       else
       {
           is_time_out = false;
           break;
       }
    }

    if(is_time_out)
    {
        read_sw_version.clear();
        emsg = emsg + QString("读取软件版本错误!");
    }

    if(my_serialport->isOpen())
        my_serialport->close();
    emit sig_pcb_info(read_id, read_sw_version, emsg);
}
// 查询pcb的id和软件版本
void SerialCommunication::read_pcb_id_and_version(QString com)
{
    QString emsg;
    bool is_time_out = true;        // 超时.
    QString read_id;
    QString read_sw_version;

    // 读取PCB-ID
    is_time_out = true;
    for(int i=0;i<2;i++)
    {
        read_id = read_pcb_id(com);
        if(read_id.indexOf("Empty") >= 0 || read_id.indexOf("cmd") >= 0 || read_id.indexOf("CMD") >= 0 || read_id.indexOf("ER") >= 0 || read_id.isEmpty())
            continue;
        else if(read_id.size() == 20)
        {
            is_time_out = false;
            break;
        }
    }

    if(is_time_out)
    {
        read_id.clear();
        emsg = QString("读取PCB-ID错误!");
    }
    // 读取软件版本
    is_time_out = true;
    for(int i=0;i<2;i++)
    {
        read_sw_version = read_pcb_sw_version(com);
        if(read_sw_version.indexOf("cmd") >= 0 || read_sw_version.indexOf("CMD") >= 0 || read_sw_version.indexOf("ER") >= 0 || read_sw_version.isEmpty())
            continue;
        else
        {
            is_time_out = false;
            break;
        }
    }

    if(is_time_out)
    {
        read_sw_version.clear();
        emsg = emsg + QString("读取软件版本错误!");
    }

    if(my_serialport->isOpen())
        my_serialport->close();
    emit sig_pcb_info(read_id, read_sw_version, emsg);
}

// crc8校验
unsigned char SerialCommunication::crc8_chk_value(unsigned char *message, int len)
{
    unsigned char crc;
    unsigned char i;
    crc = 0;
    while(len--)
    {
        crc ^= *message++;
        for(i = 0;i < 8;i++)
        {
            if(crc & 0x80)
            {
                crc = (crc << 1) ^ 0x31;
            }
            else crc <<= 1;
        }
    }
    crc = crc&0x00ff;
    return crc;
}


// 读取PCB-ID
QString SerialCommunication::read_pcb_id(QString com)
{
    QString rinfo;
    QString cmd = QString("QH+HID=?");
    try
    {
        rinfo = pcb_serial_WriteRead(com, cmd);
    }
    catch(const std::runtime_error& e)
    {
        rinfo = "";        
    }
    return rinfo;
}
// 读取PCB-ID 二代传感器
QString SerialCommunication::read_pcb_id_second(QString com)
{
    QString rinfo;
    //AA 03 00 00 00 00 FF 查询硬件版本
    QByteArray command;//查询硬件版本命令
    command[0] = 0xAA;
    command[1] = 0x03;
    command[2] = 0x00;
    command[3] = 0x01;
    command[4] = 0x00;
    command[5] = 0xFF;
    try
    {
        rinfo = pcb_serial_WriteRead_second(com, command);
    }
    catch(const std::runtime_error& e)
    {
        rinfo = "";
    }
    return rinfo;
}
// 读取软件版本
QString SerialCommunication::read_pcb_sw_version(QString com)
{
    QString rinfo;
    QString cmd = QString("QH+SID=?");
    try
    {
        rinfo = pcb_serial_WriteRead(com, cmd);
    }
    catch(const std::runtime_error& e)
    {
        rinfo = "";
    }

    return rinfo;
}

// 读取软件版本 二代传感器
QString SerialCommunication::read_pcb_sw_version_second(QString com)
{
    QString rinfo;
    QByteArray command;//查询传感器软件版本命令
    command[0] = 0xAA;
    command[1] = 0x02;
    command[2] = 0x00;
    command[3] = 0x01;
    command[4] = 0x00;
    command[5] = 0xFF;
    try
    {
        rinfo = pcb_serial_WriteRead_second(com, command);
    }
    catch(const std::runtime_error& e)
    {
        rinfo = "";
    }

    return rinfo;
}
// pcb串口通信函数
// com:串口
// cmd:发送的指令
// return:返回的信息
QString SerialCommunication::pcb_serial_WriteRead(QString com, QString cmd)
{
    QString info;
    unsigned char check_code = crc8_chk_value((unsigned char*)(cmd.toLatin1().data()), cmd.size());
    QString tmp_cmd = cmd + QString(check_code) + QString(0x0d);
    // 配置串口
    if(my_serialport == nullptr)
        my_serialport = new QSerialPort();
    my_serialport->setPortName(com);
    my_serialport->setBaudRate(9600);

    if(my_serialport->isOpen())
    {
        my_serialport->close();
        QThread::msleep(200);
    }
    if(!my_serialport->open(QIODevice::ReadWrite))
    {       
        my_serialport->close();
        QString estr = QString("写入PCB-ID:串口 %1 打开失败:%2").arg(com).arg(my_serialport->errorString());

        throw std::runtime_error(estr.toStdString().data());
    }

    char redata[256]={0};
    my_serialport->write(tmp_cmd.toLatin1());
    my_serialport->waitForBytesWritten(1000);
    requestData.clear();
    int len = 0;
    do
    {
        if(!my_serialport->waitForReadyRead(200))
            break;
        len = my_serialport->read(redata, 50);
        QByteArray tmp(redata,len);
        requestData = requestData + tmp;
        if(requestData.indexOf('$') >= 0)
            break;
    }while(requestData.size() < 40);
    my_serialport->close();

    int index_s = requestData.indexOf('#');
    int index_e;
    if(index_s >= 0)
    {
        index_e = requestData.indexOf('$', index_s + 1);
        if(index_e >= 0)
        {
            QByteArray ck_code = requestData.mid(index_e - index_s-1, 1);       // 收到数据的校验码
            QByteArray tmp = requestData.mid(index_s+1, index_e - index_s-2);   // 收到的有效数据
            unsigned char tmp_code = crc8_chk_value((unsigned char*)(tmp.data()), tmp.size());  // 收到的有效数据的校验码
            QByteArray tmp_code_tmp;
            tmp_code_tmp.append(tmp_code);
            if(ck_code == tmp_code_tmp)
                info.append(tmp);
            else
            {
                qInfo() << "read serial: check sum error: " << tmp << tmp.toHex();
                throw std::runtime_error("串口数据校验码错误!");
            }
            if(tmp.indexOf("cmd") >= 0 || tmp.indexOf("CMD") >= 0 || tmp.indexOf("ER") >= 0)
                qInfo() << "cmd error: " << tmp << tmp.toHex();
        }
        else
        {
            qInfo() << "read serial: no stop bit: " << requestData << requestData.toHex();
            throw std::runtime_error("串口数据无结束符!");
        }
    }
    else
    {
        qInfo() << "read serial: no start bit: " << requestData << requestData.toHex();
        throw std::runtime_error("串口数据无起始符!");
    }
    return info;
}
// 二代传感器
// pcb串口通信函数
// com:串口
// cmd:发送的指令
// return:返回的信息
QString SerialCommunication::pcb_serial_WriteRead_second(QString com, QByteArray cmd)
{
    QString info;
    QByteArray command_begin , command_end;//帧头、帧尾
    command_begin[0] = 0xAA;
    command_end[0] = 0xFF;
    QByteArray cmd_byte = cmd.mid(1,cmd.size()-2);
    unsigned char check_code = crc8_chk_value((unsigned char*)(cmd_byte.data()), cmd_byte.size());
    QByteArray tmp_cmd =command_begin + cmd_byte + check_code + command_end;
    // 配置串口
    if(my_serialport == nullptr)
        my_serialport = new QSerialPort();
    my_serialport->setPortName(com);
    my_serialport->setBaudRate(115200);

    if(my_serialport->isOpen())
    {
        my_serialport->close();
        QThread::msleep(200);
    }
    if(!my_serialport->open(QIODevice::ReadWrite))
    {
        my_serialport->close();
        QString estr = QString("写入PCB-ID:串口 %1 打开失败:%2").arg(com).arg(my_serialport->errorString());

        throw std::runtime_error(estr.toStdString().data());
    }

    char redata[256]={0};
    my_serialport->write(tmp_cmd);
    my_serialport->waitForBytesWritten(1000);
    requestData.clear();
    int len = 0;
    do
    {
        if(!my_serialport->waitForReadyRead(200))
            break;
        len = my_serialport->read(redata, 100);
        QByteArray tmp(redata,len);
        requestData = requestData + tmp;
        if(requestData.indexOf('$') >= 0)
            break;
    }while(requestData.size() > 1);
    my_serialport->close();

    int index_s = requestData.indexOf('#');
    int index_e;
    if(index_s >= 0)
    {
        index_e = requestData.indexOf('$', index_s + 1);
        if(index_e >= 0)
        {
            QByteArray ck_code = requestData.mid(index_e - index_s-1, 1);       // 收到数据的校验码
            QByteArray tmp = requestData.mid(index_s+1, index_e - index_s-2);   // 收到的有效数据
            unsigned char tmp_code = crc8_chk_value((unsigned char*)(tmp.data()), tmp.size());  // 收到的有效数据的校验码
            QByteArray tmp_code_tmp;
            tmp_code_tmp.append(tmp_code);
            if(ck_code == tmp_code_tmp)
                info.append(tmp);
            else
            {
                throw std::runtime_error(QString("串口数据校验码错误:%1  %2").arg(QString(tmp)).arg(QString(tmp.toHex())).toStdString().data());
            }
        }
        else
        {
            throw std::runtime_error(QString("串口数据无结束符:%1  %2").arg(QString(requestData)).arg(QString(requestData.toHex())).toStdString().data());
        }
    }
    else
    {
        throw std::runtime_error(QString("串口数据无起始符:%1  %2").arg(QString(requestData)).arg(QString(requestData.toHex())).toStdString().data());
    }
    return info;
}
