/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    bsp.cpp
 * Author:      zone53
 * Version:     1.1
 * Date:        2019/04/16
 *
 ********* 简 介 ***************************************************************
 *
 *  	运动控制封装,包含面板,及底层运动卡的封装,如若不用204C,可在进行扩展.
 *
 ********************************** 历 史 版 本 *********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2018/10/12   wzq        1.0         Initial Version
 * 2018/11/02   wzq        1.1         修复:在轴配置界面和设备配置界面保存参数后,QTreeWidget
 *                                         会丢失currentindex,导致再次保存时,通过
 *                                         getcurrentindex获得的为-1.导致保存错误.同时修复
 *                                         数据库读取,根据id来搜索.
 *                                     新增:轴配置界面和设备配置界面QTreeWidget控件在选择
 *                                         item后,增加背景色.
 *
 *******************************************************************************
 */


#include "bsp.h"
#include "ui_bsp.h"
#include "system_info.h"
#include "config_axis_io.h"
#include "bsp_define.h"
#include "../Log/loginfo.h"
#include <QDebug>


Bsp::Bsp(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BSP)
{
    ui->setupUi(this);

    //设为无边框
    setWindowFlags(Qt::FramelessWindowHint);

    this->setGeometry(NESTED_PANEL_POS_X, NESTED_PANEL_POS_Y, NESTED_PANEL_POS_W, NESTED_PANEL_POS_H);

    // 设置轴,io,其他设备数目
    axis_max_num = AXIS_MAX_NUM;
    io_max_num = IO_MAX_NUM;
    device_max_num = DEVICE_MAX_NUM;

    // 初始化 轴, io
    axis = new Bsp_Aps[axis_max_num];
    io = new Bsp_IO[io_max_num];
    device = new Bsp_Device[device_max_num];

    // 加载IO参数, 初始化 IO配置表控件
    ui->IOTable->horizontalHeader()->setStretchLastSection(true);           // 设置表格是否充满，即行末不留空
    ui->IOTable->verticalHeader()->setHidden(true);                         // 隐藏行表头
//    ui->IOTable->setSelectionBehavior (QAbstractItemView::SelectRows);     // 设置选择行为，以行为单位
//    ui->IOTable->setSelectionMode (QAbstractItemView::SingleSelection);    // 设置选择模式，选择单行
    // 设置列宽
    ui->IOTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->IOTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->IOTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->IOTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->IOTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    ui->IOTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    ui->IOTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    ui->IOTable->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    ui->IOTable->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Stretch);

    for(int i=0;i<io_max_num;i++)
    {
        ui->IOTable->setRowCount(i + 1);//总行数增加1

        ui->IOTable->setItem(i, 0, new QTableWidgetItem(QString().asprintf("%d", i)));
        ui->IOTable->item(i, 0)->setFlags(Qt::ItemIsEnabled);
        ui->IOTable->item(i, 0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        ui->IOTable->setItem(i, 1, new QTableWidgetItem(QString().asprintf("%d", 0)));
        ui->IOTable->setItem(i, 2, new QTableWidgetItem(QString().asprintf("%d", 0)));
        ui->IOTable->setItem(i, 3, new QTableWidgetItem(QString().asprintf("%d", i)));

        QComboBox *comBox = new QComboBox();
        comBox->setStyleSheet("QComboBox {\n    border: 2px solid gray;\n    border-radius: 3px;\n    min-width: 6em;\n}\n\n\nQComboBox::drop-down {\n    subcontrol-origin: padding;\n    subcontrol-position: top right;\n    width: 15px;\n    border-left-width: 1px;  \n	border-left-color: darkgray;\n    border-left-style: solid;\n    border-top-right-radius: 3px;\n    border-bottom-right-radius: 3px;\n}\n\nQComboBox::down-arrow {\n    image: url(:/icon/res/ui_down_arrow.png);\n}\n\nQComboBox::down-arrow:on {\n    top: 1px;\n    left: 1px;\n}\n\n\n");
        comBox->addItem("输入");
        comBox->addItem("输出");
        ui->IOTable->setCellWidget(i,4,comBox);
//        ui->IOTable->item(i, 4)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        QTableWidgetItem *checkBox = new QTableWidgetItem();
        checkBox->setCheckState(Qt::Unchecked);
        ui->IOTable ->setItem(i, 5, checkBox);


        QPushButton *pbutton = new QPushButton();
        pbutton->setText("测试");
        pbutton->setStyleSheet("QPushButton {border-radius: 5px;background-color: #00bcd4;font: 75 10pt \"黑体\";color: white;min-width:60px;} QPushButton:hover{background-color: #03a9f4;color: white;} QPushButton:pressed{background-color:#2196f3;border-style: inset;}");
        pbutton->setObjectName(QString().asprintf("%d", i));
        connect(pbutton, &QPushButton::clicked, this, &Bsp::pb_io_test);
        ui->IOTable ->setCellWidget(i, 6, pbutton);

        QTableWidgetItem *item7 = new QTableWidgetItem();
        item7->setBackgroundColor(QColor(255,0,0));
        ui->IOTable->setItem(i, 7, item7);
        ui->IOTable->item(i, 7)->setFlags(Qt::ItemIsEnabled);

        ui->IOTable->setItem(i, 8, new QTableWidgetItem(QString().asprintf("%s", "")));
    }


    // 关联信号与槽
    connect(ui->axis_save, &QPushButton::clicked, this, &Bsp::pb_axis_save);
    connect(ui->AxisTree,  &QTreeWidget::clicked, this, &Bsp::pb_axis_select);
    connect(ui->reset_fbpos, &QPushButton::clicked, this, &Bsp::pb_reset_encoder_pos);
    connect(ui->reset_outputpos, &QPushButton::clicked, this, &Bsp::pb_reset_output_pos);
    connect(ui->servo, &QPushButton::clicked, this, &Bsp::pb_set_servo);
    connect(ui->move_mel, &QPushButton::pressed, this, &Bsp::pb_move_negative_dir_start);
    connect(ui->move_pel, &QPushButton::pressed, this, &Bsp::pb_move_forward_dir_start);
    connect(ui->move_mel, &QPushButton::released, this, &Bsp::pb_move_negative_dir_stop);
    connect(ui->move_pel, &QPushButton::released, this, &Bsp::pb_move_forward_dir_stop);

    connect(ui->io_save, &QPushButton::clicked, this, &Bsp::pb_io_save);

    connect(ui->device_save, &QPushButton::clicked, this, &Bsp::pb_device_save);
    connect(ui->DeviceTree,  &QTreeWidget::clicked, this, &Bsp::pb_device_select);

    // 轮训定时器
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Bsp::time_Polling_AMP_Info);

    lamp_on.load(":/icon/res/lamp_signal_on.png");
    lamp_off.load(":/icon/res/lamp_signal_off.png");
    ui->alm_lamp->setScaledContents(true);
    ui->ez_lamp->setScaledContents(true);
    ui->elp_lamp->setScaledContents(true);
    ui->elm_lamp->setScaledContents(true);
    ui->inp_lamp->setScaledContents(true);
    ui->org_lamp->setScaledContents(true);
    ui->emg_lamp->setScaledContents(true);
    ui->servo_lamp->setScaledContents(true);


}

