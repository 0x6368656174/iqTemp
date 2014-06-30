#include "iqtempmainwindow.h"
#include "iqtempsensoreditdialog.h"
#include <QMessageBox>
#include <QMediaPlaylist>
#include "ui_iqtempmainwindow.h"
#include <QSettings>
#include <QFileInfo>
#include <QQuickView>
#include <QQmlContext>
#include <QQuickItem>
#include <QMultiHash>
#include "iqlogger.h"

IQTempMainWindow* IQTempMainWindow::_instance = NULL;

IQTempMainWindow::IQTempMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::IQTempMainWindow),
    _tempModel(new IQObjectBasedTableModel<IQTempSensor>(this)),
    _tempProxyModel(new IQTempProxyModel(this)),
    _mediaPlayer(new QMediaPlayer(this)),
    _warningPlaylist(new QMediaPlaylist(_mediaPlayer)),
    _criticalPlaylist(new QMediaPlaylist(_mediaPlayer)),
    _warningsCount(0),
    _criticalsCount(0)
{
    QSettings settings;
    if (!settings.contains("silentTime"))
        settings.setValue("silentTime", 300000);

    ui->setupUi(this);
    _instance = this;
    IQLogger::logger()->setDebugLogEnabled(false);

    connect(ui->actionAddSensor, SIGNAL(triggered()), this, SLOT(addNewTempSensor()));
    connect(ui->actionRemoveSensor, SIGNAL(triggered()), this, SLOT(removeCurrentTempSensor()));
    connect(ui->actionEditCurrentSensor, SIGNAL(triggered()), this, SLOT(editCurrentTempSensor()));
    connect(ui->actionMoveSensorUp, SIGNAL(triggered()), this, SLOT(moveCurrentTempSensroUp()));
    connect(ui->actionMoveSensorDown, SIGNAL(triggered()), this, SLOT(moveCurrentTempSensorDown()));

    ui->editingToolBar->addAction(ui->actionAddSensor);
    ui->editingToolBar->addAction(ui->actionRemoveSensor);
    ui->editingToolBar->addAction(ui->actionEditCurrentSensor);
    ui->editingToolBar->addSeparator();
    ui->editingToolBar->addAction(ui->actionMoveSensorUp);
    ui->editingToolBar->addAction(ui->actionMoveSensorDown);

    ui->helpToolBar->addAction(ui->actionAbout);
    ui->helpToolBar->addAction(ui->actionAboutQt);

    ui->tempTableView->addAction(ui->actionAddSensor);
    ui->tempTableView->addAction(ui->actionRemoveSensor);
    ui->tempTableView->addAction(ui->actionEditCurrentSensor);
    ui->tempTableView->addAction(ui->actionMoveSensorUp);
    ui->tempTableView->addAction(ui->actionMoveSensorDown);

    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(ui->actionAboutQt, SIGNAL(triggered()), this, SLOT(showAboutQt()));

    ui->tempTableView->setContextMenuPolicy(Qt::ActionsContextMenu);

    _tempModel->showProperty("name", tr("Sensor Name"));
    _tempModel->showProperty("temp", tr("Temp"));
    _tempModel->showProperty("humidity", tr("Humidity"));

    _tempProxyModel->setSourceModel(_tempModel);

    ui->tempTableView->setModel(_tempProxyModel);

    ui->tempTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tempTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tempTableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    connect(ui->tempTableView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(checkCurrentSelectedItem(QModelIndex,QModelIndex)));

    connect(ui->tempTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editCurrentTempSensor()));

    checkCurrentSelectedItem(QModelIndex(), QModelIndex());

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(checkEditingToolBarEnabled()));

    QFileInfo warningMP3 ("warning.mp3");
    if (warningMP3.exists())
    {
        iqInfo() << tr("Found %0. Warning sound enabled.").arg(warningMP3.absoluteFilePath());
        _warningPlaylist->addMedia(QUrl::fromLocalFile(warningMP3.absoluteFilePath()));
        _warningPlaylist->setCurrentIndex(1);
        _warningPlaylist->setPlaybackMode(QMediaPlaylist::Loop);
    }
    else
    {
        iqInfo() << tr("Not found %0. Warning sound disabled.").arg(warningMP3.absoluteFilePath());
    }

    QFileInfo criticalMP3 ("critical.mp3");
    if (criticalMP3.exists())
    {
        iqInfo() << tr("Found %0. Critical sound enabled.").arg(criticalMP3.absoluteFilePath());
        _criticalPlaylist->addMedia(QUrl::fromLocalFile(criticalMP3.absoluteFilePath()));
        _criticalPlaylist->setCurrentIndex(1);
        _criticalPlaylist->setPlaybackMode(QMediaPlaylist::Loop);
    }
    else
    {
        iqInfo() << tr("Not found %0. Critical sound disabled.").arg(criticalMP3.absoluteFilePath());
    }

    //Добавим QML
    qmlRegisterType<IQTempSensor>("ru.itquasar.iqtemp", 1, 0, "IQTempSensor");
    QQuickView *view = new QQuickView();
    view->rootContext()->setContextProperty("tempModel", _tempModel);
    view->setSource(QUrl("qrc:/qml/iqtemp/MimicPanel.qml"));
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    if (view->rootObject())
        view->rootObject()->setProperty("borderColor", ui->tabWidget->palette().color(QPalette::Mid));

    QWidget *container = QWidget::createWindowContainer(view, this);
    ui->mimicPanelLayout->addWidget(container);

    loadAllSensors();
}

