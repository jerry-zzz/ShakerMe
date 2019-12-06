#include "signalprocessing.h"
#include "filter.h"
#include "system_info.h"



SignalProcessing::SignalProcessing(QObject *parent) : QObject(parent)
{

}

// 日志信息
void SignalProcessing::log_info(QString msg)
{
#ifdef LOG_EN
    qInfo() << msg;
#endif
}

void SignalProcessing::set_product_param(Full_Product_Param dp)
{
    m_dp = dp;
}

void SignalProcessing::read_com_data_L(double hr, double br)
{
    return;
    if(!is_sample_finish_L)
    {
        if(com_data_L.size() < sample_points_L)
            com_data_L.append(hr);
        else
            is_sample_finish_L = true;
    }
}

void SignalProcessing::read_com_data_R(double hr, double br)
{
    if(!is_sample_finish_R)
    {
        if(com_data_R.size() < sample_points_R)
            com_data_R.append(hr);
        else
            is_sample_finish_R = true;
    }
}

// 判断噪声
double SignalProcessing::judgment_noise_signal(QList<double> array, int len, double distance)
{
    int *peaks = new int[len];			//不知道会找到几个,故申请的稍大
    int peaks_len = 0;
    int *valleys = new int[len];;		//不知道会找到几个,故申请的稍大
    int valleys_len = 0;

    double sum = 0.0;
    double avg = 0.0;
    for(int i=0;i<len;i++)
    {
        sum += array[i];
    }
    avg = sum / len;
    for(int i=0;i<len;i++)
    {
        array[i] -= avg;
    }

    findPeaks(array, len, distance, peaks, &peaks_len, valleys, &valleys_len);

    int idxp=0,idxv=0;
    int count = 0;
    sum = 0;
    while(peaks_len-idxp > 0 && valleys_len-idxv > 0)
    {
        if(peaks[idxp] < valleys[idxv])
        {
            double dif = array[peaks[idxp]] - array[valleys[idxv]];
            sum += dif;
            count++;
            idxp++;
            idxv++;
        }
        else
            idxv++;
    }

    double noise = sum / count;

    delete [] peaks;
    delete [] valleys;

    return  noise;
}

