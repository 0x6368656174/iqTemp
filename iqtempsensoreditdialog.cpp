#include "iqtempsensoreditdialog.h"
#include "ui_iqtempsensoreditdialog.h"

IQTempSensorEditDialog::IQTempSensorEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IQTempSensorEditDialog),
    _sensor(NULL)
{
    ui->setupUi(this);

    connect(this, SIGNAL(accepted()), this, SLOT(saveDataToSensor()));
}

IQTempSensorEditDialog::~IQTempSensorEditDialog()
{
    delete ui;
}

void IQTempSensorEditDialog::setSensor(IQTempSensor *sensor)
{
    if (_sensor == sensor)
        return;

    if (_sensor)
        disconnect(_sensor, 0, this, 0);

    _sensor = sensor;

    if (!_sensor)
        return;

    connect(_sensor, SIGNAL(tempChanged()), this, SLOT(setCurrentTempFromSensor()));
    connect(_sensor, SIGNAL(humidityChanged()), this, SLOT(setCurrentHumidity()));

    ui->nameLineEdit->setText(_sensor->name());
    ui->enabledCheckBox->setChecked(_sensor->enabled());
    ui->portSpinBox->setValue(_sensor->port());
    ui->maxIdleSpinBox->setValue(_sensor->maxTempIdle());
    ui->descriptionPlainTextEdit->setPlainText(_sensor->description());
    ui->miCriticalTempDoubleSpinBox->setValue(_sensor->minCriticalTemp());
    ui->minWarningTempDoubleSpinBox->setValue(_sensor->minWarningTemp());
    if (_sensor->temp() != -100)
    {
        ui->currentTempLabel->setText(QString::number(_sensor->temp()));
    }
    else
    {
        ui->currentTempLabel->setText(tr("Unknown"));
    }
    ui->maxWarningTempDoubleSpinBox->setValue(_sensor->maxWarningTemp());
    ui->maxCriticalDoubleSpinBox->setValue(_sensor->maxCriticalTemp());
    if (_sensor->humidity() != -100)
    {
        ui->currentHumidityLabel->setText(QString::number(_sensor->humidity()));
    }
    else
    {
        ui->currentHumidityLabel->setText(tr("Unknown"));
    }
}

void IQTempSensorEditDialog::setCurrentTempFromSensor()
{
    if (!_sensor)
        return;

    ui->currentTempLabel->setText(QString::number(_sensor->temp()));
}

void IQTempSensorEditDialog::setCurrentHumidity()
{
    if (!_sensor)
        return;

    ui->currentHumidityLabel->setText(QString::number(_sensor->humidity()));
}

void IQTempSensorEditDialog::saveDataToSensor()
{
    if (!_sensor)
        return;

    _sensor->setName(ui->nameLineEdit->text());
    _sensor->setEnabled(ui->enabledCheckBox->checkState() == Qt::Checked);
    _sensor->setPort(ui->portSpinBox->value());
    _sensor->setMaxTempIdle(ui->maxIdleSpinBox->value());
    _sensor->setDescription(ui->descriptionPlainTextEdit->toPlainText());
    _sensor->setMinCriticalTemp(ui->miCriticalTempDoubleSpinBox->value());
    _sensor->setMinWarningTemp(ui->minWarningTempDoubleSpinBox->value());
    _sensor->setMaxWarningTemp(ui->maxWarningTempDoubleSpinBox->value());
    _sensor->setMaxCriticalTemp(ui->maxCriticalDoubleSpinBox->value());
}