IQTempMainWindow* IQTempMainWindow::instance()
{
    return IQTempMainWindow::_instance;
}

void IQTempMainWindow::writeToLog(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    switch (type) {
    case QtDebugMsg:
        ui->logPlainTextEditor->appendPlainText(QString("DEBUG: %0\n")
                                               .arg(msg));
        break;
    case QtWarningMsg:
        ui->logPlainTextEditor->appendPlainText(QString("WARNING: %0\n")
                                               .arg(msg));
        break;
    case QtCriticalMsg:
        ui->logPlainTextEditor->appendPlainText(QString("CRITICAL: %0\n")
                                               .arg(msg));
        break;
    case QtFatalMsg:
        ui->logPlainTextEditor->appendPlainText(QString("FATAL: %0\n")
                                               .arg(msg));
        break;
    }
}

void IQTempMainWindow::checkCurrentSelectedItem(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    if (current.isValid())
    {
        ui->actionEditCurrentSensor->setEnabled(true);
        ui->actionRemoveSensor->setEnabled(true);

        if (current.row() > 0)
        {
            ui->actionMoveSensorUp->setEnabled(true);
        }
        else
        {
            ui->actionMoveSensorUp->setEnabled(false);
        }
        if (current.row() < _tempModel->rowCount() - 1)
        {
            ui->actionMoveSensorDown->setEnabled(true);
        }
        else
        {
            ui->actionMoveSensorDown->setEnabled(false);
        }
    }
    else
    {
        ui->actionEditCurrentSensor->setEnabled(false);
        ui->actionRemoveSensor->setEnabled(false);
        ui->actionMoveSensorDown->setEnabled(false);
        ui->actionMoveSensorUp->setEnabled(false);
    }
}

void IQTempMainWindow::addNewTempSensor()
{
    IQTempSensorEditDialog *dialog = new IQTempSensorEditDialog(this);
    IQTempSensor *sensor = new IQTempSensor(_tempModel);
    dialog->setSensor(sensor);

    if (dialog->exec())
    {
        int currentRow = ui->tempTableView->selectionModel()->currentIndex().row();
        if (currentRow != -1)
        {
            _tempModel->insert(currentRow + 1, sensor);
        }
        else
        {
            _tempModel->append(sensor);
        }

        ui->tempTableView->setCurrentIndex(_tempProxyModel->index(_tempModel->rowOf(sensor), 0));

        saveSensor(sensor);

        iqInfo() << tr("Added new sensor \"%0\"").arg(sensor->name());
    }
    else
    {
        sensor->deleteLater();
    }

    dialog->deleteLater();
}

void IQTempMainWindow::removeCurrentTempSensor()
{
    int currentRow = ui->tempTableView->selectionModel()->currentIndex().row();
    if (currentRow == -1)
        return;
    IQTempSensor *sensor = _tempModel->objectAt(currentRow);
    if (!sensor)
        return;
    if(QMessageBox::question(this, tr("Remove the sensor?"),
                             tr("Are you sure you want to remove the sensor \"%0\"?! "
                                "After removing cancel impossible. Remove the sensor?")
                             .arg(sensor->name().isEmpty()?tr("No Name"):sensor->name()))
            == QMessageBox::Yes)
    {
        QSettings settings;
        settings.beginGroup("sensors");
        settings.remove("sensor_" + QString::number(sensor->number()));
        _tempModel->remove(sensor);

        iqInfo() << tr("Removed sensor \"%0\"").arg(sensor->name());

        sensor->deleteLater();
    }
}