Bsp::~Bsp()
{
    timer->stop();
    this->close();
    delete [] axis;
    delete [] io;
    delete [] device;
    delete ui;
}

/*-------------------------------------------------------------------- 面板函数 */
void Bsp::display_panel()
{
    // 设置轴配置界面为显示界面
    ui->config->setCurrentIndex(0);

    // 刷新轴列表
    ui->AxisTree->clear();
    for(int i=0;i<axis_max_num;i++)
    {
        // 先插入轴序号
        QString index;
        index.setNum(i);
        QString axis_name = axis[i].get_axis_config_param()->name;
        QTreeWidgetItem * item=new QTreeWidgetItem(QStringList()<<index<<axis_name);
        ui->AxisTree->addTopLevelItem(item);
    }

    // 设置axis配置界面 axis index 默认为第一个
    current_axis_index = 0;
    ui->AxisTree->setCurrentItem(ui->AxisTree->topLevelItem(current_axis_index));
    QTreeWidgetItem *axis_item = ui->AxisTree->currentItem();
    axis_item->setBackgroundColor(0, Qt::green);
    axis_item->setBackgroundColor(1, Qt::green);
    load_axis_config_panel(current_axis_index);

    // 加载IO面板参数
    load_io_config_panel();


    // 刷新设备列表
    ui->DeviceTree->clear();
    for(int i=0;i<device_max_num;i++)
    {
        // 先插入轴序号
        QString index;
        index.setNum(i);
        QString axis_name = device[i].getDeviceAttribute().name;
        QTreeWidgetItem * item=new QTreeWidgetItem(QStringList()<<index<<axis_name);
        ui->DeviceTree->addTopLevelItem(item);
    }
    // 加载设备面板参数
    current_device_index = 0;
    ui->DeviceTree->setCurrentItem(ui->DeviceTree->topLevelItem(current_device_index));
    QTreeWidgetItem *device_item = ui->DeviceTree->currentItem();
    device_item->setBackgroundColor(0, Qt::green);
    device_item->setBackgroundColor(1, Qt::green);
    load_device_config_panel(current_device_index);

    //显示
    show();
    timer->start(100);
}

