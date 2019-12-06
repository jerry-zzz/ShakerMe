#include "shakerfeedback.h"
#include <QDebug>
#include <QCoreApplication>
#include <QThread>
#include <QMetaType>
#include <QMessageBox>
#include "../Sql/sql_data_cell.h"
#include "../../system_info.h"

struct FeedbackObjectPrivate
{
    FeedbackObjectPrivate() {}
    NI4461_Class *ni;
    Sql_data_cell sql_data_cell;
};

struct FeedbackDataPrivate
{
    FeedbackDataPrivate() {}
    double ao_output_fs = 30;               // hz
    double max_voltage = 5;                 // 幅值
    double fSampleRate_ao = 5000;           // 采样
    unsigned int numSampsPerChan_ao = fSampleRate_ao;
    QList<double> fAnlgArray;               // 存放acc数据
    double fSampleRate_ai_acc = 5000;       // ai0采样频率

    QString ao_dev_name = QString("%1/%2").arg("dev1").arg("ao0"); // ao设备名
    QString acc_dev_name = QString("%1/%2").arg("dev1").arg("ai0");// ai设备名

    bool feedbackFlag = false;              // 当次acc数据接收完全的标志

    QList<double> feedbackDataList;
    QList<double> heartRateList;
    QList<double> breathRateList;
    bool feedbackHBFlag = false;

    int recursionCount = 0;          // 递归次数
    int recursionCountLimits = 100;  // 递归或循环次数上限

    double niUpperLimit = 0.1;//0.00035;   // 打裸板振幅上限
    double niLowerLimit = 0.09;//0.00015;   // 打裸板振幅下限  //这两个参数是需要设定的，就是激振器的振幅


    double sensorUpper_limit = 0.7;         //
    double sensorLower_limit = 0.95;        //

    bool start_flag = false;
    bool is_running = false;
    double voltageScale = 1;
};


ShakerFeedback::ShakerFeedback(NI4461_Class *ni)
{
    this->mSelfObj = new FeedbackObjectPrivate();
    this->mSelfData = new FeedbackDataPrivate();
    this->mSelfObj->ni = ni;                    //组件
    qRegisterMetaType<QList<double>>("QList");  // 注册
    mSelfObj->sql_data_cell.set_db_path(SYSTEM_PARAM_DB_PATH);// 设置数据库路径
    qDebug()<<"feedback status:"<<this->status();
}

ShakerFeedback::~ShakerFeedback()
{
    mSelfData->feedbackDataList.clear();
    delete this->mSelfData;
    delete this->mSelfObj;
}


bool ShakerFeedback::start_shaker_feedback()
{
    qDebug()<<"start feedback by acc";
    mSelfData->max_voltage = 0.01;     // 初始化0.1V
    mSelfData->recursionCount = 0;

    double bestVoltage = 0;
    mSelfData->start_flag = true;
    bool ret = this->reviseAoOutputByAcc(mSelfData->max_voltage,bestVoltage);
    if(ret)
    {
        // mSelfData->max_voltage = bestVoltage; //本次激振器达到上一次的振幅所需要的电压， 本次电压/上次电压，就是本次激振器使用过程中的电压缩放倍数
        double originalVoltage = this->get_voltage_from_sql();
        if (originalVoltage >0 && originalVoltage <10)
        {
            mSelfData->voltageScale = bestVoltage / originalVoltage;
            emit sig_send_feedback_result(true,mSelfData->voltageScale);
        }
        else
        {
            qDebug()<<"the voltage in sql is error!";
            emit sig_send_feedback_result(false,1);
        }

    }
    else
    {
        emit sig_send_feedback_result(false,1);
        qDebug()<<'未能调节到与上次相同的尺度，请重新确认功率放大器上的旋钮位置';
    }
    return ret;
}

//用传感器开始反馈调节
bool ShakerFeedback::start_shaker_sensor_feedback()
{
    qDebug()<<"start feedback by sensor";
    mSelfData->max_voltage = 0.05;     // 初始化5V,即量程的一半
    mSelfData->recursionCount = 0;
    double bestVoltage = 0;
    this->reviseAoOutputBySensor(mSelfData->max_voltage,bestVoltage);
    mSelfData->max_voltage = bestVoltage;
    return true;
}

