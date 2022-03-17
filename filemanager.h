#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QVariantList>
#include <QFutureWatcher>

class FileManager : public QObject {
    Q_OBJECT
public:
    explicit FileManager(QObject * parent = nullptr);

    Q_INVOKABLE void findDuplicateFiles(const QString &firstDirPath, const QString &secondDirPath);

private slots:
     QList<QStringList> findDuplicates(const QString &firstDirPath, const QString &secondDirPath);
     void printResults();

signals:
    void duplicateSearchCompleted(QString result, QList<QStringList> duplicateFiles = QList<QStringList>());

private:
    QString getHashMd5(const QString &filePath);
    QByteArray getFirstBytes(const QString &filePath);

    QFutureWatcher<QList<QStringList>> m_watcher;
};
#endif // FILEMANAGER_H