void Bsp::hide_panel()
{
    hide();
    timer->stop();
}

/*--------------------------------------------------------------- 参数加载与保存 */
int Bsp::init_bsp_param()
{
    int status = 0;
    // 加载轴,IO,外设参数.
    load_all_axis_param();
    load_all_io_param();
    load_all_device_param();

    // 初始化运动控制卡
    try
    {
        Init_APS_Card();
        status = 1;
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("%1").arg(e.what()), true);
    }
    return status;
}

void Bsp::load_all_axis_param()
{
    QString emsg_axis = "";
    QString emsg_move = "";

    // 加载轴参数, 初始化 轴配置树控件
    APS_AxisInfo temp_axis;
    APS_MovePara temp_move;
    for(int i=0;i<axis_max_num;i++)
    {
        // 读取轴配置参数
        try
        {
            sql_axisconfig.read_db(sql_axisconfig.get_db_path(), "AxisInfo", i, &temp_axis, sizeof(APS_AxisInfo));
            axis[i].set_axis_config_param(&temp_axis);
        }
        catch(const std::runtime_error& e)
        {
            emsg_axis = QString("%1 %2 ").arg(emsg_axis).arg(i);//continue;
        }
        // 读取轴运动参数
        try
        {
            sql_axisconfig.read_db(sql_axisconfig.get_db_path(), "MoveParam", i, &temp_move, sizeof(APS_MovePara));
            axis[i].set_axis_move_param(&temp_move);
        }
        catch(const std::runtime_error& e)
        {
            emsg_move = QString("%1 %2 ").arg(emsg_move).arg(i);//continue;
        }
    }

    QString msg;
    if(emsg_axis.isEmpty() == false)
    {
        msg = QString("轴配置参数读取错误：%1").arg(emsg_axis);
        emit msg_log(MSG_INFO, msg, true);
    }
    if(emsg_move.isEmpty() == false)
    {
        msg = QString("轴配置参数读取错误：%1").arg(emsg_move);
        emit msg_log(MSG_INFO, msg, true);
    }
}

void Bsp::load_all_io_param()
{
    IO_Attribute temp_io;
    for(int i=0;i<io_max_num;i++)
    {
        try
        {
            sql_axisconfig.read_db(sql_axisconfig.get_db_path(), "IOInfo", i, &temp_io, sizeof(IO_Attribute));
            io[i].setIOAttribute(temp_io);
        }
        catch(const std::runtime_error& e)
        {
            emit msg_log(MSG_INFO, QString("读取IO配置参数错误\n%1").arg(e.what()), true);
            return;
        }
    }
}

void Bsp::load_all_device_param()
{
    QString emsg_device = "";
    QVector<Device_Attribute> device_list;

    Device_Attribute tmp_d;
    for(int i=0;i<device_max_num;i++)
    {
        // 读取参数
        try
        {
            sql_axisconfig.read_db(sql_axisconfig.get_db_path(), "DeviceInfo", i, &tmp_d, sizeof(Device_Attribute));
            device[i].setDeviceAttribute(tmp_d);
            device_list.append(tmp_d);
        }
        catch(const std::runtime_error& e)
        {
            emsg_device = QString("%1 %2 ").arg(emsg_device).arg(i);//continue;
        }
    }

    QString msg;
    if(emsg_device.isEmpty() == false)
    {
        msg = QString("设备参数读取错误：%1").arg(emsg_device);
        emit msg_log(MSG_INFO, msg, true);
        return;
    }

    emit sig_update_device_param(device_list);
    emit sig_update_ws(device_list);
    emit sig_update_lr(device_list);
}


