#include "laserranging.h"
#include<qdebug.h>
#include<QMessageBox>

LaserRanging::LaserRanging(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QVector<Device_Attribute>>("QVector<Device_Attribute>");  // 注册
    timer = new QTimer;
    connect(timer, &QTimer::timeout, this, &LaserRanging::channel_querl);
    serial = new QSerialPort;
    connect(serial,SIGNAL(readyRead()),this,SLOT(serial_read_all()));
}
//更新当前串口信息
void LaserRanging::slot_update_device_param(QVector<Device_Attribute> device_list)
{
    serial_com = device_list.at(3).com;
    serial->setPortName(serial_com);
    serial->setBaudRate(115200);
}
//更新当前串口信息
void LaserRanging::update_com_imformation(QString com)
{
    qDebug()<<com<<"更新当前串口信息";
    serial_com = com;
    serial->setPortName(serial_com);
    serial->setBaudRate(115200);
}
//打开串口
bool LaserRanging::open_serialport()
{
    if(!serial->open(QIODevice::ReadWrite))
    {
        qDebug()<<"false";
        return false;
    }
    else
    {
        qDebug()<<"success";
        return true;
    }
}
//关闭串口
void LaserRanging::close_serialport()
{
    serial->close();
    qDebug()<<"close_serialport";
}
//打开定时器
void LaserRanging::open_timer()
{
    timer->start(500);
}
//关闭定时器
void LaserRanging::close_timer()
{
    timer->stop();
}
//等待接收应答
void LaserRanging::waitfor_reply()
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
            //QMessageBox::information(NULL, "提示","等待接收应答失败，请检查串口是否打开!");
            send_flag = false;
            return;
        }
    }
}
//更新面板参数
QByteArray LaserRanging::update_lr_prarm(QString filter_samples,QString gain_pga,int collector_ID,QString choose_singel_channel)
{
    bool ok;
    filtersamples_str =filter_samples;
    gainpga_str = gain_pga;
    collectorID = collector_ID;
    channel_mdoe = choose_singel_channel;
    int filtersamples = filtersamples_str.toInt(&ok,10);
    int gainpga = gainpga_str.toInt(&ok,10);
    AD_REF = gainpga;//增益
    QString str =  QString("%1").arg(collectorID,2,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray command_id = mb->QString2Hex(str);
    QByteArray command_middle;
    if(filtersamples == 1){command_middle[0] = 0xF0;}
    else if (filtersamples == 2) {command_middle[0] = 0xE0;}
    else if (filtersamples == 4) {command_middle[0] = 0xd0;}
    else if (filtersamples == 8) {command_middle[0] = 0xC0;}
    else if (filtersamples == 15) {command_middle[0] = 0xB0;}
    else if (filtersamples == 30) {command_middle[0] = 0xA1;}
    else if (filtersamples == 60) {command_middle[0] = 0x92;}
    else if (filtersamples == 300) {command_middle[0] = 0x82;}
    else if (filtersamples == 500) {command_middle[0] = 0x72;}
    else if (filtersamples == 600) {command_middle[0] = 0x63;}
    else if (filtersamples == 1000) {command_middle[0] = 0x53;}
    else if (filtersamples == 1200) {command_middle[0] = 0x43;}
    else if (filtersamples == 2000) {command_middle[0] = 0x33;}
    else if (filtersamples == 3000) {command_middle[0] = 0x13;}
    else if (filtersamples == 12000) {command_middle[0] = 0x03;}

    if(gainpga == 1){command_middle[1] = 0x00;}
    else if (gainpga == 2) {command_middle[1] = 0x01;}
    else if (gainpga == 4) {command_middle[1] = 0x02;}
    else if (gainpga == 8) {command_middle[1] = 0x03;}
    else if (gainpga == 16) {command_middle[1] = 0x04;}
    else if (gainpga == 32) {command_middle[1] = 0x05;}
    else if (gainpga == 64) {command_middle[1] = 0x06;}

    return command_id + command_middle;
}
//通道实际电压值计算
double LaserRanging::voltage_calculation(QByteArray reply_channel)
{
    bool ok;
    int AD_VALUE = 0;
    AD_VALUE = reply_channel.toHex().toInt(&ok, 16);
    double OUT = 0.0 ;
    if(AD_VALUE>=8388607)
    {
        OUT=((double)AD_RESOLUTION-AD_VALUE)/AD_RESOLUTION/AD_REF;
        OUT=OUT*(-1)*10;
    }
    else
    {
        OUT=((double)AD_VALUE/AD_RESOLUTION)/AD_REF*10;
    }
    return OUT;
}

//双通道查询
void LaserRanging::dual_channel_querl()
{
    qDebug()<<"双通道查询命令";
    QByteArray command_begin,command_end,command_middle,command_all;
    command_begin[0] = 0xAA;
    command_begin[1] = 0x09;
    command_end[0] = 0x00;
    command_end[1] = 0xBB;

    command_middle = update_lr_prarm(filtersamples_str,gainpga_str,collectorID,channel_mdoe);
    command_all = command_begin + command_middle +  command_end;

    reply.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    if(send_flag==true)
    {
        double out_1 = 0.0;
        double out_2 = 0.0;
        QByteArray reply_one=reply.mid(0,3);
        QByteArray reply_two=reply.mid(3,3);
        out_1 = voltage_calculation(reply_one);//一通道电压值
        out_2 = voltage_calculation(reply_two);//二通道电压值
        qDebug()<<"一通道电压值: "<<out_1<< "二通道电压值: "<<out_2;
        emit send_result(out_1,out_2);
    }
}
//双通道触发模式
void LaserRanging::dual_channel_trigger_mode()
{
    qDebug()<<"双通道触发模式";
    QByteArray command_begin,command_end,command_middle,command_all;
    command_begin[0] = 0xAA;
    command_begin[1] = 0xC1;
    command_end[0] = 0x00;
    command_end[1] = 0xBB;

    command_middle = update_lr_prarm(filtersamples_str,gainpga_str,collectorID,channel_mdoe);
    command_all = command_begin + command_middle +  command_end;

    reply.clear();
    send_flag = false;
    serial->write(command_all);
    QThread::msleep(100);
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    this->interrupt_instruction();//中断指令发送
    this->waitfor_reply();

    double out_1 = 0.0;
    double out_2 = 0.0;
    QByteArray reply_one=reply.mid(0,3);
    QByteArray reply_two=reply.mid(3,3);
    out_1 = voltage_calculation(reply_one);//一通道电压值
    out_2 = voltage_calculation(reply_two);//二通道电压值
    qDebug()<<"一通道电压值: "<<out_1<< "二通道电压值: "<<out_2;
    emit send_result(out_1,out_2);
}
//单通道查询
void LaserRanging::channel_querl()
{
    qDebug()<<"单通道查询命令";
    bool ok;
    QString channel_str =  QString("%1").arg(channel_mdoe.toInt(&ok,10),2,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray command_channel = mb->QString2Hex(channel_str);
    QByteArray command_begin,command_end,command_middle,command_all;
    command_begin[0] = 0xAA;
    command_end[0] = 0x00;
    command_end[1] = 0xBB;

    command_middle = update_lr_prarm(filtersamples_str,gainpga_str,collectorID,channel_mdoe);
    command_all = command_begin + command_channel + command_middle +  command_end;

    reply.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    int channel = channel_mdoe.toInt(&ok,10);
    if(send_flag==true)
    {
        double out = 0.0;
        out = voltage_calculation(reply);//通道电压值
        if(channel == 1)
            emit send_result(out,0);
        else if(channel == 2)
            emit send_result(0,out);
    }
}
//单通道触发模式
void LaserRanging::channel_trigger_mode()
{
    qDebug()<<"单通道触发模式";
    bool ok;
    int channel = channel_mdoe.toInt(&ok,10);
    QByteArray command_begin,command_end,command_middle,command_all,command_channel;
    if(channel == 1)
        command_channel[0]=0xE1;
    else if(channel == 2)
        command_channel[0]=0xE2;
    command_begin[0] = 0xAA;
    command_end[0] = 0x00;
    command_end[1] = 0xBB;

    command_middle = update_lr_prarm(filtersamples_str,gainpga_str,collectorID,channel_mdoe);
    command_all = command_begin + command_channel + command_middle +  command_end;

    reply.clear();
    send_flag = false;
    serial->write(command_all);
    QThread::msleep(100);
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    this->interrupt_instruction();//中断指令发送
    this->waitfor_reply();

    if(send_flag==true)
    {
        double out = 0.0;
        out = voltage_calculation(reply);//通道电压值
        if(channel == 1)
            emit send_result(out,0);
        else if(channel == 2)
            emit send_result(0,out);
    }
}
//写采集器ID
bool LaserRanging::write_collector_ID(int collector_ID)
{
    qDebug()<<"写采集器ID";
    QByteArray command_begin,command_end,command_all;
    command_begin[0] = 0xAA;
    command_begin[1] = 0x99;//写操作
    command_begin[2] = 0xff;//跳过原始ID
    command_begin[3] = 0x00;

    command_end[0] = 0x00;
    command_end[1] = 0xBB;
    QString str =  QString("%1").arg(collector_ID,2,16,QLatin1Char('0'));//k为int型或char型都可
    QByteArray command_middle = mb->QString2Hex(str);
    command_all = command_begin + command_middle +  command_end;

    reply.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    if(send_flag==true)
    {
        if(reply == command_middle)
            return true;
        else
            return false;
    }
    else
        return false;
}
//读采集器ID
void LaserRanging::read_collector_ID()
{
    qDebug()<<"读采集器ID";
    QByteArray command_begin,command_end,command_all;
    command_begin[0] = 0xAA;
    command_begin[1] = 0x88;//读操作
    command_begin[2] = 0xff;//跳过原始ID
    command_begin[3] = 0x00;//无定义
    command_begin[4] = 0x00;//无定义

    command_end[0] = 0x00;
    command_end[1] = 0xBB;
    command_all = command_begin + command_end;

    reply.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    if(send_flag==true)
    {
        bool ok;
        emit result_id(reply.toHex().toInt(&ok,10));
    }
}
//写采集器波特率
bool LaserRanging::write_collector_baudrate(QString baudrate)
{
    bool ok;
    int baud_rate=baudrate.toInt(&ok,10);
    qDebug()<<"写采集器波特率";
    QByteArray command_begin,command_end,command_all;
    command_begin[0] = 0xAA;
    command_begin[1] = 0x77;//写操作
    command_begin[2] = 0xff;//跳过原始ID
    command_begin[3] = 0x00;//无定义

    if(baud_rate == 115200){command_begin[4]=0xff;}
    else if(baud_rate == 57600){command_begin[4]=0x02;}
    else if(baud_rate == 38400){command_begin[4]=0x03;}
    else if(baud_rate == 19200){command_begin[4]=0x04;}
    else if(baud_rate == 9600){command_begin[4]=0x05;}
    else if(baud_rate == 4800){command_begin[4]=0x06;}//波特率

    command_end[0] = 0x01;
    command_end[1] = 0xBB;
    command_all = command_begin + command_end;

    reply.clear();
    send_flag = false;
    serial->write(command_all);
    this->waitfor_reply();

    if(send_flag==true)
    {
        if(reply == command_begin.right(1))
            return true;
        else
            return false;
    }
    else
        return false;
}
//中断指令
void LaserRanging::interrupt_instruction()
{
    qDebug()<<"中断指令！";
    QByteArray command_trigger;
    command_trigger[0] = 0xAA;
    command_trigger[1] = 0x00;
    command_trigger[2] = 0x00;
    command_trigger[3] = 0x00;
    command_trigger[4] = 0x00;
    command_trigger[5] = 0x00;
    command_trigger[6] = 0xBB;
    serial->write(command_trigger);
}
//读取readready信号槽函数
void LaserRanging::serial_read_all()
{
    qDebug()<<"coming in";
    this->reply += serial->readAll();
    send_flag = true;
}
