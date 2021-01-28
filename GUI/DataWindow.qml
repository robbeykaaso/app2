import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import "../qml/gui/Basic"
import "../qml/gui/Pipe"
import Pipeline 1.0

Window{
    property string dname
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
                combo.model: ["string", "number", "on-off", "select", "event", "image", "page", "cloud", "color", "relative"]
                ratio: 0.4
                combo.onCurrentTextChanged: {
                    val.input.text = ""
                }
            }
            Edit{
                id: nm
                anchors.horizontalCenter: parent.horizontalCenter
                caption.text: qsTr("name") + ": "
                width: 240
                ratio: 0.4
            }
            Edit{
                id: val
                anchors.horizontalCenter: parent.horizontalCenter
                caption.text: qsTr("value") + ": "
                width: 240
                ratio: 0.4
                input.enabled: sel.cmds[cmb.combo.currentText] === undefined
            }
            Button{
                id: sel
                property var cmds: {"on-off": function(){
                                        Pipeline.run("_newObject", {title: "value", content: {value: true}}, "setValue_" + dname)
                                    },
                                    "event": function(){
                                        Pipeline.run("_selectFrontEvent", {}, "setValue_" + dname)
                                    },
                                    "page": function(){
                                        Pipeline.run("_selectPage", {}, "setValue_" + dname)
                                    },
                                    "cloud": function(){
                                        Pipeline.run("_selectBackEvent", {}, "setValue_" + dname)
                                    },
                                    "color": function(){
                                        Pipeline.run("_selectColor", {}, "setColor_" + dname)
                                    }}
                width: 120
                height: 30
                anchors.horizontalCenter: parent.horizontalCenter
                text: "select"
                visible: cmds[cmb.combo.currentText] !== undefined
                onClicked: cmds[cmb.combo.currentText]()
                Component.onCompleted: {
                    var st =
                    Pipeline.find("_newObject")
                    .nextB(function(aInput){
                        val.input.text = aInput.data()["value"]
                    }, "setValue_" + dname)

                    Pipeline.find("_selectPage").nextP(st)
                    Pipeline.find("_selectFrontEvent").nextP(st)
                    Pipeline.find("_selectBackEvent").nextP(st)

                    Pipeline.find("_selectColor")
                    .next(function(aInput){
                        val.input.text = aInput.data()
                    }, "setColor_" + dname, {vtype: "string"})
                }
            }
            Button{
                width: 120
                height: 30
                anchors.horizontalCenter: parent.horizontalCenter
                text: "ok"
                onClicked: Pipeline.run("new_" + dname, {type: cmb.combo.currentText, name: nm.input.text, value: val.input.text})
            }
        }
    }
    ColorSelect{

    }
}
