/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQml.Models 2.2
import QtQuick 2.7
import QtQml 2.2

import "qrc:/qml/Industry/Manufacturing/Utils.js" as Utils

Item {
    property bool isOutput: false
    property bool isManufactured: isOutput || source === 1 || source === 5

    readonly property int contentPadding: 10

    id: root
    width: 500
    height: childrenRect.height

    signal selected(int typeId)

    states: [
        State {
            PropertyChanges {
                target: glow
                glowRadius: 0
            }
        },
        State {
            name: "selected"
            PropertyChanges {
                target: glow
                glowRadius: 10
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "*"
            NumberAnimation {
                properties: "glowRadius"
                target: glow
            }
        }
    ]

    function selectCurrentType() {
        root.selected(typeId);
    }

    function setSourceTypeCombo() {
        for (var i = 0; i < sourceCombo.model.count; ++i) {
            if (sourceCombo.model.get(i).value === source) {
                sourceCombo.currentIndex = i;
                break;
            }
        }
    }

    RectangularGlow {
        id: glow
        anchors.fill: parent
        glowRadius: 0
        color: "#ffaa00"
    }

    Rectangle {
        id: header
        border.width: 1
        border.color: "black"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: typeName.height + typeName.anchors.margins * 2
        gradient: Gradient {
            GradientStop { position: 0; color: "#658ead" }
            GradientStop { position: 1; color: "#353535" }
        }

        Label {
            id: typeName
            font.bold: true
            text: name
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.margins: 5
            color: "white"
        }

        MouseArea {
            anchors.fill: parent
            onClicked: selectCurrentType()
        }
    }

    Rectangle {
        id: content
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: Math.max(icon.height, contentLayout.height) + contentPadding * 2
        color: "#171916"

        Image {
            id: icon
            source: "https://image.eveonline.com/Type/" + typeId + "_64.png"
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onClicked: {
                if (mouse.button == Qt.LeftButton) {
                    selectCurrentType();
                } else if (mouse.button == Qt.RightButton) {
                    contextMenu.x = mouse.x;
                    contextMenu.y = mouse.y;

                    contextMenu.open();
                }
            }
        }

        RowLayout {
            id: contentLayout
            anchors.topMargin: contentPadding
            anchors.left: icon.right
            anchors.top: parent.top
            anchors.right: parent.right

            GridLayout {
                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                columns: 2

                Label {
                    visible: !isOutput
                    id: sourceText
                    text: qsTr("Source:")
                    color: "#cccccc"
                }

                ComboBox {
                    visible: !isOutput
                    id: sourceCombo
                    textRole: "text"
                    Layout.fillWidth: true

                    // note: sync text with Utils.js or use getSourceName() when ListElement starts accepting scripts
                    model: ListModel {
                        ListElement { text: qsTr("Buy from source"); value: 0; }
                        ListElement { text: qsTr("Acquire for free"); value: 2; }
                        ListElement { text: qsTr("Buy at custom cost"); value: 3; }
                        ListElement { text: qsTr("Take assets then buy from source"); value: 4; }
                        ListElement { text: qsTr("Take assets then buy at custom cost"); value: 6; }
                    }

                    Component.onCompleted: {
                        if (isOutput)
                            return;

                        if (quantityProduced > 0) {
                            model.append({ text: qsTr("Manufacture"), value: 1 });
                            model.append({ text: qsTr("Take assets then manufacture"), value: 5 });
                        }

                        setSourceTypeCombo();

                        // avoid initial signal emit
                        currentIndexChanged.connect(function() {
                            if (!isOutput)
                                setupController.setSource(typeId, model.get(currentIndex).value);
                        });
                    }

                    Connections {
                        target: setupController
                        enabled: !isOutput

                        onSourceChanged: {
                            if (id == typeId)
                                setSourceTypeCombo();
                        }
                    }
                }

                Label {
                    text: qsTr("Runs:")
                    color: "#cccccc"
                    visible: isOutput
                }

                SpinBox {
                    value: runsEdit
                    to: 1000000000
                    editable: true
                    visible: isOutput

                    Component.onCompleted: {
                        // avoid initial signal emit
                        valueChanged.connect(function() {
                            runs = value;
                        });
                    }

                }

                Label {
                    text: qsTr("ME:")
                    color: "#cccccc"
                }

                RowLayout {
                    SpinBox {
                        id: me
                        value: materialEfficiencyEdit
                        to: 10
                        editable: true
                        enabled: isManufactured

                        Component.onCompleted: {
                            // avoid initial signal emit
                            valueChanged.connect(function() {
                                materialEfficiency = value;
                            });
                        }
                    }

                    Label {
                        text: qsTr("TE:")
                        color: "#cccccc"
                    }

                    SpinBox {
                        id: te
                        value: timeEfficiencyEdit
                        to: 20
                        editable: true
                        enabled: isManufactured

                        Component.onCompleted: {
                            // avoid initial signal emit
                            valueChanged.connect(function() {
                                timeEfficiency = value;
                            });
                        }
                    }
                }

                Label {
                    id: quantityText
                    text: (isOutput) ?
                          (qsTr("Quantity produced: %L1 / %2  Total: %L3").arg(quantityProduced).arg(Utils.formatDuration(time)).arg(quantityProduced * runs)) :
                          ((quantityProduced > 0) ? (qsTr("Quantity produced: %L1 / %2").arg(quantityProduced).arg(Utils.formatDuration(time))) : (qsTr("Quantity produced: N/A")))
                    color: "#cccccc"
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                }

                Label {
                    font.bold: true
                    text: (isManufactured) ? (qsTr("Total production time: %1").arg(Utils.formatDuration(totalTime))) : (qsTr("Total production time: N/A"))
                    color: "orange"
                    Layout.columnSpan: 2
                }

                Item {
                    width: childrenRect.width
                    height: childrenRect.height
                    Layout.columnSpan: 2

                    Label {
                        id: totalCosts
                        font.bold: true
                        text: qsTr("Total cost: %1").arg(Utils.formatCurrency(cost.totalCost))
                        color: "orange"
                    }

                    Image {
                        id: totalCostsInfo
                        anchors.left: totalCosts.right
                        anchors.verticalCenter: totalCosts.verticalCenter
                        visible: isManufactured
                        source: "qrc:///images/information.png"
                        asynchronous: true

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.WhatsThisCursor

                            ToolTip.delay: 500
                            ToolTip.visible: containsMouse
                            ToolTip.text: qsTr("Component cost: %1\nJob fee: %2\nJob tax: %3")
                                              .arg(Utils.formatCurrency(cost.children))
                                              .arg(Utils.formatCurrency(cost.jobFee))
                                              .arg(Utils.formatCurrency(cost.jobTax))
                        }
                    }

                    Image {
                        anchors.left: totalCostsInfo.right
                        anchors.verticalCenter: totalCostsInfo.verticalCenter
                        visible: !cost.totalVolumeBought
                        source: "qrc:///images/error.png"
                        asynchronous: true

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.WhatsThisCursor

                            ToolTip.visible: containsMouse
                            ToolTip.text: qsTr("All required volume is not available on the market. Assuming the rest will be bought at the best price.")
                        }
                    }
                }

                Item {
                    width: childrenRect.width
                    height: childrenRect.height
                    Layout.columnSpan: 2

                    Label {
                        id: profitInfo
                        font.bold: true
                        text: qsTr("Profit from selling: %1").arg(Utils.formatCurrency(profit.value - cost.totalCost))
                        color: ((profit.value - cost.totalCost) <= 0) ? ("red") : ("green")
                        Layout.columnSpan: 2
                    }

                    Image {
                        anchors.left: profitInfo.right
                        anchors.verticalCenter: profitInfo.verticalCenter
                        visible: !profit.totalVolumeSold
                        source: "qrc:///images/error.png"
                        asynchronous: true

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.WhatsThisCursor

                            ToolTip.visible: containsMouse
                            ToolTip.text: qsTr("All volume could not be sold on the market. Assuming the rest will be sold at the best price.")
                        }
                    }
                }

                Label {
                    font.bold: true
                    text: qsTr("ISK/h: %1").arg((totalTime > 0) ? (Utils.formatCurrency((profit.value - cost.totalCost) * 3600 / totalTime)) : ("N/A"))
                    color: ((profit.value - cost.totalCost) <= 0) ? ("red") : ("green")
                    Layout.columnSpan: 2
                }
            }

            Label {
                visible: !isOutput
                text: Number(quantityRequired).toLocaleString(Qt.locale(), "f", 0)
                color: "#cccccc"
                font.bold: true
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            }
        }
    }

    Menu {
        id: contextMenu

        MenuItem {
            text: qsTr("Show in EVE")
            onTriggered: setupController.showInEve(typeId)
        }
        MenuItem {
            text: qsTr("Show in Market Browser")
            onTriggered: setupController.showExternalOrders(typeId)
        }
        MenuItem {
            text: qsTr("Lookup item on eve-marketdata.com")
            onTriggered: setupController.lookupOnEveMarketdata(typeId)
        }
        MenuItem {
            text: qsTr("Lookup item on evemarketer.com")
            onTriggered: setupController.lookupOnEveMarketer(typeId)
        }
    }

    Connections {
        target: setupController
        onTypeSelected: {
            if (id == typeId)
                root.state = "selected";
            else
                root.state = "";
        }
    }

    Component.onCompleted: {
        root.selected.connect(setupController.typeSelected);
    }
}
