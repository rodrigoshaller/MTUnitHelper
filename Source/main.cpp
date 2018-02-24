/**
 * @file main.cpp
 * @author Rodrigo Haller
 * @date 24/02/2018
 * @brief This app was made to automate Unit Testing in MQL5.
 * @details It contains 3 tools that make the wholw process totally automatic.
 * Each tool is better explained in the Readme and in their respective
 * classes.
 */

#include <QCoreApplication>
#include <QThread>
#include <QDebug>

#include "mtUnitEALinker.h"
#include "mtUnitLogger.h"
#include "mtUnitTestsCompiler.h"

//#define DEBUGGING

/**
 * @brief Main method that will parse the received arguments and call the correct handler
 * @param argc
 * @param argv
 * @return (-1 = failure, 1 = success)
 */
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QDir upDir = QDir::currentPath();
#ifndef DEBUGGING
    upDir.cdUp();
#endif

#ifdef DEBUGGING
//    QStringList argvv = QStringList() << "mtUnitEALinker" << "C:\\Program Files\\MetaTrader 5\\MQL5\\Experts\\MyUnitTestEA\\Experts\\MyUnitTestEA.mq5";
//    QStringList argvv = QStringList() << "mtLogger";
//    QStringList argvv = QStringList() << "mtTestsCompiler";
    QStringList argvv;
#else
    QStringList argvv;
    for(int i = 1; i < argc; i++)
        argvv.append(argv[i]);
#endif
    if(argvv.size() == 1 && argvv[0] == "mtUnitTestsCompiler")
    {
        MTUnitTestsCompiler *testsCompiler = new MTUnitTestsCompiler();
        return testsCompiler->start(upDir.path());
    }
    if(argvv.size() == 2 && argvv[0] == "mtUnitEALinker")
    {
        if(argvv[1].contains(".mqh"))
        {
            qDebug() << "Error: Cannot run Header Files (.mqh)";
            return -1;
        }
        if(argvv[1].contains("MQL5\\Experts\\")) //Not sure if it will come as back or forward slashes
            argvv[1].remove(0, argvv[1].indexOf("MQL5\\Experts\\") + 13);
        else if(argvv[1].contains("MQL5/Experts/")) //Just in case, we do it for both cases
            argvv[1].remove(0, argvv[1].indexOf("MQL5/Experts/") + 13);
        argvv[1].replace(".mq", ".ex");

        MTUnitEALinker *eaLinker = new MTUnitEALinker();
        return eaLinker->start(upDir.path(), argvv[1]);
    }
    if(argvv.size() == 1 && argvv[0] == "mtUnitLogger")
    {
        MTUnitLogger *logger = new MTUnitLogger();
        return logger->start(upDir.path());
    }
    if(argvv.isEmpty()) //Init Watcher
    {
        MTUnitTestsCompiler *testsCompiler = new MTUnitTestsCompiler();
        testsCompiler->initWatcher(upDir.path());
        while(1)
            QCoreApplication::processEvents();
    }
    return -1;
}