// 计算心率
void SignalProcessing::calc_heart_rate(QList<double> array, int len, double distance, int *mid_dist, double *ratio, double *hr_peak)
{
    double sum = 0;
    double avg = 0;
    QList<double> n_array = array;
    for(int i=0;i<array.size();i++)
        n_array[i] = -n_array[i];

    // 正向
    int *p_peaks = new int[len];
    int p_peaks_len = 0;
    int *p_valleys = new int[len];
    int p_valleys_len = 0;

    for(int i=0;i<array.size();i++)
        sum += array[i];
    avg = sum / array.size();
    for(int i=0;i<array.size();i++)
        array[i] = array[i] - avg;
    findPeaks(array, len, distance, p_peaks, &p_peaks_len, p_valleys, &p_valleys_len);
    //--求峰值均值
    double p_avg_p = 0.0;
    sum = 0;
    for(int i=1;i<p_peaks_len-1;i++)
    {
        sum += array[p_peaks[i]];
    }
    p_avg_p = sum / (p_peaks_len-2);


    // 反向
    int *n_peaks = new int[len];
    int n_peaks_len = 0;
    int *n_valleys = new int[len];
    int n_valleys_len = 0;

    for(int i=0;i<n_array.size();i++)
        sum += n_array[i];
    avg = sum / n_array.size();
    for(int i=0;i<n_array.size();i++)
        n_array[i] = n_array[i] - avg;
    findPeaks(n_array, len, distance, n_peaks, &n_peaks_len, n_valleys, &n_valleys_len);
    //--求峰值均值
    double n_avg_p = 0.0;
    sum = 0;
    for(int i=1;i<n_peaks_len-1;i++)
    {
        sum += n_array[n_peaks[i]];
    }
    n_avg_p = sum / (n_peaks_len-2);

    QList<double> data;
    int *peaks = nullptr;
    int peaks_len = 0;
    double avg_p = 0;       // 峰均值
    if(n_avg_p > p_avg_p)
    {
        peaks = n_peaks;
        peaks_len = n_peaks_len;
        data = n_array;
        avg_p = n_avg_p;
    }
    else
    {
        peaks = p_peaks;
        peaks_len = p_peaks_len;
        data = array;
        avg_p = p_avg_p;
    }


    // 求心率间期中值
    QList<int> dist;
    for(int i=2;i<peaks_len-1;i++)
    {
        dist.append(peaks[i] - peaks[i-1]);
    }
    qSort(dist.begin(), dist.end());
    qInfo() << "Interval:"<< dist;

    if((peaks_len-3) % 2)
    {
        int mid_index = (peaks_len-3) / 2;
        *mid_dist = dist[mid_index];   // 间期中值
    }
    else
    {
        int mid_index = (peaks_len-3) / 2;
        *mid_dist = (int)(dist[mid_index] + dist[mid_index-1]) / 2;   // 间期中值
    }

    // 求底噪区间
    QList<int> area_lower;
    QList<int> area_upper;
    for(int i=1;i<peaks_len-2;i++)
        area_lower.append(peaks[i] + 340);
    for(int i=2;i<peaks_len-1;i++)
        area_upper.append(peaks[i] - 60);

    findPeaks(data, len, 10, p_peaks, &p_peaks_len, p_valleys, &p_valleys_len);

    // 求底噪峰值均值.
    double avg_np = 0.0;
    sum = 0;
    int isok = 0;
    int ok_count = 0;
    for(int i=0;i<p_peaks_len;i++)
    {
        isok = 0;
        for(int j=0;j<area_upper.size();j++)
        {
            if(p_peaks[i] > area_lower[j] && p_peaks[i] < area_upper[j])
            {
                isok = 1;
                break;
            }
        }
        if(isok)
        {
            sum += array[p_peaks[i]];
            ok_count++;
        }
    }
    avg_np = sum / ok_count;

    // 求心跳峰值与底噪峰值的比
    *ratio = abs(avg_p / avg_np);

    // 峰均值
    *hr_peak = avg_p;
    delete [] p_peaks;
    delete [] p_valleys;
    delete [] n_peaks;
    delete [] n_valleys;
    peaks = nullptr;
}


/*
void SignalProcessing::calc_heart_rate(QList<double> array, int len, double distance, int *mid_dist, double *ratio, double *hr_peak)
{
    int *peaks = new int[len];			//不知道会找到几个,故申请的稍大
    int peaks_len = 0;
    int *valleys = new int[len];;		//不知道会找到几个,故申请的稍大
    int valleys_len = 0;

    double sum = 0;
    double avg = 0;
    for(int i=0;i<array.size();i++)
        sum += array[i];
    avg = sum / array.size();
    for(int i=0;i<array.size();i++)
        array[i] = array[i] - avg;

    findPeaks(array, len, distance, peaks, &peaks_len, valleys, &valleys_len);

    // 求心率间期中值
    QList<int> dist;
    for(int i=1;i<peaks_len;i++)
    {
        dist.append(peaks[i] - peaks[i-1]);
    }
    qSort(dist.begin(), dist.end());

    int mid_index = peaks_len / 2;
    *mid_dist = dist[mid_index];   // 间期中值

    // 求峰值均值
    double avg_p = 0.0;
    sum = 0;
    for(int i=0;i<peaks_len;i++)
    {
        sum += array[peaks[i]];
    }
    avg_p = sum / peaks_len;

    // 求底噪区间
    QList<int> area_lower;
    QList<int> area_upper;
    for(int i=1;i<peaks_len-2;i++)
        area_lower.append(peaks[i] + 340);
    for(int i=2;i<peaks_len-1;i++)
        area_upper.append(peaks[i] - 60);

    findPeaks(array, len, 10, peaks, &peaks_len, valleys, &valleys_len);

    // 求底噪峰值均值.
    double avg_np = 0.0;
    sum = 0;
    int isok = 0;
    int ok_count = 0;
    for(int i=0;i<peaks_len;i++)
    {
        isok = 0;
        for(int j=0;j<area_upper.size();j++)
        {
            if(peaks[i] > area_lower[j] && peaks[i] < area_upper[j])
            {
                isok = 1;
                break;
            }
        }
        if(isok)
        {
            sum += array[peaks[i]];
            ok_count++;
        }
    }
    avg_np = sum / ok_count;

    // 求心跳峰值与底噪峰值的比
    *ratio = abs(avg_p / avg_np);

    // 峰均值
    *hr_peak = avg_p;
    delete [] peaks;
    delete [] valleys;
}
*/

