#ifndef IQOBJECTBASEDTABLEMODEL_H
#define IQOBJECTBASEDTABLEMODEL_H

#define FIRST_QML_ROLE Qt::UserRole + 1000

#include <QAbstractTableModel>
#include <QMetaProperty>
#include <QDebug>
#include <QStringList>

class IQObjectBasedTableModelQObjectInterface : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit IQObjectBasedTableModelQObjectInterface(QObject *parent = 0);


    Q_INVOKABLE virtual QObject *get(const int index) const = 0;

signals:
    void invisibleDataChanged(int row, const QString &property);

protected slots:
    void onObjectChanged();

    virtual void emitDataChangedSignal(QObject *object, const QString &property) = 0;

protected:
    int _indexOfOnObjectChangedMethod;
    QHash<int, QString> _propertySignalsIndex;

};

template <typename T>
class IQObjectBasedTableModel : public IQObjectBasedTableModelQObjectInterface
{
public:
    explicit IQObjectBasedTableModel(QObject *parent = 0);

    void append(T* object);

    void insert(const int row, T* object);

    int rowOf(T* object) const;

    T* objectAt(const int row) const;

    virtual QObject * get(const int index) const;

    void remove(T* object);

    void clear();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    Qt::ItemFlags flags(const QModelIndex &index) const;

    void setPropertyEditable(const QString &property, const bool editable = true);

    void showProperty(const QString &property, const QString &header = QString());

    void hideProperty(const QString &property);

    void setPropertyHeader(const QString &property, const QString &header = QString());

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QHash<int, QByteArray> roleNames() const;

private:
    QList<T *> _items;

    QStringList _propertyNames;

    QList<QString> _visibleProperties;
    QSet<QString> _editableProperties;
    //1 - имя свойства, 2 - его заголовок
    QHash<QString, QString> _propertyHeaders;

    virtual void emitDataChangedSignal(QObject *object, const QString &property);
};






template <typename T>
IQObjectBasedTableModel<T>::IQObjectBasedTableModel(QObject *parent) :
    IQObjectBasedTableModelQObjectInterface(parent)
{
    for (int i = 0; i < T::staticMetaObject.propertyCount(); i++)
    {
        _propertyNames << QString(T::staticMetaObject.property(i).name());
    }

    _indexOfOnObjectChangedMethod = metaObject()->indexOfMethod(QMetaObject::normalizedSignature("onObjectChanged()").constData());

    for (int i = 0; i < T::staticMetaObject.propertyCount(); i++)
    {
        QMetaProperty objectProperty = T::staticMetaObject.property(i);
        if (!objectProperty.isValid())
            continue;

        _propertySignalsIndex[objectProperty.notifySignalIndex()] =
                QString(objectProperty.name());
    }
}

template <typename T>
int IQObjectBasedTableModel<T>::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return _items.count();
}

template <typename T>
int IQObjectBasedTableModel<T>::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return _visibleProperties.count();
}

template <typename T>
QHash<int, QByteArray> IQObjectBasedTableModel<T>::roleNames() const
{
    QHash<int, QByteArray> result;

    for(int i = 0; i < _propertyNames.count(); i++)
    {
        result[FIRST_QML_ROLE + i] = _propertyNames[i].toLocal8Bit();
    }

    return result;
}

template <typename T>
QVariant IQObjectBasedTableModel<T>::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int column = index.column();

    if (row >= rowCount() || row < 0)
        return QVariant();

    if (column >= _propertyNames.count())
        return QVariant();

    if (role >= FIRST_QML_ROLE + _propertyNames.count())
        return QVariant();

    T* item = _items[row];
    if (!item)
        return QVariant();

    QString propertyName = "";
    if (role < FIRST_QML_ROLE)
    {
        propertyName = _visibleProperties[column];
    }
    else
    {
        int firsQmlRole = FIRST_QML_ROLE;
        int propertyIndex = role - firsQmlRole;
        propertyName = _propertyNames[propertyIndex];
    }

    if (propertyName.isEmpty())
        return QVariant();
    if (role == Qt::DisplayRole
            || role == Qt::EditRole
            || role > FIRST_QML_ROLE)
        return item->property(propertyName.toLocal8Bit().constData());

    return QVariant();
}

