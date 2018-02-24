/**
 * @file mtUnitLogger.cpp
 * @author Rodrigo Haller
 * @date 24/02/2018
 * @brief Hijacks the LogFile and add colors to the output.
 * @details This class looks for the output log file generated by MetaEditor,
 * hijacks this file to the Runners folder and adds some color to the output
 * to make it more visual effective.
 * @note In order to use this class, the argument received by mtUnitHelper.exe
 * must be: mtUnitLogger
 * @warning The colored output does not work directly from MetaEditor, so if
 * you want to use it, I suggest you to follow the instructions for using this
 * tool in Sublime Text 3.
 */

#include "mtUnitLogger.h"

/**
 * @brief Search for the log file, hijacks it and add colors
 * @param rootDir
 * @return The result of the procedure. (0 = failure, 1 = success)
 */
int MTUnitLogger::start(QString rootDir)
{
    QFile inputFile(rootDir + "/Runners/logFolderPath.ini");
    if(inputFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Catching output...";
        QTextStream in(&inputFile);
        QString logFilePath = in.readAll().trimmed();
        QFileInfo file(logFilePath);
        if(!file.exists())
        {
            qDebug() << "File: " << logFilePath << " does not exist!";
            return -1;
        }
        inputFile.close();

        QString logFileName = QDateTime::currentDateTime().toString("yyyyMMdd") + ".log"; //Name the file with today's date
        QFile logFile(logFilePath + "\\" + logFileName);
        if(logFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QFile newLogFile(rootDir + "/Runners/logFile.log");
            if(!newLogFile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                qDebug() << "Error: Could not write the logFile in: " << QString(rootDir + "/Runners/logFile.log");
                return -1;
            }
            QTextStream inLog(&logFile);
            QTextStream outLog(&newLogFile);
            while(!inLog.atEnd())
            {
                QString line = inLog.readLine();
                line.remove(0, 5); //Removes the first 5 useless characteres from the log...
                if(line.contains("OK")) //Add green color if the test pass
                {
                    line.prepend("\e[32m");
                    line.append("\e[0m");
                }
                if(line.contains("***FAIL***")) //Add red if it fails
                {
                    line.prepend("\e[31m");
                    line.append("\e[0m");
                }
                outLog << line << endl;
            }
            logFile.close();
            newLogFile.close();
            QFile::remove(logFilePath + "\\" + logFileName); //Remove the original log so the new one will be fresh
        }
        else
        {
            qDebug() << "Error: Log file not found: " << logFilePath + logFileName;
            return -1;
        }
    }
    else
    {
        qDebug() << "logFolderPath.ini not found";
        return -1;
    }
    qDebug() << "logFile.log generated successfully!";
    return 1;
}