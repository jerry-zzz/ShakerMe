#include "weighing_sensor.h"
#include "qdebug.h"
#include <QMessageBox>
#include <QTime>

WeighingSensor::WeighingSensor(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QVector<Device_Attribute>>("QVector<Device_Attribute>");  // 注册
    mb = new ModBus();//new modbus类
    timer = new QTimer;
    connect(timer, &QTimer::timeout, this, &WeighingSensor::read_current_numeric_command);
    serial = new QSerialPort;
    connect(serial,SIGNAL(readyRead()),this,SLOT(serial_read_all()));
}
//更新当前串口信息
void WeighingSensor::slot_update_device_param(QVector<Device_Attribute> device_list)
{
    serial_com = device_list.at(2).com;
    serial->setPortName(serial_com);
    serial->setBaudRate(19200);
}
//更新当前串口信息
void WeighingSensor::update_com_imformation(QString com)
{
    qDebug()<<com<<"更新当前串口信息";
    serial_com = com;
    serial->setPortName(serial_com);
    serial->setBaudRate(19200);
}
//打开串口
bool WeighingSensor::open_serialport()
{
    if(!serial->open(QIODevice::ReadWrite))
    {
        qDebug()<<"false";
        return false;
    //        QMessageBox::information(NULL, "提示","串口打开失败!");
    }
    else
    {
        qDebug()<<"success";
        return true;
    }
}
//关闭串口
void WeighingSensor::close_serialport()
{
    serial->close();
    qDebug()<<"close_serialport";
}
//打开定时器
void WeighingSensor::open_timer()
{
    timer->start(500);
}
//关闭定时器
void WeighingSensor::close_timer()
{
    timer->stop();
}
//等待接收应答
void WeighingSensor::waitfor_reply()
{
    int count = 0;
    while(!send_flag)
    {
        QThread::msleep(100);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        count++;
        if(count>10)
        {
            qDebug()<<"REPLY FAIL";
//            QMessageBox::information(NULL, "提示","等待接收应答失败，请检查串口是否打开!");
            send_flag = false;
            return;
        }
    }
}

