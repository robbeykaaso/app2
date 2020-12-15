import QtQuick 2.12
import QtQuick.Controls 2.5
import "../gui/Basic"
import "../gui/TreeNodeView"
import QSGBoard 1.0

ApplicationWindow {
    id: mainwindow
    visible: true
    width: 1200
    height: 800
    Column{
        anchors.fill: parent
        Rectangle{
            width: parent.width
            height: 60
            color: "azure"
            Row{
                anchors.fill: parent
                spacing: 5
                Text {
                    text: qsTr("购物商城app")
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 16
                }
                Button{
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("保存")
                    font.pixelSize: 12
                    width: 40
                    height: 40
                }
                Button{
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("刷新")
                    font.pixelSize: 12
                    width: 40
                    height: 40
                }
                Item{
                    width: 400
                    height: parent.height
                }
                Button{
                    text: qsTr("组合")
                    font.pixelSize: 12
                    anchors.verticalCenter: parent.verticalCenter
                    width: 40
                    height: 40
                }
                Button{
                    text: qsTr("整理")
                    font.pixelSize: 12
                    anchors.verticalCenter: parent.verticalCenter
                    width: 40
                    height: 40
                }
                Button{
                    text: qsTr("打散")
                    font.pixelSize: 12
                    anchors.verticalCenter: parent.verticalCenter
                    width: 40
                    height: 40
                }
            }

        }
        Row{
            width: parent.width
            height: parent.height - 60
            Column{
                width: 60
                height: parent.height
                spacing: 5
                Button{
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("页面")
                    font.pixelSize: 12
                    width: 40
                    height: 80
                }
                Button{
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("元素")
                    font.pixelSize: 12
                    width: 40
                    height: 80
                }
                Button{
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("库")
                    font.pixelSize: 12
                    width: 40
                    height: 30
                    onClicked: front.show()
                }
            }

            Item{
                width: parent.width - 120
                height: parent.height
                QSGBoard{
                    name: "background"
                    plugins: [{type: "select"}]
                    anchors.fill: parent
                    Component.onDestruction: {
                        beforeDestroy()
                    }
                }
                Row{
                    anchors.fill: parent
                    Column{
                        property int del_size: 0
                        width: parent.width * 0.3 + del_size
                        height: parent.height
                        Rectangle{
                            property int del_size: 0
                            width: parent.width
                            height: parent.height * 0.5 + del_size
                            color: "gray"
                        }
                        Sizable{
                            width: parent.width
                            height: 1
                        }
                        Rectangle{
                            property int del_size: 0
                            width: parent.width
                            height: parent.height * 0.5 + del_size
                            color: "azure"
                            border.color: "black"
                            TreeNodeView {
                                root: "com"
                                anchors.fill: parent
                            }
                        }
                    }
                    Sizable{

                    }
                    Column{
                        property int del_size: 0
                        width: parent.width * 0.35 + del_size
                        height: parent.height
                        Row{
                            width: parent.width
                            height: 30
                            Combo{
                                width: parent.width - 120
                                height: parent.height
                                caption.text: qsTr("前端事件") + ":"
                                combo.model: [qsTr("登录事件"), qsTr("xx事件"), qsTr("yy事件")]
                                ratio: 0.2
                            }
                            Button{
                                text: qsTr("添加")
                                width: 60
                                font.pixelSize: 10
                                height: parent.height
                            }
                            Button{
                                text: qsTr("已创数据")
                                width: 60
                                font.pixelSize: 10
                                height: parent.height
                            }
                        }
                        Rectangle{
                            width: parent.width
                            height: parent.height - 30
                            QSGBoard{
                                name: "frontend"
                                plugins: [{type: "select"}]
                                anchors.fill: parent
                                Component.onDestruction: {
                                    beforeDestroy()
                                }
                            }
                            border.color: "black"
                        }
                    }
                    Sizable{

                    }
                    Column{
                        property int del_size: 0
                        width: parent.width * 0.35 + del_size
                        height: parent.height
                        Row{
                            width: parent.width
                            height: 30
                            Combo{
                                width: parent.width - 120
                                height: parent.height
                                caption.text: qsTr("云服务器事件") + ":"
                                combo.model: [qsTr("登录事件"), qsTr("xx事件"), qsTr("yy事件")]
                                ratio: 0.2
                            }
                            Button{
                                text: qsTr("添加")
                                width: 60
                                font.pixelSize: 10
                                height: parent.height
                            }
                            Button{
                                text: qsTr("已创数据")
                                width: 60
                                font.pixelSize: 10
                                height: parent.height
                            }
                        }
                        Rectangle{
                            width: parent.width
                            height: parent.height - 30
                            QSGBoard{
                                name: "backend"
                                plugins: [{type: "select"}]
                                anchors.fill: parent
                                Component.onDestruction: {
                                    beforeDestroy()
                                }
                            }
                            border.color: "black"
                        }
                    }
                }
            }
            Column{
                width: 60
                height: parent.height
                spacing: 5
                Button{
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("数据")
                    font.pixelSize: 12
                    width: 40
                    height: 80
                }
                Button{
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("事件")
                    font.pixelSize: 12
                    width: 40
                    height: 80
                }
                Button{
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("库")
                    font.pixelSize: 12
                    width: 40
                    height: 30
                    onClicked: back.show()
                }
            }
        }

    }
    FrontPanel{
        id: front
    }
    BackPanel{
        id: back
    }
}