// 加载轴配置界面
void Bsp::load_axis_config_panel(int index)
{
    APS_AxisInfo temp_axis = *axis[index].get_axis_config_param();
    APS_MovePara temp_move = *axis[index].get_axis_move_param();

    temp_axis.pulseFactor = temp_axis.pulseFactor == 0?1:temp_axis.pulseFactor;

    temp_aps.set_axis_config_param(&temp_axis);
    temp_aps.set_axis_move_param(&temp_move);

    // 基本参数
    QString name(temp_axis.name);
    ui->name->setText(name);
    ui->cardtype->setCurrentIndex(temp_axis.card_type);
    ui->axistype->setCurrentIndex(temp_axis.axis_type);
    ui->cardid->setValue(temp_axis.card_id);
    ui->axisid->setValue(temp_axis.axis_id);
    ui->pulse->setValue(temp_axis.pulseFactor);
    ui->extencode->setCurrentIndex(temp_axis.extEncode);

    // step 1
    ui->ecl_en->setCurrentIndex(temp_axis.error_check_level_en);
    ui->ecl_value->setValue(temp_axis.error_check_level_value);
    ui->spel_en->setCurrentIndex(temp_axis.soft_pel_en);
    ui->spel_value->setValue(temp_axis.soft_pel_value / temp_axis.pulseFactor);
    ui->smel_en->setCurrentIndex(temp_axis.soft_mel_en);
    ui->smel_value->setValue(temp_axis.soft_mel_value / temp_axis.pulseFactor);
    ui->el_stopmode->setCurrentIndex(temp_axis.el_stop_mode);

    // step 2
    ui->alm_logic->setCurrentIndex(temp_axis.almLogic);
    ui->ez_logic->setCurrentIndex(temp_axis.ezLogic);
    ui->el_logic->setCurrentIndex(temp_axis.elLogic);
    ui->inp_logic->setCurrentIndex(temp_axis.inpLogic);
    ui->org_logic->setCurrentIndex(temp_axis.orgLogic);
    ui->emg_logic->setCurrentIndex(temp_axis.emgLogic);

    // step 3
    ui->encodermode->setCurrentIndex(temp_axis.encoder_mode);
    ui->encoderdir->setCurrentIndex(temp_axis.encoder_dir);

    // step 4
    ui->outputmode->setCurrentIndex(temp_axis.output_mode);

    // 回零设置
    ui->home_mode->setCurrentIndex(temp_axis.homeMode);
    ui->home_dir->setCurrentIndex(temp_axis.homeDir);
    ui->home_ez->setCurrentIndex(temp_axis.homeEZA);
    ui->home_acc->setValue(temp_axis.homeAcc / temp_axis.pulseFactor);
    ui->home_vm->setValue(temp_axis.homeVm / temp_axis.pulseFactor);
    ui->home_vo->setValue(temp_axis.homeVo / temp_axis.pulseFactor);
    ui->home_shift->setValue(temp_axis.homeShift / temp_axis.pulseFactor);
    ui->home_timeout->setValue(temp_axis.homeTimeout);

    // 运动设置
    ui->move_vs->setValue(temp_move.vs / temp_axis.pulseFactor);
    ui->move_vm->setValue(temp_move.vm / temp_axis.pulseFactor);
    ui->move_ve->setValue(temp_move.ve / temp_axis.pulseFactor);
    ui->move_acc->setValue(temp_move.acc / temp_axis.pulseFactor);
    ui->move_dec->setValue(temp_move.dec / temp_axis.pulseFactor);
    ui->move_s->setValue(temp_move.sfac);
    ui->move_timeout->setValue(temp_move.timeout);

}

