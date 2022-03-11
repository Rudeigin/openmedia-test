import QtQuick 2.11
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

RowLayout {
    id: root
    property alias path: dirName.text
    TextField {
        id: dirName
        Layout.fillWidth: true
    }

    Button {
        text: qsTr("Выбрать папку")
        onClicked: {
            fileDialog.folder = dirName.text == "" ? fileDialog.shortcuts.home
                                                   : "file://" + dirName.text
            fileDialog.open()
        }

        FileDialog {
            id: fileDialog
            selectFolder: true
            onAccepted: {
                if(fileUrl !== "") {
                    dirName.text = String(fileUrl).replace("file://", "")
                }
            }
        }
    }
}
