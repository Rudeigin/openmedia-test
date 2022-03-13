#include "filemanager.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QtConcurrent>

FileManager::FileManager(QObject *parent) : QObject(parent), m_processedFileCount(0) {
    qRegisterMetaType<QList<QStringList>>();
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
    int firstDirCount = getFilesCount(firstDirPath);
    if(firstDirCount == 0) {
        emit duplicateSearchCompleted(tr("В директории %1 не найдено файлов").arg(firstDirPath));
        return;
    }
    int secondDirCount = getFilesCount(secondDirPath);
    if(secondDirCount == 0) {
        emit duplicateSearchCompleted(tr("В директории %1 не найдено файлов").arg(secondDirPath));
        return;
    }

    // Для ускорения поиска используем в качестве папки, файлы которой сравниваем с файлами другой папки,
    // ту, в которой меньше файлов
    if(firstDirCount <= secondDirCount) {
        m_sourceDirPath = firstDirPath;
        m_comparedDirPath = secondDirPath;
    }
    else {
        m_sourceDirPath = secondDirPath;
        m_comparedDirPath = firstDirPath;
    }

    QtConcurrent::run(this, &FileManager::findDuplicates);
}

void FileManager::findDuplicates() {
    QList<QStringList> result;
    QDirIterator it(m_sourceDirPath, QStringList(),  QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString sourceFilePath = it.next();
        QFileInfo entryInf(sourceFilePath);
        if(!entryInf.isReadable())
            continue;

        QString hash = getHashMd5(sourceFilePath);
        QStringList duplicates;
        QDirIterator it(m_comparedDirPath, QStringList(),  QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString comparedFilePath = it.next();
            QFileInfo entryInf(comparedFilePath);
            if(!entryInf.isReadable())
                continue;

            if(entryInf.size() != entryInf.size())
                continue;

            if(hash == getHashMd5(comparedFilePath))
                duplicates.append(comparedFilePath);
        }
        if(!duplicates.isEmpty()) {
            duplicates.prepend(sourceFilePath);
            result.append(duplicates);
        }

        m_processedFileCount++;
        emit processedCountChanged(m_processedFileCount);
    }

    int count = 0;
    foreach (QStringList list, result)
        count += list.count();
    emit duplicateSearchCompleted(tr("Поиск завершён. Найдено дубликатов: %1").arg(count), result);

    m_processedFileCount = 0;
    m_sourceDirPath = "";
    m_comparedDirPath = "";
}

int FileManager::getFilesCount(const QString &dirPath) {
    int count = 0;
    QDirIterator it(dirPath, QStringList(),  QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        count++;
    }
    return count;
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
