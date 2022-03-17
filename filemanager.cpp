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

    QFuture<QList<QStringList>> future = QtConcurrent::run(this,
                                                           &FileManager::findDuplicates,
                                                           firstDirPath,
                                                           secondDirPath);
    m_watcher.setFuture(future);
}

QList<QStringList> FileManager::findDuplicates(const QString &firstDirPath, const QString &secondDirPath) {
    QDirIterator it(firstDirPath, QStringList(),  QDir::Files, QDirIterator::Subdirectories);
    QHash<int, QMultiHash<QByteArray, QString>> filesToCompare;
    while (it.hasNext()) {
        QString sourceFilePath = it.next();
        QFileInfo sourceInfo(sourceFilePath);
        if(!sourceInfo.isReadable())
            continue;

        // Если в 1 папке уже обработали файл с таким же размером, то просто добавляем текущий файл в список
        int sourceSize = sourceInfo.size();
        if(filesToCompare.contains(sourceSize)) {
            QByteArray data = getFirstBytes(sourceFilePath);
            if(!data.isEmpty()) {
                QMultiHash<QByteArray, QString> files = filesToCompare.value(sourceSize);
                files.insert(data, sourceFilePath);
                filesToCompare.insert(sourceSize, files);
            }
            continue;
        }

        QMultiHash<QByteArray, QString> files;
        QDirIterator secondIt(secondDirPath, QStringList(),  QDir::Files, QDirIterator::Subdirectories);
        while (secondIt.hasNext()) {
            QString filePath = secondIt.next();
            QFileInfo inf(filePath);
            if(!inf.isReadable())
                continue;

            if(inf.size() == sourceSize) {
                QByteArray data = getFirstBytes(filePath);
                if(!data.isEmpty())
                    files.insert(data, filePath);
            }
        }

        if(!files.isEmpty()) {
            QByteArray data = getFirstBytes(sourceFilePath);
            if(!data.isEmpty())
                files.insert(data, sourceFilePath);

            filesToCompare.insert(sourceSize, files);
        }
    }

    QList<QStringList> result;
    QHashIterator<int, QMultiHash<QByteArray, QString>> iter(filesToCompare);
    while (iter.hasNext()) {
        QMultiHash<QString, QString> duplicates;

        const QMultiHash<QByteArray, QString> &comparedFiles = iter.next().value();
        foreach (const QByteArray &key, comparedFiles.uniqueKeys()) {
            QStringList files = comparedFiles.values(key);
            if(files.count() > 1) {
                foreach (const QString &file, files) {
                    duplicates.insert(getHashMd5(file), file);
                }
            }
        }

        foreach (const QString &hash, duplicates.uniqueKeys()) {
            const QStringList &files = duplicates.values(hash);
            if(files.count() > 1) {
                result.append(files);
            }
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
    if (!file.open(QIODevice::ReadOnly))
        return QString();

    // Файлы могут быть очень большого размера, поэтому считываем данные блоками
    QCryptographicHash hash(QCryptographicHash::Md5);
    while (!file.atEnd()) {
        QByteArray data = file.read(8192);
        hash.addData(data);
    }
    return hash.result();
}

QByteArray FileManager::getFirstBytes(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return QByteArray();

    QByteArray data = file.read(10);
    file.close();

    return data;
}