// 保存轴配置界面
void Bsp::save_axis_config_panel(int index)
{
    APS_AxisInfo temp_axis;
    APS_MovePara temp_move;


    // 基本参数
    std::string str = ui->name->text().toStdString();
    const char* ch = str.c_str();
    sprintf(temp_axis.name, "%s", ch);
    temp_axis.card_type = ui->cardtype->currentIndex();
    temp_axis.axis_type = ui->axistype->currentIndex();
    temp_axis.card_id = ui->cardid->value();
    temp_axis.axis_id = ui->axisid->value();
    temp_axis.pulseFactor = ui->pulse->value()==0?1:ui->pulse->value();
    temp_axis.extEncode = ui->extencode->currentIndex();

    // step 1
    temp_axis.error_check_level_en = ui->ecl_en->currentIndex();
    temp_axis.error_check_level_value = ui->ecl_value->value();
    temp_axis.soft_pel_en = ui->spel_en->currentIndex();
    temp_axis.soft_pel_value = ui->spel_value->value() * temp_axis.pulseFactor;
    temp_axis.soft_mel_en = ui->smel_en->currentIndex();
    temp_axis.soft_mel_value = ui->smel_value->value() * temp_axis.pulseFactor;
    temp_axis.el_stop_mode = ui->el_stopmode->currentIndex();

    // step 2
    temp_axis.almLogic = ui->alm_logic->currentIndex();
    temp_axis.ezLogic = ui->ez_logic->currentIndex();
    temp_axis.elLogic = ui->el_logic->currentIndex();
    temp_axis.inpLogic = ui->inp_logic->currentIndex();
    temp_axis.orgLogic = ui->org_logic->currentIndex();
    temp_axis.emgLogic = ui->emg_logic->currentIndex();

    // step 3
    temp_axis.encoder_mode = ui->encodermode->currentIndex();
    temp_axis.encoder_dir = ui->encoderdir->currentIndex();

    // step 4
    temp_axis.output_mode = ui->outputmode->currentIndex();

    // 回零设置
    temp_axis.homeMode = ui->home_mode->currentIndex();
    temp_axis.homeDir = ui->home_dir->currentIndex();
    temp_axis.homeEZA = ui->home_ez->currentIndex();
    temp_axis.homeAcc = ui->home_acc->value() * temp_axis.pulseFactor;
    temp_axis.homeVm = ui->home_vm->value() * temp_axis.pulseFactor;
    temp_axis.homeVo = ui->home_vo->value() * temp_axis.pulseFactor;
    temp_axis.homeShift = ui->home_shift->value() * temp_axis.pulseFactor;
    temp_axis.homeTimeout = ui->home_timeout->value();

    // 运动设置
    temp_move.vs = ui->move_vs->value() * temp_axis.pulseFactor;
    temp_move.vm = ui->move_vm->value() * temp_axis.pulseFactor;
    temp_move.ve = ui->move_ve->value() * temp_axis.pulseFactor;
    temp_move.acc = ui->move_acc->value() * temp_axis.pulseFactor;
    temp_move.dec = ui->move_dec->value() * temp_axis.pulseFactor;
    temp_move.sfac = ui->move_s->value();
    temp_move.timeout = ui->move_timeout->value();

    QByteArray param1((char*)&temp_axis, sizeof(APS_AxisInfo));
    try
    {
        sql_axisconfig.save_db(sql_axisconfig.get_db_path(), "AxisInfo", index, param1);
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("保存轴配置参数错误\n%1").arg(e.what()), true);
        return;
    }

    QByteArray param2((char*)&temp_move, sizeof(APS_MovePara));
    try
    {
        sql_axisconfig.save_db(sql_axisconfig.get_db_path(), "MoveParam", index, param2);
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("保存轴运动参数错误\n%1").arg(e.what()), true);
        return;
    }

    temp_aps.set_axis_config_param(&temp_axis);
    temp_aps.set_axis_move_param(&temp_move);

    axis[index].set_axis_config_param(&temp_axis);
    axis[index].set_axis_move_param(&temp_move);

    // 设置板卡参数
    temp_aps.set_axis_param_for_card();


    emit msg_log(MSG_INFO, "保存轴配置参数成功!", true);
}

