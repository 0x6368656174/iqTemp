#include "iqtempsensor.h"
#include <QSettings>
#include <QRegExp>

IQTempSensor::IQTempSensor(QObject *parent) :
    QObject(parent),
    _number(-1),
    _enabled(true),
    _name(""),
    _port(0),
    _description(""),
    _minCriticalTemp(-40),
    _minWarningTemp(-30),
    _temp(-100),
    _maxWarniingTemp(30),
    _maxCriticalTemp(40),
    _humidity(-100),
    _maxTempIdle(10000),
    _tempIdleTimeout(true),
    _tempIdleTimer(new QTimer(this)),
    _oldState(IdleTimeout),
    _enableWarning(true),
    _enableCritical(true),
    _criticalWarningCheckTimer(new QTimer(this)),
    _socket(new QUdpSocket(this))
{
    connect(_tempIdleTimer, SIGNAL(timeout()), this, SLOT(onIdleTimeout()));
    _tempIdleTimer->setSingleShot(true);

    connect(_criticalWarningCheckTimer, SIGNAL(timeout()), this, SLOT(checkCriticalWarningEnable()));
    _criticalWarningCheckTimer->setSingleShot(true);

    connect(_socket, SIGNAL(readyRead()), this, SLOT(readSocketData()));
}

IQTempSensor::State IQTempSensor::healthState() const
{
    if (!_enabled)
    {
        return Disabled;
    }
    else if (_tempIdleTimeout)
    {
        return IdleTimeout;
    }
    else if (_temp <= _minCriticalTemp)
    {
        return MinTempCritical;
    }
    else if (_temp <= _minWarningTemp)
    {
        return MinTempWarning;
    }
    else if (_temp >= _maxCriticalTemp)
    {
        return MaxTempCritical;
    }
    else if (_temp >= _maxWarniingTemp)
    {
        return MaxTempWarning;
    }
    return Ok;
}

void IQTempSensor::setNumer(const qint32 number)
{
    if (_number == number)
        return;
    _number = number;

    emit numberChanged();
}

void IQTempSensor::setEnabled(const bool enabled)
{
    if (_enabled == enabled)
        return;
    _enabled = enabled;

    emit enabledChanged();
}

void IQTempSensor::setName(const QString &name)
{
    if (_name == name)
        return;
    _name = name;

    emit nameChanged();
}

void IQTempSensor::setPort(const quint16 &port)
{
    if (_port == port)
        return;
    _port = port;

    if (_port != 0)
    {
        _socket->close();
        _socket->bind(_port, QUdpSocket::ShareAddress);
    }

    emit portChanged();
}

void IQTempSensor::setDescription(const QString &description)
{
    if (_description == description)
        return;
    _description = description;

    emit descriptionChanged();
}

void IQTempSensor::setMinCriticalTemp(const qreal temp)
{
    if (_minCriticalTemp == temp)
        return;
    _minCriticalTemp = temp;

    emit minCriticalTempChanged();

    checkState();
}

void IQTempSensor::setMinWarningTemp(const qreal temp)
{
    if (_minWarningTemp == temp)
        return;
    _minWarningTemp = temp;

    emit minWarningTempChanged();

    checkState();
}

void IQTempSensor::setTemp(const qreal temp)
{
    if (_temp != temp)
    {
        _temp = temp;

        emit tempChanged();
    }

    _tempIdleTimeout = false;
    _tempIdleTimer->start(_maxTempIdle);

    checkState();
}

void IQTempSensor::setMaxWarningTemp(const qreal temp)
{
    if (_maxWarniingTemp == temp)
        return;
    _maxWarniingTemp = temp;

    emit tempChanged();

    checkState();
}

void IQTempSensor::setMaxCriticalTemp(const qreal temp)
{
    if (_maxCriticalTemp == temp)
        return;
    _maxCriticalTemp = temp;

    emit tempChanged();

    checkState();
}

void IQTempSensor::setHumidity(const qreal humidity)
{
    if (_humidity == humidity)
        return;
    _humidity = humidity;

    emit humidityChanged();
}

void IQTempSensor::setMaxTempIdle(const qint32 maxIdle)
{
    if (_maxTempIdle == maxIdle)
        return;
    _maxTempIdle = maxIdle;

    emit maxTempIdleChanged();
}

void IQTempSensor::onIdleTimeout()
{
    _tempIdleTimeout = true;
    checkState();
}

void IQTempSensor::checkState()
{
    if (_oldState == healthState())
        return;

    State oldState = _oldState;

    _oldState = healthState();
    emit healthStateChanged();

    if (oldState >= MinTempCritical && healthState() < MinTempCritical)
    {
        QSettings settings;
        _criticalWarningCheckTimer->start(settings.value("silentTime").toInt());
    }
    else if (oldState >= MinTempWarning && healthState() < MinTempWarning)
    {
        QSettings settings;
        _criticalWarningCheckTimer->start(settings.value("silentTime").toInt());
    }
    else if (healthState() == MinTempWarning || healthState() == MaxTempWarning)
    {
        if (_enableWarning)
        {
            _enableWarning = false;
            emit warning();
        }
    }
    else if (healthState() == MinTempCritical || healthState() == MaxTempCritical)
    {
        _enableWarning = false;
        if (_enableCritical)
        {
            _enableCritical = false;
            emit critical();
        }
    }
}

void IQTempSensor::checkCriticalWarningEnable()
{
    if (healthState() < MinTempCritical)
    {
        _enableCritical = true;
    }
    if (healthState() < MinTempWarning)
    {
        _enableCritical = true;
        _enableWarning = true;
    }
}

void IQTempSensor::readSocketData()
{
    while (_socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(_socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        _socket->readDatagram(datagram.data(), datagram.size(),
                              &sender, &senderPort);

        QString str (datagram);

        QRegExp rxlen("\\[temp= {0,2}(-{0,1}\\d+), hum= {0,2}(\\d+)\\]");
        int pos = rxlen.indexIn(str);
        if (pos > -1) {
            QString temp = rxlen.cap(1);
            QString hum = rxlen.cap(2);

            setTemp(temp.toFloat());
            setHumidity(hum.toFloat());
        }
    }
}
