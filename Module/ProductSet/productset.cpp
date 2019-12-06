/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    productset.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	产品设置面板.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#include "productset.h"
#include "ui_productset.h"
#include "system_info.h"
#include "../Log/loginfo.h"
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>


ProductSet::ProductSet(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProductSet)
{
    ui->setupUi(this);

    //设为无边框
    setWindowFlags(Qt::FramelessWindowHint);
    this->setGeometry(NESTED_PANEL_POS_X, NESTED_PANEL_POS_Y, NESTED_PANEL_POS_W, NESTED_PANEL_POS_H);
    int column = ui->sine_table->columnCount();//先获取当前的字段个数
    for(int i=0;i<column;i++)
    {
        ui->sine_table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
    }
    // 定时器
    panel_refresh_timer = new QTimer();
    connect(panel_refresh_timer, &QTimer::timeout, this, &ProductSet::refresh_panel_AMP_Info);


    // 信号槽
    connect(ui->sine_add, &QPushButton::clicked, this, &ProductSet::pb_sine_add);
    connect(ui->sine_delete, &QPushButton::clicked, this, &ProductSet::pb_sine_delete);
    connect(ui->newP, &QPushButton::clicked, this, &ProductSet::pb_new_product);
    connect(ui->loadP, &QPushButton::clicked, this, &ProductSet::pb_load_product);
    connect(ui->productlist, &QListWidget::doubleClicked, this, &ProductSet::pb_load_product);
    connect(ui->deleteP, &QPushButton::clicked, this, &ProductSet::pb_delete_product);
    connect(ui->save, &QPushButton::clicked, this, &ProductSet::save_param);
    connect(ui->sine_dirset, &QPushButton::clicked, this, &ProductSet::pb_sine_dir_set);
    connect(ui->sine_table,&QTableWidget::itemClicked,this,&ProductSet::sine_table_itemClicked);
    connect(ui->sine_table,&QTableWidget::itemChanged,this,&ProductSet::sine_table_itemChanged);
    connect(ui->ecl_en,&QComboBox::currentTextChanged,this,&ProductSet::cur_model_change);
    //-- add your code ---------------------------------------------------------
    connect(ui->move_pos_shaker,&QPushButton::clicked,this,&ProductSet::move_pos_shaker);
    connect(ui->move_pos_shaker_to_bed,&QPushButton::clicked,this,&ProductSet::move_pos_shaker_to_bed);
    connect(ui->read_pos_z,&QPushButton::clicked,this,&ProductSet::read_pos_z);

}

ProductSet::~ProductSet()
{
    this->close();
    delete ui;
}

/*
 *==============================================================================
 *   初始化产品模块/设置权限/界面显示/界面隐藏
 *==============================================================================
 */

// 设置面板权限
void ProductSet::set_current_level(int level)
{
    current_level = level;
}

// 显示界面
void ProductSet::display_panel()
{
    // 操作员权限
    if(current_level == OP_LEVEL)
    {        
        ui->save->setEnabled(false);
        ui->newP->setEnabled(false);
        ui->deleteP->setEnabled(false);

        //-- add your code -----------------------------------------------------
        ui->sine_table->setEditTriggers(QAbstractItemView::NoEditTriggers);    // 禁止编辑
        ui->sine_add->setEnabled(false);
        ui->sine_delete->setEnabled(false);
    }
    else
    {
        ui->save->setEnabled(true);
        ui->newP->setEnabled(true);
        ui->deleteP->setEnabled(true);

        //-- add your code -----------------------------------------------------

        ui->sine_table->setEditTriggers(QAbstractItemView::AllEditTriggers);
        ui->sine_add->setEnabled(true);
        ui->sine_delete->setEnabled(true);
    }

    if(mws->open_serialport() == true)
    {
        m_pTimer_mws->start(1000);
        ws_com_timer_flag = false;
    }
    if(mlr->open_serialport() == true)
    {
        m_pTimer_mlr->start(1000);
        lr_com_timer_flag = false;
    }

    load_param();
    //panel_refresh_timer->start(1000);
    //显示
    show();
}

// 隐藏界面
void ProductSet::hide_panel()
{
    //panel_refresh_timer->stop();
    if(ws_com_timer_flag == false)
    {
         mws->close_serialport();
         m_pTimer_mws->stop();
    }
    if(lr_com_timer_flag == false)
    {
        mlr->close_serialport();
        m_pTimer_mlr->stop();
    }
    hide();
}



// 定时器更新界面io 电机位置信息 压力传感器数值
void ProductSet::refresh_panel_AMP_Info(void)
{
    // 读取反馈位置
    //double fbpos = m_bsp->axis[0].APS_GetFbPos();
    // 更新反馈位置
    //ui->shaker_z_pos->setValue(fbpos);
}
//读取当前激振器传感器的数值
void ProductSet::read_pos_z()
{
    // 读取反馈位置
    double fbpos = m_bsp->axis[0].APS_GetFbPos();
    // 更新反馈位置
    ui->shaker_z_pos->setValue(fbpos);
}


//接收压力传感器数值
void ProductSet::rece_current_value_ws(double result)
{
    ui->current_value_ws->setText(QString::number(result,10,5));
}
//接收激光传感器数值
void ProductSet::rece_current_value_lr(double result)
{
    ui->current_value_lr->setText(QString::number(result,10,5));
}

