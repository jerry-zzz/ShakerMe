/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    detection.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	检测系统的面板.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#include "detection.h"
#include "ui_detection.h"
#include "system_info.h"
#include "../Log/loginfo.h"
#include <QDebug>

Detection::Detection(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Detection)
{
    ui->setupUi(this);

    //设为无边框
    setWindowFlags(Qt::FramelessWindowHint);
    this->setGeometry(NESTED_PANEL_POS_X, NESTED_PANEL_POS_Y, NESTED_PANEL_POS_W, NESTED_PANEL_POS_H);

    timer = new QTimer;
    connect(timer, &QTimer::timeout, this, &Detection::slot_update_graph);
    //-- add your code ---------------------------------------------------------
    // 信号槽
    pcb_com = new SerialCommunication;
    connect(pcb_com, &SerialCommunication::sig_pcb_info, this, &Detection::slot_pcb_info);
    connect(this, &Detection::sig_read_pcb_id_and_version, pcb_com, &SerialCommunication::read_pcb_id_and_version);
    connect(this, &Detection::sig_read_pcb_id_and_version_second, pcb_com, &SerialCommunication::read_pcb_id_and_version_second);

    //new
    mSerialport = new QSerialPort;
    connect(mSerialport,SIGNAL(readyRead()),this,SLOT(serial_read_all()));

    //控件响应
    connect(ui->com, &QPushButton::clicked, this, &Detection::btn_serialport_clicked);
    connect(ui->test, &QPushButton::clicked, this, &Detection::pb_test);

    // 其他
    thread_sp = new QThread;
    sp = new SignalProcessing;
    //connect(this, &Detection::sig_output_com_data, sp, &SignalProcessing::read_com_data_L);
    connect(this, &Detection::sig_save_data_into_file,sp,&SignalProcessing::save_data_into_file);

    connect(sp, &SignalProcessing::sig_update_graph_replot, this, &Detection::slot_update_graph_replot);

    connect(this, &Detection::sig_detection_sensor_signal_detection, sp, &SignalProcessing::sensor_signal_detection);
    connect(this, &Detection::sig_detection_stop_sensor_signal_detection, sp, &SignalProcessing::stop_sensor_signal_detection);
    sp->moveToThread(thread_sp);
    thread_sp->start();

    //init graph
    heart_graph = ui->graph_L->addGraph();
    heart_graph->setPen(QPen(Qt::red));

    breath_graph = ui->graph_R->addGraph();
    breath_graph->setPen(QPen(Qt::blue));

    ui->graph_L->xAxis->setLabel("t");
    ui->graph_L->yAxis->setLabel("ad-h");
    ui->graph_L->xAxis->setRange(0, 10 * 1000);
    ui->graph_L->yAxis->setRange(-1, 1);
    ui->graph_L->setInteraction(QCP::iRangeDrag,true);
    ui->graph_L->setInteraction(QCP::iRangeZoom,true);
    ui->graph_L->axisRect()->setRangeZoomAxes(0, ui->graph_L->yAxis);
    ui->graph_L->setInteraction(QCP::iSelectPlottables,true);

    ui->graph_R->xAxis->setLabel("t");
    ui->graph_R->yAxis->setLabel("ad-b");
    ui->graph_R->xAxis->setRange(0, 10 * 1000);
    ui->graph_R->yAxis->setRange(-1, 1);
    ui->graph_R->setInteraction(QCP::iRangeDrag,true);
    ui->graph_R->setInteraction(QCP::iRangeZoom,true);
    ui->graph_R->axisRect()->setRangeZoomAxes(0, ui->graph_R->yAxis);
    ui->graph_R->setInteraction(QCP::iSelectPlottables,true);

    //设置滑动条控件的最小值
    ui->horizontalSlider->setMinimum(2);
    //设置滑动条控件的最大值
    ui->horizontalSlider->setMaximum(50);
    //设置滑动条控件的值
    ui->horizontalSlider->setValue(10);
    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setLineEditValue(int)));
    connect(ui->filter_set,&QPushButton::clicked, this,&Detection::filter_set);
}