void WeighingSensor::slot_read_current_numeric_command()
{
    read_current_numeric_command();
}
//读取当前数值命令
bool WeighingSensor::read_current_numeric_command()
{
    qDebug()<<"读取当前数值命令";
    //double result = index++ ;
    //emit send_result(result);
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x03;
    command[2] = 0x9C;
    command[3] = 0x40;
    command[4] = 0x00;
    command[5] = 0x02;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();
    if(send_flag == true)
    {
        if(mb->ModBus_reply_analyze(reply_all))
        {
            double result = mb->parseSensorReply(reply_all);
            emit send_result(result);
            return true;
        }
        else
            return false;
    }
    else
        return false;
}
//当前重量清零
bool WeighingSensor::clear_current_weight()
{
    qDebug()<<"当前重量清零";
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x06;
    command[3] = 0x2A;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;
    command[9] = 0x00;
    command[10] = 0x01;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<reply_all;
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//砝码校准方法
bool WeighingSensor::weight_calibration_method(int farmar_weight)
{
    QByteArray command_write;
    command_write[0] = 0x01;
    command_write[1] = 0x10;
    command_write[2] = 0x06;
    command_write[3] = 0x28;
    command_write[4] = 0x00;
    command_write[5] = 0x02;
    command_write[6] = 0x04;
    command_write[7] = 0x00;
    command_write[8] = 0x00;

    QString str =  QString("%1").arg(farmar_weight,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray farmarweight = mb->QString2Hex(str);
    command_write=command_write+farmarweight;
    QByteArray command_write_all = mb->ModBus_command_analyze(command_write);
    reply_all.clear();
    send_flag = false;
    serial->write(command_write_all);
    this->waitfor_reply();

    qDebug()<<"加载重量保持稳定后，向寄存器地址写入标定重量:"<<"\n"<<reply_all;
    if(mb->ModBus_reply_analyze(reply_all))
    {
        QByteArray command_check;
        command_check[0] = 0x01;
        command_check[1] = 0x10;
        command_check[2] = 0x06;
        command_check[3] = 0x2A;
        command_check[4] = 0x00;
        command_check[5] = 0x02;
        command_check[6] = 0x04;
        command_check[7] = 0x00;
        command_check[8] = 0x00;
        command_check[9] = 0x00;
        command_check[10] = 0x0B;

        QByteArray command_check_all = mb->ModBus_command_analyze(command_check);
        reply_all.clear();
        send_flag = false;
        serial->write(command_check_all);
        this->waitfor_reply();

        qDebug()<<"确认输入的标定重量有效:"<<"\n"<<reply_all;
        if(mb->ModBus_reply_analyze(reply_all))
            return true;
        else
            return false;
    }
    else
        return false;
}
//数字校准方法
bool WeighingSensor::digital_calibration_method(int sensor_sensitivity,int sensor_range)
{
    qDebug()<<"数字校准方法";
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x7C;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(sensor_sensitivity,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray sensorsensitivity = mb->QString2Hex(str);
    command=command+sensorsensitivity;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<"输入传感器传感器灵敏度："<<"\n"<<sensor_sensitivity;
    if(mb->ModBus_reply_analyze(reply_all))
    {
        QByteArray command_write;
        command_write[0] = 0x01;
        command_write[1] = 0x10;
        command_write[2] = 0x9C;
        command_write[3] = 0x82;
        command_write[4] = 0x00;
        command_write[5] = 0x02;
        command_write[6] = 0x04;
        command_write[7] = 0x00;
        command_write[8] = 0x00;

        QString str_1 =  QString("%1").arg(sensor_range,4,16,QLatin1Char('0'));//k为int型或char型都可
        QByteArray sensorrange = mb->QString2Hex(str_1);
        command_write=command_write+sensorrange;

        QByteArray command_write_all = mb->ModBus_command_analyze(command_write);
        reply_all.clear();
        send_flag = false;
        serial->write(command_write_all);
        this->waitfor_reply();

        qDebug()<<"输入传感器量程："<<"\n"<<sensor_range;
        if(mb->ModBus_reply_analyze(reply_all))
        {
            QByteArray command_check;
            command_check[0] = 0x01;
            command_check[1] = 0x10;
            command_check[2] = 0x06;
            command_check[3] = 0x2A;
            command_check[4] = 0x00;
            command_check[5] = 0x02;
            command_check[6] = 0x04;
            command_check[7] = 0x00;
            command_check[8] = 0x00;
            command_check[9] = 0x00;
            command_check[10] = 0x0A;

            QByteArray command_check_all = mb->ModBus_command_analyze(command_check);
            reply_all.clear();
            send_flag = false;
            serial->write(command_check_all);
            this->waitfor_reply();

            qDebug()<<"确认输入的信息有效；"<<"\n";
            if(mb->ModBus_reply_analyze(reply_all))
                return true;
            else
                return false;
        }
        else
            return false;
            qDebug()<<"输入传感器量程crc16校验错误:";
    }
     else
        return false;
        qDebug()<<"输入传感器传感器灵敏度crc16校验错误:";
}
//修改小数点
bool WeighingSensor::change_float(int decimal_point)
{
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x64;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(decimal_point,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray decimalpoint = mb->QString2Hex(str);
    command=command+decimalpoint;
qDebug()<<command<<decimalpoint;
    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<" 修改小数点至:"<<decimal_point<<"位";
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//清零触发门限设置
bool WeighingSensor::zero_trigger_threshold_set(int zero_trigger_threshold)
{
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x60;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(zero_trigger_threshold,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray zerotriggerthreshold = mb->QString2Hex(str);
    command=command+zerotriggerthreshold;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<" 清零触发门限:"<<zerotriggerthreshold;
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//自动清零设置
bool WeighingSensor::auto_zero_set(int auto_zero)
{
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x5E;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(auto_zero,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray autozero = mb->QString2Hex(str);
    command=command+autozero;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<" 自动清零设置:"<<autozero;
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//量程系数设置
bool WeighingSensor::range_coefficient_set(int range_coefficient)
{
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x7A;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(range_coefficient,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray rangecoefficient = mb->QString2Hex(str);
    command=command+rangecoefficient;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<" 量程系数设置:"<<rangecoefficient;
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//上电清零范围设置
bool WeighingSensor::zero_reset_range_set(int zero_reset_range)
{
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x54;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(zero_reset_range,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray zeroresetrange = mb->QString2Hex(str);
    command=command+zeroresetrange;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<" 上电清零范围:"<<zeroresetrange;
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//数字滤波设置
bool WeighingSensor::digital_filter_set(int digital_filter)
{
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x54;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(digital_filter,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray digitalfilter = mb->QString2Hex(str);
    command=command+digitalfilter;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<" 数字滤波:"<<digitalfilter;
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//额定量程设置
bool WeighingSensor::rated_range_set(int rated_range)
{
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x68;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(rated_range,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray ratedrange = mb->QString2Hex(str);
    command=command+ratedrange;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<" 额定量程:"<<ratedrange;
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//自动校准设置
bool WeighingSensor::auto_calibration_set(int auto_calibration)
{
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x84;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(auto_calibration,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray autocalibration = mb->QString2Hex(str);
    command=command+autocalibration;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<" 自动校准:"<<autocalibration;
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//判稳范围设置
bool WeighingSensor::sentenced_stabilizing_range_set(int sentenced_stabilizing_range)
{
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x56;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(sentenced_stabilizing_range,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray sentencedstabilizingrange = mb->QString2Hex(str);
    command=command+sentencedstabilizingrange;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<" 判稳范围:"<<sentencedstabilizingrange;
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//判稳周期设置
bool WeighingSensor::sentenced_stabilizing_t_set(int sentenced_stabilizing_t)
{
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x58;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(sentenced_stabilizing_t,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray sentencedstabilizingt = mb->QString2Hex(str);
    command=command+sentencedstabilizingt;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<" 判稳周期:"<<sentencedstabilizingt;
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//通讯机码设置
bool WeighingSensor::machine_code_set(int machine_code)
{
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x6E;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(machine_code,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray machinecode = mb->QString2Hex(str);
    command=command+machinecode;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<" 通讯机码:"<<machinecode;
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//发送包间隔设置
bool WeighingSensor::send_interval_set(int send_interval)
{
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x76;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(send_interval,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray sendinterval = mb->QString2Hex(str);
    command=command+sendinterval;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<" 发送包间隔:"<<sendinterval;
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//零位跟踪范围设置
bool WeighingSensor::zero_tracking_range_set(int zero_tracking_range)
{
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x5A;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(zero_tracking_range,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray zerotrackingrange = mb->QString2Hex(str);
    command=command+zerotrackingrange;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<" 零位跟踪范围:"<<zerotrackingrange;
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//清零延时设置
bool WeighingSensor::auto_zero_delay_set(int auto_zero_delay)
{
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x62;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(auto_zero_delay,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray autozerodelay = mb->QString2Hex(str);
    command=command+autozerodelay;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<" 清零延时:"<<autozerodelay;
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//采样速率设置
bool WeighingSensor::sample_rate_set(int sample_rate)
{
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x6A;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(sample_rate,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray samplerate = mb->QString2Hex(str);
    command=command+samplerate;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<" 采样速率:"<<samplerate;
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//通讯协议设置
bool WeighingSensor::communication_protocol_set(int communication_protocol)
{
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x6E;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(communication_protocol,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray communicationprotocol = mb->QString2Hex(str);
    command=command+communicationprotocol;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<" 通讯协议:"<<communicationprotocol;
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//数据格式设置
bool WeighingSensor::data_format_set(int data_format)
{
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x70;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(data_format,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray dataformat = mb->QString2Hex(str);
    command=command+dataformat;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<" 数据格式:"<<dataformat;
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//波特率设置
bool WeighingSensor::baud_rate_set(int baud_rate)
{
    QByteArray command;
    command[0] = 0x01;
    command[1] = 0x10;
    command[2] = 0x9C;
    command[3] = 0x72;
    command[4] = 0x00;
    command[5] = 0x02;
    command[6] = 0x04;
    command[7] = 0x00;
    command[8] = 0x00;

    QString str =  QString("%1").arg(baud_rate,4,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray baudrate = mb->QString2Hex(str);
    command=command+baudrate;

    QByteArray command_all = mb->ModBus_command_analyze(command);
    reply_all.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    qDebug()<<" 波特率:"<<baudrate;
    if(mb->ModBus_reply_analyze(reply_all))
        return true;
    else
        return false;
}
//读取readready信号槽函数
void WeighingSensor::serial_read_all()
{
    qDebug()<<"coming in";
    this->reply_all += serial->readAll();
    send_flag = true;
}
