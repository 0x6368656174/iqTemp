#ifndef IQTEMPMAINWINDOW_H
#define IQTEMPMAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QUdpSocket>

#include "iqobjectbasedtablemodel.h"
#include "iqtempsensor.h"
#include "iqtempproxymodel.h"


namespace Ui {
class IQTempMainWindow;
}

class IQTempMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit IQTempMainWindow(QWidget *parent = 0);
    ~IQTempMainWindow();

    static IQTempMainWindow* instance();
    void writeToLog(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private slots:
    void checkCurrentSelectedItem(const QModelIndex & current, const QModelIndex & previous);
    void addNewTempSensor();
    void removeCurrentTempSensor();
    void editCurrentTempSensor();
    void moveCurrentTempSensorDown();
    void moveCurrentTempSensroUp();

    void showWarning();
    void showCritical();
    void remOneWarning();
    void remOneCritical();

    void showAbout();
    void showAboutQt();

    void checkEditingToolBarEnabled();

private:
    static IQTempMainWindow* _instance;
    Ui::IQTempMainWindow *ui;

    IQObjectBasedTableModel<IQTempSensor> *_tempModel;
    IQTempProxyModel *_tempProxyModel;

    QMediaPlayer *_mediaPlayer;
    QMediaPlaylist *_warningPlaylist;
    QMediaPlaylist *_criticalPlaylist;
    int _warningsCount;
    int _criticalsCount;
    void checkSound();

    void saveSensor(IQTempSensor *sensor);
    void loadAllSensors();
};

#endif // IQTEMPMAINWINDOW_H