// 加载IO置界面
void Bsp::load_io_config_panel()
{
    IO_Attribute temp_io;
    for(int i=0;i<io_max_num;i++)
    {
        temp_io = io[i].getIOAttribute();

        QTableWidgetItem *item1 = new QTableWidgetItem(QString().asprintf("%d", temp_io.card_id));
        ui->IOTable->setItem(i, 1, item1);
        ui->IOTable->item(i, 1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        QTableWidgetItem *item2 = new QTableWidgetItem(QString().asprintf("%d", temp_io.group_id));
        ui->IOTable->setItem(i, 2, item2);
        ui->IOTable->item(i, 2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        QTableWidgetItem *item3 = new QTableWidgetItem(QString().asprintf("%d", temp_io.channel));
        ui->IOTable->setItem(i, 3, item3);
        ui->IOTable->item(i, 3)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        QComboBox *item4 = (QComboBox *)ui->IOTable->cellWidget(i, 4);
        item4->setCurrentIndex(temp_io.dir);
        ui->IOTable->setCellWidget(i, 4, item4);

        if(temp_io.isReversal)
        {
            QTableWidgetItem *checkBox = new QTableWidgetItem();
            checkBox->setCheckState(Qt::Checked);
            ui->IOTable->setItem(i, 5, checkBox);
        }
        else
        {
            QTableWidgetItem *checkBox = new QTableWidgetItem();
            checkBox->setCheckState(Qt::Unchecked);
            ui->IOTable->setItem(i, 5, checkBox);
        }

        QTableWidgetItem *item8 = new QTableWidgetItem(QString().asprintf("%s", temp_io.description));
        ui->IOTable->setItem(i, 8, item8);
    }
}

// 保存IO配置界面
void Bsp::save_io_config_panel()
{
    IO_Attribute temp_io;
    for(int i=0;i<io_max_num;i++)
    {
        temp_io.card_id = ui->IOTable->item(i, 1)->text().toInt();
        temp_io.group_id = ui->IOTable->item(i, 2)->text().toInt();
        temp_io.channel = ui->IOTable->item(i, 3)->text().toInt();

        QComboBox *item4 = (QComboBox *)ui->IOTable->cellWidget(i, 4);
        temp_io.dir = item4->currentIndex();

        if(ui->IOTable->item(i, 5)->checkState ()==Qt::Checked)
            temp_io.isReversal = 1;
        else
            temp_io.isReversal = 0;

        std::string str = ui->IOTable->item(i, 8)->text().toStdString();
        const char* ch = str.c_str();
        sprintf(temp_io.description, "%s", ch);

        try
        {
            QByteArray param((char*)&temp_io, sizeof(IO_Attribute));
            sql_axisconfig.save_db(sql_axisconfig.get_db_path(), "IOInfo", i, param);
            io[i].setIOAttribute(temp_io);
        }
        catch(const std::runtime_error& e)
        {
            emit msg_log(MSG_INFO, QString("读取IO配置参数错误\n%1").arg(e.what()), true);
            return;
        }
    }

    emit msg_log(MSG_INFO, "保存IO配置参数成功!", true);
}

// 加载设备配置界面
void Bsp::load_device_config_panel(int index)
{
    Device_Attribute tmp_d = device[index].getDeviceAttribute();
    ui->d_name->setText(QString().asprintf("%s", tmp_d.name));
    ui->type->setCurrentIndex(tmp_d.type);
    ui->com->setText(QString().asprintf("%s", tmp_d.com));
    ui->baud->setValue(tmp_d.baud);
    ui->ip->setText(QString().asprintf("%s", tmp_d.ip));
    ui->port->setValue(tmp_d.port);
}

// 保存设备配置界面
void Bsp::save_device_config_panel(int index)
{
    Device_Attribute tmp_d;

    std::string str_name = ui->d_name->text().toStdString();
    const char* ch_name = str_name.c_str();
    sprintf(tmp_d.name, "%s", ch_name);

    tmp_d.type = ui->type->currentIndex();

    std::string str_com = ui->com->text().toStdString();
    const char* ch_com = str_com.c_str();
    sprintf(tmp_d.com, "%s", ch_com);

    tmp_d.baud = ui->baud->value();

    std::string str_ip = ui->ip->text().toStdString();
    const char* ch_ip = str_ip.c_str();
    sprintf(tmp_d.ip, "%s", ch_ip);

    tmp_d.port = ui->port->value();

    QByteArray param((char*)&tmp_d, sizeof(Device_Attribute));
    try
    {
        sql_axisconfig.save_db(sql_axisconfig.get_db_path(), "DeviceInfo", index, param);
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("保存设备配置参数错误\n%1").arg(e.what()), true);
        return;
    }

    emit msg_log(MSG_INFO, "保存设备配置参数成功!", true);
}

/*--------------------------------------------------------------------- 槽函数 */
/* -- 轴配置面板 --*/
void Bsp::pb_axis_save(void)
{
    // 保存
    save_axis_config_panel(current_axis_index);
    // 刷新内存中的参数
    load_all_axis_param();

    // 刷新轴列表
    ui->AxisTree->clear();
    for(int i=0;i<axis_max_num;i++)
    {
        // 先插入轴序号
        QString index;
        index.setNum(i);
        QString axis_name = axis[i].get_axis_config_param()->name;
        QTreeWidgetItem * item=new QTreeWidgetItem(QStringList()<<index<<axis_name);
        ui->AxisTree->addTopLevelItem(item);
    }

    // 刷新新的背景色
    QTreeWidgetItem *item = ui->AxisTree->findItems(QString("%1").arg(current_axis_index), Qt::MatchExactly).at(0);
    item->setBackgroundColor(0, Qt::green);
    item->setBackgroundColor(1, Qt::green);
    load_axis_config_panel(current_axis_index);
}

void Bsp::pb_axis_select(void)
{
    // 消除旧的背景色
    QTreeWidgetItem *item = ui->AxisTree->findItems(QString("%1").arg(current_axis_index), Qt::MatchExactly).at(0);
    item->setBackgroundColor(0, Qt::white);
    item->setBackgroundColor(1, Qt::white);

    current_axis_index = ui->AxisTree->currentIndex().row();
    // 刷新新的背景色
    item = ui->AxisTree->findItems(QString("%1").arg(current_axis_index), Qt::MatchExactly).at(0);
    item->setBackgroundColor(0, Qt::green);
    item->setBackgroundColor(1, Qt::green);

    load_axis_config_panel(current_axis_index);
    time_Polling_AMP_Info();


}

void Bsp::pb_reset_encoder_pos(void)
{
    temp_aps.APS_SetFbPos(0);
}

void Bsp::pb_reset_output_pos(void)
{
    temp_aps.APS_SetCmPos(0);
}

void Bsp::pb_set_servo(void)
{
    if(ui->servo->text() == "Servo Off")
    {
        temp_aps.APS_SetSVON(1);
        ui->servo->setText("Servo On");
    }
    else
    {
        temp_aps.APS_SetSVON(0);
        ui->servo->setText("Servo Off");
    }

}

void Bsp::pb_move_forward_dir_start(void)
{
    temp_aps.APS_MoveEx( 1, 0.1, APS_RM, 0);
}

void Bsp::pb_move_negative_dir_start(void)
{
    temp_aps.APS_MoveEx(-1, 0.1, APS_RM, 0);
}

void Bsp::pb_move_forward_dir_stop(void)
{
    temp_aps.APS_dStop();
}

void Bsp::pb_move_negative_dir_stop(void)
{
    temp_aps.APS_dStop();
}

/* -- IO配置面板 --*/
void Bsp::pb_io_save()
{
    // 保存
    save_io_config_panel();

    // 刷新内存中的参数
    load_all_io_param();
}

void Bsp::pb_io_test() // (int row, int column)
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    int index = btn->objectName().toInt();

    if(io[index].getIOAttribute().dir == 1)
    {
        if(io[index].GetBit())
            io[index].SetBit(0);
        else
            io[index].SetBit(1);
    }
}

