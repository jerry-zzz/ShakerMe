/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2018, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * 文 件 名： query.cpp
 * 作    者： zone53
 * 版    本： 1.0
 * 日    期： 2019/02/18
 *
 ********* 简 介 ***************************************************************
 *
 *  	检测结果记录查询面板
 *
 ********************************** 历 史 版 本 ********************************
 * <Date>       <Name>     <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/02/18   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */

#include "query.h"
#include "ui_query.h"
#include "system_info.h"
#include "../Log/loginfo.h"
#include <QMessageBox>
#include <QProcess>
#include "Module/UserInfo/jobinfo.h"

extern JobInfo jobinfo;

Query::Query(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Query)
{
    ui->setupUi(this);

    //设为无边框
    setWindowFlags(Qt::FramelessWindowHint);

    this->setGeometry(NESTED_PANEL_POS_X, NESTED_PANEL_POS_Y, NESTED_PANEL_POS_W, NESTED_PANEL_POS_H);

    // 设置tableWidget的列宽
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(9, QHeaderView::Stretch);

    //禁止单元格编辑
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->search, &QPushButton::clicked, this, &Query::pb_conditional_search);
    //connect(ui->output_query, &QPushButton::clicked, this, &Query::pb_output_conditional_search);
   // connect(ui->output_reportform, &QPushButton::clicked, this, &Query::pb_output_report_form);
}

Query::~Query()
{
    this->close();
    delete ui;
}


void Query::display_panel()
{
    table_remove_rows();

    //设置日期为当天
    QDate date=QDate::currentDate();
    //ui->calendarWidget->setSelectedDate(date);

    QDate date_s = date.addDays(-1);
    QDate date_e = date;
    ui->dateStart->setDate(date_s);
    ui->dateEnd->setDate(date_e);

    //显示
    show();
}

void Query::hide_panel()
{
    hide();
}

// 清空表格
void Query::table_remove_rows()
{
    int row = ui->tableWidget->rowCount();
    for(int i=row-1;i>=0;i--)
        ui->tableWidget->removeRow(i);

    // 设置tableWidget的列宽
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(9, QHeaderView::Stretch);
//    ui->tableWidget->horizontalHeader()->setSectionResizeMode(10, QHeaderView::Stretch);
}

