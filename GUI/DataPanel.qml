import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import Pipeline2 1.0

Window{
    property string name
    modality: Qt.NonModal
    width: 300
    height: 600

    Column{
        anchors.fill: parent
        Row{
            width: parent.width
            height: 30
            Item{
                width: 180
                height: 30
            }
            Rectangle {
                color: "silver"
                width: 80
                height: 30
                anchors.verticalCenter: parent.verticalCenter
                TextInput {
                    id: val
                    clip: true
                    anchors.fill: parent
                    selectByMouse: true
                    text: ""
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 12
                    onAccepted: {
                        Pipeline2.run(name + "_updateSelectedDataValue", text)
                    }
                    Component.onCompleted: {
                        Pipeline2.add(function(aInput){
                            text = aInput
                        }, {name: name + "_updateSelectedDataValueGUI", vtype: ""})
                    }
                }
            }
            Button{
                text: "+"
                width: 40
                height: 30
                onClicked:
                    Pipeline2.run("_newObject", {title: Pipeline2.tr("new data"), content: {name: "", value: ""}, tag: {tag: "new_" + name}})
            }
        }
        TreeView0{
            tree_name: name
            width: parent.width
            height: parent.height - 30
        }
    }
}
