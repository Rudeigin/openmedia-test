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
    void findDuplicates(const QString &firstDirPath, const QString &secondDirPath);

signals:
    void duplicateSearchCompleted(QString result, QList<QStringList> duplicateFiles = QList<QStringList>());

private:
    QString getHashMd5(const QString &filePath);
    QMultiHash<QString, QString> getHashMap(const QString &dirPath);
};
#endif // FILEMANAGER_H
