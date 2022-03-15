#include "filemanager.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QtConcurrent>
#include <QFuture>

FileManager::FileManager(QObject *parent) : QObject(parent) {
    qRegisterMetaType<QList<QStringList>>();
    connect(&m_watcher, SIGNAL(finished()), this, SLOT(printResults()));
}

void FileManager::findDuplicateFiles(const QString &firstDirPath, const QString &secondDirPath) {
    QFileInfo info(firstDirPath);
    if(!info.exists()) {
        emit duplicateSearchCompleted(tr("Директория %1 не существует").arg(firstDirPath));
        return;
    }
    info.setFile(secondDirPath);
    if(!info.exists()) {
        emit duplicateSearchCompleted(tr("Директория %1 не существует").arg(secondDirPath));
        return;
    }

    QFuture<QList<QStringList>> future = QtConcurrent::run(this, &FileManager::findDuplicates, firstDirPath, secondDirPath);
    m_watcher.setFuture(future);
}

 QList<QStringList> FileManager::findDuplicates(const QString &firstDirPath, const QString &secondDirPath) {
    QList<QStringList> result;
    QMultiHash<QString, QString> first = getHashMap(firstDirPath);
    QMultiHash<QString, QString> second = getHashMap(secondDirPath);
    foreach(QString hashSum, first.uniqueKeys()) {
        QStringList duplicates = second.values(hashSum);
        if(!duplicates.isEmpty()) {
            result.append(first.values(hashSum) + duplicates);
        }
    }
    return result;
 }

 void FileManager::printResults()  {
     QList<QStringList> result = m_watcher.result();
     int count = 0;
     foreach (QVariant list, result)
         count += list.toStringList().count();
     emit duplicateSearchCompleted(tr("Поиск завершён. Найдено дубликатов: %1").arg(count), result);
 }

QString FileManager::getHashMd5(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();

    // Файлы могут быть очень большого размера, поэтому считываем данные блоками
    QCryptographicHash hash(QCryptographicHash::Md5);
    while (!file.atEnd()) {
        QByteArray data = file.read(8192);
        hash.addData(data);
    }
    return hash.result();
}

QMultiHash<QString, QString> FileManager::getHashMap(const QString &dirPath) {
    QMultiHash<QString, QString> result;
    QDirIterator it(dirPath, QStringList(),  QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString sourceFilePath = it.next();
        result.insert(getHashMd5(sourceFilePath), sourceFilePath);
    }
    return result;
}