// 初始化产品模块
void ProductSet::init_product(QString root_dir, QString sysdb_path, Bsp *bsp, WeighingSensor *ws,LaserRanging *lr)
{
    m_bsp = bsp;
    mws = ws;
    mlr = lr;

    // 压力定时器
    m_pTimer_mws = new QTimer();
    connect(m_pTimer_mws, &QTimer::timeout, mws, &WeighingSensor::slot_read_current_numeric_command);
    // 激光定时器
    m_pTimer_mlr = new QTimer();
    connect(m_pTimer_mlr, &QTimer::timeout, mlr, &LaserRanging::channel_querl);

    tmp_root_dir = root_dir;
    product_root_dir = root_dir + PRODUCT_PATH;
    //创建根目录
    QDir dir_r(product_root_dir);
    if(!dir_r.exists())
        dir_r.mkpath(product_root_dir);//创建多级目录

    // 读取上次载入的产品程序
    sql_data_cell.set_db_path(sysdb_path);
    current_product_name = sql_data_cell.read_db(sql_data_cell.get_db_path(), "Product", "currentProduct");
    current_product_dir = product_root_dir + "/" + current_product_name;
    FullProductStru.product_name = current_product_name;
    current_product_db_path = current_product_dir + "/data.db";

    // 若当前的产品名称是 default ,则尝试创建路径
    if(current_product_name == "default")
    {
        QDir dir_r3(current_product_dir);
        if(!dir_r3.exists())
            dir_r3.mkpath(current_product_dir);//创建多级目录
    }
    // 读取产品路径下的列表,刷新界面
    update_product_list();
    // 设置默认选择的item
    QListWidgetItem *item = ui->productlist->findItems(current_product_name, Qt::MatchContains).at(0);
    item->setBackgroundColor(Qt::green);
    ui->productlist->setCurrentItem(item);

    // 加载参数
    load_param();
    this->update_table_values(ui->sine_table,this->FullProductStru.mProduct_Param_List);
    this->update_template_param_to_spin_boxs(this->FullProductStru.experiment_pattern,this->FullProductStru);
    this->update_pattern_to_combobox(this->FullProductStru.experiment_pattern);
}
/*
 *==============================================================================
 *   界面模块功能函数
 *==============================================================================
 */
/*add your code*/

// 更新系统参数
void ProductSet::slot_update_sys_param(Sys_Setup ss)
{
    m_ss = ss;
}

//移动微调位置
void ProductSet::move_pos_shaker()
{
     m_bsp->axis[0].APS_GetSVON();
    double little_distance = ui->little_distance->value();
//    double test_pos = m_ss.shaker_bed_pos - FullProductStru.shaker_part_length -FullProductStru.mattress_height - 1 + little_distance;
    int button= QMessageBox::question(NULL, "提示", "确定要 检测微调位置 吗?\n移动前,请注意安全!", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if(button == QMessageBox::Yes)
    {
        emit msg_log(MSG_INFO, QString("确定要 检测微调位置 吗?用户选择:YES"), false);
        if(m_bsp->axis[0].APS_GetSVON())
        {
            m_bsp->axis[0].APS_AMove(little_distance, m_ss.z_ratio_Z);

            try
            {
                m_bsp->axis[0].APS_WaitMoveDone();
            }
            catch(const std::runtime_error& e)
            {
                emit msg_log(MSG_ALM, QString("%1").arg(e.what()), true);
            }

            emit msg_log(MSG_INFO, "轴 运动到检测微调位置 完成!", true);
        }
        else
            emit msg_log(MSG_INFO, "请先励磁轴Z!", true);
    }
}

//移动到检测位置
void ProductSet::move_pos_shaker_to_bed()
{

    double test_pos = m_ss.shaker_bed_pos - FullProductStru.shaker_part_length -FullProductStru.mattress_height - 1;
    int button= QMessageBox::question(NULL, "提示", "确定要 检测微调位置 吗?\n移动前,请注意安全!", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if(button == QMessageBox::Yes)
    {
        emit msg_log(MSG_INFO, QString("确定要 检测微调位置 吗?用户选择:YES"), false);
        if(m_bsp->axis[0].APS_GetSVON())
        {
            m_bsp->axis[0].APS_AMove(test_pos, m_ss.z_ratio_Z);

            try
            {
                m_bsp->axis[0].APS_WaitMoveDone();
            }
            catch(const std::runtime_error& e)
            {
                emit msg_log(MSG_ALM, QString("%1").arg(e.what()), true);
            }

            emit msg_log(MSG_INFO, "轴 运动到检测微调位置 完成!", true);
        }
        else
            emit msg_log(MSG_INFO, "请先励磁轴Z!", true);
    }
}

//模式切换响应函数
void ProductSet::cur_model_change(QString model_str)
{
    this->update_pattern_to_combobox(this->FullProductStru.experiment_pattern);
}

// 刷新table表
void ProductSet::update_table_values(QTableWidget*table, QVector<Product_Param> data)
{
    table->setRowCount(0);
    table->clearContents();
    for(int i=0;i<data.size();i++)
    {
        int index = i;
        table->insertRow(index);
        QTableWidgetItem *curItem = new QTableWidgetItem(data[i].waveform_path);
        curItem->setFlags(curItem->flags() & ~Qt::ItemIsEditable);
        table->setItem(index, 0, curItem); //第一列设置成不可编辑
        table->setItem(index, 1, new QTableWidgetItem(QString::number(data[i].sampling_rate)));
        table->setItem(index, 2, new QTableWidgetItem(QString::number(data[i].acquisition_time)));
        table->setItem(index, 3, new QTableWidgetItem(QString::number(data[i].y_axis_position)));
        table->setItem(index, 4, new QTableWidgetItem(data[i].waveform_type));
        //居中
        table->item(index, 0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        table->item(index, 1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        table->item(index, 2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        table->item(index, 3)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        table->item(index, 4)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    }
}

// 信号文件目录设置
void ProductSet::pb_sine_dir_set()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                 tr("选择目录"),
                                                 tmp_root_dir);
    if(!dir.isEmpty())
    {
        ui->sine_dir->setText(dir);
    }
}

/*
 *==============================================================================
 *   读取/保存参数
 *==============================================================================
 */

// load参数
void ProductSet::load_param()
{
    /* add your code */

    try
    {
        this->read_db_of_table(current_product_db_path,"ProcessParam",this->FullProductStru.mProduct_Param_List);
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("加载 模板 参数错误:%1").arg(e.what()), true);
        return;
    }
    try
    {
        this->current_product_model= this->read_product_pattern(current_product_name);
        FullProductStru.experiment_pattern = this->current_product_model;
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("加载 产品模式 参数错误:%1").arg(e.what()), true);
        return;
    }
    try
    {
        this->read_template_param(current_product_name,this->FullProductStru);
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("加载 模版参数 参数错误:%1").arg(e.what()), true);
        return;
    }
    ui->sine_dir->setText(FullProductStru.product_name);

    /* 以下代码是模板代码,实际项目实际设计 */
//    try
//    {
//        product_param.str_default = sql_data_cell.read_db(current_product_db_path, "DefaultTemplate", "DefaultTemplate");
//    }
//    catch(const std::runtime_error& e)
//    {
//        emit msg_log(MSG_INFO, QString("加载 模板 参数错误:%1").arg(e.what()), true);
//        return;
//    }
//    ui->test_str->setText(product_param.str_default);

    // 发送信号函数,刷新参数
    emit sig_update_current_product_pattern(this->FullProductStru.experiment_pattern);
    emit sig_update_product_param(FullProductStru);
    emit sig_update_pd_param(FullProductStru);

    emit sig_get_current_procuct_name(current_product_name);
    emit sig_update_current_product_db_path(QString("%1/data.db").arg(current_product_dir));
}

