import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import Pipeline2 1.0

Window{
    modality: Qt.NonModal
    width: 300
    height: 600
    Column{
        anchors.fill: parent
        Row{
            width: parent.width
            height: 30
            Item{
                width: 220
                height: 30
            }
            Button{
                text: "口"
                width: 40
            }
            Button{
                text: "+"
                width: 40
            }
        }
        TreeView0{
            width: parent.width
            height: parent.height - 30
        }
    }
}
