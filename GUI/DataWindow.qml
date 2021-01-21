import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import "../gui/Basic"
import Pipeline2 1.0

Window{
    width: 400
    height: 200
    modality: Qt.WindowModal
    Rectangle{
        anchors.fill: parent
        color: "azure"
        Column{
            anchors.fill: parent
            spacing: 10
            Combo{
                id: cmb
                anchors.horizontalCenter: parent.horizontalCenter
                width: 240
                caption.text: qsTr("type") + ":"
                combo.model: ["string", "number", "on-off", "select", "event", "image", "page", "cloud", "relative"]
                ratio: 0.4
                combo.onCurrentTextChanged: {

                }
            }
            Edit{
                anchors.horizontalCenter: parent.horizontalCenter
                caption.text: qsTr("name") + ": "
                input.text: fs.page_text
                width: 240
                ratio: 0.4
            }
            Edit{
                anchors.horizontalCenter: parent.horizontalCenter
                caption.text: qsTr("value") + ": "
                input.text: fs.page_text
                width: 240
                ratio: 0.4
            }
            Button{
                property var cmds: {"on-off": true, "event": true, "page": true, "cloud": true}
                width: 120
                height: 30
                anchors.horizontalCenter: parent.horizontalCenter
                text: "select"
                visible: cmds[cmb.combo.currentText] !== undefined
            }
            Button{
                width: 120
                height: 30
                anchors.horizontalCenter: parent.horizontalCenter
                text: "ok"
            }
        }
    }
}