Detection::~Detection()
{
    thread_sp->quit();
    delete ui;
}

void Detection::setLineEditValue(int value)
{
  int pos = ui->horizontalSlider->value();
  QString str = QString("%1").arg(pos);
  int str_i = str.toInt();
  ui->L_hr->setValue(str_i);
}
//设置滤波
void Detection::filter_set()
{
    if(ui->filter_set->text() == "开启滤波")
    {
        ui->filter_set->setText("关闭滤波");
        is_filter_flag = true;
    }
    else if(ui->filter_set->text() == "关闭滤波")
    {
        ui->filter_set->setText("开启滤波");
        is_filter_flag = false;
    }
}
/*
 *==============================================================================
 *   初始化模块/设置权限/界面显示/界面隐藏
 *==============================================================================
 */

// 初始化
void Detection::init_Detection()
{
    ;
}

// 显示界面
void Detection::display_panel()
{
    show();
    //-- add your code -----------------------------------------------------
    timer->start(50);  //画布刷新速度
}

// 隐藏界面
void Detection::hide_panel()
{
    hide();
}

// 日志信息
void Detection::log_info(QString msg)
{
#ifdef LOG_EN
    qInfo() << msg;
#endif
}

/*
 *==============================================================================
 *   界面模块功能函数
 *==============================================================================
 */


//传感器代数
void Detection::slot_sensor_generation(int sensor_generation)
{
    if(sensor_generation == 1)
        mSensorGeneration = 0;
    else if(sensor_generation == 2)
        mSensorGeneration = 1;
}


// 刷新界面显示图形
void Detection::slot_update_graph_replot(bool en)
{
    show_graph_EN = en;
}

void Detection::slot_update_device_param(QVector<Device_Attribute> device_list)
{
    m_com = device_list.at(0);
    m_com_second = device_list.at(1);
    qDebug()<<"current com:"<<m_com.com;
}

/*add your code*/
//-------------------------------------------------------------------------槽函数
// 刷新检测模块参数
void Detection::slot_update_product_param(Full_Product_Param dp)
{
    m_dp = dp;   
    sp->set_product_param(m_dp);
}

// 测试按钮  工频检测 信号检测
void Detection::pb_test()
{

    if(ui->en_L->checkState() != Qt::Checked)
        return;

//    dete_query_pcb_info(en_L);  //查询pcb版本

    if(!is_detection)
    {

        qDebug() << "test";

//        // 工频检测
//        start_noise_detection(en_L, en_R);
//        // 等待检测完成
//        while(1)
//        {
//            if(is_noise_detection_over)
//                break;
//            QThread::msleep(1);
//            QCoreApplication::processEvents(QEventLoop::AllEvents);
//        }

//        // 信号检测
//        start_signal_detection(en_L);
//        // 等待检测完成
//        while(1)
//        {
//            if(is_signal_detection_over)
//                break;
//            QThread::msleep(1);
//            QCoreApplication::processEvents(QEventLoop::AllEvents);
//        }
    }
}

