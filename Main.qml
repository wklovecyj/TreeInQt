import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

Window {
    property var edges: []
    property var nodes: []
    property var sum: 0

    height: 600
    title: qsTr("Tree")
    visible: true
    width: 800

    Column {
        anchors.fill: parent
        spacing: 10

        Row {
            id: controlRow
            spacing: 10

            TextField {
                id: input
                placeholderText: "输入表达式"
                width: 200
            }

            Button {
                id: start
                text: "Start"

                background: Rectangle {
                    radius: 6
                    color: "red"
                }

                contentItem: Text {
                    text: start.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.fill: parent
                }

                onClicked: {
                    backend.createTree(input.text)
                    edges = backend.denseLine()
                    nodes = backend.denseNode(drawArea.width, drawArea.height)
                    sum = backend.sum()
                    Qt.callLater(edgeCanvas.requestPaint)
                }
            }
            Text {
                id: result
                text: "结果：" + sum
                color: "black"
                font.pointSize: 16
            }
            Text {
                id: nodeNum
                text: "节点数：" + nodes.length
                color: "black"
                font.pointSize: 16
            }
        }

        Item {
            id: drawArea
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: controlRow.bottom
            anchors.bottom: parent.bottom

            Canvas {
                id: edgeCanvas
                anchors.fill: parent

                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)

                    ctx.lineWidth = 2
                    ctx.strokeStyle = "skyblue"

                    var map = {}
                    for (var i = 0; i < nodes.length; i++) {
                        map[nodes[i].id] = nodes[i]
                    }

                    for (var k = 0; k < edges.length; k++) {
                        var e = edges[k]
                        var a = map[e.from]
                        var b = map[e.to]
                        if (!a || !b) continue

                        ctx.beginPath()
                        ctx.moveTo(a.x, a.y)
                        ctx.lineTo(b.x, b.y)
                        ctx.stroke()
                    }
                }
            }

            Repeater {
                model: nodes

                delegate: Rectangle { //TODO :这里的大小可以随着节点数以及窗口大小而动态调整
                    width: 50
                    height: 50
                    radius: 25
                    color: "green"
                    border.width: 2
                    border.color: "skyblue"
                    x: modelData.x - width / 2
                    y: modelData.y - height / 2

                    Text {
                        anchors.centerIn: parent
                        text: modelData.label
                        color: "white"
                    }
                }
            }
        }
    }
}