// 保存参数
void ProductSet::save_param()
{
    QString cur_row_text = "";
    QStringList tempListStr = {};
    /* 以下代码是模板代码,实际项目实际设计 */
    int process_rows = ui->sine_table->rowCount();
    int process_field_count = ui->sine_table->columnCount();
    this->clear_product_param_list(this->FullProductStru.mProduct_Param_List);
    for(int i=0;i<process_rows;i++){
        tempListStr.clear();
        for(int j=0;j<process_field_count;j++){
            QTableWidgetItem *item = ui->sine_table->item(i,j);
            cur_row_text = item->text();
            tempListStr.append(cur_row_text);
        }
        this->product_param.waveform_path = tempListStr[0];
        this->product_param.sampling_rate = tempListStr[1].toInt();
        this->product_param.acquisition_time = tempListStr[2].toInt();
        this->product_param.y_axis_position = tempListStr[3].toInt();
        this->product_param.waveform_type = tempListStr[4];




        this->insert_product_param_to_list(this->FullProductStru.mProduct_Param_List,this->product_param);
    }

    try
    {
        this->save_db_of_table(current_product_db_path,"ProcessParam",this->FullProductStru.mProduct_Param_List);
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("模板 流程参数保存失败：%1").arg(e.what()), true);
        return;
    }

//    this->get_template_param(this->read_product_pattern(current_product_name));
    this->get_template_param(0,this->FullProductStru);
    try
    {
        this->save_template_param(this->current_product_name,this->FullProductStru);
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("模板 参数保存失败：%1").arg(e.what()), true);
        return;
    }

    for(Product_Param val : FullProductStru.mProduct_Param_List)
    {
        QString sourceDir = ROOT_PATH_TWO + val.waveform_path;
        copyFileToPath(sourceDir, current_product_dir+"/"+val.waveform_path, true);
    }


    // 参数保存成功提示
    emit msg_log(MSG_INFO, "保存参数成功!", true);
    // 重新加载,并向其他模块更新参数
    this->load_param();
}

//拷贝文件
bool ProductSet::copyFileToPath(QString sourceDir ,QString toDir, bool coverFileIfExist)
{

    toDir.replace("\\","/");
    if (sourceDir == toDir){
            return true;
        }
    if (!QFile::exists(sourceDir)){
        return false;
    }
    QDir *createfile     = new QDir;
    bool exist = createfile->exists(toDir);
    if (exist){
        if(coverFileIfExist){
            createfile->remove(toDir);
        }
    }
    if(!QFile::copy(sourceDir, toDir))
    {
        return false;
    }
    return true;
}

/*
 *==============================================================================
 *   模块固有功能函数
 *==============================================================================
 */

// 获取当前产品名称
QString ProductSet::get_current_procuct_name ()
{
    return current_product_name;
}