// 非递归方式寻找NI最佳输出电压（循环
bool ShakerFeedback::reviseAoOutputByAcc(double curVoltage,double &bestVoltage)
{
    bool isContinue = false;
    bool ret = false;
    bool curFunRet = false;
    double nextVoltage;
    this->mSelfData->is_running = true;
    while(1)
    {
        ret = this->reviseAoOutputByAiCell(curVoltage,isContinue,nextVoltage);
        curVoltage = nextVoltage;   //用于下一次
        bestVoltage = nextVoltage;  //用于返回
        if(this->mSelfData->start_flag == false)
        {
            curFunRet = false;
            break;
        }
        if(ret && !isContinue)
        {
            qDebug()<<"acc feedback success";
            curFunRet = true;
            break;
        }
        else if(!ret && !isContinue)
        {
            qDebug()<<"out of limits or NI error";
            curFunRet = false;
            break;
        }
        else if(ret && !isContinue)
        {
            qDebug()<<"out of revise counts";
            curFunRet = false;
            break;
        }

    }
    this->mSelfData->is_running = false;
    return curFunRet;
}


bool ShakerFeedback::reviseAoOutputBySensor(double curVoltage, double &bestVoltage)
{
    bool isContinue = false;
    bool ret = false;
    double nextVoltage;
    while(1)
    {
        ret = this->reviseAoOutputBySensorCell(curVoltage,isContinue,nextVoltage);
        curVoltage = nextVoltage;   //用于下一次
        bestVoltage = nextVoltage;  //用于返回
        if(ret && !isContinue)
        {
            qDebug()<<"sensor feedback success";
           return true;
        }
        else if(!ret && !isContinue)
        {
            qDebug()<<"out of limits or NI error";
            return false;
        }
        else if(ret && !isContinue)
        {
            qDebug()<<"out of revise counts";
        }
    }
    return true;
}


// 递归方式寻找NI最佳输出电压（消耗内存）
bool ShakerFeedback::reviseAoOutputByAccRecursive(double voltage,double &bestVoltage)
{
    bool isContinue = false;
    double nextVoltage = 0;
    int ret = this->reviseAoOutputByAiCell(voltage,isContinue,nextVoltage);
    bestVoltage = nextVoltage;
    voltage = nextVoltage;
    if(ret && !isContinue)
    {
        qDebug()<<"feedback success";
        return true;
    }
    else if(!ret && !isContinue)
    {
        qDebug()<<"out of limits or NI error";
        return false;
    }
    else if(ret && !isContinue)
    {
        qDebug()<<"out of revise counts";
    }
    return this->reviseAoOutputByAccRecursive(voltage,bestVoltage);
}

//用acc反馈控制ni输出
bool ShakerFeedback::reviseAoOutputByAiCell(double voltage,bool &isContinue,double &nextVoltage)
{
    QList<double> fAnlgArray = this->create_ao_wave(mSelfData->ao_output_fs,
                                                    voltage,
                                                    mSelfData->fSampleRate_ao,
                                                    mSelfData->numSampsPerChan_ao);

    // AO
    this->mSelfObj->ni->set_NI4461_AO_param(mSelfData->ao_output_fs,
                                            fAnlgArray,
                                            mSelfData->ao_dev_name,
                                            voltage,
                                            mSelfData->fSampleRate_ao,
                                            mSelfData->numSampsPerChan_ao);

    this->mSelfObj->ni->start_ao_task();
    int timeCount = 0;
    while(!mSelfObj->ni->ao->get_ao_status());  // 加计数
//    {
//        QThread::msleep(10);
//        QCoreApplication::processEvents(QEventLoop::AllEvents);
//        timeCount ++;
//        if (timeCount >= 6)
//        {
//            isContinue = false;
//            return false;
//        }
//    }
    // AI
    this->set_acc_feedback_status(false);
    this->mSelfData->feedbackDataList.clear();

    this->mSelfObj->ni->set_NI4461_AI0_param( mSelfData->acc_dev_name, mSelfData->fSampleRate_ai_acc);
    this->mSelfObj->ni->start_ai0_task(0);

    timeCount = 0;
    while (!mSelfObj->ni->ai0->get_ai_status());
//    {
//        QThread::msleep(10);
//        QCoreApplication::processEvents(QEventLoop::AllEvents);
//        timeCount ++;
//        if (timeCount >= 6)
//        {
//            isContinue = false;
//            return false;
//        }
//    }

    if(!this->wait_for_feedback_data())
    {
        mSelfObj->ni->stop_ao_task();
        while(mSelfObj->ni->ao->get_ao_status());
        mSelfObj->ni->stop_ai0_task();
        while (mSelfObj->ni->ai0->get_ai_status());
        isContinue = false;
        return false;
    }

    mSelfObj->ni->stop_ao_task();
    while(mSelfObj->ni->ao->get_ao_status());

    mSelfObj->ni->stop_ai0_task();
    while (mSelfObj->ni->ai0->get_ai_status() || mSelfObj->ni->ai0->get_ai_recv_status());
    QThread::msleep(30);

    qDebug()<<"start duel the wave";
    double waveAmp = this->get_wave_amplitude(this->mSelfData->feedbackDataList,5000,12000);
    double next_voltage_tmp = 0;
    if(this->get_next_voltage_by_acc_limits(waveAmp,voltage,next_voltage_tmp))
    {
        if(voltage == next_voltage_tmp)
        {
            isContinue = false;
            return true;
        }
        nextVoltage = next_voltage_tmp;
    }
    else
    {
        isContinue  = false;
        return false;
    }
    mSelfData->recursionCount++;
    if(mSelfData->recursionCount >= mSelfData->recursionCountLimits)
    {
        qDebug()<<"迭代超过100次";
        isContinue = false;
        return true;
    }
    qDebug()<<"current voltage:"<<voltage;
    qDebug()<<"recursionCount"<<mSelfData->recursionCount;
    emit sig_send_current_voltage(voltage);
    isContinue = true;
    return false;
}