template <typename T>
void IQObjectBasedTableModel<T>::append(T *object)
{
    insert(_items.count(), object);
}

template <typename T>
int IQObjectBasedTableModel<T>::rowOf(T *object) const
{
    return _items.indexOf(object);
}

template <typename T>
void IQObjectBasedTableModel<T>::insert(const int row, T *object)
{
    if (_items.contains(object))
        return;

    int rowToInsert = row < 0?0:row;

    if (rowToInsert > rowCount())
        rowToInsert = rowCount();

    emit beginInsertRows(QModelIndex(), rowToInsert, rowToInsert);

    _items.insert(rowToInsert, object);

    for (int i = 0; i < object->metaObject()->propertyCount(); i++)
    {
        QMetaProperty objectProperty = object->metaObject()->property(i);
        if (!objectProperty.isValid())
            continue;

        QMetaMethod notifiSignal = objectProperty.notifySignal();
        if (!notifiSignal.isValid())
            continue;

        connect(object,
                notifiSignal,
                this,
                metaObject()->method(_indexOfOnObjectChangedMethod));
    }

    emit endInsertRows();
}

template <typename T>
void IQObjectBasedTableModel<T>::emitDataChangedSignal(QObject *object, const QString &property)
{
    T* senderObject = qobject_cast<T*>(object);

    if (!_items.contains(senderObject))
        return;

    int itemRow = _items.indexOf(senderObject);
    int propertyColumn = _visibleProperties.indexOf(property);

    if (propertyColumn != -1)
    {
        QModelIndex topLeft = index(itemRow, propertyColumn);
        QModelIndex bottomRigth = index(itemRow, propertyColumn);
        emit dataChanged(topLeft, bottomRigth);
    }
    else
    {
        QModelIndex topLeft = index(itemRow, 0);
        QModelIndex bottomRigth = index(itemRow, 0);
        emit dataChanged(topLeft, bottomRigth);

        emit invisibleDataChanged(itemRow, property);
    }
}

template <typename T>
void IQObjectBasedTableModel<T>::showProperty(const QString &property, const QString &header)
{
    if (_visibleProperties.contains(property))
        return;

    emit beginInsertColumns(QModelIndex(), _visibleProperties.count(), _visibleProperties.count());
    _visibleProperties.append(property);
    _propertyHeaders[property] = header;
    emit endInsertColumns();
}

template <typename T>
void IQObjectBasedTableModel<T>::hideProperty(const QString &property)
{
    if (!_visibleProperties.contains(property))
        return;

    emit beginRemoveColumns(QModelIndex(), _visibleProperties.indexOf(property), _visibleProperties.indexOf(property));
    _visibleProperties.removeAll(property);
    emit endRemoveColumns();
}

template <typename T>
void IQObjectBasedTableModel<T>::setPropertyHeader(const QString &property, const QString &header)
{
    if (_propertyHeaders[property] == header)
        return;

    _propertyHeaders[property] = header;

    if (_visibleProperties.contains(property))
        emit headerDataChanged(Qt::Horizontal, _visibleProperties.indexOf(property), _visibleProperties.indexOf(property));
}

