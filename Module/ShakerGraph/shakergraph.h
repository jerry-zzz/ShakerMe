#ifndef SHAKERGRAPH_H
#define SHAKERGRAPH_H

#include <QWidget>
#include "QCustomPlot.h"

namespace Ui {
class ShakerGraph;
}

class ShakerGraph : public QWidget
{
    Q_OBJECT

public:
    explicit ShakerGraph(QWidget *parent = 0);
    ~ShakerGraph();
    void display_panel();

    void hide_panel();

    void myzoom_config();

    //接收数据
    void receive_file_data();

signals:
/*- Module Function ----------------------------------------------------------*/
    // 日志记录
    void msg_log(int type, QString msg, bool msgbox);
public slots:
    void setLineEditValue(int value);//界面活动取秒
    void slot_update_graph();//定时更新界面
    void init_sensor_data_buff();//清空图形界面

    //接收波形buff数据
    void slot_receive_wavebuff(QList<double> waveBuff);
    void slot_receive_shaker_acc_buff(double *readArray,int read_length);
    void slot_receive_shaker_force_buff(double *readArray,int read_length);
    void slot_receive_sensor_buff(double hr, double br);
private:
    Ui::ShakerGraph *ui;

    QTimer *timer;      // 定时刷新界面图形
    //graph 时间标记
    unsigned long long graph_index_h = 0;
    unsigned long long graph_index_doc = 0;
    unsigned long long graph_index_acc = 0;
    unsigned long long graph_index_force = 0;
    int doc_index = 0;
    QList<double> doc_waveBuff;
    //心跳波形图
    QCPGraph* heart_graph = nullptr;
    //呼吸波形图
    QCPGraph* breath_graph = nullptr;
    //文件波形图
    QCPGraph* doc_graph = nullptr;
    //激振器加速度传感器波形图
    QCPGraph* acc_graph = nullptr;
    //激振器压力传感器波形图
    QCPGraph* force_graph = nullptr;
    int remove_data_index = 75000;//10分钟
    int remove_sensor_data_len = 15000; //15秒

};

#endif // SHAKERGRAPH_H