//用sensor反馈控制ni输出
bool ShakerFeedback::reviseAoOutputBySensorCell(double voltage,bool &isContinue,double &nextVoltage)
{
    QList<double> fAnlgArray = this->create_ao_wave(mSelfData->ao_output_fs,
                                                     voltage,
                                                     mSelfData->fSampleRate_ao,
                                                     mSelfData->numSampsPerChan_ao);

     // AO
     this->mSelfObj->ni->set_NI4461_AO_param(mSelfData->ao_output_fs,
                                             fAnlgArray,
                                             mSelfData->ao_dev_name,
                                             voltage,
                                             mSelfData->fSampleRate_ao,
                                             mSelfData->numSampsPerChan_ao);

     this->mSelfObj->ni->start_ao_task();
     while(!mSelfObj->ni->ao->get_ao_status());

     if(!wait_for_sensor_feedback_data())
     {
         QMessageBox::warning(NULL,"warning","check keeson sensor connections");
         return false;
     }
     double waveAmp = this->get_wave_amplitude(this->mSelfData->feedbackDataList,500,1000);

     double next_voltage_tmp = 0;
     if(this->get_next_voltage_by_sensor_limits(waveAmp,voltage,next_voltage_tmp))
     {
         if(voltage == next_voltage_tmp)
         {
             isContinue = false;
             return true;
         }
         nextVoltage = next_voltage_tmp;
     }
     else
     {
         isContinue  = false;
         return false;
     }
     mSelfData->recursionCount++;
     if(mSelfData->recursionCount >= mSelfData->recursionCountLimits)
     {
         qDebug()<<"迭代超过100次";
         isContinue = false;
         return true;
     }
     qDebug()<<"current voltage:"<<mSelfData->max_voltage;
     qDebug()<<"recursionCount"<<mSelfData->recursionCount;

     isContinue = true;
     return false;

}

// 等待acc数据采集完成（一个检测周期内）
bool ShakerFeedback::wait_for_feedback_data()
{
    int counts = 0;
//    while (!this->get_ni_status())
    qDebug()<<"开始等待";
    while (!get_acc_feedback_status())
    {
       QThread::msleep(10);
       QCoreApplication::processEvents(QEventLoop::AllEvents);
       counts ++;
       if (counts >=350)     //4秒时间等3秒的数据，暂定
       {
           qDebug()<<"等待失败,超过时间限制";
           return false;
       }
       continue;
    }
    qDebug()<<"等待成功";
    return true;
}

// 等待sensor数据采集完成
bool ShakerFeedback::wait_for_sensor_feedback_data()
{
    int counts = 0;
    while(!get_sensor_feedback_status())
    {
        QThread::msleep(100);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        counts ++;
        if (counts >=35)     //4秒时间等3秒的数据，暂定
        {
            qDebug()<<"等待失败,超过时间限制";
            return false;
        }
        continue;
    }
    return true;
}

//acc采集信号
void ShakerFeedback::slot_from_ai0_feedback_data(double *readArray, int readLength)
{

    if(!this->status())
    {
        return;
    }
    if(readLength > 5000 || readLength <=0)
    {
        qDebug()<<"readLength error:"<<readLength;
    }
    for(int i=0;i<readLength;i++)
    {
        this->mSelfData->feedbackDataList.append(readArray[i]);
    }
    if(this->mSelfData->feedbackDataList.size() >= 15000)
    {
        QList<double> tempList;
        for(int i=mSelfData->feedbackDataList.size()-15000;i<mSelfData->feedbackDataList.size();i++)
        {
            tempList.append(mSelfData->feedbackDataList.at(i));
        }
        mSelfData->feedbackDataList.clear();
        mSelfData->feedbackDataList = tempList;
//        qDebug()<<"current feedbackDataList size:"<<mSelfData->feedbackDataList.size();
        this->set_acc_feedback_status(true);            //acc接收到3秒的数据/5000采样
    }

//    mSelfData->feedbackDataList.clear();
    return;
}