void IQTempMainWindow::editCurrentTempSensor()
{
    int currentRow = ui->tempTableView->selectionModel()->currentIndex().row();
    if (currentRow == -1)
        return;
    IQTempSensor *sensor = _tempModel->objectAt(currentRow);
    if (!sensor)
        return;

    IQTempSensorEditDialog *dialog = new IQTempSensorEditDialog(this);
    dialog->setSensor(sensor);

    if(dialog->exec())
    {
        saveSensor(sensor);

        iqInfo() << tr("Changed sensor \"%0\"").arg(sensor->name());
    }

}

void IQTempMainWindow::moveCurrentTempSensroUp()
{
    int currentRow = ui->tempTableView->selectionModel()->currentIndex().row();
    if (currentRow == -1)
        return;

    if(_tempModel->moveRow(QModelIndex(), currentRow, QModelIndex(), currentRow - 1))
    {
        checkCurrentSelectedItem(ui->tempTableView->selectionModel()->currentIndex(), QModelIndex());

        saveSensor(_tempModel->objectAt(currentRow));
        saveSensor(_tempModel->objectAt(currentRow - 1));

        iqInfo() << tr("Sensor \"%0\" moved up").arg(_tempModel->objectAt(currentRow - 1)->name());
    }
}

void IQTempMainWindow::moveCurrentTempSensorDown()
{
    int currentRow = ui->tempTableView->selectionModel()->currentIndex().row();
    if (currentRow == -1)
        return;

    if(_tempModel->moveRow(QModelIndex(), currentRow, QModelIndex(), currentRow + 2))
    {
        checkCurrentSelectedItem(ui->tempTableView->selectionModel()->currentIndex(), QModelIndex());

        saveSensor(_tempModel->objectAt(currentRow));
        saveSensor(_tempModel->objectAt(currentRow + 1));

        iqInfo() << tr("Sensor \"%0\" moved down").arg(_tempModel->objectAt(currentRow + 1)->name());
    }
}

IQTempMainWindow::~IQTempMainWindow()
{
    delete ui;
}

void IQTempMainWindow::saveSensor(IQTempSensor *sensor)
{
    if (!sensor)
        return;

    QSettings settings;
    settings.beginGroup("sensors");

    if (sensor->number() == -1)
    {
        //Найдем пустой id
        qint32 maxSensorId = -1;

        QStringList sensors = settings.childGroups();
        foreach (QString sensorGroup, sensors)
        {
            QString number = sensorGroup.mid(7);
            if (number.toInt() > maxSensorId)
                maxSensorId = number.toInt();
        }

        sensor->setNumer(maxSensorId + 1);
    }

    settings.beginGroup("sensor_" + QString::number(sensor->number()));

    settings.setValue("enabled", sensor->enabled());
    settings.setValue("name", sensor->name());
    settings.setValue("port", sensor->port());
    settings.setValue("description", sensor->description());
    settings.setValue("minCriticalTemp", sensor->minCriticalTemp());
    settings.setValue("minWarningTemp", sensor->minWarningTemp());
    settings.setValue("maxWarningTemp", sensor->maxWarningTemp());
    settings.setValue("maxCriticalTemp", sensor->maxCriticalTemp());
    settings.setValue("maxTempIdle", sensor->maxTempIdle());
    settings.setValue("row", _tempModel->rowOf(sensor));
}

void IQTempMainWindow::loadAllSensors()
{
    QList<IQTempSensor *> oldSensors;
    for (int i = 0; i < _tempModel->rowCount(); i++)
    {
        oldSensors << _tempModel->objectAt(i);
    }
    _tempModel->clear();

    qDeleteAll(oldSensors);

    QSettings settings;

    settings.beginGroup("sensors");

    QMultiMap<int, IQTempSensor*> sensorOrder;
    QStringList sensors = settings.childGroups();
    foreach (QString sensorGroup, sensors)
    {
        QString number = sensorGroup.mid(7);

        IQTempSensor *sensor = new IQTempSensor(_tempModel);
        connect(sensor, SIGNAL(warning()), this, SLOT(showWarning()));
        connect(sensor, SIGNAL(critical()), this, SLOT(showCritical()));

        sensor->setNumer(number.toInt());

        settings.beginGroup(sensorGroup);

        sensor->setEnabled(settings.value("enabled").toBool());
        sensor->setName(settings.value("name").toString());
        sensor->setPort(settings.value("port").toUInt());
        sensor->setDescription(settings.value("description").toString());
        sensor->setMinCriticalTemp(settings.value("minCriticalTemp").toReal());
        sensor->setMinWarningTemp(settings.value("minWarningTemp").toReal());
        sensor->setMaxWarningTemp(settings.value("maxWarningTemp").toReal());
        sensor->setMaxCriticalTemp(settings.value("maxCriticalTemp").toReal());
        sensor->setMaxTempIdle(settings.value("maxTempIdle").toInt());

        sensorOrder.insert(settings.value("row").toInt(), sensor);

        settings.endGroup();
    }

    QMutableMapIterator<int, IQTempSensor *> sensorOrderI(sensorOrder);
    while (sensorOrderI.hasNext())
    {
        sensorOrderI.next();
        _tempModel->append(sensorOrderI.value());
    }
}

