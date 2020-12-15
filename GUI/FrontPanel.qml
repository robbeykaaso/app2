import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import Pipeline2 1.0

Window{
    modality: Qt.NonModal
    width: 300
    height: 600
    ListView{
        anchors.fill: parent
        delegate: Rectangle{
            width: parent.width
            height: 30
            Text {
                text: nm
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: 16
            }
            border.color: "black"
            MouseArea{
                anchors.fill: parent
                onClicked: Pipeline2.run("createFrontEndCom", nm)
            }
        }
        ScrollBar.vertical: ScrollBar {
        }
        model: ListModel{
            ListElement{
                nm: "com1"
            }
            ListElement{
                nm: "com2"
            }
        }
    }
}
