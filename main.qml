import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Window {
    visible: true
    minimumWidth: 640
    minimumHeight: 480
    title: qsTr("Поиск одинаковых файлов")

    ColumnLayout {
        id: root
        anchors.fill: parent
        anchors.margins: 10

        Label {
            text: qsTr("Выберите папки:")
        }

        DirBrowser {
            id: firstDir
            Layout.fillWidth: true
        }

        DirBrowser {
            id: secondDir
            Layout.fillWidth: true
        }

        Button {
            id: start
            enabled: firstDir.path !== "" && secondDir.path !== "" &&
                     firstDir.path !== secondDir.path
            text: qsTr("Начать поиск")
            onClicked: {
                waitDialog.open()
                FileManager.findDuplicateFiles(firstDir.path, secondDir.path)
            }
        }

        Frame {
            Layout.fillWidth: true
            Layout.fillHeight: true
            ListView {
                id: equalFiles
                anchors.fill: parent
                spacing: 5
                clip: true
                delegate: Label {
                    width: equalFiles.width
                    text: (index+1) + ". " + equalFiles.createString(modelData)
                    elide: Text.ElideRight
                }

                function createString(list) {
                    var resultStr = ""
                    for(const fileName of list) {
                        resultStr = resultStr + fileName + "\t"
                    }
                    return resultStr
                }
            }
        }

        Connections {
            target: FileManager
            onDuplicateSearchCompleted: {
                waitDialog.resultText = result
                equalFiles.model = duplicateFiles
            }
        }
    }

    Dialog {
        id: waitDialog
        height: contentChildren.height
        width: contentChildren.width
        anchors.centerIn: parent
        title: qsTr("Выполняется поиск")
        modal: true
        closePolicy: Popup.NoAutoClose
        property string resultText

        Text {
            id: state
            text: waitDialog.resultText === "" ? qsTr("Выполняется поиск...") : waitDialog.resultText
            anchors.centerIn: parent
        }

        footer: DialogButtonBox {
            Button {
                text: qsTr("Закрыть")
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                enabled: waitDialog.resultText !== ""
            }
        }

        onRejected: {
            resultText = ""
        }
    }
}
