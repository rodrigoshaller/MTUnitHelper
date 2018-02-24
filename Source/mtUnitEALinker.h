/**
 * @file mtUnitEALinker.h
 * @author Rodrigo Haller
 * @date 24/02/2018
 * @brief Update the config filed required by the MetaTerminal when running an Expert Advisor.
 * @details This class is used to update the config filed required by
 * the MetaTerminal when running an Expert Advisor. It looks for the
 * EA Parameter and updates it with the name of the generated .ex5 file.
 * @note In order to use this class, the argument received by mtUnitHelper.exe
 * must be: mtUnitEALinker PathOfTheEA.mq5File
 */

#ifndef MTUNITEALINKER_H
#define MTUNITEALINKER_H

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>

class MTUnitEALinker
{
public:
    MTUnitEALinker(){}
    ~MTUnitEALinker(){}

    int start(QString rootDir, QString eaPath);
};

#endif // MTUNITEALINKER_H
