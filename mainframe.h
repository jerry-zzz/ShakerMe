#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <QWidget>
#include <QTimer>
#include "Module/Log/loginfo.h"
#include "Module/BSP/bsp.h"
#include "Module/Main/mainpanel.h"
#include "Module/Process/process.h"
#include "Module/EquipmentDebug/equipmentdebug.h"
#include "Module/SystemSetup/systemsetup.h"
#include "Module/UserInfo/userinfo.h"
#include "Module/UserInfo/jobinfo.h"
#include "Module/Login/login.h"
#include "Module/Detection/detection.h"
#include "Module/ProductSet/productset.h"
#include "Module/Exciter/NI4461/ni4461_class.h"
#include "Module/Exciter/shaker.h"
#include "Module/Exciter/shakerfeedback.h"
#include "Module/ShakerGraph/shakergraph.h"
#include "Module/Query/query.h"


namespace Ui {
class MainFrame;
}

class MainFrame : public QWidget
{
    Q_OBJECT

/*- Programmer Define Design -------------------------------------------------*/
    // 界面index
    #define PANLE_MAIN          0           //主界面
    #define PANEL_PRODUCTSET    1           //产品
    #define PANLE_TIME          2           //时效分析
    #define PANLE_USER          3           //用户管理
    #define PANLE_LOGINFO       4           //日志
    #define PANLE_SYSPARAM      10          //系统设置
    #define PANLE_DEBUG         20          //设备调试
    #define PANLE_CONFIG        30          //设备配置
    #define PANLE_TEST          40          //检测系统
    #define PANLE_EXCITER       50          //激振器调试
    #define PANLE_GRAPH         60          //图形显示
    #define PANLE_QUERY         70          //查询面板



public:
/*- Module Function ----------------------------------------------------------*/
    explicit MainFrame(QWidget *parent = 0);
    ~MainFrame();
    // 按钮初始化
    void init_pb_connect();
    // 界面切换函数
    void switch_panel(int last_panel);

/*- Programmer Add Function --------------------------------------------------*/

signals:
/*- Module Function ----------------------------------------------------------*/
    // 日志消息
    void msg_log(int type, QString msg, bool msgbox);

/*- Programmer Add Function --------------------------------------------------*/
public slots:
/*- Module Function ----------------------------------------------------------*/
    void slot_current_time();
    void slot_signal_check_timer();
    void pb_start();
    void pb_emg();
    void pb_reset();
    void pb_close();
    void pb_minimize();
    // 用户登录函数, 更新用户id, 各面板权限信息
    void slot_user_login(QString id, int permissions);
    void slot_user_logout();
    // 获取当前产品名称
    void slot_get_current_procuct_name(QString product_name);
    // 刷新信号灯状态
    void update_signal_lamp(int lamp_status);
    void slot_sensor_generation(int sensor_generation);

/*- Programmer Add Function --------------------------------------------------*/
    /*- 1 ------------------------------------*/
    void pb_main();             // 主界面
    void pb_product_set();      // 产品管理
    void pb_test();             // 检测系统
    /*- 2 ------------------------------------*/
    void pb_sysparam();         // 系统设置
    void pb_debug();            // 设备调试
    // add your code 可添加机械调试模块
    void pb_exciter();
    void pb_shakergraph();      //图形显示

    /*- 3 ------------------------------------*/
    void pb_config();           // 设备配置
    void pb_user();             // 用户管理
    void pb_loginfo();          // 日志记录
    /*----------------------------------------*/
    void slot_update_device_param(QVector<Device_Attribute> device_list);

private:
/*- Module Variable ----------------------------------------------------------*/
    Ui::MainFrame *ui;
    // 流程
    Process *tprocess;
    // 时间显示定时器
    QTimer *current_Timer;
    // 按钮信号检测定时器
    QTimer *signal_Timer;
    QTimer *m_pTimer;
    //当前界面的index
    int current_panel_index = 0;
    // 用户id和权限
    QString m_id="";
    int m_permissions_level=0;
    // 板卡初始化状态
    int bsp_status;
    // 记录下机时间
    bool can_record_logout_time = 0;
    // 工作信息记录对象
    JobInfo *m_jobinfo;
    // 运动控制库
    Bsp *ui_bsp;
/*- Programmer Add Variable --------------------------------------------------*/
    // 各子面板
    MainPanel *ui_main;
    ProductSet *ui_product;
    LogInfo *ui_log;
    EquipmentDebug *ui_ed;
    SystemSetup *ui_setup;
    UserInfo *ui_user;
    Login *ui_login;
    Detection *ui_dect;
    Shaker *ui_shaker;
    ShakerGraph *ui_graph;
    Query *ui_query;

    WeighingSensor *mws;    //压力传感器  
    Device_Attribute m_com_ws;//串口信息
    LaserRanging *mlr; //激光传感器
    Device_Attribute m_com_lr;//串口信息

    QThread *threadmws;
    QThread *threadmlr;

    ShakerFeedback *shakerFb;
    QThread *threadShakerFb;

    NI4461_Class *ni;
};



#endif // MAINFRAME_H
