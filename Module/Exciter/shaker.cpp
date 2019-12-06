#include "shaker.h"
#include "ui_shaker.h"
#include<QDebug>
#include<system_info.h>


struct DataPrivate
{
    QString curRadioChecked = "";

    //外部访问———激振器启动是否成功的标志
    bool is_shaker_start_success = false;
    bool is_shaker_set_finished = false;


    // ni电压输出缩放倍数，为了跟原始水平保持一致（比如同样设置1v的输出电压，能达到同样的激振效果，即ACC的值相同），每次功放断电之后，通过软件寻找到电压缩放倍数
    bool feedbackRet = false;
    double m_VoltageScale = 1.000;
};



Shaker::Shaker(QWidget *parent,NI4461_Class* niObject,ShakerFeedback *shakerFb) :
    QWidget(parent),
    ui(new Ui::Shaker)
{
    ui->setupUi(this);

    //设为无边框
    setWindowFlags(Qt::FramelessWindowHint);
    this->setGeometry(NESTED_PANEL_POS_X, NESTED_PANEL_POS_Y, NESTED_PANEL_POS_W, NESTED_PANEL_POS_H);

    this->init_graph();
    this->ni  = niObject;
    this->shakerFb = shakerFb;
    mSelfData = new DataPrivate();  //数据存放结构体

    connect(ni, &NI4461_Class::sig_output_ai0_read_data, this, &Shaker::get_acc_data);
    connect(ni, &NI4461_Class::sig_output_ai1_read_data, this, &Shaker::get_force_data);


    connect(ui->start, &QPushButton::clicked, this, &Shaker::pb_start_clicked);
    connect(ui->sine_btn, &QPushButton::clicked, this, &Shaker::pb_sine_dir_set);


    connect(ui->zoom_g,&QRadioButton::clicked, this, &Shaker::myzoom_config);
    connect(ui->zoom_h,&QRadioButton::clicked, this, &Shaker::myzoom_config);
    connect(ui->zoom_v,&QRadioButton::clicked, this, &Shaker::myzoom_config);

    connect(ui->radioButton_sine,&QRadioButton::clicked,this,&Shaker::radio_sine_file_clicked);
    connect(ui->radioButton_file,&QRadioButton::clicked,this,&Shaker::radio_sine_file_clicked);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeUpDate()));
    this->mAoParam.waveBuf.clear();
}

Shaker::~Shaker()
{
    delete mSelfData;
    mSelfData = NULL;
    delete ui;
}


void Shaker::init_graph()
{
    //init graph
    ui->GraphRT->addGraph();
    ui->GraphRT->addGraph();
    ui->GraphRT->addGraph();
    ui->GraphRT->addGraph();
    ui->GraphRT->graph(0)->setPen(QPen(Qt::red));
    ui->GraphRT->graph(1)->setPen(QPen(Qt::darkRed));
    ui->GraphRT->graph(2)->setPen(QPen(Qt::blue));
    ui->GraphRT->graph(3)->setPen(QPen(Qt::darkBlue));

    ui->GraphRT->xAxis->setLabel("time");
    ui->GraphRT->yAxis->setLabel("voltage");
    ui->GraphRT->xAxis->setRange(0, 10000);
    ui->GraphRT->yAxis->setRange(-1.0, 1.0);
    ui->GraphRT->setInteraction(QCP::iRangeDrag,true);
    ui->GraphRT->setInteraction(QCP::iRangeZoom,true);
    ui->GraphRT->axisRect()->setRangeZoomAxes(ui->GraphRT->xAxis, ui->GraphRT->yAxis);
  //  ui->GraphRT->axisRect()->setRangeZoom();
    ui->GraphRT->setInteraction(QCP::iSelectPlottables,true);
  //  ui->GraphRT->rescaleAxes(true);           //自动显示轴范围
}

// 显示界面
void Shaker::display_panel()
{
    show();     // 显示界面
   // panel_refresh_timer->start(50);   // 开启界面刷新定时器
}

