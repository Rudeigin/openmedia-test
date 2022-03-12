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

    // Рекурсивная функция подсчёта количества всех файлов в папке
    int getFilesCount(const QString &dirPath);
    QList<QStringList> recoursiveFindDuplicate(const QString &dirPath);
    // Рекурсивная функция. Передаём сразу хэш файла, дубликаты которого ищем в директории dirPath,
    // чтобы не вычислять его каждый раз при поиске во вложенных папках
    QStringList findDuplicateByHash(const QString &hash, const int &fileSize, const QString &dirPath);
    QString getHashMd5(const QString &filePath);
};
#endif // FILEMANAGER_H
