#include "shakergraph.h"
#include "ui_shakergraph.h"
#include "system_info.h"

#include <qdebug.h>

ShakerGraph::ShakerGraph(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShakerGraph)
{
    ui->setupUi(this);


    //设为无边框
    setWindowFlags(Qt::FramelessWindowHint);
    this->setGeometry(NESTED_PANEL_POS_X, NESTED_PANEL_POS_Y, NESTED_PANEL_POS_W, NESTED_PANEL_POS_H);

    timer = new QTimer;
    connect(timer, &QTimer::timeout, this, &ShakerGraph::slot_update_graph);

    heart_graph = ui->Graph_Sensor->addGraph();//传感器心跳通道
    heart_graph->setPen(QPen(Qt::red));

    breath_graph = ui->Graph_Sensor->addGraph();//传感器呼吸通道
    breath_graph->setPen(QPen(Qt::blue));

    doc_graph = ui->Graph_Doc->addGraph();//文件波形
    doc_graph->setPen(QPen(Qt::darkGreen));

    acc_graph = ui->Graph_Shaker->addGraph();//加速度传感器波形
    acc_graph->setPen(QPen(Qt::darkGray));

    force_graph = ui->Graph_Shaker->addGraph();//压力传感器波形
    force_graph->setPen(QPen(Qt::darkBlue));

    ui->Graph_Doc->xAxis->setLabel("time");
    ui->Graph_Doc->yAxis->setLabel("file");
    ui->Graph_Doc->xAxis->setRange(0, 10 * 1000);
    ui->Graph_Doc->yAxis->setRange(-1, 1);
    ui->Graph_Doc->setInteraction(QCP::iRangeDrag,true);
    ui->Graph_Doc->setInteraction(QCP::iRangeZoom,true);
    ui->Graph_Doc->setInteraction(QCP::iSelectPlottables,true);

    ui->Graph_Sensor->xAxis->setLabel("time");
    ui->Graph_Sensor->yAxis->setLabel("ad-h");
    ui->Graph_Sensor->xAxis->setRange(0, 10 * 1000);
    ui->Graph_Sensor->yAxis->setRange(-1, 1);
    ui->Graph_Sensor->setInteraction(QCP::iRangeDrag,true);
    ui->Graph_Sensor->setInteraction(QCP::iRangeZoom,true);
    ui->Graph_Sensor->setInteraction(QCP::iSelectPlottables,true);

    ui->Graph_Shaker->addGraph();
    ui->Graph_Shaker->addGraph();
    ui->Graph_Shaker->graph(0)->setPen(QPen(Qt::red));
    ui->Graph_Shaker->graph(1)->setPen(QPen(Qt::blue));

    ui->Graph_Shaker->xAxis->setLabel("time");
    ui->Graph_Shaker->yAxis->setLabel("voltage");
    ui->Graph_Shaker->xAxis->setRange(0, 10 * 1000);
    ui->Graph_Shaker->yAxis->setRange(-1.0, 1.0);
    ui->Graph_Shaker->setInteraction(QCP::iRangeDrag,true);
    ui->Graph_Shaker->setInteraction(QCP::iRangeZoom,true);
    ui->Graph_Shaker->setInteraction(QCP::iSelectPlottables,true);

    //设置滑动条控件的最小值
    ui->horizontalSlider->setMinimum(2);
    //设置滑动条控件的最大值
    ui->horizontalSlider->setMaximum(50);
    //设置滑动条控件的值
    ui->horizontalSlider->setValue(10);
    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setLineEditValue(int)));

    //界面调整
    connect(ui->zoom_g,&QRadioButton::clicked, this, &ShakerGraph::myzoom_config);
    connect(ui->zoom_h,&QRadioButton::clicked, this, &ShakerGraph::myzoom_config);
    connect(ui->zoom_v,&QRadioButton::clicked, this, &ShakerGraph::myzoom_config);

}

ShakerGraph::~ShakerGraph()
{
    delete ui;
}

void ShakerGraph::display_panel()
{
    show();     //显示
    timer->start(50);  //画布刷新速度
}

void ShakerGraph::hide_panel()
{
    hide();
}

// ai波形显示设定
void ShakerGraph::myzoom_config()
{
    if(ui->zoom_g->isChecked() == true)
    {
        ui->Graph_Doc->axisRect()->setRangeZoomAxes(ui->Graph_Doc->xAxis, ui->Graph_Doc->yAxis);
        ui->Graph_Sensor->axisRect()->setRangeZoomAxes(ui->Graph_Sensor->xAxis, ui->Graph_Sensor->yAxis);
        ui->Graph_Shaker->axisRect()->setRangeZoomAxes(ui->Graph_Shaker->xAxis, ui->Graph_Shaker->yAxis);
    }
    else if(ui->zoom_h->isChecked() == true)
    {
        ui->Graph_Doc->axisRect()->setRangeZoomAxes(ui->Graph_Doc->xAxis, 0);
        ui->Graph_Sensor->axisRect()->setRangeZoomAxes(ui->Graph_Sensor->xAxis, 0);
        ui->Graph_Shaker->axisRect()->setRangeZoomAxes(ui->Graph_Shaker->xAxis, 0);
    }
    else if(ui->zoom_v->isChecked() == true)
    {
        ui->Graph_Doc->axisRect()->setRangeZoomAxes(0, ui->Graph_Doc->yAxis);
        ui->Graph_Sensor->axisRect()->setRangeZoomAxes(0, ui->Graph_Sensor->yAxis);
        ui->Graph_Shaker->axisRect()->setRangeZoomAxes(0, ui->Graph_Shaker->yAxis);
    }
}

