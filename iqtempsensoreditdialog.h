#ifndef IQTEMPSENSOREDITDIALOG_H
#define IQTEMPSENSOREDITDIALOG_H

#include <QDialog>
#include "iqtempsensor.h"

namespace Ui {
class IQTempSensorEditDialog;
}

class IQTempSensorEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IQTempSensorEditDialog(QWidget *parent = 0);
    ~IQTempSensorEditDialog();

    void setSensor(IQTempSensor *sensor);

private slots:
    void setCurrentTempFromSensor();
    void setCurrentHumidity();
    void saveDataToSensor();

private:
    Ui::IQTempSensorEditDialog *ui;
    IQTempSensor *_sensor;
};

#endif // IQTEMPSENSOREDITDIALOG_H
