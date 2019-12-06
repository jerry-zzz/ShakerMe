#-------------------------------------------------
#
# Project created by QtCreator 2018-06-07T08:28:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets sql serialport printsupport multimedia axcontainer

TARGET = ShakerME
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS QT_MESSAGELOGCONTEXT

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
    mainframe.cpp \
    Module/BSP/bsp.cpp \
    Module/BSP/bsp_aps.cpp \
    Module/BSP/bsp_io.cpp \
    Module/BSP/bsp_device.cpp \
    Module/BSP/sql_axisconfig.cpp \
    Module/Detection/detection.cpp \
    Module/EquipmentDebug/equipmentdebug.cpp \
    Module/Log/loginfo.cpp \
    Module/Log/sql_log.cpp \
    Module/Log/sql_thread.cpp \
    Module/Login/login.cpp \
    Module/Login/registeruser.cpp \
    Module/Main/mainpanel.cpp \
    Module/Process/process.cpp \
    Module/ProductSet/productset.cpp \
    Module/Sql/sql_bindata.cpp \
    Module/Sql/sql_data_cell.cpp \
    Module/SystemSetup/systemsetup.cpp \
    Module/UserInfo/jobinfo.cpp \
    Module/UserInfo/personalcenter.cpp \
    Module/UserInfo/sql_account.cpp \
    Module/UserInfo/sql_jobinfo.cpp \
    Module/UserInfo/userinfo.cpp \
    QCustomPlot.cpp \
    Module/Detection/serialcommunication.cpp \
    Module/Detection/signalprocessing.cpp \
    Module/Exciter/shaker.cpp \
    Module/Exciter/NI4461/ni4461_ai.cpp \
    Module/Exciter/NI4461/ni4461_ao.cpp \
    Module/Exciter/NI4461/ni4461_class.cpp \
    Module/ShakerGraph/shakergraph.cpp \
    Module/EquipmentDebug/modbus.cpp \
    Module/EquipmentDebug/weighing_sensor.cpp \
    Module/EquipmentDebug/laserranging.cpp \
    Module/Exciter/shakerfeedback.cpp \
    Module/Query/query.cpp \
    Module/Detection/filter.cpp

HEADERS += \
        config_axis_io.h \
    mainframe.h \
    Module/BSP/bsp.h \
    Module/BSP/bsp_aps.h \
    Module/BSP/bsp_define.h \
    Module/BSP/bsp_device.h \
    Module/BSP/bsp_io.h \
    Module/BSP/sql_axisconfig.h \
    Module/Detection/detection.h \
    Module/EquipmentDebug/equipmentdebug.h \
    Module/Log/loginfo.h \
    Module/Log/sql_log.h \
    Module/Log/sql_thread.h \
    Module/Login/login.h \
    Module/Login/registeruser.h \
    Module/Main/mainpanel.h \
    Module/Process/process.h \
    Module/ProductSet/productset.h \
    Module/Sql/sql_bindata.h \
    Module/Sql/sql_data_cell.h \
    Module/SystemSetup/systemsetup.h \
    Module/UserInfo/jobinfo.h \
    Module/UserInfo/personalcenter.h \
    Module/UserInfo/sql_account.h \
    Module/UserInfo/sql_jobinfo.h \
    Module/UserInfo/userinfo.h \
    QCustomPlot.h \
    system_info.h \
    Module/Detection/serialcommunication.h \
    Module/Detection/signalprocessing.h \
    Module/Exciter/NIDAQmx.h \
    Module/Exciter/shaker.h \
    Module/Exciter/NI4461/ni4461_ai.h \
    Module/Exciter/NI4461/ni4461_ao.h \
    Module/Exciter/NI4461/ni4461_class.h \
    Module/ShakerGraph/shakergraph.h \
    Module/EquipmentDebug/modbus.h \
    Module/EquipmentDebug/weighing_sensor.h \
    Module/EquipmentDebug/laserranging.h \
    Module/Exciter/shakerfeedback.h \
    Module/Query/query.h \
    Module/Detection/filter.h

FORMS += \
        mainframe.ui \
    Module/BSP/bsp.ui \
    Module/Detection/detection.ui \
    Module/EquipmentDebug/equipmentdebug.ui \
    Module/Log/loginfo.ui \
    Module/Login/login.ui \
    Module/Login/registeruser.ui \
    Module/Main/mainpanel.ui \
    Module/ProductSet/productset.ui \
    Module/SystemSetup/systemsetup.ui \
    Module/UserInfo/personalcenter.ui \
    Module/UserInfo/userinfo.ui \
    Module/Exciter/shaker.ui \
    Module/ShakerGraph/shakergraph.ui \
    Module/Query/query.ui \

RESOURCES += \
    res.qrc

RC_FILE = logo_icon.rc
#RC_ICONS = logo_icon.ico

win32: LIBS += -L$$PWD/Module/BSP/AMP_208C/ -lAPS168

INCLUDEPATH += $$PWD/Module/BSP/AMP_208C
DEPENDPATH += $$PWD/Module/BSP/AMP_208C

DISTFILES += \
    Module/Exciter/NIDAQmx.lib


win32: LIBS += -L$$PWD/Module/Exciter/ -lNIDAQmx

INCLUDEPATH += $$PWD/Module/Exciter
DEPENDPATH += $$PWD/Module/Exciter

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/Module/Exciter/NIDAQmx.lib
