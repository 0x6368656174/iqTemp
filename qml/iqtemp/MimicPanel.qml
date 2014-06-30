import QtQuick 2.2
import QtGraphicalEffects 1.0
import ru.itquasar.iqtemp 1.0

Rectangle
{
    property color borderColor: "black"
    border.color: borderColor
    border.width: 1
    FlowListView {
        id: tempView
        anchors.fill: parent
        model: tempModel

        delegate: Item {
            width: 120
            height: 120

            Rectangle{
                anchors.fill: parent
                anchors.margins: 5
                color: {
                    if (healthState === IQTempSensor.IdleTimeout) {
                        return "#FFF042"
                    }
                    else if (healthState === IQTempSensor.MinTempWarning) {
                        return "#BDE4FF"
                    }
                    else if (healthState === IQTempSensor.MaxTempWarning) {
                        return "#FFC7C7"
                    }
                    else if (healthState === IQTempSensor.MinTempCritical) {
                        return "#0064A8"
                    }
                    else if (healthState === IQTempSensor.MaxTempCritical) {
                        return "#A80002"
                    }
                    return "#9DFF85"
                }
                border.color: color != "#ffffff"?Qt.darker(color, 2):"#A1DBE2";
                border.width: 2
                radius: 3

                MouseArea {
                    anchors.fill: parent
                    onClicked: tempView.currentIndex = index
                }

                Text {
                    id: tempText
                    text: temp == -100?qsTr("Undefined"):temp.toFixed(1)+"°C"
                    color: "#191919"

                    anchors.centerIn: parent
                    font.bold: true
                    anchors.verticalCenterOffset: 15
                    font.pixelSize: temp == -100?16:25
                }

                Text {
                    id: nameText
                    text: name
                    anchors.margins: 5
                    anchors.verticalCenter: parent.top
                    anchors.verticalCenterOffset: 30
                    anchors.left: parent.left
                    anchors.right: parent.right
                    horizontalAlignment: Text.Center
                    wrapMode: Text.WordWrap
                    elide: Text.ElideRight
                    maximumLineCount: 3
                    color: "#191919"
                }

                Text {
                    text: humidity!=-100?qsTr("Humidity: " ) + humidity.toFixed(1) + "%":qsTr("Humidity: Undefined")
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 10
                    font.pixelSize: 10
                }
            }
        }
    }
}