// 隐藏界面
void Shaker::hide_panel()
{
    //panel_refresh_timer->stop();
    hide();
}


//外部设置当前传感器代数
void Shaker::slot_sensor_generation(int sensor_generation)
{
    if(sensor_generation == 1)
        mSensorGeneration = 0;
    else if(sensor_generation == 2)
        mSensorGeneration = 1;
}

// ai波形显示设定
void Shaker::myzoom_config()
{
    if(ui->zoom_g->isChecked() == true)
    {
        ui->GraphRT->axisRect()->setRangeZoomAxes(ui->GraphRT->xAxis, ui->GraphRT->yAxis);
    }
    else if(ui->zoom_h->isChecked() == true)
    {
        ui->GraphRT->axisRect()->setRangeZoomAxes(ui->GraphRT->xAxis, 0);
    }
    else if(ui->zoom_v->isChecked() == true)
    {
        ui->GraphRT->axisRect()->setRangeZoomAxes(0, ui->GraphRT->yAxis);
    }
}

// 选择播放文件波形还是sine,如果当前激振器正在播放，不允许切换
void Shaker::radio_sine_file_clicked()
{
    if(ui->start->text() == QString("结束"))
    {
        if(mSelfData->curRadioChecked == 'file')
        {
            ui->radioButton_file->setChecked(true);

        }
        else if(mSelfData->curRadioChecked == 'sine')
        {
            ui->radioButton_sine->setChecked(true);
        }
    }
    if(ui->radioButton_file->isChecked())
    {
        mSelfData->curRadioChecked = 'file';
    }
    else
    {
        mSelfData->curRadioChecked = 'sine';
    }
}

// 获取错误描述
QString Shaker::get_error_string(int error_code)
{
    char errorString[512] = {0};
    const int str_len = 512;
    DAQmxGetErrorString(error_code, errorString, str_len);
    return QString(errorString);
}

// 开启定时器
void Shaker::start_my_timer()
{
    if(this->timer->isActive())
    {
        this->timer->stop();
    }
    else
    {
        this->time_count = 0;
        this->timer->start(1000); //一秒刷新一次
    }
}

// 关闭定时器
void Shaker::stop_my_timer()
{
    if(this->timer->isActive())
    {
        this->ui->start->setText("开始");
        this->timer->stop();
        this->time_count = 0;
    }
    else
    {
        this->ui->start->setText("开始");
        this->time_count = 0;
    }
}

//timer更新函数
void Shaker::timeUpDate()
{
    this->time_count++;
    qDebug()<<"Time left:"<<mAoParam.RecvTime - time_count<<" S";
    if(time_count>=this->mAoParam.RecvTime)
    {
        this->stop_my_timer();
        this->pb_stop();
    }
}

/*************************控件响应函数*****************************/

// 激振器界面启动按钮
void Shaker::pb_start_clicked()
{
    if(ui->radioButton_file->isChecked())
    {
        this->start_shaker_for_file();
    }
    else if(ui->radioButton_sine->isChecked())
    {
        this->start_shaker_for_sine();
    }
    else
    {
        QMessageBox::warning(this,"警告","当前未选择播放的波形");
        return;
    }
}

//生成sine波形，并开始播放波形
void Shaker::start_shaker_for_sine()
{
    QString cur_str = ui->start->text();
    if(cur_str == "开始")
    {
        if(this->is_shaker_running())
        {
            qDebug()<<"shaker is running";
            return;
        }
        if(shakerFb->status())
        {
            qDebug()<<"feedback is running";// 这里可以改成弹出框
            return;
        }
        if(!this->pb_start())
        {
            qDebug()<<"start failed";
            return;
        }
        ui->start->setText("结束");
    }
    else if(cur_str == "结束")
    {
        this->pb_stop();
        ui->start->setText("开始");
    }
}

