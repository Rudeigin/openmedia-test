#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QVariantList>

class FileManager : public QObject {
    Q_OBJECT
public:
    explicit FileManager(QObject * parent = nullptr);

    Q_INVOKABLE void findDuplicateFiles(const QString &firstDirPath, const QString &secondDirPath);

private slots:
    void findDuplicates();

signals:
    void processedCountChanged(const int &processedCount);
    void duplicateSearchCompleted(QString result, QList<QStringList> duplicateFiles = QList<QStringList>());

private:
    QString m_sourceDirPath;
    QString m_comparedDirPath;
    int m_processedFileCount;

    int getFilesCount(const QString &dirPath);
    QString getHashMd5(const QString &filePath);
};
#endif // FILEMANAGER_H
