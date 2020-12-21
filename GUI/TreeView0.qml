//ref: https://blog.csdn.net/colouroo/article/details/44700357
import QtQuick 2.0
import Pipeline2 1.0

Item {
   width: 600
   height: 600
   property string sel
   property string tree_name

   function buildTree(aModel, aData, aLevel = 0){
       if (aData === undefined)
           return
       aModel.clear()
       var idx = 0
       for (var i in aData){
           aModel.append({"name": aData[i]["name"], "type": aData[i]["type"], "level": aLevel, "subNode": [], "idd": aData[i]["id"]})
           buildTree(aModel.get(idx++).subNode, aData[i]["children"], aLevel + 1)
       }
   }

   //Model
   ListModel {
      id: objModel
      Component.onCompleted: {
          var tmp = [
                      {
                          "type": "folder",
                          "name": "folder0",
                          "children": [
                              {
                                  "type": "page",
                                  "name": "page0",
                                  "children": [
                                      {
                                          "id": "id0",
                                          "type": "image",

                                          "name": "image0",
                                          "position": [],
                                          "comment": "",

                                          "source": "" //url
                                      },
                                      {
                                          "id": "id1",
                                          "type": "text",

                                          "name": "text0",
                                          "position": [],
                                          "comment": "",
                                          "relative_position": [],

                                          "content": "",
                                          "size": 16,
                                          "color": "green",
                                          "bold": ""
                                      },
                                      {
                                          "id": "id2",
                                          "type": "shape",

                                          "name": "text0",
                                          "position": [],
                                          "comment": "",
                                          "relative_position": [],

                                          "direction": {
                                              "color": "green",
                                              "border": {
                                                  "type": "line",
                                                  "color": "red"
                                              },
                                              "radius": 30
                                          }
                                      }
                                  ]
                              },
                              {
                                  "type": "folder",
                                  "name": "folder0",
                                  "children": [
                                      {
                                          "id": "id3",
                                          "type": "text",
                                          "name": "text0_0",
                                          "range": [0, 0, 50, 50],
                                          "content": ""
                                      },
                                  ]
                              }
                          ]
                      }
                  ]
          //buildTree(objModel, tmp)

//          objModel.append({"name":"Zero","level":0,"subNode":[]})
//          objModel.append({"name":"One","level":0,"subNode":[]})
//          objModel.append({"name":"Two","level":0,"subNode":[]})
//          objModel.get(1).subNode.append({"name":"Three","level":1,"subNode":[]})
//          objModel.get(1).subNode.append({"name":"Four","level":1,"subNode":[]})
//          objModel.get(1).subNode.get(0).subNode.append({"name":"Five","level":2,"subNode":[]})
      }
   }

   //Delegate
   Component {
         id: objRecursiveDelegate
         Column {
            id: objRecursiveColumn
            MouseArea {
               width: objRow.implicitWidth
               height: objRow.implicitHeight
               onDoubleClicked: {
                  for(var i = 1; i < parent.children.length - 1; ++i) {
                     parent.children[i].visible = !parent.children[i].visible
                  }
               }
               onClicked: {
                   sel = idd
                   Pipeline2.run(tree_name + "_treeViewSelected", "", {tag: "manual"})
               }
               Row {
                  id: objRow
                  Item {
                     height: 1
                     width: model.level * 20
                  }
                  Text {
                     id: txt
                     color: sel === idd ? "blue" : "black"
                     text: (objRecursiveColumn.children.length > 2 ?
                              objRecursiveColumn.children[1].visible ?
                              qsTr("-  ") : qsTr("+ ") : qsTr("   ")) + model.name + "(" + type + ")"
                    // color: objRecursiveColumn.children.length > 2 ? "blue" : "green"
                  }
               }
            }
            Repeater {
               model: subNode
               delegate: objRecursiveDelegate
            }
         }
      }

   //View
      ListView {
         anchors.fill: parent
         model: objModel
         delegate: objRecursiveDelegate
      }

      Component.onCompleted: {
          Pipeline2.add(function(aInput){
              return {data: sel, out: {}}
          }, {name: tree_name + "_treeViewSelected", type: "Partial", vtype: ""})

          Pipeline2.add(function(aInput){
              if (aInput["select"] !== undefined)
                  sel = aInput["select"]
              buildTree(objModel, [aInput["data"]])
              return {out: {}}
          }, {name: tree_name + "_updateTreeView"})
      }
}