//读取wave文件，并开始播放波形
void Shaker::start_shaker_for_file()
{
    QString cur_str = ui->start->text();
    if(cur_str == "开始")
    {
        if(this->is_shaker_running())
        {
            return;
        }
        if(shakerFb->status())
        {
            qDebug()<<"feedback is running";// 这里可以改成弹出框
            return;
        }
        this->start_my_timer();
        if(!this->pb_start_wav())
        {
            this->stop_my_timer();
            return;
        }
        ui->start->setText("结束");
    }
    else if(cur_str == "结束")
    {
        this->pb_stop();
        this->stop_my_timer();
        ui->start->setText("开始");
    }
}


// sine文件目录路径设置
void Shaker::pb_sine_dir_set()
{
    QString dir = QFileDialog::getOpenFileName(this);
    if(!dir.isEmpty())
    {
        ui->sine_let->setText(dir);
        QFileInfo fi = QFileInfo(dir);
        this->dir_name = fi.baseName();
        qDebug()<<this->dir_name;
    }
}




/*******************************功能函数********************************/

/*
 *fileName: 波形文件的完整路径
 *requestLength: 需要读取的的数据长度(点的个数=采样率x输出时间秒数)
 * waveBuff：返回存放波形的buff
*/
bool Shaker::read_wave_from_file(QString fileName,int requestLength, QList<double> &waveBuff)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this,"警告","当前选择的波形文件路径有错误！");
        return false;
    }
    int index_line=0;
    while(!file.atEnd()) {
        QByteArray line = file.readLine();
        QString str(line);
        double val=str.toDouble();
        waveBuff.append(val);
        index_line++;
        if (index_line >= requestLength)
        {            
            break;
        }        
    }
    if(!scale(waveBuff)) // 这一步是对文件波形进行缩放
    {
        return false;
    }
    file.close();
    return true;
}

// 文件波形幅值缩放(已添加功率放大器电压缩放倍数)
bool Shaker::scale(QList<double> &waveBuff)
{
    if(waveBuff.size() <= 0)
    {
        return false;
    }
    double zoomVoltage = ui->doubleSpinBox_file_zoom->value();  //单位V
    double tempMax = abs(waveBuff.at(0));
    double tempMin = abs(waveBuff.at(0));
    double valueAbs = abs(waveBuff.at(0));
    for(int i=0;i<waveBuff.size();i++)
    {
        if(waveBuff.at(i)>= 0)
        {
            valueAbs = waveBuff.at(i);
        }
        else
        {
            valueAbs = -waveBuff.at(i);
        }
        if(valueAbs > tempMax)
        {
            tempMax = valueAbs;
        }
        if(valueAbs < tempMin)
        {
            tempMin = valueAbs;
        }
    }
    double zoom = 1;
    if(tempMax > 1)
    {
        zoom = zoomVoltage / tempMax;   //如果波形的最大值超过1，说明这不是一个归一化的波形，只能按照最大幅值进行缩放
    }
    else
    {
        zoom = zoomVoltage;
    }
    for(int i=0;i<waveBuff.size();i++)
    {
        waveBuff[i] = waveBuff.at(i)* zoom* mSelfData->m_VoltageScale;
    }
    return true;
}

/*
 * wavePath:设置波形路径（传波形路径
 * aoSample:设置采样率
 * recvTime:设置采样时间
*/
bool Shaker::set_ao_param(QString wavePath, int aoSample, int recvTime)
{
    bool nRet = false;
    this->mAoParam.NiDevName = ui->dev_name->text();
    this->mAoParam.AoDevName = ui->ao_dev->text();
    this->mAoParam.WavePath = wavePath;  //传进来
    this->mAoParam.AoSample = aoSample;  //传进来
    this->mAoParam.RecvTime = recvTime;  //传进来
    this->mAoParam.waveBuf.clear();
    nRet = this->read_wave_from_file(mAoParam.WavePath,int(mAoParam.AoSample * mAoParam.RecvTime),this->mAoParam.waveBuf);
    if(nRet == false)
    {
        return false;
    }
    this->mAoParam.WaveFre = ui->ao_fre->value();        // HZ 只有测试的时候有效
    this->mAoParam.WaveMaxVoltage = ui->ao_amp->value(); // V,幅值，测试的时候有效
    emit sig_send_wavebuff(mAoParam.waveBuf);            // 文件波形，要在图形显示界面展示
    return true; //有些参数是否合适  也要在这个函数中判断
}

