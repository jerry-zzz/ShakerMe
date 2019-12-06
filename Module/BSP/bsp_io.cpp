/*
 *******************************************************************************
 *
 *         COPYRIGHT (c) 2019, 浙江清华长三角研究院 智能装备与技术中心
 *         All Rights Reserved.
 *
 *******************************************************************************
 *
 * FileName:    bsp_io.cpp
 * Author:      zone53
 * Version:     1.0
 * Date:        2019/04/16
 *
 ********* 简 介 ****************************************************************
 *
 *  	凌华运动控制卡 AMP_204C/208C 的IO部分的封装库.
 *
 ********************************** 历 史 版 本 **********************************
 * <Date>       <Author>   <Version>   <Description>
 * ***********  *********  **********  *****************************************
 * 2019/04/16   wzq        1.0         Initial Version
 *
 *******************************************************************************
 */



#include "bsp_io.h"
#include <QDebug>

Bsp_IO::Bsp_IO(QObject *parent) : QObject(parent)
{

}

/*
 * @ brief  设置IO属性
 * @ param  attr:   io属性(方向和极性)
 * @ retval .
 */
void Bsp_IO::setIOAttribute(IO_Attribute attr)
{
    ioa = attr;
}

/*
 * @ brief 获取IO属性
 * @ param  .
 * @ retval ioa: io属性(方向和极性)
 */
IO_Attribute Bsp_IO::getIOAttribute(void)
{
    return ioa;
}


/*
 * @ brief  设置IO某一位状态
 * @ param  value: 设置的值(0/1)
 * @ retval .
 */
void Bsp_IO::SetBit(I32 value)
{
    if(ioa.isReversal)
        value = value == 0?1:0;
    APS_write_d_channel_output(ioa.card_id, ioa.group_id, ioa.channel, value);
}

/*
 * @ brief  获取IO某一位状态
 * @ param  .
 * @ retval value: io的状态
 */
I32 Bsp_IO::GetBit()
{
    I32 io_Data=0;
    I32 value=0;
    if(ioa.dir)
        APS_read_d_output(ioa.card_id, ioa.group_id, &io_Data);
    else
        APS_read_d_input(ioa.card_id, ioa.group_id, &io_Data);
    value = ((io_Data>>ioa.channel)&0x01);
    if(ioa.isReversal)
        value = value == 0?1:0;
    return value;
}
