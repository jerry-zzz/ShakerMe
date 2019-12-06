/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    main.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	总界面调度，程序入口.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */



#include "mainframe.h"
#include <QApplication>
#include <QDateTime>
#include <QMutex>
#include "windows.h"
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <QThread>
#include "system_info.h"
#include <QInputDialog>



void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //只运行一个进程
    HANDLE hMutex = CreateMutex(NULL, TRUE, PROCESS_NAME);     // 创建一个互斥量来保证只运行一个实例
    if(hMutex != NULL)
    {
        if(GetLastError() == ERROR_ALREADY_EXISTS)
        {
            QMessageBox::critical(qApp->activeWindow(), QObject::tr("An instance of this application has been run!"), QObject::tr("Only one instance can be run at a time!"));
            return 1;
        }
    }

    //创建根目录
    QString path(ROOT_PATH);
    QDir dir_r(path);
    if(!dir_r.exists())
        dir_r.mkpath(path);//创建多级目录

    //开启调戏信息记录
#ifdef LOG_EN
//    qInstallMessageHandler(outputMessage);
//    qInfo("\n\n\n\n\n");
#endif


    MainFrame w;
    return a.exec();
}



void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QFileInfo fileinfo = QFileInfo(QString(context.file));
    QString file_name = fileinfo.fileName();
    QString context_info = QString("[%1:%2-%3]").arg(file_name).arg(context.line).arg(VERSION);
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    QString log_type;
    QString message;
    switch(type)
    {
        //qDebug()      调试信息
    case QtDebugMsg:
    {
        log_type = QString("Debug:  ");
        message = QString("%1  %2  %3  %4\n").arg(current_date_time).arg(log_type).arg(msg).arg(context_info);
        break;
    }
        //qWarning()    报警信息
    case QtWarningMsg:
    {
        log_type = QString("Warning:");
        message = QString("%1  %2  %3  %4\n").arg(current_date_time).arg(log_type).arg(msg).arg(context_info);
        break;
    }
        //qInfo()       操作信息
    case QtInfoMsg:
    {
        log_type = QString("Info:   ");
        message = QString("%1  %2  %3  %4\n").arg(current_date_time).arg(log_type).arg(msg).arg(context_info);
        break;
    }
    default:
        break;
    }

    QFile f(QString("%1/debug_info.log").arg(ROOT_PATH));
    f.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text);
    f.write(message.toStdString().data());
    f.close();
}