// 获取当前产品文件夹路径
QString ProductSet::get_current_procuct_dir()
{
    return current_product_dir;
}

// 读取产品路径下的列表
void ProductSet::update_product_list()
{
    ui->productlist->clear();
    QDir sourceDir(product_root_dir);
    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList)
    {
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if(fileInfo.isDir())    // 当为目录时，递归的进行copy
        {
            ui->productlist->addItem(fileInfo.fileName());
        }
    }
}

// 新建产品
void ProductSet::pb_new_product()
{
    QString name = ui->name->text();
    QString new_p_dir = product_root_dir + "/" + name;
    int model = ui->combo_new_prodect_model->currentIndex();
    if(name.isEmpty())
    {
        emit msg_log(MSG_INFO, "请输入产品名称后再创建!", true);
        return;
    }
    if(model == -1)
    {
        emit msg_log(MSG_INFO, "请选择产品模式后再创建!", true);
        return;
    }
    //创建根目录
    QDir dir_r(new_p_dir);
    if(dir_r.exists())
    {
        emit msg_log(MSG_INFO, "您要创建的产品已经存在!", true);
        return;
    }
    else
    {
        if(copyDirectoryFiles(current_product_dir, new_p_dir, true))
            emit msg_log(MSG_INFO, QString("产品: %1 创建成功!").arg(name), true);
    }
    update_product_list();

    this->save_product_pattern(name,model);
    this->get_template_param(model,this->FullProductStru);//从界面上获取模版参数
    this->save_template_param(name,this->FullProductStru);//保存模版参数到数据库


    // 设置背景色
    QListWidgetItem *item = ui->productlist->findItems(current_product_name, Qt::MatchExactly).at(0);
    item->setBackgroundColor(Qt::green);
}

// 载入产品
void ProductSet::pb_load_product()
{
    // 重置背景色
    QListWidgetItem *item = ui->productlist->findItems(current_product_name, Qt::MatchExactly).at(0);
    item->setBackgroundColor(Qt::white);

    QString select_product = ui->productlist->currentItem()->text();
    // 保存当前产品名称
    try
    {
        sql_data_cell.save_db(sql_data_cell.get_db_path(), "Product", "currentProduct", select_product);
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("产品加载错误：%1").arg(e.what()), true);
        return;
    }
    current_product_name = select_product;
    FullProductStru.product_name = select_product;

    current_product_dir = product_root_dir + "/" + current_product_name;
    current_product_db_path = current_product_dir + "/data.db";

    load_param();

    // 设置新的item的背景色
    item = ui->productlist->findItems(current_product_name, Qt::MatchContains).at(0);
    item->setBackgroundColor(Qt::green);
    this->update_table_values(ui->sine_table,this->FullProductStru.mProduct_Param_List);
    this->update_template_param_to_spin_boxs(this->FullProductStru.experiment_pattern,this->FullProductStru);
    this->update_pattern_to_combobox(this->FullProductStru.experiment_pattern);

}

// 删除产品
void ProductSet::pb_delete_product()
{
    //
    QString tmp_name = ui->productlist->currentItem()->text();
    if(tmp_name == current_product_name)
    {
        emit msg_log(MSG_INFO, "当前载入产品不可删除!", true);
        return;
    }
    if(QMessageBox::information(0, "提示", QString("确定要删除产品: %1 吗?").arg(tmp_name), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)==QMessageBox::Yes)
    {
        emit msg_log(MSG_INFO, QString("确定要删除产品: %1 吗?用户选择：YES").arg(tmp_name), true);
        QString tmp_path = product_root_dir + "/" + tmp_name;
        if(DeleteDir(tmp_path))
            emit msg_log(MSG_INFO, QString("产品: %1 删除成功!").arg(tmp_name), true);
        else
            emit msg_log(MSG_INFO, QString("产品: %1 删除失败!").arg(tmp_name), true);
    }

    update_product_list();

    // 设置背景色
    QListWidgetItem *item = ui->productlist->findItems(current_product_name, Qt::MatchExactly).at(0);
    item->setBackgroundColor(Qt::green);
}

//拷贝文件夹：
bool ProductSet::copyDirectoryFiles(const QString &fromDir, const QString &toDir, bool coverFileIfExist)
{
    QDir sourceDir(fromDir);
    QDir targetDir(toDir);
    if(!targetDir.exists()){    // 如果目标目录不存在，则进行创建
        if(!targetDir.mkdir(targetDir.absolutePath()))
            return false;
    }

    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList){
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if(fileInfo.isDir()){    // 当为目录时，递归的进行copy
            if(!copyDirectoryFiles(fileInfo.filePath(),
                targetDir.filePath(fileInfo.fileName()),
                coverFileIfExist))
                return false;
        }
        else{            // 当允许覆盖操作时，将旧文件进行删除操作
            if(coverFileIfExist && targetDir.exists(fileInfo.fileName())){
                targetDir.remove(fileInfo.fileName());
            }

            // 进行文件copy
            if(!QFile::copy(fileInfo.filePath(),
                targetDir.filePath(fileInfo.fileName()))){
                    return false;
            }
        }
    }
    return true;
}

