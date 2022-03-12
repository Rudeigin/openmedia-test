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
    QList<QStringList> duplicateFiles = recoursiveFindDuplicate(m_sourceDirPath);

    int count = 0;
    foreach (QVariant list, duplicateFiles)
        count += list.toStringList().count();
    emit duplicateSearchCompleted(tr("Поиск завершён. Найдено дубликатов: %1").arg(count), duplicateFiles);

    m_processedFileCount = 0;
    m_sourceDirPath = "";
    m_comparedDirPath = "";
}

int FileManager::getFilesCount(const QString &dirPath) {
    int count = 0;
    foreach(QString entry, QDir(dirPath).entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString entryPath = dirPath + QDir::separator() + entry;
        QFileInfo entryInf(entryPath);
        if(entryInf.isDir()) {
            count += getFilesCount(entryPath);
        }
        else {
            count++;
        }
    }
    return count;
}

QList<QStringList> FileManager::recoursiveFindDuplicate(const QString &dirPath) {
    QDir::Filters entryFilter = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
    QDir firstDir(dirPath);
    if(firstDir.isEmpty(entryFilter))
        return QList<QStringList>();

    QList<QStringList> result;
    foreach(QString entry, firstDir.entryList(entryFilter)) {
        QString entryPath = dirPath + QDir::separator() + entry;
        QFileInfo entryInf(entryPath);
        if(entryInf.isDir()) {
            QList<QStringList> duplicates = recoursiveFindDuplicate(entryPath);
            if(!duplicates.isEmpty())
                result.append(duplicates);
        }
        else {
            if(!entryInf.isReadable())
                continue;

            QStringList duplicates = findDuplicateByHash(getHashMd5(entryPath), entryInf.size(), m_comparedDirPath);
            if(!duplicates.isEmpty()) {
                duplicates.prepend(entryPath);
                result.append(duplicates);
            }

            m_processedFileCount++;
            emit processedCountChanged(m_processedFileCount);
        }
    }
    return result;
}

QStringList FileManager::findDuplicateByHash(const QString &hash, const int &fileSize, const QString &dirPath) {
    QDir::Filters entryFilter = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
    QDir dir(dirPath);
    if(dir.isEmpty(entryFilter))
        return QStringList();

    QStringList result;
    foreach(QString entry, dir.entryList(entryFilter)) {
        QString entryPath = dirPath + QDir::separator() + entry;
        QFileInfo entryInf(entryPath);
        if(!entryInf.isReadable())
            continue;

        if(entryInf.isDir()) {
            result.append(findDuplicateByHash(hash, fileSize, entryPath));
        }
        else {
            if(entryInf.size() != fileSize)
                continue;

            if(hash == getHashMd5(entryPath))
                result.append(entryPath);
        }
    }
    return result;
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
