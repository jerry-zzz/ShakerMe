/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    bsp_device.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	串口,网口的定义库,后期如果有新的接口再进行设计定义.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */


#include "bsp_device.h"
#include <QDebug>

Bsp_Device::Bsp_Device(QObject *parent) : QObject(parent)
{

}


/*
 * @ brief
 * @ param  .
 * @ retval
 */

/*
 * @ brief  设置设备属性
 * @ param  attr:   设备属性
 * @ retval .
 */
void Bsp_Device::setDeviceAttribute(Device_Attribute attr)
{
    da = attr;
}

/*
 * @ brief 获取设备属性
 * @ param  .
 * @ retval da: 设备属性
 */
Device_Attribute Bsp_Device::getDeviceAttribute(void)
{
    return da;
}