//ai和ao的采样暂时保持一致
bool Shaker::set_ai_param(int aiNo, int aiSample)
{
    this->mAiParam[aiNo].NiDevName = ui->dev_name->text();
    if(aiNo == 0)
    {
        this->mAiParam[aiNo].AiDevName = ui->acc_dev->text();
    }
    else
    {
        this->mAiParam[aiNo].AiDevName = ui->force_dev->text();
    }
    if(aiSample <1000 || aiSample >10000)
    {
        return false;
    }
    this->mAiParam[aiNo].AiSample = aiSample;
    return true;
}

//从界面上获取ao、ai的采样频率和采集时间
bool Shaker::get_ao_ai_param_from_ui()
{
    QString wave_path = ui->sine_let->text();
    int AoSample = ui->ao_sample->value();
    int RecvTime = ui->recv_time->value();
    if(!this->set_ao_param(wave_path,AoSample,RecvTime))
    {
        return false;
    }

    int Ai0Sample = ui->acc_sample->value();
    if(!this->set_ai_param(0,Ai0Sample))
    {
        return false;
    }

    int Ai1Sample = ui->force_sample->value();
    if(!this->set_ai_param(1,Ai1Sample))
    {
        return false;
    }
    return true;
}

// 应该把文件模式和sine模式的参数设置统一的
bool Shaker::get_ao_ai_param_for_sine_model_from_ui()
{
    return false;
}

//设置激振器运行状态
void Shaker::set_shaker_running_flag(bool flag)
{
    this->mRunningFlag = flag;
}

// 读取激振器运行状态
bool Shaker::is_shaker_running()
{
    return this->mRunningFlag;
}

// 从外部开始设置的标记
void Shaker::start_setting()
{
    mSelfData->is_shaker_set_finished = false;
}

// 外部访问设置成功与否
bool Shaker::is_setting_finished()
{
    return mSelfData->is_shaker_set_finished;
}

// 外部访问启动成功与否
bool Shaker::is_started_success()
{
    return mSelfData->is_shaker_start_success;
}


//------------------激振器输出波形---------------------------
//读取文件波形及播放
bool Shaker::pb_start_wav()
{
    //判断运行状态
    if(this->is_shaker_running())
    {
        qDebug()<<"激振器运行中";
        return false;
    }
    //从界面上获取信息
    if(!this->get_ao_ai_param_from_ui())
    {
        qDebug()<<"a0,ai参数有问题！";
        return false;
    }
    QString ao_dev_name = QString("%1/%2").arg(this->mAoParam.NiDevName).arg(this->mAoParam.AoDevName);

    ni->set_NI4461_AO_param(this->mAoParam.WaveFre, this->mAoParam.waveBuf, ao_dev_name, this->mAoParam.WaveMaxVoltage, this->mAoParam.AoSample, this->mAoParam.AoSample);
    ni->start_ao_task();

    //麒盛传感器
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd-hh.mm.ss");
    QString filepath = "WAV/" + dir_name + current_date;
    qDebug()<<filepath;

    // 等待麒盛传感器开始采集
    this->wait_serial_open_complete(filepath); //等待串口打开完成，无论成功与否
    if(!this->get_serial_opened_result())
    {
        qDebug()<<"serial open failed";
        return false;
    }
    // AI
    this->start_ai_task();    //根据界面上的勾选，ai开始采集

    //将激振器标记为运行状态
    this->set_shaker_running_flag(true);
    return true;
}

