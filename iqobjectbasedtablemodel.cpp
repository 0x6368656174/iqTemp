#include "iqobjectbasedtablemodel.h"

IQObjectBasedTableModelQObjectInterface::IQObjectBasedTableModelQObjectInterface(QObject *parent) :
    QAbstractTableModel(parent)
{
}

void IQObjectBasedTableModelQObjectInterface::onObjectChanged()
{
    QObject * senderItem = sender();
    if (!senderItem)
        return;

    if (senderSignalIndex() < 0)
        return;

    QString property = _propertySignalsIndex[senderSignalIndex()];
    if (property.isEmpty())
        return;

    emitDataChangedSignal(senderItem, property);
}
