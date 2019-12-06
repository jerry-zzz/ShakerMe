#ifndef PRODUCTSET_H
#define PRODUCTSET_H

#include <QWidget>
#include <QTableWidget>
#include <QVector>
#include "../Sql/sql_data_cell.h"
#include <QComboBox>
#include <QStringList>
#include"Module/EquipmentDebug/equipmentdebug.h"
#include "../BSP/bsp.h"
#include "../SystemSetup/systemsetup.h"
#include "Module/EquipmentDebug/weighing_sensor.h"

namespace Ui {
class ProductSet;
}

#define PRODUCT_PATH    "/product"

//一条流程的结构体
typedef struct
{
    QString waveform_path;     //波形文件
    int sampling_rate;         //采样率
    int acquisition_time;      //采集时间
    double y_axis_position;       //y轴点位
    QString waveform_type;     //波形类型（用处不大）
}Product_Param;

//一个完整产品的结构体
typedef struct
{
   QString product_name;                        //产品名称
   int experiment_pattern;                      //实验模式
   double sensor_height;                        //mm/传感器高度
   double sensor_out_board_height;              //mm/传感器凸出床板高度
   double mattress_height;                      //床垫高度
   double shaker_press_depth;                   //激振器下压深度
   double shaker_part_length;                   //激振器加装部件长度   
   double little_distance;                      //微调距离
   double shaker_z_pos;                         //激振器z轴测试点位
   QVector<Product_Param> mProduct_Param_List;  //流程参数

}Full_Product_Param;


class ProductSet : public QWidget
{
    Q_OBJECT

public:
    explicit ProductSet(QWidget *parent = 0);
    ~ProductSet();

/*- Module Function ----------------------------------------------------------*/
    // 初始化产品模块
    void init_product(QString root_dir, QString sysdb_path, Bsp *bsp, WeighingSensor *ws,LaserRanging *lr);
    // 设置面板权限
    void set_current_level(int level);
    // 显示界面
    void display_panel();
    // 隐藏界面
    void hide_panel();

    // 获取当前产品名称
    QString get_current_procuct_name ();
    // 获取当前产品文件夹路径
    QString get_current_procuct_dir();
    // 读取产品路径下的列表
    void update_product_list();
    // load参数
    void load_param();

    //拷贝文件夹
    bool copyDirectoryFiles(const QString &fromDir, const QString &toDir, bool coverFileIfExist);
    //删除文件夹
    bool DeleteDir(const QString &path);
    //更新流程参数到控件上
    void update_table_values(QTableWidget*table, QVector<Product_Param> data);
    //更新模版数据到面板上
    void update_template_param_to_spin_boxs(int pattern,Full_Product_Param data);
    //更新产品模式到面板上
    void update_pattern_to_combobox(int pattern);
    //清空存放流程参数的结构体迭代器
    bool clear_product_param_list(QVector<Product_Param> &Product_Param_List);
    //插入一条流程到迭代器中
    bool insert_product_param_to_list(QVector<Product_Param> &Product_Param_List,Product_Param productParam);

    //从界面上获取模板参数
    bool get_template_param(int pattern,Full_Product_Param &data);

    /**********数据库操作**********/
    // 保存到数据库
    void save_db_of_table(QString dbPath, QString table, QVector<Product_Param> data);
    // 从数据库读取
    void read_db_of_table(QString dbPath, QString table, QVector<Product_Param> &data);

    bool save_product_pattern(QString productName,int modelIndex);

    int read_product_pattern(QString productName);

    bool save_template_param(QString productName,Full_Product_Param data);

    bool read_template_param(QString productName,Full_Product_Param &data);

    bool copyFileToPath(QString sourceDir ,QString toDir, bool coverFileIfExist);

    bool isDigitStr(QString src); //判断一个字符串中是否都是数字
/*- Programmer Add Function --------------------------------------------------*/
signals:
/*- Module Function ----------------------------------------------------------*/
    void sig_update_current_product_db_path(QString db_path);
    void sig_get_current_procuct_name(QString product_name);
    void sig_update_current_product_pattern(int pattern);
    // 日志记录
    void msg_log(int type, QString msg, bool msgbox);

/*- Programmer Add Function --------------------------------------------------*/
    void sig_update_product_param(Full_Product_Param &dp);

    void stop_shaker(bool flag);

    void sig_update_pd_param(Full_Product_Param &dp);

public slots:
/*- Module Function ----------------------------------------------------------*/
    void pb_new_product();
    void pb_load_product();
    void pb_delete_product();
    void save_param();
    void cur_model_change(QString model_str); //类床/海绵/裸板/带壳裸板

    void refresh_panel_AMP_Info(void);

    void rece_current_value_ws(double result);
    void rece_current_value_lr(double result);


/*- Programmer Add Function --------------------------------------------------*/
    void pb_sine_add();
    void pb_sine_delete();
    void pb_sine_dir_set();

    QString choose_waveform();
    void move_pos_shaker();
    void move_pos_shaker_to_bed();
    void slot_update_sys_param(Sys_Setup ss);

    //读取当前激振器传感器的数值
    void read_pos_z();

private slots:
    void sine_table_itemClicked(QTableWidgetItem *item);
    void sine_table_itemChanged(QTableWidgetItem *item);

private:
/*- Module Variable ----------------------------------------------------------*/
    Ui::ProductSet *ui;
    Sys_Setup m_ss;         // 系统设置参数
    Bsp *m_bsp;             // 运动控制库
    WeighingSensor *mws;    //压力传感器
    LaserRanging *mlr;      //激光传感器

    Equipment_Param m_ep;   // 设备调试参数

    QTimer *panel_refresh_timer;          // 界面状态轮询定时器
    QTimer *m_pTimer_mws;// 压力传感器定时器
    QTimer *m_pTimer_mlr;// 激光传感器定时器

    QReadWriteLock pLock;
    int current_level = 0;
    Sql_data_cell sql_data_cell;
    QString product_root_dir;       // 产品文件夹根目录
    QString current_product_name;   // 当前产品名称
    QString current_product_dir;    // 当前产品路径
    QString current_product_db_path;// 当前产品数据库文件路径
    int current_product_model;      //当前产品模式

/*- Programmer Add Variable --------------------------------------------------*/
    Product_Param product_param;          // 当前产品设置界面存一组参数的结构体
    Full_Product_Param FullProductStru;   //整个产品的完整信心描述
    QString tmp_root_dir;    

/*- tablewidget param --------------------------------------------------------*/
    QString test_file_name;
    double test_duration;
    double test_y_pos;
    double test_sample_param;
    QString signal_type;

    int slot_product_param_row=0;

    bool ws_com_timer_flag=true;//压力传感器串口打开判断
    bool lr_com_timer_flag=true;//激光传感器串口打开判断

};

#endif // PRODUCTSET_H
