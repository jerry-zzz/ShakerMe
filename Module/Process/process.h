#ifndef PROCESS_H
#define PROCESS_H

#include <QThread>
#include <QWidget>
#include <QObject>
#include <QReadWriteLock>
#include <QtSql>
#include "../EquipmentDebug/equipmentdebug.h"
#include "../SystemSetup/systemsetup.h"
#include "../Detection/detection.h"
#include "../BSP/bsp.h"
#include "Module/Main/mainpanel.h"
#include "Module/ProductSet/productset.h"
#include "Module/Exciter/shaker.h"

class Process : public QThread
{
    Q_OBJECT
public:
/*- Module Function ----------------------------------------------------------*/
    Process();
    ~Process();

    // 初始化流程模块
    void init_Process(Bsp *bsp, Shaker *skr);
    // 日志记录
    void log_info(QString msg);
    // 判断是否线程运行
    bool is_running();
    // 设置运行一次标志位
    void set_run_next();
/*- Programmer Add Function --------------------------------------------------*/

    //旋压气缸是否压紧
    bool is_cylinder_tight_L(void);
    bool is_cylinder_tight_R(void);

    // 获取传感器区分状态
    bool is_differ(void);

    //类床模式
    void class_bed_mode();
    //海绵模式
    void sponge_mode();
    //带壳模式
    void shelled_mode();
    //裸板模式
    void bare_board_mode();

    //载台Y轴到检测位置
    void platform_y_move_to_check_pos(double val);
    //等待载台Y轴到位
    void wait_platform_y_in_place(void);

    //载台Z轴到检测位置
    void platform_z_move_to_check_pos(double val);
    //等待载台Z轴到位
    void wait_platform_z_in_place(void);

    // 激振器z轴到检测位置
    void shaker_z_move_to_check_pos(double val);
    // 等待激振器z轴到位
    void wait_shaker_z_in_place(void);
    // 激振器z轴到安全位置
    void shaker_z_move_to_safe_state(double val);
    //退料
    void return_of_product();

signals:
/*- Module Function ----------------------------------------------------------*/

    void sig_reflash_platform_status( QString status);
    void sig_reflash_axis_platform_z_status(QString status, int ismove);
    void sig_reflash_axis_shaker_z_status(QString status, int ismove);
    void sig_reflash_axis_platform_y_status(QString status, int ismove);


    // 日志记录
    void msg_log(int type, QString msg, bool msgbox);
    // 更新信号灯状态
    void sig_update_signal_lamp(int status);

    // 更新检测结果
    void sig_update_check_result(int result, QString eInfo, QString pcb_info);

/*- Programmer Add Function --------------------------------------------------*/
    // 查询pcb的id和软件版本
    void sig_read_pcb_info(int en);

    void startkk(QString test_file_name);
    void stopkk();

    void sig_set_ni_param_and_start_ao_ai(QString wavePath, int aoSample, int recvTime, QString product_name);
public slots:
/*- Module Function ----------------------------------------------------------*/
    // 更新当前产品名称
    void slot_update_current_procuct_name(QString product_name);
    // 更新设备调试参数
    void slot_update_ep_param(Equipment_Param ep);
    // 更新系统参数
    void slot_update_sys_param(Sys_Setup ss);
    // 查询pcb的id和软件版本
    void slot_output_pcb_info(QString id, QString version);

/*- Programmer Add Function --------------------------------------------------*/
    void slot_mode_num(int mode_num);
    // 更新产品参数
    void slot_update_pd_param(Full_Product_Param pp);

protected:
/*- Module Function ----------------------------------------------------------*/
    // 线程
    void run();
/*- Programmer Add Function --------------------------------------------------*/

private:
/*- Module Variable ----------------------------------------------------------*/
    Bsp *m_bsp;             // 运动控制库
    int run_next = 0;       // 运行一次流程标志位
    int isrunning=0;        // 返回是否运行的状态
    Equipment_Param m_ep;   // 设备调试参数
    Sys_Setup m_ss;         // 系统设置参数
    QString current_product_name;   // 当前产品名称
    Full_Product_Param m_pp;     //产品参数
    Shaker *m_skr;                 //激振器界面

/*- Programmer Add Variable --------------------------------------------------*/
    // pcb id和软件版本
    bool is_info_received;
    QString m_info_id, m_info_version;
    //模式选择
    int choose_mode_num;
    int choose_generation_num;
    // 是否进行检测标志位
    int is_test= 0;
    bool is_finished_flag = false;
};

#endif // PROCESS_H
