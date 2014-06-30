#define NAME_COLUMN_INDEX 0
#define TEMP_COLUMN_INDEX 1
#define HUMIDITY_COLUMN_INDEX 2

#include "iqtempproxymodel.h"
#include <QBrush>

IQTempProxyModel::IQTempProxyModel(QObject *parent) :
    QIdentityProxyModel(parent)
{
}

void IQTempProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    IQObjectBasedTableModel<IQTempSensor> *tempModel = dynamic_cast<IQObjectBasedTableModel<IQTempSensor> *>(sourceModel);
    if (tempModel)
        _tempModel = tempModel;

    connect(_tempModel, SIGNAL(invisibleDataChanged(int,QString)), this, SLOT(checkInvisivleProperty(int,QString)));

    QIdentityProxyModel::setSourceModel(sourceModel);
}

QVariant IQTempProxyModel::data(const QModelIndex &proxyIndex, int role) const
{
    IQTempSensor *sensor = _tempModel->objectAt(mapToSource(proxyIndex).row());
    if (!sensor)
        return QIdentityProxyModel::data(proxyIndex, role);

    int column = mapToSource(proxyIndex).column();

    if (role == Qt::BackgroundRole)
    {
        if (sensor->healthState() == IQTempSensor::Disabled)
        {
            return QBrush(QColor("#DCDCDC"));
        }
        else if (column == TEMP_COLUMN_INDEX)
        {
            switch (sensor->healthState())
            {
            case IQTempSensor::MinTempCritical:
                return QBrush(QColor("#0064A8"));
            case IQTempSensor::MinTempWarning:
                return QBrush(QColor("#BDE4FF"));
            case IQTempSensor::MaxTempWarning:
                return QBrush(QColor("#FFC7C7"));
            case IQTempSensor::MaxTempCritical:
                return QBrush(QColor("#A80002"));
            case IQTempSensor::IdleTimeout:
                return QBrush(QColor("#FFF042"));
            default:
                return QBrush(QColor("#9DFF85"));
            }
        }
    }
    else if (role == Qt::ToolTipRole)
    {
        if (sensor->healthState() == IQTempSensor::Disabled)
        {
            return tr("Sensor Disabled");
        }
        else if (column == TEMP_COLUMN_INDEX)
        {
            switch (sensor->healthState())
            {
            case IQTempSensor::MinTempCritical:
                return tr("Temperature Below Critical");
            case IQTempSensor::MinTempWarning:
                return tr("Temperature Below Normal");
            case IQTempSensor::MaxTempWarning:
                return tr("Temperature Above Normal");
            case IQTempSensor::MaxTempCritical:
                return tr("Temperature Above Critical");
            case IQTempSensor::IdleTimeout:
                return tr("Idle Timed Out");
            default:
                return tr("Temperatur Ok");
            }
        }
        else if (column == NAME_COLUMN_INDEX)
        {
             return sensor->description();
        }
    }
    else if (role == Qt::DisplayRole && (column == TEMP_COLUMN_INDEX || column == HUMIDITY_COLUMN_INDEX))
    {
        QVariant modelData = QIdentityProxyModel::data(proxyIndex, role).toReal();
        if (modelData != -100)
        {
            return modelData;
        }
        else
        {
            return tr("Unknown");
        }
    }


    return QIdentityProxyModel::data(proxyIndex, role);
}

void IQTempProxyModel::checkInvisivleProperty(int row, const QString &property)
{
    if (property != "healthState")
        return;

    emit dataChanged(index(row, TEMP_COLUMN_INDEX), index(row, TEMP_COLUMN_INDEX));
}