/*
 * @ brief  计算周期和峰峰值
 * @ param  输入:
 *          *array:     计算的数组
 *          len:        *array的长度
 *          distance    找寻峰的间隔
 *          输出
 *          *fs:        计算的频率
 *          *ptp:       计算的峰峰值.
 * @ retval .
 */
void SignalProcessing::calc_fs_and_ptp(QList<double> array, int len, double distance, double *fs, double *ptp)
{
    // 计算周期和频率
    int *peaks = new int[len];			//不知道会找到几个,故申请的稍大
    int peaks_len = 0;
    int *valleys = new int[len];;		//不知道会找到几个,故申请的稍大
    int valleys_len = 0;

    findPeaks(array, len, distance, peaks, &peaks_len, valleys, &valleys_len);

    double temp_fs_sum = 0;
    double temp_peak_sum = array[peaks[1]];
    double temp_valley_sum = array[valleys[1]];
    for(int i=2;i<peaks_len-1;i++)
    {
        temp_fs_sum += peaks[i] - peaks[i-1];
        temp_peak_sum += array[peaks[i]];
    }
    for(int i=2;i<valleys_len-1;i++)
    {
        temp_fs_sum += valleys[i] - valleys[i-1];
        temp_valley_sum += array[valleys[i]];
    }

    *fs = 1000 / (temp_fs_sum / (peaks_len - 3 + valleys_len - 3));
    *ptp = (temp_peak_sum / (peaks_len - 2)) - (temp_valley_sum / (valleys_len - 2));

    delete []peaks;
    delete []valleys;
}

/*
 * 函数:  findPeaks
 * 参数:  *src        源数据数组
 *       src_lenth   源数据数组长度
 *       distance    峰与峰,谷与谷的搜索间距
 *       *indMax     找到的峰的index数组
 *       *indMax_len 数组长度
 *       *indMin     找到的谷的index数组
 *       *indMin_len 数组长度
 */
void SignalProcessing::findPeaks(QList<double> src, double src_lenth, double distance, int *indMax, int *indMax_len, int *indMin, int *indMin_len)
{
    int *sign = (int*)malloc(src_lenth * sizeof(int));
    int max_index = 0,
        min_index = 0;
    *indMax_len = 0;
    *indMin_len = 0;

    for (int i = 1; i<src_lenth; i++)
    {
        double diff = src[i] - src[i - 1];
        if (diff>0)          sign[i - 1] = 1;
        else if (diff<0) sign[i - 1] = -1;
        else                sign[i - 1] = 0;
    }
    for (int j = 1; j<src_lenth - 1; j++)
    {
        double diff = sign[j] - sign[j - 1];
        if (diff<0)      indMax[max_index++] = j;
        else if (diff>0)indMin[min_index++] = j;
    }

    int *flag_max_index = (int *)malloc(sizeof(int)*(max_index>min_index ? max_index : min_index));
    int *idelete = (int *)malloc(sizeof(int)*(max_index>min_index ? max_index : min_index));
    int *temp_max_index = (int *)malloc(sizeof(int)*(max_index>min_index ? max_index : min_index));
    int bigger = 0;
    double tempvalue = 0;
    int i, j, k;
    //波峰
    for (int i = 0; i < max_index; i++)
    {
        flag_max_index[i] = 0;
        idelete[i] = 0;
    }
    for (i = 0; i < max_index; i++)
    {
        tempvalue = -1;
        for (j = 0; j < max_index; j++)
        {
            if (!flag_max_index[j])
            {
                if (src[indMax[j]] > tempvalue)
                {
                    bigger = j;
                    tempvalue = src[indMax[j]];
                }
            }
        }
        flag_max_index[bigger] = 1;
        if (!idelete[bigger])
        {
            for (k = 0; k < max_index; k++)
            {
                idelete[k] |= ((indMax[k] - distance <= indMax[bigger]) & (indMax[bigger] <= indMax[k] + distance));
            }
            idelete[bigger] = 0;
        }
    }
    for (i = 0, j = 0; i < max_index; i++)
    {
        if (!idelete[i])
            temp_max_index[j++] = indMax[i];
    }
    for (i = 0; i < max_index; i++)
    {
        if (i < j)
            indMax[i] = temp_max_index[i];
        else
            indMax[i] = 0;
    }
    max_index = j;

    //波谷
    for (int i = 0; i < min_index; i++)
    {
        flag_max_index[i] = 0;
        idelete[i] = 0;
    }
    for (i = 0; i < min_index; i++)
    {
        tempvalue = 1;
        for (j = 0; j < min_index; j++)
        {
            if (!flag_max_index[j])
            {
                if (src[indMin[j]] < tempvalue)
                {
                    bigger = j;
                    tempvalue = src[indMin[j]];
                }
            }
        }
        flag_max_index[bigger] = 1;
        if (!idelete[bigger])
        {
            for (k = 0; k < min_index; k++)
            {
                idelete[k] |= ((indMin[k] - distance <= indMin[bigger]) & (indMin[bigger] <= indMin[k] + distance));
            }
            idelete[bigger] = 0;
        }
    }
    for (i = 0, j = 0; i < min_index; i++)
    {
        if (!idelete[i])
            temp_max_index[j++] = indMin[i];
    }
    for (i = 0; i < min_index; i++)
    {
        if (i < j)
            indMin[i] = temp_max_index[i];
        else
            indMin[i] = 0;
    }
    min_index = j;

    *indMax_len = max_index;
    *indMin_len = min_index;

    free(sign);
    free(flag_max_index);
    free(temp_max_index);
    free(idelete);
}

