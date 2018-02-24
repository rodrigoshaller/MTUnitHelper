/**
 * @file mtUnitTestsCompiler.h
 * @author Rodrigo Haller
 * @date 24/02/2018
 * @brief Scan all Test Files and generates the MTUnitAllTests.mqh file
 * @details This class scan the Test folder looking for all included
 * Test Suites and Test Cases. Then it writes a file called MTUnitAllTests.mqh
 * containing all basic declarations you would have had to write manually.
 * There is also a Watcher that monitors the Test folder and trigger the start
 * method whenever a change is made to that directory.
 * @note In order to use this class, the argument received by mtUnitHelper.exe
 * must be: mtUnitTestsCompiler
 * @note This class can also be executed in Watcher mode, to enter this mode,
 * do not pass any argument to myUnitHelper.exe
 * @warning In this case, the app will keep running until you close it.
 */

#ifndef MTUNITTESTSCOMPILER_H
#define MTUNITTESTSCOMPILER_H

#include <QObject>
#include <QMap>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QStringList>
#include <QTextStream>
#include <QFileSystemWatcher>
#include <QDebug>

class MTUnitTestsCompiler : public QObject
{
    Q_OBJECT
public:
    MTUnitTestsCompiler(){}
    ~MTUnitTestsCompiler(){}
    void initWatcher(QString rootDir);
    int start(QString rootDir);

public slots:
    void handleDirectoryChanged(QString dir);
    void handleFileChanged(QString file);

private:
    QFileSystemWatcher *m_watcher;

    QStringList findTestFiles(QString testFolderPath);
    QMap<QString, QStringList> testsExtractor(QStringList testFiles);
    QString clearComments(QString line, bool &commentMode);
    QString classChecker(QString line);
    QString testCaseChecker(QString line, bool &found);

    void printMapClassXTestCases(QMap<QString, QStringList> map);
    void writeHeader(QTextStream &outMTUnitFile);
    void writeMTUnitClass(QTextStream &outMTUnitFile);
    void writeEnd(QTextStream &outMTUnitFile);
    void writeIncludes(QTextStream &outMTUnitFile, QStringList testsFound);
    void writeRunAllTests(QTextStream &outMTUnitFile, QMap<QString, QStringList> mapClassXTestCases);
};

#endif // MTUNITTESTSCOMPILER_H