void IQTempMainWindow::showWarning()
{
    IQTempSensor *sensor = qobject_cast<IQTempSensor *>(sender());
    if (!sensor)
        return;

    _warningsCount++;

    QMessageBox *warningMessage = new QMessageBox(QMessageBox::Warning,
                                                  tr("Sensor \"%0\". Warning temp!").arg(sensor->name()),
                                                  tr("Warning! Temperature sensor \"%0\" married normal values​​!").arg(sensor->name()),
                                                  QMessageBox::Ok);

    iqWarning() << tr("Warning! Temperature sensor \"%0\" married normal values​​!").arg(sensor->name());

    connect(warningMessage, SIGNAL(finished(int)), this, SLOT(remOneWarning()));

    warningMessage->show();

    checkSound();
}

void IQTempMainWindow::showCritical()
{
    IQTempSensor *sensor = qobject_cast<IQTempSensor *>(sender());
    if (!sensor)
        return;

    _criticalsCount++;

    QMessageBox *criticalMessage = new QMessageBox(QMessageBox::Critical,
                                                  tr("Sensor \"%0\". Critical temp!").arg(sensor->name()),
                                                  tr("Warning! Temperature sensor \"%0\" was published in the critical values​​!").arg(sensor->name()),
                                                  QMessageBox::Ok);

    iqCritical() << tr("Warning! Temperature sensor \"%0\" was published in the critical values​​!").arg(sensor->name());

    connect(criticalMessage, SIGNAL(finished(int)), this, SLOT(remOneCritical()));

    criticalMessage->show();

    checkSound();
}

void IQTempMainWindow::checkSound()
{
    if (_criticalsCount > 0)
    {
        if (_mediaPlayer->playlist() == _criticalPlaylist && _mediaPlayer->state() == QMediaPlayer::PlayingState)
            return;

        _mediaPlayer->setPlaylist(_criticalPlaylist);
        _mediaPlayer->play();

        return;
    }
    else if (_warningsCount > 0)
    {
        if (_mediaPlayer->playlist() == _warningPlaylist && _mediaPlayer->state() == QMediaPlayer::PlayingState)
            return;

        _mediaPlayer->setPlaylist(_warningPlaylist);
        _mediaPlayer->play();

        return;
    }

    _mediaPlayer->stop();
}

void IQTempMainWindow::remOneWarning()
{
    QMessageBox *messageBox = qobject_cast<QMessageBox *>(sender());
    if (!messageBox)
        return;

    messageBox->deleteLater();

    _warningsCount--;
    checkSound();
}

void IQTempMainWindow::remOneCritical()
{
    QMessageBox *messageBox = qobject_cast<QMessageBox *>(sender());
    if (!messageBox)
        return;

    messageBox->deleteLater();

    _criticalsCount--;
    checkSound();
}

void IQTempMainWindow::checkEditingToolBarEnabled()
{
    if (ui->tabWidget->currentIndex() == 0)
    {
        ui->editingToolBar->setEnabled(true);
    }
    else
    {
        ui->editingToolBar->setEnabled(false);
    }
}

void IQTempMainWindow::showAbout()
{
    QMessageBox::about(this, tr("About IqTemp"),
                       tr("IqTemp - application for automatic monitoring of temperature sensors. It "
                          "automatically collects and displays agregitiruet information about the "
                          "current state of the sensors. Just in case of excess kreticheskih temperature "
                          "values ​​provided by automatic voice notification displaying messages about it.\n\n"

                          "There are two display modes: as a table and as a mimic. Editing sensors "
                          "only when you select the table.\n\n"

                          "Author: Pavel Puchkov"));
}

void IQTempMainWindow::showAboutQt()
{
    QMessageBox::aboutQt(this);
}