// 显示查询结果
void Query::table_show_query_result(QStringList rlist, bool is_hr_ng, bool is_br_ng, bool is_burn_ng, bool is_id_write_ng, bool is_id_read_ng, bool is_ver_read_ng)
{
    // 设置tableWidget的列宽
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(8, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(9, QHeaderView::ResizeToContents);
//    ui->tableWidget->horizontalHeader()->setSectionResizeMode(10, QHeaderView::ResizeToContents);

    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    for(int i=0;i<10;i++)
    {
        ui->tableWidget->setItem(row, i, new QTableWidgetItem(rlist.at(i)));
        ui->tableWidget->item(row, i)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        if(i == 5 && is_hr_ng)
            ui->tableWidget->item(row, i)->setBackgroundColor(Qt::red);
        else if(i == 6 && is_br_ng)
            ui->tableWidget->item(row, i)->setBackgroundColor(Qt::red);
        else if(i == 7 && is_id_write_ng)
            ui->tableWidget->item(row, i)->setBackgroundColor(Qt::red);
        else if(i == 8 && is_id_read_ng)
            ui->tableWidget->item(row, i)->setBackgroundColor(Qt::red);
        else if(i == 9 && is_ver_read_ng)
            ui->tableWidget->item(row, i)->setBackgroundColor(Qt::red);
    }
}



//-------------------------------------------------------------------------槽函数
// 更新系统设置参数
void Query::slot_from_SystemSetup_update_sys_param(Sys_Setup ss)
{
    m_ss = ss;
}


// 按照条件查找
#define BURN_EN             0x01        // 烧录程序NG
#define ID_WRIGHT_EN        0x02        // id写入NG
#define ID_READ_EN          0x04        // id读取NG
#define VERSION_READ_EN     0x08        // 软件版本读取NG
#define CHECK_HR_EN         0x10        // 心跳通道NG
#define CHECK_BR_EN         0X20        // 呼吸通道NG
void Query::pb_conditional_search()
{
    table_remove_rows();

    QString date_s = ui->dateStart->date().toString("yyyy-MM-dd") + " 00:00:00";
    QString date_e = ui->dateEnd->date().addDays(1).toString("yyyy-MM-dd") + " 00:00:00";

    int select_type = 0x00;
//    // 条件选择
   /* if(ui->burn_en->isChecked() == true)
        select_type = select_type | BURN_EN;*/
    if(ui->id_write_en->isChecked() == true)
        select_type = select_type | ID_WRIGHT_EN;
    if(ui->id_read_en->isChecked() == true)
        select_type = select_type | ID_READ_EN;
    if(ui->ver_read_en->isChecked() == true)
        select_type = select_type | VERSION_READ_EN;
    if(ui->hr_en->isChecked() == true)
        select_type = select_type | CHECK_HR_EN;
    if(ui->br_en->isChecked() == true)
        select_type = select_type | CHECK_BR_EN;

    int is_OK = 0;  // 1-合格; 0-不合格; -1-所记录
    if(ui->ok->isChecked() == true)
        is_OK = 1;
    else if(ui->ng->isChecked() == true)
        is_OK = 0;
    else if(ui->all->isChecked() == true)
        is_OK = -1;

    try
    {
        query_from_select_time_and_conditional(TEST_RECORD_FILE, date_s, date_e, select_type, is_OK);
    }
    catch(const std::runtime_error& e)
    {
        emit msg_log(MSG_INFO, QString("按条件查询错误：\n%1").arg(e.what()), true);
    }
}

// 导出按照条件查找的记录
/*
void Query::pb_output_conditional_search()
{
    QString file = QString(m_ss.record_path) + QString("/激振器检测-搜索记录-%1.xlsx").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss"));

    int row = ui->tableWidget->rowCount();
    if(row > 0)
    {
        this->setCursor(Qt::WaitCursor);
        if(export_excel_based_on_search(file))
        {
            QProcess process;
            process.startDetached("explorer /select, " + file.replace("/", "\\"));
        }
        this->setCursor(Qt::ArrowCursor);
    }
}
*/
/*
// 导出报表
void Query::pb_output_report_form()
{
    QDate date = ui->calendarWidget->selectedDate();

    QStringList op_list = jobinfo.get_user_id_history(date);

    QString file = QString(m_ss.record_path) + QString("/PCB检测-日生产报表-%1.xlsx").arg(date.toString("yyyy-MM-dd"));
    this->setCursor(Qt::WaitCursor);
    if(excel_write_data(file, date, op_list))
    {
        QProcess process;
        process.startDetached("explorer /select, " + file.replace("/", "\\"));
    }
    this->setCursor(Qt::ArrowCursor);
}
*/

//-----------------------------------------------------------------------功能函数

#include <ActiveQt/QAxObject>
bool Query::excel_write_data(QString file, QDate date, QStringList op_list)
{
    bool status = false;
    // step1：连接控件
    QAxObject* excel = new QAxObject(this);
    excel->setControl("Excel.Application");  // 连接Excel控件
    excel->dynamicCall("SetVisible (bool Visible)", "false"); // 不显示窗体
    excel->setProperty("DisplayAlerts", false);  // 不显示任何警告信息。如果为true, 那么关闭时会出现类似"文件已修改，是否保存"的提示

    // step2: 打开工作簿
    QAxObject* workbooks = excel->querySubObject("WorkBooks"); // 获取工作簿集合
    workbooks->dynamicCall("Add"); // 新建一个工作簿
    QAxObject* workbook = excel->querySubObject("ActiveWorkBook"); // 获取当前工作簿

    // step3: 打开sheet
    QAxObject* worksheet = workbook->querySubObject("WorkSheets(int)", 1); // 获取工作表集合的工作表1， 即sheet1

    //操作单元格
    QAxObject *cell = nullptr;
    QAxObject *font = nullptr;
    QString merge_cell;
    QAxObject *merge_range = nullptr;
    QAxObject* border = nullptr;

    {
    // A1
    cell = worksheet->querySubObject("Cells(int,int)", 1, 1);
    cell->setProperty("Value", "麒盛科技");  //设置单元格值
    cell->setProperty("RowHeight", 20);  //设置单元格行高
    cell->setProperty("ColumnWidth", 16.88);  //设置单元格列宽
    cell->setProperty("HorizontalAlignment", -4108); //左对齐（xlLeft）：-4131  居中（xlCenter）：-4108  右对齐（xlRight）：-4152
    cell->setProperty("VerticalAlignment", -4108);  //上对齐（xlTop）-4160 居中（xlCenter）：-4108  下对齐（xlBottom）：-4107
    cell->setProperty("WrapText", true);  //内容过多，自动换行
    font = cell->querySubObject("Font");  //获取单元格字体
    font->setProperty("Name", QStringLiteral("宋体"));  //设置单元格字体
    font->setProperty("Bold", true);  //设置单元格字体加粗
    font->setProperty("Size", 11);  //设置单元格字体大小
    font->setProperty("Color", QColor(255, 0, 0));  //设置单元格字体颜色

    // A2
    cell = worksheet->querySubObject("Cells(int,int)", 2, 1);
    cell->setProperty("Value", "KEESON");  //设置单元格值
    cell->setProperty("RowHeight", 20);  //设置单元格行高
    cell->setProperty("ColumnWidth", 16.88);  //设置单元格列宽
    cell->setProperty("HorizontalAlignment", -4108); //左对齐（xlLeft）：-4131  居中（xlCenter）：-4108  右对齐（xlRight）：-4152
    cell->setProperty("VerticalAlignment", -4108);  //上对齐（xlTop）-4160 居中（xlCenter）：-4108  下对齐（xlBottom）：-4107
    cell->setProperty("WrapText", true);  //内容过多，自动换行
    font = cell->querySubObject("Font");  //获取单元格字体
    font->setProperty("Name", QStringLiteral("宋体"));  //设置单元格字体
    font->setProperty("Bold", true);  //设置单元格字体加粗
    font->setProperty("Size", 11);  //设置单元格字体大小
    font->setProperty("Color", QColor(255, 0, 0));  //设置单元格字体颜色

    // A3
    cell = worksheet->querySubObject("Cells(int,int)", 3, 1);
    cell->setProperty("Value", "测试时间");  //设置单元格值
    cell->setProperty("RowHeight", 18);  //设置单元格行高
    font = cell->querySubObject("Font");  //获取单元格字体
    font->setProperty("Name", QStringLiteral("宋体"));  //设置单元格字体
    font->setProperty("Bold", true);  //设置单元格字体加粗
    font->setProperty("Size", 11);  //设置单元格字体大小
    font->setProperty("Color", QColor(0, 0, 0));  //设置单元格字体颜色

    // B1
    cell = worksheet->querySubObject("Cells(int,int)", 1, 2);
    cell->setProperty("Value", "PCB测试日生产报表");  //设置单元格值
    cell->setProperty("ColumnWidth", 16);  //设置单元格列宽
    font = cell->querySubObject("Font");  //获取单元格字体
    font->setProperty("Name", QStringLiteral("宋体"));  //设置单元格字体
    font->setProperty("Bold", true);  //设置单元格字体加粗
    font->setProperty("Size", 18);  //设置单元格字体大小
    font->setProperty("Color", QColor(0, 0, 0));  //设置单元格字体颜色
    merge_cell = QString("B1:E2");
    merge_range = worksheet->querySubObject("Range(const QString&)", merge_cell);
    merge_range->setProperty("HorizontalAlignment", -4108);
    merge_range->setProperty("VerticalAlignment", -4108);
    merge_range->setProperty("WrapText", true);
    merge_range->setProperty("MergeCells", true);  //合并单元格

    // B3
    cell = worksheet->querySubObject("Cells(int,int)", 3, 2);
    cell->setProperty("Value", "产品型号");  //设置单元格值
    font = cell->querySubObject("Font");  //获取单元格字体
    font->setProperty("Name", QStringLiteral("宋体"));  //设置单元格字体
    font->setProperty("Bold", true);  //设置单元格字体加粗
    font->setProperty("Size", 11);  //设置单元格字体大小
    font->setProperty("Color", QColor(0, 0, 0));  //设置单元格字体颜色


    // C1
    cell = worksheet->querySubObject("Cells(int,int)", 1, 3);
    cell->setProperty("ColumnWidth", 10);  //设置单元格列宽

    // C3
    cell = worksheet->querySubObject("Cells(int,int)", 3, 3);
    cell->setProperty("Value", "测试数量");  //设置单元格值
    font = cell->querySubObject("Font");  //获取单元格字体
    font->setProperty("Name", QStringLiteral("宋体"));  //设置单元格字体
    font->setProperty("Bold", true);  //设置单元格字体加粗
    font->setProperty("Size", 11);  //设置单元格字体大小
    font->setProperty("Color", QColor(0, 0, 0));  //设置单元格字体颜色

    // D1
    cell = worksheet->querySubObject("Cells(int,int)", 1, 4);
    cell->setProperty("ColumnWidth", 10);  //设置单元格列宽

    // D3
    cell = worksheet->querySubObject("Cells(int,int)", 3, 4);
    cell->setProperty("Value", "合格数量");  //设置单元格值
    font = cell->querySubObject("Font");  //获取单元格字体
    font->setProperty("Name", QStringLiteral("宋体"));  //设置单元格字体
    font->setProperty("Bold", true);  //设置单元格字体加粗
    font->setProperty("Size", 11);  //设置单元格字体大小
    font->setProperty("Color", QColor(0, 0, 0));  //设置单元格字体颜色

    // E3
    cell = worksheet->querySubObject("Cells(int,int)", 3, 5);
    cell->setProperty("ColumnWidth", 10);  //设置单元格列宽
    cell->setProperty("Value", "不合格数量");  //设置单元格值
    font = cell->querySubObject("Font");  //获取单元格字体
    font->setProperty("Name", QStringLiteral("宋体"));  //设置单元格字体
    font->setProperty("Bold", true);  //设置单元格字体加粗
    font->setProperty("Size", 11);  //设置单元格字体大小
    font->setProperty("Color", QColor(0, 0, 0));  //设置单元格字体颜色

    // F2
    cell = worksheet->querySubObject("Cells(int,int)", 2, 6);
    cell->setProperty("Value", QString("%1 月").arg(date.month()));  //设置单元格值
    cell->setProperty("ColumnWidth", 12);  //设置单元格列宽
    cell->setProperty("HorizontalAlignment", -4152); //左对齐（xlLeft）：-4131  居中（xlCenter）：-4108  右对齐（xlRight）：-4152
    cell->setProperty("VerticalAlignment", -4108);  //上对齐（xlTop）-4160 居中（xlCenter）：-4108  下对齐（xlBottom）：-4107
    cell->setProperty("WrapText", true);  //内容过多，自动换行
    font = cell->querySubObject("Font");  //获取单元格字体
    font->setProperty("Name", QStringLiteral("宋体"));  //设置单元格字体
    font->setProperty("Bold", true);  //设置单元格字体加粗
    font->setProperty("Size", 11);  //设置单元格字体大小
    font->setProperty("Color", QColor(0, 0, 0));  //设置单元格字体颜色

    // F3
    cell = worksheet->querySubObject("Cells(int,int)", 3, 6);
    cell->setProperty("Value", "合格率");  //设置单元格值
    font = cell->querySubObject("Font");  //获取单元格字体
    font->setProperty("Name", QStringLiteral("宋体"));  //设置单元格字体
    font->setProperty("Bold", true);  //设置单元格字体加粗
    font->setProperty("Size", 11);  //设置单元格字体大小
    font->setProperty("Color", QColor(0, 0, 0));  //设置单元格字体颜色

    // G1
    cell = worksheet->querySubObject("Cells(int,int)", 1, 7);
    cell->setProperty("Value", QString("%1 年").arg(date.year()));  //设置单元格值
    cell->setProperty("ColumnWidth", 12);  //设置单元格列宽
    cell->setProperty("HorizontalAlignment", -4152); //左对齐（xlLeft）：-4131  居中（xlCenter）：-4108  右对齐（xlRight）：-4152
    cell->setProperty("VerticalAlignment", -4108);  //上对齐（xlTop）-4160 居中（xlCenter）：-4108  下对齐（xlBottom）：-4107
    cell->setProperty("WrapText", true);  //内容过多，自动换行
    font = cell->querySubObject("Font");  //获取单元格字体
    font->setProperty("Name", QStringLiteral("宋体"));  //设置单元格字体
    font->setProperty("Bold", true);  //设置单元格字体加粗
    font->setProperty("Size", 11);  //设置单元格字体大小
    font->setProperty("Color", QColor(0, 0, 0));  //设置单元格字体颜色

    // G2
    cell = worksheet->querySubObject("Cells(int,int)", 2, 7);
    cell->setProperty("Value", QString("%1 日").arg(date.day()));  //设置单元格值
    cell->setProperty("HorizontalAlignment", -4152); //左对齐（xlLeft）：-4131  居中（xlCenter）：-4108  右对齐（xlRight）：-4152
    cell->setProperty("VerticalAlignment", -4108);  //上对齐（xlTop）-4160 居中（xlCenter）：-4108  下对齐（xlBottom）：-4107
    cell->setProperty("WrapText", true);  //内容过多，自动换行
    font = cell->querySubObject("Font");  //获取单元格字体
    font->setProperty("Name", QStringLiteral("宋体"));  //设置单元格字体
    font->setProperty("Bold", true);  //设置单元格字体加粗
    font->setProperty("Size", 11);  //设置单元格字体大小
    font->setProperty("Color", QColor(0, 0, 0));  //设置单元格字体颜色

    // G3
    cell = worksheet->querySubObject("Cells(int,int)", 3, 7);
    cell->setProperty("Value", "操作者");  //设置单元格值
    cell->setProperty("WrapText", true);  //内容过多，自动换行
    font = cell->querySubObject("Font");  //获取单元格字体
    font->setProperty("Name", QStringLiteral("宋体"));  //设置单元格字体
    font->setProperty("Bold", true);  //设置单元格字体加粗
    font->setProperty("Size", 11);  //设置单元格字体大小
    font->setProperty("Color", QColor(0, 0, 0));  //设置单元格字体颜色
    }

    // 读取数据库
    QString starttime;
    QString endtime;
    int i,j;
    int row = 4;
    int sum = 0;
    int sum_ok = 0;
    int sum_ng = 0;
    int merge_row = 0;
    int merge_lenth = 0;
    for(i=8,j=row;i<20;i++)
    {
        starttime = date.toString("yyyy-MM-dd") + QString(" %1:00:00").arg(i, 2, 10, QChar('0'));
        endtime = date.toString("yyyy-MM-dd") + QString(" %1:00:00").arg(i+1, 2, 10, QChar('0'));
        merge_row = j;
        merge_lenth = 0;
        try
        {
            // QHash<QString, QList<int>>: <产品型号, [总数, 合格数, 不合格数]>
            QHash<QString, QList<int>> info = query_from_select_time(TEST_RECORD_FILE, starttime, endtime);
            QMutableHashIterator<QString, QList<int>> k(info);
            while(k.hasNext())
            {
                QString tmp = k.next().key();
                int tmp_sum = info[tmp][0];
                int tmp_ok = info[tmp][1];
                int tmp_ng = info[tmp][2];
                sum += tmp_sum;
                sum_ok += tmp_ok;
                sum_ng += tmp_ng;

                // B列 产品型号
                cell = worksheet->querySubObject("Cells(int,int)", merge_row + merge_lenth, 2);
                cell->setProperty("Value", tmp);  //设置单元格值

                // C列 测试总数
                cell = worksheet->querySubObject("Cells(int,int)", merge_row + merge_lenth, 3);
                cell->setProperty("Value", QString::number(tmp_sum));  //设置单元格值

                // D列 测试合格数
                cell = worksheet->querySubObject("Cells(int,int)", merge_row + merge_lenth, 4);
                cell->setProperty("Value", QString::number(tmp_ok));  //设置单元格值

                // E列 测试不合格数
                cell = worksheet->querySubObject("Cells(int,int)", merge_row + merge_lenth, 5);
                cell->setProperty("Value", QString::number(tmp_ng));  //设置单元格值

                // F列 合格率
                cell = worksheet->querySubObject("Cells(int,int)", merge_row + merge_lenth, 6);
                cell->setProperty("Value", QString("%1%").arg(QString::number(tmp_ok*100.0/tmp_sum, 'f', 2)));  //设置单元格值

                merge_lenth++;
            }
            if(merge_lenth == 0)
            {
                // B列 产品型号
                cell = worksheet->querySubObject("Cells(int,int)", merge_row + merge_lenth, 2);
                cell->setProperty("Value", "----");  //设置单元格值

                // C列 测试总数
                cell = worksheet->querySubObject("Cells(int,int)", merge_row + merge_lenth, 3);
                cell->setProperty("Value", QString::number(0));  //设置单元格值

                // D列 测试合格数
                cell = worksheet->querySubObject("Cells(int,int)", merge_row + merge_lenth, 4);
                cell->setProperty("Value", QString::number(0));  //设置单元格值

                // E列 测试不合格数
                cell = worksheet->querySubObject("Cells(int,int)", merge_row + merge_lenth, 5);
                cell->setProperty("Value", QString::number(0));  //设置单元格值

                // F列 合格率
                cell = worksheet->querySubObject("Cells(int,int)", merge_row + merge_lenth, 6);
                cell->setProperty("Value", QString("NA"));  //设置单元格值
                merge_lenth = 1;
            }

            // A 时间
            cell = worksheet->querySubObject("Cells(int,int)", merge_row, 1);
            cell->setProperty("Value", QString("%1:00-%2:00").arg(i, 2, 10, QChar('0')).arg(i+1, 2, 10, QChar('0')));  //设置单元格值
            cell->setProperty("RowHeight", 15);  //设置单元格行高

            // G 操作员
            cell = worksheet->querySubObject("Cells(int,int)", merge_row, 7);
            cell->setProperty("Value", op_list.at(i-8));  //设置单元格值

            if(merge_lenth > 1)
            {
                merge_cell = QString("A%1:A%2").arg(merge_row).arg(merge_row+merge_lenth-1);
                merge_range = worksheet->querySubObject("Range(const QString&)", merge_cell);
                merge_range->setProperty("WrapText", true);
                merge_range->setProperty("MergeCells", true);  //合并单元格

                merge_cell = QString("G%1:G%2").arg(merge_row).arg(merge_row+merge_lenth-1);
                merge_range = worksheet->querySubObject("Range(const QString&)", merge_cell);
                merge_range->setProperty("WrapText", true);
                merge_range->setProperty("MergeCells", true);  //合并单元格
            }
            j = j + merge_lenth;
        }
        catch(const std::runtime_error& e)
        {
            emit msg_log(MSG_INFO, QString("导出报表错误\n数据查询错误:%1").arg(e.what()), true);
            goto Err;
        }
    }

    // 合计
    cell = worksheet->querySubObject("Cells(int,int)", j, 1);
    cell->setProperty("Value", "合计");  //设置单元格值
    cell->setProperty("RowHeight", 15);  //设置单元格行高

    cell = worksheet->querySubObject("Cells(int,int)", j, 3);
    cell->setProperty("Value", QString::number(sum));  //设置单元格值

    cell = worksheet->querySubObject("Cells(int,int)", j, 4);
    cell->setProperty("Value", QString::number(sum_ok));  //设置单元格值

    cell = worksheet->querySubObject("Cells(int,int)", j, 5);
    cell->setProperty("Value", QString::number(sum_ng));  //设置单元格值

    cell = worksheet->querySubObject("Cells(int,int)", j, 6);
    cell->setProperty("Value", QString("%1%").arg(QString::number(sum_ok*100.0/sum, 'f', 2)));  //设置单元格值

    merge_cell = QString("A3:G%1").arg(j);
    merge_range = worksheet->querySubObject("Range(const QString&)", merge_cell);
    merge_range->setProperty("HorizontalAlignment", -4108); //左对齐（xlLeft）：-4131  居中（xlCenter）：-4108  右对齐（xlRight）：-4152
    merge_range->setProperty("VerticalAlignment", -4108);  //上对齐（xlTop）-4160 居中（xlCenter）：-4108  下对齐（xlBottom）：-4107
    border = merge_range->querySubObject("Borders");
    border->setProperty("Color", QColor(0, 0, 0));   //设置单元格边框色

    // step6: 保存文件
    // 方式二：另存为
    workbook->dynamicCall("SaveAs(const QString&)", QDir::toNativeSeparators(file)); //保存到filepath
    status = true;
Err:
    // 注意一定要用QDir::toNativeSeparators, 将路径中的"/"转换为"\", 不然一定保存不了
    workbook->dynamicCall("Close (Boolean)", false);  //关闭文件
    excel->dynamicCall("Quit()");   // 关闭Excel
    return status;
}


bool Query::export_excel_based_on_search(QString file)
{
    bool status = false;
    // step1：连接控件
    QAxObject* excel = new QAxObject(this);
    excel->setControl("Excel.Application");  // 连接Excel控件
    excel->dynamicCall("SetVisible (bool Visible)", "false"); // 不显示窗体
    excel->setProperty("DisplayAlerts", false);  // 不显示任何警告信息。如果为true, 那么关闭时会出现类似"文件已修改，是否保存"的提示

    // step2: 打开工作簿
    QAxObject* workbooks = excel->querySubObject("WorkBooks"); // 获取工作簿集合
    workbooks->dynamicCall("Add"); // 新建一个工作簿
    QAxObject* workbook = excel->querySubObject("ActiveWorkBook"); // 获取当前工作簿

    // step3: 打开sheet
    QAxObject* worksheet = workbook->querySubObject("WorkSheets(int)", 1); // 获取工作表集合的工作表1， 即sheet1

    //操作单元格
    QAxObject *cell = nullptr;
    QAxObject *font = nullptr;
    QString merge_cell;
    QAxObject *merge_range = nullptr;
    QAxObject* border = nullptr;

    // A1
    cell = worksheet->querySubObject("Cells(int,int)", 1, 1);
    cell->setProperty("Value", "序号");  //设置单元格值
    cell->setProperty("RowHeight", 16);  //设置单元格行高
    cell->setProperty("ColumnWidth", 20);  //设置单元格列宽
    // B1
    cell = worksheet->querySubObject("Cells(int,int)", 1, 2);
    cell->setProperty("Value", "时间");  //设置单元格值
    // C1
    cell = worksheet->querySubObject("Cells(int,int)", 1, 3);
    cell->setProperty("Value", "PCB-ID");  //设置单元格值
    // D1
    cell = worksheet->querySubObject("Cells(int,int)", 1, 4);
    cell->setProperty("Value", "软件版本");  //设置单元格值
    // E1
    cell = worksheet->querySubObject("Cells(int,int)", 1, 5);
    cell->setProperty("Value", "结果");  //设置单元格值
    // F1
    cell = worksheet->querySubObject("Cells(int,int)", 1, 6);
    cell->setProperty("Value", "心跳通道");  //设置单元格值
    // G1
    cell = worksheet->querySubObject("Cells(int,int)", 1, 7);
    cell->setProperty("Value", "呼吸通道");  //设置单元格值
    // H1
    cell = worksheet->querySubObject("Cells(int,int)", 1, 8);
    cell->setProperty("Value", "程序烧录");  //设置单元格值
    // I1
    cell = worksheet->querySubObject("Cells(int,int)", 1, 9);
    cell->setProperty("Value", "id写入");  //设置单元格值
    // J1
    cell = worksheet->querySubObject("Cells(int,int)", 1, 10);
    cell->setProperty("Value", "id读取");  //设置单元格值
    // K1
    cell = worksheet->querySubObject("Cells(int,int)", 1, 11);
    cell->setProperty("Value", "软件版本读取");  //设置单元格值

    int i,j;
    int row = ui->tableWidget->rowCount();
    int column = ui->tableWidget->columnCount();
    QStringList cell_info;
    for(i=0;i<row;i++)
    {
        cell_info.clear();
        for(j=0;j<column;j++)
        {
            QTableWidgetItem *item = ui->tableWidget->item(i, j);

            cell = worksheet->querySubObject("Cells(int,int)", i+2, j+1);

            if(j == 2)
                cell->setProperty("NumberFormatLocal", "@");  // 设置为文本
            cell->setProperty("Value", item->text());  //设置单元格值
            cell->setProperty("RowHeight", 16);  //设置单元格行高
            cell->setProperty("ColumnWidth", 22);  //设置单元格列宽
        }
    }

    merge_cell = QString("A1:K%1").arg(i+1);
    merge_range = worksheet->querySubObject("Range(const QString&)", merge_cell);
    merge_range->setProperty("HorizontalAlignment", -4108); //左对齐（xlLeft）：-4131  居中（xlCenter）：-4108  右对齐（xlRight）：-4152
    merge_range->setProperty("VerticalAlignment", -4108);  //上对齐（xlTop）-4160 居中（xlCenter）：-4108  下对齐（xlBottom）：-4107
    border = merge_range->querySubObject("Borders");
    border->setProperty("Color", QColor(0, 0, 0));   //设置单元格边框色

    // step6: 保存文件
    // 方式二：另存为
    workbook->dynamicCall("SaveAs(const QString&)", QDir::toNativeSeparators(file)); //保存到filepath
    status = true;

    // 注意一定要用QDir::toNativeSeparators, 将路径中的"/"转换为"\", 不然一定保存不了
    workbook->dynamicCall("Close (Boolean)", false);  //关闭文件
    excel->dynamicCall("Quit()");   // 关闭Excel
    return status;
}


//-------------------------------------------------------------------数据库操作函数
/*
 * @ brief  根据时间段获取检测记录
 * @ param  dbPaht      db文件路径
 * @ param  starttime   起始时间
 * @ param  endtime     结束时间
 * @ retval None
 */
QHash<QString, QList<int>> Query::query_from_select_time(QString dbPath, QString starttime, QString endtime)
{
    QList<int> default_empty;
    QHash<QString, QList<int>> hash_product;
    QHash<QString, int> hash_serial_number;

    QWriteLocker Locker(&readLock);
    QSqlDatabase db;
    if(QSqlDatabase::contains("my_query_from_select_time"))
      db = QSqlDatabase::database("my_query_from_select_time");
    else
      db = QSqlDatabase::addDatabase("QSQLITE", "my_query_from_select_time");

    db.setDatabaseName(dbPath);

    if(!db.open())
    {
        QString estr;
        estr = "my_query_from_select_time open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);
    qry.prepare( "CREATE TABLE IF NOT EXISTS detection (id INTEGER PRIMARY KEY, datetime TEXT, pcbID TEXT, version TEXT, signal_result TEXT, power TEXT, hinfo TEXT, binfo TEXT, result TEXT, product_name TEXT)"  );
    if( !qry.exec() )
    {
        QString estr;
        estr = "my_query_from_select_time:CREATE error " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //查询
    QString cmd = QString("SELECT * FROM detection where datetime(datetime)>=datetime('%1') and datetime(datetime)<datetime('%2')").arg(starttime).arg(endtime);
    qry.prepare(cmd);
    if(!qry.exec())
    {
        QString estr;
        estr = "my_query_from_select_time:SELECT error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        int count_ok = 0;
        int count_ng = 0;

        while (qry.next())
        {
            // 依据传感器序列号,只搜寻第一条序列号的结果.后有重复出现,则过滤(麒盛-李成要求)
            int is_exist = hash_serial_number.value(qry.value(2).toString(), 0);        // pcb id
            if(!is_exist)
                hash_serial_number.insert(qry.value(2).toString(), 1);
            else
                continue;

            int result = qry.value(8).toInt();     // 结果
            if(result)
            {
                count_ok = 1;
                count_ng = 0;
            }
            else
            {
                count_ok = 0;
                count_ng = 1;
            }

            QString product_name = qry.value(9).toString();    // 产品名称
            QList<int> tmp = hash_product.value(product_name, default_empty);
            if(tmp.isEmpty())
            {
                tmp.append(1);
                tmp.append(count_ok);
                tmp.append(count_ng);
                hash_product.insert(product_name, tmp);
            }
            else
            {
                hash_product[product_name][0] = hash_product[product_name][0] + 1;          // 总数
                hash_product[product_name][1] = hash_product[product_name][1] + count_ok;   // ok数
                hash_product[product_name][2] = hash_product[product_name][2] + count_ng;   // ng数
            }
        }
    }
    db.close();
    return hash_product;
}


/*
 * @ brief  根据时间段且条件信息获取检测记录
 * @ param  dbPaht      db文件路径
 * @ param  starttime   起始时间
 * @ param  endtime     结束时间
 * @ param  *sum        输出总的检测数目
 * @ param  *ng_num     输出不良品数目
 * @ retval None
 */
void Query::query_from_select_time_and_conditional(QString dbPath, QString starttime, QString endtime, int conditional, int is_OK)
{
    QReadLocker Locker(&readLock);
    QSqlDatabase db;
    if(QSqlDatabase::contains("query_from_select_time_and_conditional"))
      db = QSqlDatabase::database("query_from_select_time_and_conditional");
    else
      db = QSqlDatabase::addDatabase("QSQLITE", "query_from_select_time_and_conditional");

    db.setDatabaseName(dbPath);

    if(!db.open())
    {
        QString estr;
        estr = "query_from_select_time_and_conditional open error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    QSqlQuery qry(db);
    qry.prepare( "CREATE TABLE IF NOT EXISTS detection (id INTEGER PRIMARY KEY, datetime TEXT, pcbID TEXT, version TEXT, signal_result TEXT, power TEXT, hinfo TEXT, binfo TEXT, result TEXT, product_name TEXT)"  );
    if( !qry.exec() )
    {
        QString estr;
        estr = "query_from_select_time_and_conditional:CREATE error " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }

    //查询
    QString cmd = QString("SELECT * FROM detection where datetime(datetime)>=datetime('%1') and datetime(datetime)<datetime('%2')").arg(starttime).arg(endtime);
    qry.prepare(cmd);
    if(!qry.exec())
    {
        QString estr;
        estr = "query_from_select_time_and_conditional:SELECT error! " + db.lastError().text();
        throw std::runtime_error(estr.toLatin1().data());
    }
    else
    {
        int count = 0;
        int result = 0;
        bool is_eligible = false;
        QStringList data;
//        QHash<QString, int> hash_serial_number;

        while (qry.next())
        {
//            // 依据传感器序列号,只搜寻第一条序列号的结果.后有重复出现,则过滤(麒盛-李成要求)
//            int is_exist = hash_serial_number.value(qry.value(2).toString(), 0);      // pcb-id
//            if(!is_exist)
//                hash_serial_number.insert(qry.value(2).toString(), 1);
//            else
//                continue;

            data.clear();
            is_eligible = false;

            if(is_OK == 1 && qry.value(8).toInt() == 1)      // 查询合格的
            {
                is_eligible = true;
            }
            else if(is_OK == 0 && qry.value(8).toInt() == 0 && conditional == 0x00) // 查询不合格的,且没勾选搜索条件
            {
                is_eligible = true;
            }
            else if(is_OK == 0 && qry.value(8).toInt() == 0) // 查询不合格的
            {
                result = qry.value(4).toInt();
                /*if(conditional & BURN_EN)  // 烧录NG使能
                {
                    if(result&BURN_NG)
                        is_eligible = true;
                    else
                        continue;
                }*/
                if(conditional & ID_WRIGHT_EN)  // id写入NG使能
                {
                    if(result&ID_WRITE_NG)
                        is_eligible = true;
                    else
                        continue;
                }
                if(conditional & ID_READ_EN)  // id读取NG使能
                {
                    if(result&ID_READ_NG)
                        is_eligible = true;
                    else
                        continue;
                }
                if(conditional & VERSION_READ_EN)  // 软件版本读取NG使能
                {
                    if(result&VERSION_READ_NG)
                        is_eligible = true;
                    else
                        continue;
                }
                if(conditional & CHECK_HR_EN)  // 心跳通道NG使能
                {
                    if(result&CHECK_HR_NG)
                        is_eligible = true;
                    else
                        continue;
                }
                if(conditional & CHECK_BR_EN)  // 呼吸通道NG使能
                {
                    if(result&CHECK_BR_NG)
                        is_eligible = true;
                    else
                        continue;
                }
            }
            else if(is_OK == -1)
            {
                is_eligible = true;
            }

            if(is_eligible)
            {
                data.append(QString::number(++count));
                data.append(qry.value(1).toString());   // 时间
                data.append(qry.value(2).toString());   // pcb id
                data.append(qry.value(3).toString());   // 软件版本
                data.append(qry.value(8).toInt()==1?"合格":"不合格");   // 结果
                data.append(qry.value(6).toString());   // 心跳通道的结果记录
                data.append(qry.value(7).toString());   // 呼吸通道的结果记录
                /*if(qry.value(4).toInt() & BURN_NG)
                {
                    data.append("失败");                 // 程序烧录结果
                    data.append("");                    // id写入结果
                    data.append("");                    // id读取结果
                    data.append("");                    // 软件版本读取结果
                }
                else
                {
                    data.append("成功");                 // 程序烧录结果
                    if(qry.value(4).toInt() & ID_WRITE_NG)
                        data.append("失败");             // id写入结果
                    else
                        data.append("");
                    if(qry.value(4).toInt() & ID_READ_NG)
                        data.append("失败");             // id读取结果
                    else
                        data.append("");
                    if(qry.value(4).toInt() & VERSION_READ_NG)
                        data.append("失败");             // 软件版本读取结果
                    else
                        data.append("");
                }*/

                int signal_result = qry.value(4).toInt();
                table_show_query_result(data,
                                        signal_result&CHECK_HR_NG,
                                        signal_result&CHECK_BR_NG,
                                        signal_result&BURN_NG,
                                        signal_result&ID_WRITE_NG,
                                        signal_result&ID_READ_NG,
                                        signal_result&VERSION_READ_NG);
             }
        }
//        hash_serial_number.clear();
    }
    db.close();

}