// 查询pcb信息
void Detection::dete_query_pcb_info(int en)
{
    is_info_received = false;
    m_info_id.clear();
    m_info_version.clear();

//qDebug() << "1:" << QTime::currentTime().toString("hh:mm:ss.zzz");
    if(en)
    {
        is_info_received = false;
        if(mSensorGeneration == 0)
        {
            emit sig_read_pcb_id_and_version(QString().asprintf("%s",m_com.com));
        }
        else if(mSensorGeneration == 1)
        {
            emit sig_read_pcb_id_and_version_second(QString().asprintf("%s",m_com_second.com));
        }
        while(!is_info_received)
        {
            QThread::msleep(1);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
        m_info_id = tmp_id;
        m_info_version = tmp_version;
        ui->tmp_id->setText(m_info_id);
        ui->tmp_version->setText(m_info_version);
        qDebug()<<m_info_id<<m_info_version;
    }
 //   qDebug() << "2:" << QTime::currentTime().toString("hh:mm:ss.zzz");
    emit sig_output_pcb_info(m_info_id, m_info_version);
}

// 返回pcb的id和软件版本
void Detection::slot_pcb_info(QString id, QString version, QString error_msg)
{
    tmp_id = id;
    tmp_version = version;
    QString msg = error_msg;
    is_info_received = true;
//    if(!msg.isEmpty())
//        qInfo() << "pcb info:" << msg;
}


/***************重构代码******************/

// 串口开始/结束按钮响应函数-->麒盛传感器数据接收
void Detection::btn_serialport_clicked()
{
    bool cur_serialport_status = this->get_serialport_status();
    if(ui->com->text() == "打开串口" && !cur_serialport_status)
    {
        if(mSensorGeneration == 0)
        {
            if(!open_com(mSerialport, m_com.com, m_com.baud))
            {
                return;
            }
        }
        else if(mSensorGeneration == 1)
        {
            if(!open_com(mSerialport, m_com_second.com, m_com_second.baud))
            {
                return;
            }
        }

        show_graph_EN = true;
        ui->com->setText("关闭串口");
        ui->com->setStyleSheet("QPushButton{border-radius:10px;background-color:rgb(92, 163, 99);font:75 12pt \"微软雅黑\";color: white;}");
        this->set_serialport_status(true);  // 设置为运行
        this->init_sensor_data_buff();
    }
    else if(ui->com->text() == "关闭串口")
    {
        show_graph_EN = false;
        close_com(mSerialport);
        this->set_serialport_status(false); // 设置为停止
        ui->com->setText("打开串口");
        ui->com->setStyleSheet("QPushButton{border-radius:10px;background-color:rgb(250, 50, 50);font:75 12pt \"微软雅黑\";color: white;}");
        QDateTime current_date_time =QDateTime::currentDateTime();
        QString current_date =current_date_time.toString("yyyy.MM.dd-hh.mm.ss");
        QString filename = "DECT/dect" + current_date ;
        emit sig_save_data_into_file(filename,this->heartData, this->breathData,this->mSerialStartTime,this->mSerialStopTime);
    }

}

// 清空存放串口数据的缓存区
void Detection::init_sensor_data_buff()
{
    emit sig_clear_serial_data();
    this->requestData.clear();
    this->heartData.clear();
    this->breathData.clear();
    // 清空graph中的数据
    this->graph_index = 0;
    this->heart_graph->data().data()->clear();
    this->breath_graph->data().data()->clear();
}



//当前传感器代数
void Detection::set_sensor_generation(int generation)
{
    qDebug()<<"Moudel <detection> current sensor generation:"<< generation;
    if(generation == 0)
    {
        this->mSensorGeneration = 0;
    }
    else
    {
        this->mSensorGeneration = 1;
    }
}


// 获得当前传感器是哪一代
int Detection::get_sensor_generation()
{
   return this->mSensorGeneration;
}

// 设置当前串口运行状态
void Detection::set_serialport_status(bool status)
{
    this->mSerialPortStatus = status;
}

// 获得当前串口运行状态
bool Detection::get_serialport_status()
{
    return this->mSerialPortStatus;
}

// 打开串口
bool Detection::open_com(QSerialPort *serial, QString com, int baud)
{
    serial->setPortName(com);
    serial->setReadBufferSize(1024);
    if(serial->isOpen())
    {
        serial->close();
        QThread::msleep(100);
    }

    serial->setBaudRate(baud);
    if(serial->open(QIODevice::ReadWrite))
    {
        this->send_start_command(serial);
    }
    else
    {
        emit msg_log(MSG_ALM, QString("%1").arg("串口打开失败"), true);
        return false;
    }

    QDateTime time = QDateTime::currentDateTime();                // 获取系统现在的时间
    this->mSerialStartTime = time.toString("hh:mm:ss.zzz") + "\n";// 设置系统时间显示格式

    return true;
}

// 关闭串口
bool Detection::close_com(QSerialPort *serial)
{
    if(serial->isOpen())
    {
        serial->close();
    }
    else
    {
        emit msg_log(MSG_ALM, QString("%1").arg("串口关闭失败"), true);
        return false;
    }
    QDateTime time = QDateTime::currentDateTime();                // 获取系统现在的时间
    this->mSerialStopTime = time.toString("hh:mm:ss.zzz") + "\n"; // 设置系统时间显示格式
    return true;
}

// 给麒盛传感器发送开始命令
void Detection::send_start_command(QSerialPort *serial)
{
    if(mSensorGeneration == 0)
    {
       serial->setBaudRate(9600);
       serial->write("start");
       serial->waitForBytesWritten(1000);
       serial->setBaudRate(115200);
       QThread::msleep(100);
    }
    else
    {
        QByteArray command;
        command[0] = 0xAA;
        command[1] = 0x00;
        command[2] = 0x00;
        command[3] = 0x00;
        command[4] = 0x00;
        command[5] = 0x00;
        command[6] = 0xFF;
        serial->write(command);
        QThread::msleep(100);
    }
}

// 串口信号readready 响应函数
void Detection::serial_read_all()
{
//    return;
    this->requestData += mSerialport->readAll();
    int cur_sensor_generation = this->get_sensor_generation();
    if(cur_sensor_generation == 0)
    {
        //一代的解析函数
        this->parseSensorDataForFirstGeneration(this->requestData,this->heartData,this->breathData);
    }
    else if(cur_sensor_generation == 1)
    {
        this->parseSensorDataForSecondGeneration(this->requestData,this->heartData,this->breathData);
    }
}

// 二代传感器，校验数据包，返回校验值
int Detection::check_byte(QByteArray checkData,int count)
{
    int tmp_sum = 0;
    if(checkData.size() < count)
        return -1;
    for(int i=0;i<=count;i++){
        tmp_sum += byte(checkData[i]);
    }
    tmp_sum = tmp_sum & 0xff;
    return tmp_sum;
}

// 二代传感器数据归一化
double Detection::signal_normalization(QByteArray recvData)
{
    int databit_value = 8388608.0;      //归一化分母

//    qDebug()<<"recvData"<<int(recvData[0]);
//    qDebug()<<"recvData1"<<recvData[1];
//    qDebug()<<"recvData2"<<int(recvData[2]);
    int tmpdata = recvData[2] & 0x000000FF;
    tmpdata |= (recvData[1]<<8) & 0x0000FF00;
    tmpdata |= (recvData[0]<<16) & 0x00FF000;
//    qDebug()<<"tmpdata"<<double(tmpdata);

    double trans_result = strtol(recvData.toHex().data(), NULL, 16);
//    qDebug()<<"trans_result"<<trans_result;

    if (trans_result >= databit_value)
    {
        trans_result = trans_result - databit_value;
    }
    else
    {
        trans_result = trans_result + databit_value;
    }
    trans_result = (trans_result - databit_value)*1.0 / databit_value;
    return trans_result;
}

// 解析二代传感器原始数据
void Detection::parseSensorDataForSecondGeneration(QByteArray &recvData, QByteArray& heartData, QByteArray &breathData)
{
    FilterFunc filter;
    if(recvData.size() <8)
    {
        return ;
    }
    int pos = 0;
    while (1)
    {
        pos = recvData.indexOf('m', pos);
        if (pos != -1)
        {
            if (recvData.size() - pos >= 8)
            {
                QByteArray ck_code = recvData.mid(pos+7, 1);       // 收到数据的校验码
                QByteArray tmp = recvData.mid(pos, 7);             // 收到的有效数据
                int tmp_sum = 0;
                tmp_sum = check_byte(tmp,6);                       //校验

                if(tmp_sum != byte(ck_code[0])){
                    pos += 1;
                    recvData = recvData.mid(pos, recvData.size() - pos);
                    qDebug()<<"failed";
                    continue;
                }
                QByteArray heart_values = recvData.mid(pos + 1, 3);
                QByteArray breath_values = recvData.mid(pos + 4, 3);
                heartData.append(heart_values);
                breathData.append(breath_values);
                double heart_trans_result = this->signal_normalization(heart_values);  //归一化，用于图形显示
                double breath_trans_result = this->signal_normalization(breath_values);
                if(show_graph_EN)
                {
                    if(is_filter_flag)
                    {
                        filter_index++;
                        L_ai_hr.append(heart_trans_result);
                        L_ai_br.append(breath_trans_result);
                        if(is_move_filter==true)
                        {
                            if(filter_index>= 1000)
                            {
                                filter_index = 0;
                                for(int k = 0;k<1000;k++)
                                {
                                    L_ai_hr.removeAt(k);
                                    L_ai_br.removeAt(k);
                                }

                                QString low_pass_str = ui->low_pass_filter->currentText();
                                bool ok;
                                int low_pass = low_pass_str.toInt(&ok,10);
                                QList<double> filter_hr_data = filter.my_filter(L_ai_hr, 1, 0.5, low_pass);
                                QList<double> filter_br_data = filter.my_filter(L_ai_br, 1, 0.5, low_pass);

                                for(int k = 2000;k<3000;k++)
                                {
                                    this->set_data_to_graph(heart_graph, breath_graph, filter_hr_data[k], filter_br_data[k]);
                                }
                            }
                         }

                        if(is_move_filter == false)
                        {
                            if(filter_index >= 3000)
                            {

                                filter_index = 0;
                                QString low_pass_str = ui->low_pass_filter->currentText();
                                bool ok;
                                int low_pass = low_pass_str.toInt(&ok,10);
                                QList<double> filter_hr_data = filter.my_filter(L_ai_hr, 1, 0.5, low_pass);
                                QList<double> filter_br_data = filter.my_filter(L_ai_br, 1, 0.5, low_pass);

                                // 画波形
                                for(int k=0;k<filter_hr_data.size();k++)
                                {
                                    this->set_data_to_graph(heart_graph, breath_graph, filter_hr_data[k], filter_br_data[k]);
                                    L_ai_hr.clear();
                                    L_ai_br.clear();
                                }
                                //is_move_filter = true;
                            }
                        }

                    }
                    else
                    {
                        //update graph
                        this->set_data_to_graph(heart_graph, breath_graph, heart_trans_result, breath_trans_result);
                    }
                }
            }
            else
            {
                recvData = recvData.mid(pos, recvData.size() - pos);
                break;
            }
        }
        else
        {
            recvData.clear();
            break;
        }
        pos += 8;
    }
}

// 解析一代传感器原始数据
void Detection::parseSensorDataForFirstGeneration(QByteArray &recvData, QByteArray& heartData, QByteArray &breathData)
{
    const int frame_len = 7;
    double sign_heart = 0, sign_breath = 0;
    int databit_value = 2048.0;      //归一化分母

    if(recvData.size() >= frame_len)
    {
        int pos = 0;
        while(1)
        {
            pos = recvData.indexOf('m', pos);
            if(pos != -1)
            {
                if(recvData.size() - pos >= frame_len)
                {
//                    heartData.append(recvData.mid(pos + 1, 3));
//                    breathData.append(recvData.mid(pos + 4, 3));

                    sign_heart = strtol(recvData.mid(pos+1,3).toHex().data(), NULL, 16);
                    sign_heart = (sign_heart - databit_value)*1.0 / databit_value;
                    sign_breath = strtol(recvData.mid(pos+4,3).toHex().data(), NULL, 16);
                    sign_breath = (sign_breath - databit_value)*1.0 / databit_value;


                    if(sign_heart > 1.0)
                        sign_heart = 0;
                    else if(sign_heart < -1.0)
                        sign_heart = 0;

                    if(sign_breath > 1.0)
                        sign_breath = 0;
                    else if(sign_breath < -1.0)
                        sign_breath = 0;

                    if(show_graph_EN)
                    {
                        //update graph
                        this->set_data_to_graph(heart_graph, breath_graph, sign_heart, sign_breath);
                    }
                }
                else
                {
                    recvData = recvData.mid(pos, recvData.size() - pos);
                    break;
                }
            }
            else
            {
                recvData.clear();
                break;
            }
            pos += frame_len;
        }
    }
}

//保存数据到txt文本中
bool Detection::save_data_into_file(QString fileName)
{
    QFile serial_txt;
    QString filePath  = "";
    QString path = QDir::currentPath();
    filePath = path + "/" + fileName + "_RecvData.txt";
    serial_txt.setFileName(filePath);
    if(!serial_txt.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug()<<"file can not open";
        return false;
    }
    //write time start
    serial_txt.write(this->mSerialStartTime.toLatin1());
    for(int i=0;i<this->heartData.size()/3;i++)
    {
        serial_txt.write(this->heartData.mid(i*3,3).toHex());
        serial_txt.write(" ");
        serial_txt.write(this->breathData.mid(i*3,3).toHex());
        serial_txt.write("\n");
    }
    //write time end
    serial_txt.write(this->mSerialStopTime.toLatin1());
    return true;
}

// 更新数据到Graph
void Detection::set_data_to_graph(QCPGraph* index_h,QCPGraph* index_b, double hr, double br)
{
//    return;
    emit sig_output_com_data(hr, br);
    index_h->addData(graph_index, hr);
    index_b->addData(graph_index, br);
    graph_index++;
    if(graph_index%remove_data_index == 0)
    {
        index_h->data().data()->removeBefore(graph_index-remove_data_index/2);
        index_b->data().data()->removeBefore(graph_index-remove_data_index/2);
        qDebug()<<"detection执行删除操作："<<graph_index;
    }
    int hb_num = ui->L_hr->value();    //界面上设置的x轴时间 10秒
    if(graph_index > (unsigned)(hb_num * 1000))
    {
        ui->graph_L->xAxis->setRange(graph_index - hb_num * 1000, graph_index);
    }
    else
    {
        ui->graph_L->xAxis->setRange(0, hb_num*1000);
    }
    if(graph_index > (unsigned)(hb_num * 1000))
    {
        ui->graph_R->xAxis->setRange(graph_index - hb_num * 1000, graph_index);
    }
    else
    {
        ui->graph_R->xAxis->setRange(0, hb_num*1000);
    }
}

//定时刷新Graph
void Detection::slot_update_graph()
{
    if(ui->en_L->checkState() == Qt::Checked)
    {
        ui->graph_L->replot();
    }
    if(ui->en_L_2->checkState() == Qt::Checked)
    {
        ui->graph_R->replot();
    }

}


/*------------------外部调用函数--------------------*/

void Detection::slot_start_recv_sensor_data(QString fileSaveName,int generation)
{

    if(this->get_serialport_status())
    {
        emit sig_send_serial_open_result(false);
        return;
    }
    this->set_sensor_generation(generation);
    if(generation == 0)
    {
        if(!this->open_com(mSerialport, m_com.com, m_com.baud))
        {
            emit msg_log(MSG_ALM, QString("%1").arg("流程运行中：串口打开失败"), true);
            return;
        }
    }
    else if(generation == 1)
    {
        if(!this->open_com(mSerialport, m_com_second.com, m_com_second.baud))
        {
            emit msg_log(MSG_ALM, QString("%1").arg("流程运行中：串口打开失败"), true);
            return;
        }
    }
    this->mFileSaveName = fileSaveName;
    this->init_sensor_data_buff();
    show_graph_EN = true;
    this->set_serialport_status(true);  // 设置为运行
    emit sig_send_serial_open_result(true);
}

void Detection::slot_stop_recv_sensor_data()
{
    if(!this->get_serialport_status())
    {
//        return;
    }
    show_graph_EN = false;
    if(!this->close_com(mSerialport))
    {
        emit msg_log(MSG_ALM, QString("%1").arg("流程运行中：串口关闭失败"), false);
    }
    this->set_serialport_status(false); // 设置为停止
    // save data into file
    emit sig_save_data_into_file(this->mFileSaveName,this->heartData, this->breathData,this->mSerialStartTime,this->mSerialStopTime);
    this->heartData.clear();
    this->breathData.clear();
}
/*------------------------------------------------*/

/***************************************/

//信号检测
void Detection::start_signal_detection()
{
    if(mSensorGeneration == 0)
    {
        sp->sensor_signal_detection(m_com.com);
    }
    else if(mSensorGeneration == 1)
    {
        sp->sensor_signal_detection(m_com_second.com);
    }

}

void Detection::stop_signal_detection()
{
    sp->stop_sensor_signal_detection();
}



