/* -- 设备配置面板 --*/
void Bsp::pb_device_save()
{
    // 保存
    save_device_config_panel(current_device_index);
    // 刷新内存中的参数
    load_all_device_param();
    // 刷新设备列表
    ui->DeviceTree->clear();
    for(int i=0;i<device_max_num;i++)
    {
        // 先插入轴序号
        QString index;
        index.setNum(i);
        QString axis_name = device[i].getDeviceAttribute().name;
        QTreeWidgetItem * item=new QTreeWidgetItem(QStringList()<<index<<axis_name);
        ui->DeviceTree->addTopLevelItem(item);
    }

    // 刷新新的背景色
    QTreeWidgetItem *item = ui->DeviceTree->findItems(QString("%1").arg(current_device_index), Qt::MatchExactly).at(0);
    item->setBackgroundColor(0, Qt::green);
    item->setBackgroundColor(1, Qt::green);
}

void Bsp::pb_device_select()
{
    // 消除旧的背景色
    QTreeWidgetItem *item = ui->DeviceTree->findItems(QString("%1").arg(current_device_index), Qt::MatchExactly).at(0);
    item->setBackgroundColor(0, Qt::white);
    item->setBackgroundColor(1, Qt::white);

    current_device_index = ui->DeviceTree->currentIndex().row();
    // 刷新新的背景色
    item = ui->DeviceTree->findItems(QString("%1").arg(current_device_index), Qt::MatchExactly).at(0);
    item->setBackgroundColor(0, Qt::green);
    item->setBackgroundColor(1, Qt::green);

    load_device_config_panel(current_device_index);
}