//删除文件夹
bool ProductSet::DeleteDir(const QString &path)
{
    if (path.isEmpty()){
        return false;
    }
    QDir dir(path);
    if(!dir.exists()){
        return true;
    }
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //设置过滤
    QFileInfoList fileList = dir.entryInfoList(); // 获取所有的文件信息
    foreach (QFileInfo file, fileList){ //遍历文件信息
        if (file.isFile()){ // 是文件，删除
            file.dir().remove(file.fileName());
        }else{ // 递归删除
            DeleteDir(file.absoluteFilePath());
        }
    }
    return dir.rmpath(dir.absolutePath()); // 删除文件夹
}

// 信号测试添加一行
void ProductSet::pb_sine_add()
{
    int row = ui->sine_table->rowCount();
    ui->sine_table->insertRow(row);

    for(int i=0;i<ui->sine_table->columnCount();i++)
    {
        QTableWidgetItem *curItem = new QTableWidgetItem("0");
        if(i == 0)
        {
            curItem->setFlags(curItem->flags() & ~Qt::ItemIsEditable);
        }
        ui->sine_table->setItem(row, i, curItem);
        ui->sine_table->item(row, i)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    }
}

// 信号测试删除一行
void ProductSet::pb_sine_delete()
{
    ui->sine_table->removeRow(ui->sine_table->currentRow());
}

//更新模版数据到面板上
void ProductSet::update_template_param_to_spin_boxs(int pattern,Full_Product_Param data)
{
    this->ui->sensor_height->setValue(data.sensor_height);
    this->ui->sensor_out_board_height->setValue(data.sensor_out_board_height);
    this->ui->mattress_height->setValue(data.mattress_height);
    this->ui->shaker_press_depth->setValue(data.shaker_press_depth);
    this->ui->shaker_part_length->setValue(data.shaker_part_length);
    this->ui->little_distance->setValue(data.little_distance);
    this->ui->shaker_z_pos->setValue(data.shaker_z_pos);
}

//更新产品模式到下拉框
void ProductSet::update_pattern_to_combobox(int pattern)
{
    if (pattern <0 || pattern >3)
    {
        pattern = -1;
    }
    this->ui->ecl_en->setCurrentIndex(pattern);

    if(pattern == 0)
    {
        ui->label_pattern->setText("床垫厚度:");
    }
    else if(pattern == 1)
    {
        ui->label_pattern->setText("海绵工装厚度:");
    }
    else if(pattern == 2){
        ui->label_pattern->setText("裸板工装厚度:");
    }
    else if(pattern == 3){
        ui->label_pattern->setText("带壳裸板工装厚度:");
    }

}

//从界面上获取模板参数
/*
 * pattern:
 * 0:类床
 * 1：海绵
 * 2：裸板
 * 3：带壳裸板
*/
bool ProductSet::get_template_param(int pattern,Full_Product_Param &data)
{
    pattern = 0;
    QString str_temp = "";
    if (pattern == 0 || pattern == 1)
    {
        str_temp = this->ui->sensor_height->text();
        if(str_temp.isEmpty())
        {
            emit msg_log(MSG_INFO, "请输入传感器高度!", true);
            return false;
        }
        data.sensor_height = str_temp.toDouble();

        str_temp = this->ui->sensor_out_board_height->text();
        if(str_temp.isEmpty())
        {
            emit msg_log(MSG_INFO, "请输入传感器突出床板高度!", true);
            return false;
        }
        data.sensor_out_board_height = str_temp.toDouble();
    }

    str_temp = this->ui->mattress_height->text();
    if(str_temp.isEmpty())
    {
        emit msg_log(MSG_INFO, "请输入床垫厚度!", true);
        return false;
    }
    data.mattress_height = str_temp.toDouble();

    str_temp = this->ui->shaker_press_depth->text();
    if(str_temp.isEmpty())
    {
        emit msg_log(MSG_INFO, "请输入激振器头预压深度!", true);
        return false;
    }
    data.shaker_press_depth = str_temp.toDouble();


    str_temp = this->ui->shaker_part_length->text();
    if(str_temp.isEmpty())
    {
        emit msg_log(MSG_INFO, "请输入激振器头加装部件长度!", true);
        return false;
    }
    data.shaker_part_length = str_temp.toDouble();

    str_temp = this->ui->little_distance->text();
    if(str_temp.isEmpty())
    {
        emit msg_log(MSG_INFO, "请输入微调距离!", true);
        return false;
    }
    data.little_distance = str_temp.toDouble();

    str_temp = this->ui->shaker_z_pos->text();
    if(str_temp.isEmpty())
    {
        emit msg_log(MSG_INFO, "请读取激振器数值!", true);
        return false;
    }
    data.shaker_z_pos = str_temp.toDouble();
    return true;
}

//选择波形文件
QString ProductSet::choose_waveform()
{
    QString file_name = ".";
    int cur_focus_column = ui->sine_table->currentColumn();
    if(cur_focus_column == 0){
        QString file_name_full = QFileDialog::getOpenFileName(this,
                                     tr("选择波形"),
                                     "F",
                                     tr("文本文件(*txt)"));
        if(file_name_full.length() == 0){
            return ".";
        }

        int cur_focus_row = ui->sine_table->currentRow();
        file_name = QFileInfo(file_name_full).fileName();
//        qDebug()<<file_name;
        // 进行文件copy
        QString to_dir = current_product_dir+"/"+file_name;
        if(QFile::exists(to_dir)){
            QFile::remove(to_dir);
        }
        if(!QFile::copy(file_name_full,to_dir)){
            qDebug()<<"拷贝失败";
            return ".";
        }

        return file_name;
    }
}