// crc8校验
unsigned char SignalProcessing::crc8_chk_value(unsigned char *message, int len)
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

//保存数据到txt文本中 ,当前类的对象应该放在线程中（toThread）
bool SignalProcessing::save_data_into_file(QString fileName,QByteArray heartData,QByteArray breathData,QString startTime, QString stopTime)
{
//    emit Processing_data(heartData,breathData);

    QString path = QString("%1/sample_data").arg(ROOT_PATH);
    QDir dir_r(path);
    if(!dir_r.exists())
        dir_r.mkpath(path);//创建多级目录
    QString path_wav = QString("%1/sample_data/WAV").arg(ROOT_PATH);
    QDir dir_wav(path_wav);
    if(!dir_wav.exists())
        dir_wav.mkpath(path_wav);//创建多级目录
    QString path_sin = QString("%1/sample_data/SIN").arg(ROOT_PATH);
    QDir dir_sin(path_sin);
    if(!dir_sin.exists())
        dir_sin.mkpath(path_sin);//创建多级目录
    QString path_dect = QString("%1/sample_data/DECT").arg(ROOT_PATH);
    QDir dir_dect(path_dect);
    if(!dir_dect.exists())
        dir_dect.mkpath(path_dect);//创建多级目录

    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_year =current_date_time.toString("yyyy");
    QString current_day =current_date_time.toString("MM.dd");
    QString path_process = QString("%1/sample_data/%2/%3/%4").arg(ROOT_PATH).arg(current_year).arg(current_day).arg(m_dp.product_name);
    QDir dir_process(path_process);
    if(!dir_process.exists())
        dir_process.mkpath(path_process);//创建多级目录


    QFile file(path);
    if (file.exists())
    {
        file.remove();
    }

    QFile serial_txt;
    QString filePath  = "";
    filePath = path + "/" + fileName + "_RecvData.txt";
    serial_txt.setFileName(filePath);
    if(!serial_txt.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug()<<"file can not open";
        return false;
    }
    //write time start
    serial_txt.write(startTime.toLatin1());
    for(int i=0;i<heartData.size()/3;i++)
    {
        serial_txt.write(heartData.mid(i*3,3).toHex());
        serial_txt.write(" ");
        serial_txt.write(breathData.mid(i*3,3).toHex());
        serial_txt.write("\n");
    }
    //write time end
    serial_txt.write(stopTime.toLatin1());
    return true;
}


void SignalProcessing::stop_sensor_signal_detection()
{
    my_serialport->close();
    QByteArray aa;
    this->parseSensorData(aa);
}