//麒盛传感器采集数据
void ShakerFeedback::slot_from_detection_sensor_data(double heartRate, double breathRate)
{
    if(mSelfData->heartRateList.size() < 1500)
    {
        mSelfData->heartRateList.append(heartRate);
        mSelfData->breathRateList.append(breathRate);
    }
    if(mSelfData->heartRateList.size() >= 1500)  //1000采样
    {
        mSelfData->feedbackHBFlag = true;
    }
}

// 创建ao输出的电压波形Q
QList<double> ShakerFeedback::create_ao_wave(double ao_output_fs,double maxVoltage,double fSampleRate_ao,unsigned int numSampsPerChan_ao)
{

    // 创建AO波形
    QList<double> fAnlgArray;
    fAnlgArray.clear();
    for(uInt32 nIndex=0; nIndex<numSampsPerChan_ao; nIndex++)
    {
       fAnlgArray.append((float64)(maxVoltage*sin(2.0*3.141592653589 * ao_output_fs * nIndex / fSampleRate_ao)));
    }
    return fAnlgArray;
}

// 检测acc信号幅值,单位：重力加速度g
double ShakerFeedback::get_wave_amplitude(QList<double> waveBuff,int beginInx, int endInx)
{
    QList<double> waveTmp ;
    int endLine = 0;
    if (waveBuff.size() > endInx)
    {
        endLine = endInx;
    }
    else {
        endLine = waveBuff.size();
    }
    if (waveBuff.size() <= beginInx)
    {
        qDebug()<<"waveBuff size error";
    }
    for (int i=beginInx;i<endLine;i++)
    {
        waveTmp.append(waveBuff.at(i));
    }
    double maxTmp = waveTmp.at(0);
    double minTmp = waveTmp.at(0);
    for (int i=0;i<waveTmp.size();i++)
    {
        if (waveTmp.at(i) > maxTmp)
        {
            maxTmp = waveTmp.at(i);
        }
        if(waveTmp.at(i) < minTmp)
        {
            minTmp = waveTmp.at(i);
        }
    }
    qDebug()<<"maxTmp:"<<maxTmp<<"minTmp:"<<minTmp;
    return maxTmp;
}

// 获取下一次的输出电压
/*
 * curAmplitude: 当前加速度幅值
 * curVoltage: 当前电压
 * nextVoltage: 返回值：下一次输出电压
*/
bool ShakerFeedback::get_next_voltage_by_acc_limits(double curAmplitude,double curVoltage, double &nextVoltage)
{
    double offSet;
    if(curVoltage > 0.2 && curVoltage < 9.8)
    {
        offSet = 0.2;
    }
    else if(curVoltage >=0.1 && curVoltage <= 0.2)
    {
        offSet = 0.02;
    }
    else if(curVoltage <= 9.9 && curVoltage >= 9.8)
    {
        offSet = 0.02;
    }
    else
    {
        offSet = 0.002;
    }

    double ratio = (curAmplitude - mSelfData->niUpperLimit) / mSelfData->niUpperLimit;
    if((ratio < 0.1 && ratio >0) ||(ratio > -0.1 && ratio <=0) ||(curVoltage<=0.008||curVoltage>=9.92))
    {
//        if(curVoltage >=0.003)
//        {
//            offSet = 0.002;
//        }
//        else if(curVoltage< 0.003 && curVoltage >=0.0004)
//        {
//            offSet = curVoltage/10;
//        }
        if(curVoltage <= 0.02)
        {
            offSet = curVoltage/10;
        }
        else if(curVoltage >= 9.98)
        {
            offSet = (10-curVoltage)/10;
        }
        else
        {
            offSet = 0.002;
        }
    }

    if(curAmplitude > mSelfData->niUpperLimit)
    {
       nextVoltage = curVoltage - offSet;// 继续调小输出
    }
    else if(curAmplitude < mSelfData->niLowerLimit)
    {
        nextVoltage = curVoltage + offSet;// 继续调大输出
    }
    else
    {
        nextVoltage = curVoltage;         // 找到合理的区间
    }

    if (nextVoltage <=0 || nextVoltage >=10 || nextVoltage <= 0.000005 || nextVoltage >= 9.999995)   // 最后需要再做一次确认
    {
        qDebug()<<"nextVoltage,offset return false:"<<nextVoltage<<","<<offSet;
        return false;
    }
    return true;
}