//sine波形生成及播放(已增加缩放倍数)
bool Shaker::pb_start()
{
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd-hh.mm.ss");
    QString fre_str = ui->ao_fre->text();
    double dou_fre = fre_str.toDouble();
    QString filepath =  "SIN/sin" + QString("%1").arg(dou_fre) + "HZ_" + current_date;
    qDebug()<<filepath;

    // 等待麒盛传感器开始采集
    this->wait_serial_open_complete(filepath); //等待串口打开完成，无论成功与否
    if(!this->get_serial_opened_result())
    {
        qDebug()<<"serial open failed";
        return false;
    }
    //判断运行状态
    if(this->is_shaker_running() || ni->ao->get_ao_status() || ni->ai0->get_ai_status() || ni->ai1->get_ai_status())
    {
        qDebug()<<"激振器运行中";
        return false;
    }
    // AO
    double ao_output_fs = ui->ao_fre->value();  //输出频率
    QString ao_dev_name = QString("%1/%2").arg(ui->dev_name->text()).arg(ui->ao_dev->text());//ao名称
    double max_voltage = ui->ao_amp->value();
    qDebug()<<"mSelfData->m_VoltageScale :"<<mSelfData->m_VoltageScale;
    max_voltage = max_voltage * mSelfData->m_VoltageScale;   //最大幅值*缩放
    double fSampleRate_ao = ui->ao_sample->value(); // 采样率
    unsigned int numSampsPerChan_ao = fSampleRate_ao;


    // 创建AO波形
    QList<double> fAnlgArray;
    for(uInt32 nIndex=0; nIndex<numSampsPerChan_ao/ao_output_fs; nIndex++)
    {
        fAnlgArray.append((float64)(max_voltage*sin(2.0*3.141592653589 * ao_output_fs * nIndex / fSampleRate_ao)));
    }

    ni->set_NI4461_AO_param(ao_output_fs, fAnlgArray, ao_dev_name, max_voltage, fSampleRate_ao, numSampsPerChan_ao);
    ni->start_ao_task();


    // AI
    if(ui->acc_en->checkState() == Qt::Checked)
    {
        QString acc_dev_name = QString("%1/%2").arg(ui->dev_name->text()).arg(ui->acc_dev->text());
        double fSampleRate_ai_acc = ui->acc_sample->value();
        ni->set_NI4461_AI0_param(acc_dev_name, fSampleRate_ai_acc);
        ni->start_ai0_task(0);

    }

    if(ui->force_en->checkState() == Qt::Checked)
    {
        QString force_dev_name = QString("%1/%2").arg(ui->dev_name->text()).arg(ui->force_dev->text());
        double fSampleRate_ai_force = ui->force_sample->value();
        ni->set_NI4461_AI1_param(force_dev_name, fSampleRate_ai_force);
        ni->start_ai1_task(1);
    }

    this->set_shaker_running_flag(true);
    return true;
}

//停止波形输出
void Shaker::pb_stop()
{   
    this->stop_ai_task(); // 关闭ai
    ni->stop_ao_task();   // 关闭ao
    emit sig_stop_signal_detection();
    this->set_shaker_running_flag(false);
}

//开启AI任务，这个函数有待优化，主要是ai0和ai1两个通道同时创建会有问题
bool Shaker::start_ai_task()
{
    if(ui->acc_en->checkState() == Qt::Checked)
    {
        QString acc_dev_name = QString("%1/%2").arg(this->mAiParam[0].NiDevName).arg(this->mAiParam[0].AiDevName);
        ni->set_NI4461_AI0_param(acc_dev_name, double(this->mAiParam[0].AiSample));
        ni->start_ai0_task(0);
    }
    if(ui->force_en->checkState() == Qt::Checked)
    {
        QString force_dev_name = QString("%1/%2").arg(this->mAiParam[1].NiDevName).arg(this->mAiParam[1].AiDevName);
        ni->set_NI4461_AI1_param(force_dev_name,  double(this->mAiParam[1].AiSample));
        ni->start_ai1_task(1);
    }
    return true;
}