// 信号测试
void SignalProcessing::sensor_signal_detection(QString com)
{
    my_serialport = new QSerialPort;
    my_serialport->setPortName(com);
    my_serialport->open(QIODevice::ReadWrite);
    //设置波特率
    my_serialport->setBaudRate(115200);
    requestData.clear();
    heartData.clear();
    breathData.clear();
    timeData.clear();
    connect(my_serialport, SIGNAL(readyRead()), this, SLOT(serial_readup()),Qt::DirectConnection);
}


void SignalProcessing::serial_readup()
{
    requestData += my_serialport->readAll();
    return;
}

void SignalProcessing::serial_readup1()
{
    requestData += my_serialport->readAll();
    //双通道
    if (requestData.size() >= 8)
    {
        //qDebug()<<"jinlaile ...";
        int pos = 0;
        while (1)
        {
            pos = requestData.indexOf('m', pos);
            if (pos != -1)
            {
                if (requestData.size() - pos >= 8)
                {
                    QByteArray ck_code = requestData.mid(pos+7, 1);       // 收到数据的校验码
                    QByteArray tmp = requestData.mid(pos, 7);   // 收到的有效数据
                    int tmp_sum = 0;
                    for(int i=0;i<=6;i++){
                        tmp_sum += byte(tmp[i]);
                    }
                    tmp_sum = tmp_sum & 0xff;
                    unsigned char tmp_code = crc8_chk_value((unsigned char*)(tmp.data()), tmp.size());  // 收到的有效数据的校验码
                    if(tmp_sum != byte(ck_code[0])){
                        pos += 1;
                        requestData = requestData.mid(pos, requestData.size() - pos);
                        qDebug()<<"failed";
//                        throw std::runtime_error("串口数据校验码错误!");
                        continue;
                    }
                    heartData.append(requestData.mid(pos + 1, 3));
                    breathData.append(requestData.mid(pos + 4, 3));
                    time_index++;
                    if (time_index % 32 == 1)
                    {
                        QDateTime time = QDateTime::currentDateTime();//获取系统现在的时间
                        QString str = time.toString("hh:mm:ss.zzz") + "\n";//设置系统时间显示格式
                        timeData.append(str);
                    }
                }
                else
                {
                    requestData = requestData.mid(pos, requestData.size() - pos);
                    break;
                }
            }
            else
            {
                requestData.clear();
                break;
            }
            pos += 8;
        }
    }
}

//解析传感器数据
void SignalProcessing::parseSensorData(QByteArray recvData)
{
    QString path = QDir::currentPath();
    qDebug()<<m_dp.product_name;
    path = path + "/" + "data_" +m_dp.product_name+ ".txt";
    qDebug()<<"path"<<path;
    serial_txt.setFileName(path);
    serial_txt.open(QIODevice::WriteOnly | QIODevice::Text);

    //双通道
    if (requestData.size() >= 8)
    {
        int pos = 0;
        while (1)
        {
            pos = requestData.indexOf('m', pos);
            if (pos != -1)
            {
                if (requestData.size() - pos >= 8)
                {
                    QByteArray ck_code = requestData.mid(pos+7, 1);       // 收到数据的校验码
                    QByteArray tmp = requestData.mid(pos, 7);   // 收到的有效数据
                    int tmp_sum = 0;
                    for(int i=0;i<=6;i++){
                        tmp_sum += byte(tmp[i]);
                    }
                    tmp_sum = tmp_sum & 0xff;
//                    unsigned char tmp_code = crc8_chk_value((unsigned char*)(tmp.data()), tmp.size());  // 收到的有效数据的校验码
                    if(tmp_sum != byte(ck_code[0])){
                        pos += 1;
                        requestData = requestData.mid(pos, requestData.size() - pos);
                        qDebug()<<"failed";
//                        throw std::runtime_error("串口数据校验码错误!");
                        continue;
                    }

                    serial_txt.write(requestData.mid(pos + 1, 3).toHex());
                    serial_txt.write(" ");
                    serial_txt.write(requestData.mid(pos + 4, 3).toHex());
                    serial_txt.write("\n");
                }
                else
                {
                    requestData = requestData.mid(pos, requestData.size() - pos);
                    break;
                }
            }
            else
            {
                requestData.clear();
                break;
            }
            pos += 8;
        }
    }
    serial_txt.close();
}