template <typename T>
QVariant IQObjectBasedTableModel<T>::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);

    if (orientation == Qt::Vertical)
        return QAbstractTableModel::headerData(section, orientation, role);

    if (section >= _visibleProperties.count() || section < 0)
        return QAbstractTableModel::headerData(section, orientation, role);

    QString property = _visibleProperties[section];

    if (role == Qt::DisplayRole && !_propertyHeaders[property].isEmpty())
    {
        return _propertyHeaders[property];
    }
    else
    {
        return property;
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

template <typename T>
T* IQObjectBasedTableModel<T>::objectAt(const int row) const
{
    if (row < 0 || row >= _items.count())
        return NULL;

    return _items[row];
}

template <typename T>
QObject * IQObjectBasedTableModel<T>::get(const int index) const
{
    return qobject_cast<QObject*> (objectAt(index));
}

template <typename T>
void IQObjectBasedTableModel<T>::remove(T *object)
{
    if (!_items.contains(object))
        return;

    emit beginRemoveRows(QModelIndex(), _items.indexOf(object), _items.indexOf(object));
    _items.removeAll(object);
    emit endRemoveRows();
}

template <typename T>
void IQObjectBasedTableModel<T>::clear()
{
    if (rowCount() == 0)
        return;

    emit beginRemoveRows(QModelIndex(), 0, rowCount() - 1);

    _items.clear();

    emit endRemoveRows();
}

template <typename T>
bool IQObjectBasedTableModel<T>::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int row = index.row();
    if (row < 0 || row >= _items.count())
        return false;

    int column = index.column();
    if (column < 0 || column >= _visibleProperties.count())
        return false;

    if (role >= FIRST_QML_ROLE + _propertyNames.count())
        return false;

    T* object = _items[row];
    if (!object)
        return false;

    QString propertyName = "";
    if (role < FIRST_QML_ROLE)
    {
        propertyName = _visibleProperties[index.column()];
    }
    else
    {
        int propertyIndex = role - FIRST_QML_ROLE;
        propertyName = _propertyNames[propertyIndex];
    }

    if (propertyName.isEmpty())
        return false;

    bool result = object->setProperty(propertyName.toLocal8Bit().constData(), value);

    if (result)
        emit dataChanged(this->index(row,column), this->index(row, column));

    return result;
}

template <typename T>
void IQObjectBasedTableModel<T>::setPropertyEditable(const QString &property, const bool editable)
{
    if (editable)
    {
        _editableProperties.insert(property);
    }
    else
    {
        _editableProperties.remove(property);
    }
}

template <typename T>
Qt::ItemFlags IQObjectBasedTableModel<T>::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

    if (index.column() < 0 || index.column() >= _visibleProperties.count())
        return defaultFlags;

    if (index.row() < 0 || index.row() >= _items.count())
        return defaultFlags;

    QString propertyName = _visibleProperties[index.column()];

    QMetaProperty property = T::staticMetaObject.property(_propertyNames.indexOf(propertyName));

    if (_editableProperties.contains(propertyName)
            && property.isWritable())
        defaultFlags = defaultFlags | Qt::ItemIsEditable;

    return defaultFlags;
}

template <typename T>
bool IQObjectBasedTableModel<T>::moveRows(const QModelIndex &sourceParent,
                                       int sourceRow,
                                       int count,
                                       const QModelIndex &destinationParent,
                                       int destinationChild)
{
    Q_UNUSED(sourceParent);
    Q_UNUSED(destinationParent);
    if (sourceRow < 0 || sourceRow >= _items.count())
        return false;

    int correctedCount = _items.count() - (sourceRow + count) < 0 ? _items.count() - sourceRow : count;

    int correctedDestinationChild = destinationChild;

    if (destinationChild < 0)
        correctedDestinationChild = 0;

    if (destinationChild + correctedCount > _items.count() + 1)
        correctedDestinationChild = _items.count() - correctedCount;

    if (sourceRow == correctedDestinationChild)
        return false;

    emit beginMoveRows(sourceParent, sourceRow, sourceRow + correctedCount - 1, destinationParent, correctedDestinationChild);

    QList<T *> temp;

    for (int i = sourceRow + correctedCount - 1; i >= sourceRow ; i--)
    {
        temp << _items[i];
        _items.removeAt(i);
    }

    for (int i = 0; i < correctedCount; i++)
    {
        if (destinationChild < sourceRow)
        {
            _items.insert(destinationChild + i, temp[i]);
        }
        else
        {
            _items.insert(destinationChild + i - 1, temp[i]);
        }
    }

    emit endMoveRows();

    return true;
}

#endif // IQOBJECTBASEDTABLEMODEL_H