bool ShakerFeedback::get_next_voltage_by_sensor_limits(double curAmplitude,double curVoltage, double &nextVoltage)
{
    double offSet;
    if(curVoltage > 0.2 && curVoltage < 9.8)
    {
        offSet = 0.2;
    }
    else if(curVoltage >=0.1 && curVoltage <= 0.2)
    {
        offSet = 0.02;
    }
    else if(curVoltage <= 9.9 && curVoltage >= 9.8)
    {
        offSet = 0.02;
    }
    else
    {
        offSet = 0.002;
    }

    double ratio = (curAmplitude - mSelfData->niUpperLimit) / mSelfData->niUpperLimit;
    if(ratio < 0.1 && ratio >0)
    {
        offSet = 0.002;
    }
    else if(ratio > -0.1 && ratio <=0)
    {
        offSet = 0.002;
    }

    if(curAmplitude > mSelfData->sensorUpper_limit)
    {
       nextVoltage = curVoltage - offSet;// 继续调小输出
    }
    else if(curAmplitude < mSelfData->sensorLower_limit)
    {
        nextVoltage = curVoltage + offSet;// 继续调大输出
    }
    else
    {
        nextVoltage = curVoltage;         // 找到合理的区间
    }

    if (nextVoltage <=0 || nextVoltage >=10)
    {
        return false;
    }
    return true;
}

// 获得NI的运行状态
bool ShakerFeedback::get_ni_status()
{
    bool aoStatus = this->mSelfObj->ni->ao->get_ao_status();
    bool aiStatus = this->mSelfObj->ni->ai0->get_ai_status();
    if (aoStatus || aiStatus)
    {
        return true; // ai和ao 至少有一个在运行
    }
    return false;
}

// 设置acc数据传进来的标记位
void ShakerFeedback::set_acc_feedback_status(bool flag)
{
    this->mSelfData->feedbackFlag = flag;
}

// 获得acc数据传进来的标记位
bool ShakerFeedback::get_acc_feedback_status()
{
    return this->mSelfData->feedbackFlag;
}

// 设置sensor数据的标记位
void ShakerFeedback::set_sensor_feedback_status(bool flag)
{
    mSelfData->feedbackHBFlag = flag;
}

// 获得sensor数据传感器的标记位
bool ShakerFeedback::get_sensor_feedback_status()
{
    return mSelfData->feedbackHBFlag;
}


/*波形缩放函数*/
QList<double> ShakerFeedback::scale(QList<double> oriWave,int multiples)
{
    QList<double> scaleBuff;
    double maxValue = oriWave.at(0);
    for(int i=0;i<oriWave.size();i++)
    {
        if(oriWave.at(i) > maxValue)
        {
            maxValue = oriWave.at(0);
        }
        scaleBuff.append(oriWave.at(i)/multiples);
    }
    return scaleBuff;
}


// 保存ni最大输出电压到数据库
bool ShakerFeedback::save_voltage_to_sql(double voltage)
{
    QString voltageStr = QString::number(voltage,'g',10);
    mSelfObj->sql_data_cell.save_db(mSelfObj->sql_data_cell.get_db_path(), "ShakerVoltage", "bareBoardVoltage", voltageStr);
    qDebug()<<this->get_voltage_from_sql();
    return true;
}

// 读取上一次ni最大输出电压
double ShakerFeedback::get_voltage_from_sql()
{
    double voltage = 0;
    try
    {
        QString voltageStr = mSelfObj->sql_data_cell.read_db(mSelfObj->sql_data_cell.get_db_path(), "ShakerVoltage", "bareBoardVoltage");
        qDebug()<<"voltageStr"<<voltageStr;
        voltage  = voltageStr.toDouble();
        qDebug()<<voltage;
    }
    catch(const std::runtime_error& e)
    {
        qDebug()<<"no voltage exists"<<e.what();
    }
    return voltage;
}


// 获得运行状态
bool ShakerFeedback::status()
{
    return this->mSelfData->is_running;
}


// 获得电压缩放倍数
double ShakerFeedback::get_voltage_scale()
{
    return this->mSelfData->voltageScale;
}

/****************************************/


// 外部信号启动函数，当前类的对象运行在自己的线程中
void ShakerFeedback::slot_start()
{
    this->start_shaker_feedback();
}

// 结束当前feedback
void ShakerFeedback::slot_stop()
{
    this->mSelfData->start_flag = false;
}


void ShakerFeedback::slot_reset(double voltage)
{
    this->save_voltage_to_sql(voltage);
    emit sig_send_feedback_result(true,1);
}
