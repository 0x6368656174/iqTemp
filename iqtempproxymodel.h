#ifndef IQTEMPPROXYMODEL_H
#define IQTEMPPROXYMODEL_H

#include <QIdentityProxyModel>
#include "iqobjectbasedtablemodel.h"
#include "iqtempsensor.h"

class IQTempProxyModel : public QIdentityProxyModel
{
    Q_OBJECT
public:
    explicit IQTempProxyModel(QObject *parent = 0);

    QVariant data(const QModelIndex &proxyIndex, int role = Qt::EditRole) const;

    void setSourceModel(QAbstractItemModel *sourceModel);

private slots:
    void checkInvisivleProperty(int row, const QString &property);

private:
    IQObjectBasedTableModel<IQTempSensor> *_tempModel;
};

#endif // IQTEMPPROXYMODEL_H
