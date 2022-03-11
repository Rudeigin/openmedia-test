#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QVariantList>

class FileManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(QList<QStringList> duplicateFiles READ duplicateFiles NOTIFY duplicateFilesChanged)
public:
    explicit FileManager(QObject * parent = nullptr);

    QList<QStringList> duplicateFiles() const;

    Q_INVOKABLE void findDuplicateFiles(const QString &firstDirPath, const QString &secondDirPath);

private slots:
    void findDuplicates();

signals:
    void processedCountChanged(const int &processedCount);
    void duplicateSearchCompleted(QString result);

    void duplicateFilesChanged(QList<QStringList> duplicateFiles);

private:
    QString m_sourceDirPath;
    QString m_comparedDirPath;
    QList<QStringList> m_duplicateFiles;
    int m_processedFileCount;

    QList<QStringList> recoursiveFindDuplicate(const QString &dirPath);
    QStringList findDuplicateByHash(const QString &hash, const int &fileSize, const QString &dirPath);
    QString getHashMd5(const QString &filePath);
};
#endif // FILEMANAGER_H