//往结构体迭代器中插入一个产品参数(一行)结构体
bool ProductSet::insert_product_param_to_list(QVector<Product_Param> &Product_Param_List, Product_Param productParam)
{
    try
    {
        Product_Param_List.push_back(productParam);
        return true;
    }
    catch(const std::runtime_error& e)
    {
        qDebug()<<QString("结构体Vector插入失败：%1").arg(e.what());
        return false;
    }

}

//清空结构体迭代器
bool ProductSet::clear_product_param_list(QVector<Product_Param> &Product_Param_List)
{
    Product_Param_List.clear();
    return true;
}

/*
 *==============================================================================
 *   数据库操作
 *==============================================================================
 */

/*
 * @ brief  保存到数据库(适用于表结构的数据保存)
 * @ param  dbPath:     db文件路径
 * @ param  table:      表名称
 * @ param  data:      表结构的数据
 * @ retval .
 */
//void ProductSet::save_db_of_table(QString dbPath, QString table, QVector<QStringList> data)
void ProductSet::save_db_of_table(QString dbPath, QString table, QVector<Product_Param> data)
{
    QWriteLocker Locker(&pLock);

    QSqlDatabase db;
    if(QSqlDatabase::contains("ProductSet_save"))
        db = QSqlDatabase::database("ProductSet_save");
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE", "ProductSet_save");
    }
    db.setDatabaseName(dbPath);

    if(!db.open())
    {
        QString estr;
        estr = "ProductSet_save open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);

    /* 此处指令根据实际情况变更 */
    QString cmd;
//    cmd = QString("CREATE TABLE IF NOT EXISTS %1 (id INTEGER PRIMARY KEY, "
//                  "file TEXT, cycle TEXT, ypos TEXT, sample TEXT, type TEXT)").arg(table);
    cmd = QString("CREATE TABLE IF NOT EXISTS %1 (id INTEGER PRIMARY KEY,WaveformPath TEXT,SamplingRate TEXT, AcquisitionTime TEXT, YAxisPosition TEXT, Wavetype TEXT)").arg(table);
    qry.prepare(cmd);
    if( !qry.exec() )
    {
        QString estr;
        estr = "ProductSet_save:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //查询
    qry.prepare(QString("SELECT * FROM %1").arg(table));
    if( !qry.exec() )
    {
        QString estr;
        estr = "ProductSet_save:SELECT error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        if(qry.next())
        {
            qry.prepare(QString("DELETE FROM %1").arg(table));
            if( !qry.exec() )
            {
                QString estr;
                estr = "sql_data_save:DELETE error " + qry.lastError().text();
                throw std::runtime_error(estr.toLatin1().data());
            }
        }
        for(int i=0;i<data.size();i++)
        {
            qry.prepare(QString("INSERT INTO %1 VALUES (?, ?, ?, ?, ?, ?)").arg(table));
            qry.addBindValue(i);
            qry.addBindValue(data[i].waveform_path);
            qry.addBindValue(data[i].sampling_rate);
            qry.addBindValue(data[i].acquisition_time);
            qry.addBindValue(data[i].y_axis_position);
            qry.addBindValue(data[i].waveform_type);


            if( !qry.exec() )
            {
                QString estr;
                estr = "ProductSet_save:INSERT error " + qry.lastError().text();
                throw std::runtime_error(estr.toLatin1().data());
            }
        }
    }
    db.close();
}

/*
 * @ brief  从数据库读取(适用于表结构的数据读取)
 * @ param  dbPath:     db文件路径
 * @ param  table:      表名称
 * @ param  *data:      表结构的数据
 * @ retval .
 */
//void ProductSet::read_db_of_table(QString dbPath, QString table, QVector<QStringList> *data)
void ProductSet::read_db_of_table(QString dbPath, QString table, QVector<Product_Param> &data)
{
    QReadLocker Locker(&pLock);
    QSqlDatabase db;
    if(QSqlDatabase::contains("ProductSet_read"))
      db = QSqlDatabase::database("ProductSet_read");
    else
      db = QSqlDatabase::addDatabase("QSQLITE", "ProductSet_read");

    db.setDatabaseName( dbPath );// 设置数据库名与路径, 此时是放在上一个目录

    if( !db.open() )
    {
        QString estr;
        estr = "ProductSet_read open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);
    // 若不存在,则创建
    /* 此处指令根据实际情况变更 */
    QString cmd;
    //cmd = QString("CREATE TABLE IF NOT EXISTS %1 (id INTEGER PRIMARY KEY, "
      //            "file TEXT, cycle TEXT, ypos TEXT, sample TEXT, type TEXT)").arg(table);
    cmd = QString("CREATE TABLE IF NOT EXISTS %1 (id INTEGER PRIMARY KEY,WaveformPath TEXT, SamplingRate TEXT, AcquisitionTime TEXT, YAxisPosition TEXT, Wavetype TEXT)").arg(table);
    qry.prepare(cmd);

    if( !qry.exec() )
    {
        QString estr;
        estr = "ProductSet_read:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //查询
    qry.prepare(QString("SELECT * FROM %1").arg(table));
    if( !qry.exec() )
    {
        QString estr;
        estr = "ProductSet_read:SELECT error! " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        this->clear_product_param_list(data);
        while(qry.next())
        {
            this->product_param.waveform_path = qry.value(1).toString();
            this->product_param.sampling_rate = qry.value(2).toString().toInt();
            this->product_param.acquisition_time = qry.value(3).toString().toInt();
            this->product_param.y_axis_position = qry.value(4).toString().toInt();
            this->product_param.waveform_type = qry.value(5).toString();
            this->insert_product_param_to_list(data,this->product_param);
        }
    }
    db.close();
}

//保存产品模式
bool ProductSet::save_product_pattern(QString productName,int modelIndex)
{
    QString dbPath = product_root_dir+"/"+productName+"/data.db";
    QString table = "ProductModel";

    QWriteLocker Locker(&pLock);

    QSqlDatabase db;
    if(QSqlDatabase::contains("ProductSet_save"))
        db = QSqlDatabase::database("ProductSet_save");
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE", "ProductSet_save");
    }
    db.setDatabaseName(dbPath);

    if(!db.open())
    {
        QString estr;
        estr = "ProductSet_save open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);

    /* 此处指令根据实际情况变更 */
    QString cmd;
    cmd = QString("CREATE TABLE IF NOT EXISTS %1 (id INTEGER PRIMARY KEY,ProductModel TEXT)").arg(table);
    qry.prepare(cmd);
    if( !qry.exec() )
    {
        QString estr;
        estr = "ProductSet_save:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //查询
    qry.prepare(QString("SELECT * FROM %1").arg(table));
    if( !qry.exec() )
    {
        QString estr;
        estr = "ProductSet_save:SELECT error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        if(qry.next())
        {
            qry.prepare(QString("DELETE FROM %1").arg(table));
            if( !qry.exec() )
            {
                QString estr;
                estr = "sql_data_save:DELETE error " + qry.lastError().text();
                throw std::runtime_error(estr.toLatin1().data());
            }
        }

        qry.prepare(QString("INSERT INTO %1 VALUES (?, ?)").arg(table));
        qry.addBindValue(0);
        qry.addBindValue(QString::number(modelIndex));
        if( !qry.exec() )
        {
            QString estr;
            estr = "ProductSet_save:INSERT error " + qry.lastError().text();
            throw std::runtime_error(estr.toLatin1().data());
        }

    }
    db.close();
}

//读取产品模式
int ProductSet::read_product_pattern(QString productName)
{
    int product_model = -1;
    QString dbPath = product_root_dir+"/"+productName+"/data.db";
    QString table = "ProductModel";

    QReadLocker Locker(&pLock);
    QSqlDatabase db;
    if(QSqlDatabase::contains("ProductSet_read"))
      db = QSqlDatabase::database("ProductSet_read");
    else
      db = QSqlDatabase::addDatabase("QSQLITE", "ProductSet_read");

    db.setDatabaseName( dbPath );// 设置数据库名与路径, 此时是放在上一个目录

    if( !db.open() )
    {
        QString estr;
        estr = "ProductSet_read open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);
    // 若不存在,则创建
    /* 此处指令根据实际情况变更 */
    QString cmd;
    cmd = QString("CREATE TABLE IF NOT EXISTS %1 (id INTEGER PRIMARY KEY,ProductModel TEXT)").arg(table);
    qry.prepare(cmd);

    if( !qry.exec() )
    {
        QString estr;
        estr = "ProductSet_read:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //查询
    qry.prepare(QString("SELECT * FROM %1").arg(table));
    if( !qry.exec() )
    {
        QString estr;
        estr = "ProductSet_read:SELECT error! " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        while(qry.next())
        {
            product_model = qry.value(1).toString().toInt();
        }
    }
    db.close();
    return product_model;
}

//保存模版参数
bool ProductSet::save_template_param(QString productName,Full_Product_Param data)
{
    QWriteLocker Locker(&pLock);
    QString dbPath = product_root_dir+"/"+productName+"/data.db";
    QString table = "PatternParam";
    QSqlDatabase db;
    if(QSqlDatabase::contains("ProductSet_save"))
        db = QSqlDatabase::database("ProductSet_save");
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE", "ProductSet_save");
    }
    db.setDatabaseName(dbPath);

    if(!db.open())
    {
        QString estr;
        estr = "ProductSet_save open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);

    /* 此处指令根据实际情况变更 */
    QString cmd;
    cmd = QString("CREATE TABLE IF NOT EXISTS %1 (id INTEGER PRIMARY KEY,SensorHeight TEXT,SensorOutBoardHeight TEXT, MattressHeight TEXT, ShakerPressDepth TEXT, ShakerPartLength TEXT ,LittleDintance TEXT, ShakerPos TEXT)").arg(table);
    qry.prepare(cmd);
    if( !qry.exec() )
    {
        QString estr;
        estr = "ProductSet_save:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //查询
    qry.prepare(QString("SELECT * FROM %1").arg(table));
    if( !qry.exec() )
    {
        QString estr;
        estr = "ProductSet_save:SELECT error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        if(qry.next())
        {
            qry.prepare(QString("DELETE FROM %1").arg(table));
            if( !qry.exec() )
            {
                QString estr;
                estr = "sql_data_save:DELETE error " + qry.lastError().text();
                throw std::runtime_error(estr.toLatin1().data());
            }
        }
        for(int i=0;i<1;i++)
        {
            qry.prepare(QString("INSERT INTO %1 VALUES (?, ?, ?, ?, ?, ?, ?, ?)").arg(table));
            qry.addBindValue(i);
            qry.addBindValue(data.sensor_height);
            qry.addBindValue(data.sensor_out_board_height);
            qry.addBindValue(data.mattress_height);
            qry.addBindValue(data.shaker_press_depth);
            qry.addBindValue(data.shaker_part_length);
            qry.addBindValue(data.little_distance);
            qry.addBindValue(data.shaker_z_pos);



            if( !qry.exec() )
            {
                QString estr;
                estr = "ProductSet_save:INSERT error " + qry.lastError().text();
                throw std::runtime_error(estr.toLatin1().data());
            }
        }
    }
}

//读取模版参数
bool ProductSet::read_template_param(QString productName,Full_Product_Param &data)
{
    QReadLocker Locker(&pLock);
    QString dbPath = product_root_dir+"/"+productName+"/data.db";
    QString table = "PatternParam";
    QSqlDatabase db;
    if(QSqlDatabase::contains("ProductSet_read"))
      db = QSqlDatabase::database("ProductSet_read");
    else
      db = QSqlDatabase::addDatabase("QSQLITE", "ProductSet_read");

    db.setDatabaseName( dbPath );// 设置数据库名与路径, 此时是放在上一个目录

    if( !db.open() )
    {
        QString estr;
        estr = "ProductSet_read open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);
    // 若不存在,则创建
    /* 此处指令根据实际情况变更 */
    QString cmd;
    cmd = QString("CREATE TABLE IF NOT EXISTS %1 (id INTEGER PRIMARY KEY,SensorHeight TEXT,SensorOutBoardHeight TEXT, MattressHeight TEXT, ShakerPressDepth TEXT, ShakerPartLength TEXT, LittleDintance TEXT,ShakerPos TEXT)").arg(table);
    qry.prepare(cmd);

    if( !qry.exec() )
    {
        QString estr;
        estr = "ProductSet_read:CREATE error " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //查询
    qry.prepare(QString("SELECT * FROM %1").arg(table));
    if( !qry.exec() )
    {
        QString estr;
        estr = "ProductSet_read:SELECT error! " + qry.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        while(qry.next())
        {
            data.sensor_height = qry.value(1).toString().toDouble();
            data.sensor_out_board_height = qry.value(2).toString().toDouble();
            data.mattress_height = qry.value(3).toString().toDouble();
            data.shaker_press_depth = qry.value(4).toString().toDouble();
            data.shaker_part_length = qry.value(5).toString().toDouble();
            data.little_distance = qry.value(6).toString().toDouble();
            data.shaker_z_pos = qry.value(7).toString().toDouble();
        }
    }
    db.close();
    return true;
}

// 产品编辑表格单击响应函数
void ProductSet::sine_table_itemClicked(QTableWidgetItem *item)
{
//    qDebug()<<"itemClicked"<<item->column()<<item->row();
    if (item->column() != 0)
    {
        return;
    }
    QString file_name = this->choose_waveform();
    if(file_name == ".")
    {
        return;
    }
    else {
        item->setText(file_name);
        this->ui->sine_table->setItem(item->row(),item->column(),item);
    }
}

// 产品编辑表格改变响应函数
void ProductSet::sine_table_itemChanged(QTableWidgetItem *item)
{
    if(item->column() == 1)
    {
        QString cur_text = item->text();
        int cur_value = cur_text.toInt();
        if(!this->isDigitStr(cur_text))
        {
            QMessageBox::warning(this,"warning","You should input a integer for sample rate ！");
            item->setText("5000");
            this->ui->sine_table->setItem(item->row(),item->column(),item);
        }
        else if(cur_value<0 || cur_value > 90000 || cur_text.length()>8)
        {
            QMessageBox::warning(this,"warning","You input a large number,please check it！");
            item->setText("5000");
            this->ui->sine_table->setItem(item->row(),item->column(),item);
        }
    }
    else if(item->column() == 2)
    {
        QString cur_text = item->text();
        int cur_value = cur_text.toInt();
        if(!this->isDigitStr(cur_text))
        {
            QMessageBox::warning(this,"warning","You should input a integer ！");
            item->setText("10");
            this->ui->sine_table->setItem(item->row(),item->column(),item);
        }
        else if(cur_value<0 || cur_value > 1000 || cur_text.length()>8)
        {
            QMessageBox::warning(this,"warning","You input large number,please check it！");
        }
    }
    else if(item->column() == 3)
    {
        QString cur_text = item->text();
        int cur_value = cur_text.toInt();
        if(!this->isDigitStr(cur_text))
        {
            QMessageBox::warning(this,"warning","You should input a integer ！");
            item->setText("300");
            this->ui->sine_table->setItem(item->row(),item->column(),item);
        }
        else if(cur_value<0 || cur_value > 1000 || cur_text.length()>8)
        {
            QMessageBox::warning(this,"warning","0-1000！");
            item->setText("300");
            this->ui->sine_table->setItem(item->row(),item->column(),item);
        }
    }
}

//判断字符串是否是一个正整数
bool ProductSet::isDigitStr(QString src)
{
    QByteArray ba = src.toLatin1();
    const char *s = ba.data();
    while(*s && *s>='0' && *s <='9')s++;
    if(*s)
    {
        return false;
    }
    else
    {
        return true;
    }
}

