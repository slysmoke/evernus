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
import QtQuick.Controls 2.2
import QtQuick.Controls 1.4 as OldControls
import QtQuick.Layouts 1.3
import QtQuick 2.7

import "qrc:/qml/Industry/Manufacturing"

import "qrc:/qml/Industry/Manufacturing/Utils.js" as Utils

Item {
    ScrollView {
        id: visualView
        anchors.fill: parent
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn

        ColumnLayout {
            Repeater {
                model: TypeDelegateModel {
                    model: setupModel
                    isOutput: true

                    Component.onCompleted: onCreated.connect(setupController.outputViewCreated)
                }
            }
        }
    }

    OldControls.TreeView {
        id: textView
        anchors.fill: parent
        visible: false

        OldControls.TableViewColumn {
            title: qsTr("Name")
            role: "name"
        }
        OldControls.TableViewColumn {
            title: qsTr("Quantity produced")
            role: "quantityProduced"
        }
        OldControls.TableViewColumn {
            title: qsTr("Quantity required")
            role: "quantityRequired"
        }
        OldControls.TableViewColumn {
            title: qsTr("Source")
            role: "source"
        }
        OldControls.TableViewColumn {
            title: qsTr("Time per run")
            role: "time"
        }
        OldControls.TableViewColumn {
            title: qsTr("Runs")
            role: "runs"
        }
        OldControls.TableViewColumn {
            title: qsTr("ME")
            role: "materialEfficiency"
        }
        OldControls.TableViewColumn {
            title: qsTr("TE")
            role: "timeEfficiency"
        }
        OldControls.TableViewColumn {
            title: qsTr("Total time")
            role: "totalTime"
        }
        OldControls.TableViewColumn {
            title: qsTr("Total cost")
            role: "cost"
            horizontalAlignment: Text.AlignRight
        }
        OldControls.TableViewColumn {
            title: qsTr("Component cost")
            role: "cost"
            horizontalAlignment: Text.AlignRight
        }
        OldControls.TableViewColumn {
            title: qsTr("Job fee")
            role: "cost"
            horizontalAlignment: Text.AlignRight
        }
        OldControls.TableViewColumn {
            title: qsTr("Job tax")
            role: "cost"
            horizontalAlignment: Text.AlignRight
        }
        OldControls.TableViewColumn {
            title: qsTr("Total profit")
            role: "profit"
            horizontalAlignment: Text.AlignRight
        }

        model: setupModel
        itemDelegate: Label {
            function getTextValue(value, column) {
                switch (column) {
                case 3:
                    return Utils.getSourceName(value);
                case 4:
                case 8:
                    return Utils.formatDuration(value);
                case 9:
                    return (typeof value.totalCost !== "undefined") ? (Utils.formatCurrency(value.totalCost)) : ("");
                case 10:
                    return (typeof value.children !== "undefined") ? (Utils.formatCurrency(value.children)) : ("");
                case 11:
                    return (typeof value.jobFee !== "undefined") ? (Utils.formatCurrency(value.jobFee)) : ("");
                case 12:
                    return (typeof value.jobTax !== "undefined") ? (Utils.formatCurrency(value.jobTax)) : ("");
                case 13:
                    return (typeof value.value !== "undefined") ? (Utils.formatCurrency(value.value)) : ("");
                default:
                    return value;
                }
            }

            color: styleData.textColor
            elide: styleData.elideMode
            horizontalAlignment: styleData.textAlignment
            text: getTextValue(styleData.value, styleData.column)
        }
    }

    Connections {
        target: setupController

        onToggleViewType: {
            visualView.visible = !visualView.visible;
            textView.visible = !textView.visible;
        }
    }
}
