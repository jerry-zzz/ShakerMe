#ifndef BSP_DEVICE_H
#define BSP_DEVICE_H

#include <QObject>
#include "bsp_define.h"

class Bsp_Device : public QObject
{
    Q_OBJECT
public:
    explicit Bsp_Device(QObject *parent = nullptr);

    void setDeviceAttribute(Device_Attribute attr);
    Device_Attribute getDeviceAttribute(void);

signals:

public slots:

private:
    Device_Attribute da;
};

#endif // BSP_DEVICE_H