bool Shaker::stop_ai_task()
{
    ni->stop_ai0_task();
    ni->stop_ai1_task();
    return true;
}

void Shaker::init_serial_wait_flag()
{
    this->mWaitSerialOpenFlag = false;
}

bool Shaker::get_serial_opened_result()
{
    return this->mIsSerialOpenSuccess;
}

void Shaker::wait_serial_open_complete(QString wavePath)
{
    this->init_serial_wait_flag();
    emit sig_start_signal_detection(wavePath,mSensorGeneration);  //麒盛传感器开始采集
    int time_count = 0;
    while(!this->mWaitSerialOpenFlag)
    {
        QThread::msleep(50);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        time_count++;
        if(time_count >= 6)
        {
            this->mIsSerialOpenSuccess = false;
            break;
        }
    }
}



/**************************外部调用函数***************************/

//启动激振器，开始输出波形
bool Shaker::pb_start_wav_for_process()
{
    //判断运行状态
    if(this->is_shaker_running())
    {
        qDebug()<<"激振器运行中";
        return false;
    }
    //设置激振器状态为运行(完成后状态会在定时器中设置为停止
    this->set_shaker_running_flag(true);

    //开启定时器
    this->start_my_timer();

    //读取界面上的设置参数
    //this->get_ao_ai_param_from_ui();
    QString ao_dev_name = QString("%1/%2").arg(this->mAoParam.NiDevName).arg(this->mAoParam.AoDevName); // 完整的ao设备名称

    // 设置AO波形
    ni->set_NI4461_AO_param(this->mAoParam.WaveFre, this->mAoParam.waveBuf, ao_dev_name, this->mAoParam.WaveMaxVoltage, this->mAoParam.AoSample, this->mAoParam.AoSample);

    // 启动AO输出
    ni->start_ao_task();

    // AI
    this->start_ai_task();    //根据界面上的勾选，ai开始采集

    //麒盛传感器
//    emit sig_start_signal_detection();  //麒盛传感器开始采集
    this->set_shaker_running_flag(true);
    return true;
}


//外部等待激振器输出完成
void Shaker::wait_for_wave_finish()
{
    QTime startTime = QTime::currentTime();
    QTime stopTime = QTime::currentTime();

    int elapsed = 0;
    if(!this->is_shaker_running())
    {
        this->pb_stop();
        return;
    }
    while (1)
    {
        QThread::msleep(1000);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        stopTime = QTime::currentTime();
        qDebug()<<stopTime;
        elapsed = startTime.secsTo(stopTime);
        if(elapsed >= this->mAoParam.RecvTime)
        {
            break;
        }
    }
    this->pb_stop();
}


//設置：文件路徑/波形原始采樣率/采集時間
//返回值true表示設置成功/false表示設置失敗
bool Shaker::set_ni_param(QString wavePath, int aoSample, int recvTime)
{
    if(!this->set_ao_param(wavePath,aoSample,recvTime))
    {
        qDebug()<<"ao参数有问题";
        return false;
    }
    if(!this->set_ai_param(0,aoSample))
    {
        qDebug()<<"ai0参数有问题";
        return false;
    }
    if(!this->set_ai_param(1,aoSample))
    {
        qDebug()<<"ai1参数有问题";
        return false;
    }
    return true;
}

