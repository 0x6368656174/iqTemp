#ifndef IQTEMPSENSOR_H
#define IQTEMPSENSOR_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QUdpSocket>

class IQTempSensor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint32 number READ number WRITE setNumer NOTIFY numberChanged)
    Q_PROPERTY(State healthState READ healthState NOTIFY healthStateChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(quint16 port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(qreal minCriticalTemp READ minCriticalTemp WRITE setMinCriticalTemp NOTIFY minCriticalTempChanged)
    Q_PROPERTY(qreal minWarningTemp READ minWarningTemp WRITE setMinWarningTemp NOTIFY minWarningTempChanged)
    Q_PROPERTY(qreal temp READ temp WRITE setTemp NOTIFY tempChanged)
    Q_PROPERTY(qreal maxWarningTemp READ maxWarningTemp WRITE setMaxWarningTemp NOTIFY maxWarningTempChanged)
    Q_PROPERTY(qreal maxCriticalTemp READ maxCriticalTemp WRITE setMaxCriticalTemp NOTIFY maxCriticalTempChanged)
    Q_PROPERTY(qreal humidity READ humidity WRITE setHumidity NOTIFY humidityChanged)
    Q_PROPERTY(qint32 maxTempIdle READ maxTempIdle WRITE setMaxTempIdle NOTIFY maxTempIdleChanged)

    Q_ENUMS(State)
public:
    enum State
    {
        Disabled,
        IdleTimeout,
        Ok,
        MinTempWarning,
        MaxTempWarning,
        MinTempCritical,
        MaxTempCritical
    };

    explicit IQTempSensor(QObject *parent = 0);

    inline qint32 number() const {return _number;}

    void setNumer(const qint32 number);

    State healthState() const;

    inline bool enabled() const {return _enabled;}

    void setEnabled(const bool enabled);

    inline QString name() const {return _name;}

    void setName(const QString &name);

    inline quint16 port() const {return _port;}

    void setPort(const quint16 &port);

    inline QString description() const {return _description;}

    void setDescription(const QString &description);

    inline qreal minCriticalTemp() const {return _minCriticalTemp;}

    void setMinCriticalTemp(const qreal temp);

    inline qreal minWarningTemp() const {return _minWarningTemp;}

    void setMinWarningTemp(const qreal temp);

    inline qreal temp() const {return _temp;}

    void setTemp(const qreal temp);

    inline qreal maxWarningTemp() const {return _maxWarniingTemp;}

    void setMaxWarningTemp(const qreal temp);

    inline qreal maxCriticalTemp() const {return _maxCriticalTemp;}

    void setMaxCriticalTemp(const qreal temp);

    inline qreal humidity() const {return _humidity;}

    void setHumidity(const qreal humidity);

    inline qint32 maxTempIdle() {return _maxTempIdle;}

    void setMaxTempIdle(const qint32 maxIdle);

signals:
    void warning();
    void critical();

    void numberChanged();
    void enabledChanged();
    void healthStateChanged();
    void nameChanged();
    void portChanged();
    void descriptionChanged();
    void minCriticalTempChanged();
    void minWarningTempChanged();
    void tempChanged();
    void maxWarningTempChanged();
    void maxCriticalTempChanged();
    void humidityChanged();
    void maxTempIdleChanged();

private slots:
    void onIdleTimeout();
    void checkCriticalWarningEnable();

    void readSocketData();

private:
    qint32 _number;

    bool _enabled;

    QString _name;
    quint16 _port;
    QString _description;

    qreal _minCriticalTemp;
    qreal _minWarningTemp;
    qreal _temp;
    qreal _maxWarniingTemp;
    qreal _maxCriticalTemp;

    qreal _humidity;

    qint64 _maxTempIdle;
    bool _tempIdleTimeout;
    QTimer *_tempIdleTimer;

    State _oldState;
    void checkState();

    bool _enableWarning;
    bool _enableCritical;
    QTimer *_criticalWarningCheckTimer;

    QUdpSocket *_socket;
};

#endif // IQTEMPSENSOR_H
