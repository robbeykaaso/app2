import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import Pipeline 1.0

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
                        Pipeline.run(name + "_updateSelectedDataValue", text)
                    }
                    Component.onCompleted: {
                        Pipeline.add(function(aInput){
                            text = aInput.data()
                        }, {name: name + "_updateSelectedDataValueGUI", vtype: "string"})
                    }
                }
            }
            Button{
                text: "+"
                width: 40
                height: 30
                onClicked:
                    wd.show()
                    //Pipeline2.run("_newObject", {title: Pipeline2.tr("new data"), content: {name: "", type: {model: ["string", "number", "on-off", "select", "event", "image", "page", "cloud", "relative"], index: 0}, value: ""}, tag: {tag: "new_" + name}})
            }
        }
        TreeView0{
            tree_name: name
            width: parent.width
            height: parent.height - 30
        }
    }
    DataWindow{
        id: wd
        dname: name
    }
}