/*--------------------------------------------------------------------- 定时器 */
void Bsp::time_Polling_AMP_Info(void)
{
    // axis config
    if(ui->config->currentIndex() == 0)
    {
        // step 2
        if(temp_aps.APS_GetALM())
            ui->alm_lamp->setPixmap(lamp_on);
        else
            ui->alm_lamp->setPixmap(lamp_off);

        if(temp_aps.APS_GetEZ())
            ui->ez_lamp->setPixmap(lamp_on);
        else
            ui->ez_lamp->setPixmap(lamp_off);

        if(temp_aps.APS_GetPEL())
            ui->elp_lamp->setPixmap(lamp_on);
        else
            ui->elp_lamp->setPixmap(lamp_off);

        if(temp_aps.APS_GetMEL())
            ui->elm_lamp->setPixmap(lamp_on);
        else
            ui->elm_lamp->setPixmap(lamp_off);

        if(temp_aps.APS_GetINP())
            ui->inp_lamp->setPixmap(lamp_on);
        else
            ui->inp_lamp->setPixmap(lamp_off);

        if(temp_aps.APS_GetORG())
            ui->org_lamp->setPixmap(lamp_on);
        else
            ui->org_lamp->setPixmap(lamp_off);

        if(temp_aps.APS_GetEMG())
            ui->emg_lamp->setPixmap(lamp_on);
        else
            ui->emg_lamp->setPixmap(lamp_off);

        // step 3
        ui->fbPos->setValue(temp_aps.APS_GetFbPos());

        // step 4
        ui->outputPos->setValue(temp_aps.APS_GetCmPos());
        ui->fbPos_2->setValue(temp_aps.APS_GetFbPos());
        if(temp_aps.APS_GetSVON())
        {
            ui->servo_lamp->setPixmap(lamp_on);
            ui->servo->setText("Servo On");
            ui->servo->setStyleSheet("QPushButton{border-radius:5px;background-color:rgb(92, 163, 99);font:75 9pt \"黑体\";color: white;}");
        }
        else
        {
            ui->servo_lamp->setPixmap(lamp_off);
            ui->servo->setText("Servo Off");
            ui->servo->setStyleSheet("QPushButton{border-radius:5px;background-color:rgb(250, 50, 50);font:75 9pt \"黑体\";color: white;}");
        }
    }
    else if(ui->config->currentIndex() == 1)
    {
        for(int i=0;i<io_max_num;i++)
        {
            if(io[i].GetBit())
                ui->IOTable->item(i, 7)->setBackgroundColor(QColor(0,255,0));
            else
                ui->IOTable->item(i, 7)->setBackgroundColor(QColor(255,0,0));
        }
    }
}

/*--------------------------------------------------------------- APS板卡初始化 */
/*
 * @ brief  运动控制卡初始化, 包括轴回零参数设置, I/O端口和逻辑配置, 软限位设置等
 * @ retval
 * @ throw  板卡初始化失败.
 */
void Bsp::Init_APS_Card()
{
    I32  boardID_InBits = 0;
    I32  mode = INIT_AUTO_CARD_ID;

    I32 error = APS_initial (&boardID_InBits, mode);
    if (error < 0)
    {
        throw std::runtime_error(QString("APS板卡初始化失败!\nerror code:%1").arg(error).toStdString().data());
    }

    for (int i = 0; i < 32; i++)
    {
        if ((boardID_InBits >> i) & 0x1)
        {
            // 设置板卡的PRB_EMG_LOGIC参数
            APS_set_board_param(i, PRB_EMG_LOGIC, 1);
        }
    }

    // 需要保证轴参数已经load
    for (int i = 0; i < axis_max_num; i++)
    {
        // 轴参数写入
        axis[i].set_axis_param_for_card();
        // 轴励磁
        axis[i].APS_SetSVON(1);
    }
}
