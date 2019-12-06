/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    jobinfo.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	此函数用于统计工人上班时间，下班时间，以及工作检测的产品数量，包括合格数量、
 *      不合格数量、和总数。
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */

#include "jobinfo.h"
#include <QDateTime>
#include <QMessageBox>



JobInfo::JobInfo(QObject *parent) : QObject(parent)
{

}


// 记录上机时间
void JobInfo::record_login_time(QString user_id)
{
    temp_user_id = user_id;
    QDateTime current_date_time =QDateTime::currentDateTime();
    login_time =current_date_time.toString("yyyy-MM-dd hh:mm:ss");

    logout_time = "";
    qualified_num = 0;
    unqualified_num = 0;
    total_num = 0;

    try{
        sql_job.save_db(temp_user_id, login_time, logout_time, qualified_num, unqualified_num, total_num);
    }
    catch(const std::runtime_error& e)
    {
        QMessageBox::about(NULL, "通知", QString("记录上机时间错误：%1").arg(e.what()));
        return;
    }

}

// 记录下机时间
void JobInfo::record_logout_time()
{
    QDateTime current_date_time =QDateTime::currentDateTime();
    logout_time =current_date_time.toString("yyyy-MM-dd hh:mm:ss");

    try{
        sql_job.update_job_info(temp_user_id, login_time, logout_time, qualified_num, unqualified_num, total_num);
    }
    catch(const std::runtime_error& e)
    {
//        QMessageBox::about(NULL, "通知", QString("记录下机时间错误：%1").arg(e.what()));

        return;
    }
}

// 记录当前用户生产数量
void JobInfo::record_current_user_work_num(int ok_num, int false_num)
{
    qualified_num += ok_num;
    unqualified_num += false_num;
    total_num = qualified_num + unqualified_num;

    try{
        sql_job.update_job_info(temp_user_id, login_time, logout_time, qualified_num, unqualified_num, total_num);
    }
    catch(const std::runtime_error& e)
    {
        QMessageBox::about(NULL, "通知", QString("记录当前用户生产数量错误：\n%1").arg(e.what()));
        return;
    }
}





// 获取上机时间
QString JobInfo::get_login_time()
{
    return login_time;
}

// 获取下机时间
QString JobInfo::get_logout_time()
{
    return logout_time;
}

// 获取合格品数目
unsigned int JobInfo::get_qualified_num()
{
    return qualified_num;
}

// 获取不合格品数目
unsigned int JobInfo::get_unqualified_num()
{
    return unqualified_num;
}

// 获取总数目
unsigned int JobInfo::get_total_num()
{
    return total_num;
}

void JobInfo::get_jobinfo_history(JobData *jobdata, int *readNum)
{
    try
    {
        sql_job.read_db_some_record(temp_user_id, jobdata, readNum);
    }
    catch(const std::runtime_error& e)
    {
        QMessageBox::about(NULL, "通知", QString("获取员工历史记录错误：\n%1").arg(e.what()));
        return;
    }
}

QStringList JobInfo::get_user_id_history(QDate date)
{
    QStringList user_list;
    try
    {
        QDateTime starttime = QDateTime::fromString(date.toString("yyyy-MM-dd") + QString(" 08:00:00"), "yyyy-MM-dd hh:mm:ss");
        QDateTime endtime = QDateTime::fromString(date.toString("yyyy-MM-dd") + QString(" 20:00:00"), "yyyy-MM-dd hh:mm:ss");

        user_list = sql_job.read_db_from_select_time(starttime, endtime);
    }
    catch(const std::runtime_error& e)
    {
        QMessageBox::about(NULL, "通知", QString("获取用户名称错误：\n%1").arg(e.what()));
    }
    return user_list;
}