//外部开启激振器输出
void Shaker::slot_set_ni_param_and_start_ao_ai(QString wavePath, int aoSample, int recvTime , QString product_name)
{
    mSelfData->is_shaker_set_finished = false;

    //判断运行状态
    if(this->is_shaker_running() || shakerFb->status())
    {
        qDebug()<<"激振器运行中";
        mSelfData->is_shaker_set_finished = true;
        mSelfData->is_shaker_start_success = false;
        return;
    }
    // 设置NI参数
    if(!this->set_ni_param(wavePath,aoSample,recvTime))
    {
        mSelfData->is_shaker_set_finished = true;
        mSelfData->is_shaker_start_success = false;
        return;
    }
    // 启动NI工作
    if(!this->pb_start_wav_for_process())
    {
        mSelfData->is_shaker_set_finished = true;
        mSelfData->is_shaker_start_success = false;
        return;
    }
    // 判断串口是否开启成功？？？
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("hh.mm.ss");
    QString current_date_day =current_date_time.toString("MM.dd");
    QString current_date_year =current_date_time.toString("yyyy");
    QFileInfo fi = QFileInfo(wavePath);
    QString filepath = fi.baseName();
    filepath =current_date_year + "/" + current_date_day + "/" + product_name + "/" + filepath + "-" + current_date;
    qDebug()<<filepath;
    this->wait_serial_open_complete(filepath); // 等待串口打开完成，无论成功与否

    if(this->get_serial_opened_result()) // 判断串口是否打开成功
    {
        mSelfData->is_shaker_start_success = true;
    }
    else
    {
        mSelfData->is_shaker_start_success = false;   //因为串口打开失败
    }
    mSelfData->is_shaker_set_finished = true;
    return;
}

// 由串口类发送过来
void Shaker::slot_recv_serial_open_result(bool serialFlag)
{
    this->mWaitSerialOpenFlag = true;
    qDebug()<<"serialFlag:"<<serialFlag;
    this->mIsSerialOpenSuccess = serialFlag;
}


void Shaker::slot_recv_feedback_result(bool result, double voltageScale)
{
    mSelfData->feedbackRet = result;
    mSelfData->m_VoltageScale = voltageScale;
    qDebug()<<"收到feedback结果";
}

//-------------------------------------------------------------------------槽函数

// 获取加速度计传感器通道的数据
void Shaker::get_acc_data(double *readArray, int read_length)
{
    emit sig_send_shaker_acc_buff(readArray,read_length);
    for(int i=0;i<read_length;i++)
    {
        ui->GraphRT->graph(0)->addData(acc_org_index++, readArray[i]);
        if(acc_org_index > 10000)
            ui->GraphRT->xAxis->setRange(acc_org_index - 10000, acc_org_index);
        if(acc_org_index%75000 == 0)
        {
           ui->GraphRT->graph(0)->data().data()->removeBefore(acc_org_index-75000/2);
//           qDebug()<<ui->GraphRT->graph(0)->data().data()->size();
           qDebug()<<"shaker执行删除操作："<<acc_org_index;
           qDebug()<<'\r\n';
        }
    }

    ui->GraphRT->replot();
}

// 获取力传感器通道的数据
void Shaker::get_force_data(double *readArray, int read_length)
{
    emit sig_send_shaker_force_buff(readArray,read_length);
    for(int i=0;i<read_length;i++)
    {
        ui->GraphRT->graph(1)->addData(acc_org_index++, readArray[i]);
        if(acc_org_index > 10000)
            ui->GraphRT->xAxis->setRange(acc_org_index - 10000, acc_org_index);
    }
    ui->GraphRT->replot();
}

// 获取麒盛传感器通道数据
void Shaker::get_sensor_data(double heartRate, double breathRate)
{
    // send data to shakerfeedback;
}



//  测试代码
void Shaker::on_pushButton_feedback_start_clicked()
{
    if(this->is_shaker_running() || shakerFb->status())
    {
        qDebug()<<"激振器运行中";
        return;
    }
    emit sig_to_feedback_start();

}

void Shaker::on_pushButton_feedback_stop_clicked()
{
    qDebug()<<"pushButton_test_stop";
//    pMyTest->set_run_stop();
    emit sig_to_feedback_stop();
//    pb_stop();
}