//接收波形文件buff数据
void ShakerGraph::slot_receive_wavebuff(QList<double> waveBuff)
{   
    doc_waveBuff = waveBuff;
    qDebug()<<"it's my wavebuff.";
}

//接收激振器加速度传感器buff
void ShakerGraph::slot_receive_shaker_acc_buff(double *readArray,int read_length)
{
    //return;
    for(int i = 0; i < read_length ; i++ )
    {
        acc_graph->addData(graph_index_acc,readArray[i]);
        graph_index_acc++;
        if(graph_index_acc%remove_data_index == 0)
        {
            this->acc_graph->data().data()->removeBefore(graph_index_acc-remove_data_index/2);
            qDebug()<<"shakergragh acc 执行删除操作："<<graph_index_acc;
        }
    }

    int hb_num = ui->L_hr->value();    //界面上设置的x轴时间 10秒    
    if(graph_index_acc > (unsigned)(hb_num * 1000))
    {
        ui->Graph_Shaker->xAxis->setRange(graph_index_acc - hb_num * 1000, graph_index_acc);
    }
    else
    {
        ui->Graph_Shaker->xAxis->setRange(0, hb_num*1000);
    }

    ui->Graph_Shaker->replot();
//    qDebug()<<"it's my shaker_acc_buff.";
}

//接收激振器压力传感器buff
void ShakerGraph::slot_receive_shaker_force_buff(double *readArray,int read_length)
{
    for(int i = 0; i < read_length ; i++ )
    {
        force_graph->addData(graph_index_force,readArray[i]);
        graph_index_force++;

        if(graph_index_force%remove_data_index == 0)
        {
            this->force_graph->data().data()->removeBefore(graph_index_force-remove_data_index/2);
        }
    }

    int hb_num = ui->L_hr->value();    //界面上设置的x轴时间 10秒
    if(graph_index_force > (unsigned)(hb_num * 1000))
    {
        ui->Graph_Shaker->xAxis->setRange(graph_index_force - hb_num * 1000, graph_index_force);
    }
    else
    {
        ui->Graph_Shaker->xAxis->setRange(0, hb_num*1000);
    }
    ui->Graph_Shaker->replot();
//    qDebug()<<"it's my shaker_acc_buff.";
}


//接收传感器buff数据 显示图形
void ShakerGraph::slot_receive_sensor_buff(double hr, double br)
{
    if(ui->en_H->checkState() == Qt::Checked)
    {
        heart_graph->addData(graph_index_h, hr);
    }
    if(ui->en_B->checkState() == Qt::Checked)
    {
        breath_graph->addData(graph_index_h, br);
    }

    int hb_num = ui->L_hr->value();    //界面上设置的x轴时间 10秒
    graph_index_h++;
    if(graph_index_h%remove_sensor_data_len == 0)
    {
        this->heart_graph->data().data()->removeBefore(graph_index_h-remove_sensor_data_len/2);
        this->breath_graph->data().data()->removeBefore(graph_index_h-remove_sensor_data_len/2);
        qDebug()<<"shakergragh sensor 执行删除操作："<<graph_index_h;
    }

    if(graph_index_h > (unsigned)(hb_num * 1000))
    {
        ui->Graph_Sensor->xAxis->setRange(graph_index_h - hb_num * 1000, graph_index_h);
    }
    else
    {
        ui->Graph_Sensor->xAxis->setRange(0, hb_num*1000);
    }
//    qDebug()<<"doc_waveBuff.size():"<<doc_waveBuff.size();
    if(doc_waveBuff.size()>1)
    {
        doc_graph->addData(graph_index_doc,doc_waveBuff[doc_index]);
        doc_index++;
        if(doc_index >= doc_waveBuff.size())
        {
            doc_index = 0;
        }
        else
        {
            graph_index_doc++;
        }
        if(graph_index_h > (unsigned)(hb_num * 1000))
        {
            ui->Graph_Doc->xAxis->setRange(graph_index_doc - hb_num * 1000, graph_index_doc);
        }
        else
        {
            ui->Graph_Doc->xAxis->setRange(0, hb_num*1000);
        }
    }
}

// 清空存放串口数据的缓存区
void ShakerGraph::init_sensor_data_buff()
{
    // 清空graph中的数据
    this->graph_index_h = 0;
    this->graph_index_doc = 0;
    this->graph_index_acc = 0;

    this->doc_graph->data().data()->clear();
    this->heart_graph->data().data()->clear();
    this->breath_graph->data().data()->clear();
    this->acc_graph->data().data()->clear();
    this->force_graph->data().data()->clear();
}
//滑动取秒
void ShakerGraph::setLineEditValue(int value)
{
  int pos = ui->horizontalSlider->value();
  QString str = QString("%1").arg(pos);
  int str_i = str.toInt();
  ui->L_hr->setValue(str_i);
}
//定时刷新Graph
void ShakerGraph::slot_update_graph()
{
    ui->Graph_Sensor->replot();
    ui->Graph_Doc->replot();
}
