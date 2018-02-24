/**
 * @file mtUnitEALinker.cpp
 * @author Rodrigo Haller
 * @date 24/02/2018
 * @brief Update the config filed required by the MetaTerminal when running an Expert Advisor.
 * @details This class is used to update the config filed required by
 * the MetaTerminal when running an Expert Advisor. It looks for the
 * EA Parameter and updates it with the name of the generated .ex5 file.
 * @note In order to use this class, the argument received by mtUnitHelper.exe
 * must be: mtUnitEALinker PathOfTheEA.mq5File
 */

#include "mtUnitEALinker.h"

/**
 * @brief Open the config file and edit the EA parameter
 * @param rootDir
 * @param eaPath
 * @return The result of the procedure. (-1 = failure, 1 = success)
 */
int MTUnitEALinker::start(QString rootDir, QString eaPath)
{
    QFile inputFile(rootDir + "/Runners/autoRunTest.ini"); //Config file that the MetaTerminal needs
    if(inputFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Generating config file to run " << eaPath;
        QTextStream in(&inputFile);
        QStringList content = in.readAll().split('\n');
        for(int i = 0; i < content.size(); i++)
        {
            QString line = content[i];
            if(line.startsWith("Expert="))
                content[i] = "Expert=" + QString(eaPath);
        }
        inputFile.close();

        QFile outputFile(rootDir + "/Runners/autoRunTest.ini"); //Overwrites it
        if(outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&outputFile);
            for(int i = 0; i < content.size(); i++)
            {
                out << content[i];
                if(i < content.size()-1)
                    out << endl;
            }
            outputFile.close();
        }
        else
        {
            qDebug() << "Error writing autoRunTest.ini";
            return -1;
        }
    }
    else
    {
        qDebug() << "autoRunTest.ini not found";
        return -1;
    }
    qDebug() << "autoRunTest.ini configured successfully!";
    return 1;
}
