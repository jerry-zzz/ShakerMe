#ifndef BSP_IO_H
#define BSP_IO_H

#include <QObject>
#include "AMP_208C/APS168.h"
#include "AMP_208C/APS_Define.h"
#include "AMP_208C/ErrorCodeDef.h"
#include "AMP_208C/type_def.h"
#include "bsp_define.h"






class Bsp_IO : public QObject
{
    Q_OBJECT
public:
    explicit Bsp_IO(QObject *parent = nullptr);

    void setIOAttribute(IO_Attribute attr);
    IO_Attribute getIOAttribute(void);

    void SetBit(I32 value);
    I32 GetBit();

signals:

public slots:

private:
    IO_Attribute ioa;
};

#endif // BSP_IO_H
