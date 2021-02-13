import QtQuick 2.12
import QtQuick.Controls 2.5
import Pipeline 1.0

Rectangle{
    property bool needtr: false
    property string group: "shape"
    property string label
    property var colormap: ({})
    property int fontsize: 14
    signal updatelabel(string aLabel)
    width: 80
    height: 30
    color: colormap[label] ? (colormap[label]["color"] ? colormap[label]["color"] : "white") : "white"
    border.color: "black"
    clip: true
    Label{
        text: needtr ? Pipeline.tr(label) : label
        font.pixelSize: fontsize
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
    }
    MouseArea{
        hoverEnabled: true
        anchors.fill: parent
        onClicked: {
            showMenu()
        }
        onEntered: {
            ToolTip.visible = true
        }
        onExited: {
            ToolTip.visible = false
        }
        ToolTip.text: needtr ? Pipeline.tr(label) : label
    }
    Menu{
        id: lblmenu
        y: parent.height
        width: parent.width
        /*MenuItem{
            text: Pipeline.tr("world")
            onClicked: {
                label = text
                updatelabel(label)
            }
        }
        MenuItem{
            text: Pipeline.tr("hello")
        }*/
    }
    /*Component{
        id: lbl
        MenuItem{
            onClicked: {
                label = text
                updatelabel(label)
            }
        }
    }*/

    Component{
        id: itm
        MenuItem{
            property int fontsize
            property string org_text
            font.pixelSize: fontsize
            onClicked: {
                label = text
                updatelabel(org_text)
            }
            ToolTip.visible: hovered
            ToolTip.text: text
        }
    }

    function showMenu(){
        lblmenu.open()
    }

    function updateMenu(aLabels){
        for (var i = lblmenu.count - 1; i >= 0; --i)
            lblmenu.removeItem(lblmenu.itemAt(i))
        for (var j in aLabels){
            var tmp = itm.createObject(lblmenu.contentItem, {org_text: aLabels[j]["id"], text: aLabels[j]["name"], fontsize: fontsize})
            lblmenu.addItem(tmp)
        }
    }
}
