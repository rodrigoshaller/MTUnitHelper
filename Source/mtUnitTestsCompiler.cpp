/**
 * @file mtUnitTestsCompiler.cpp
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

#include "mtUnitTestsCompiler.h"

/**
 * @brief Keeps an eye on the Test directory and whenever a change is found
 * it will automatically generates a new MTUnitAllTests.mqh file
 * @param rootDir
 */
void MTUnitTestsCompiler::initWatcher(QString rootDir)
{
    m_watcher = new QFileSystemWatcher();
    m_watcher->addPath(rootDir + "/Test");
    foreach(QString dir, m_watcher->directories())
    {
        qDebug() << "Watching Directory: " << dir;
        foreach(QString testFile, findTestFiles(dir))
            m_watcher->addPath(dir + "/" + testFile);

    }
    QObject::connect(m_watcher, SIGNAL(directoryChanged(QString)), this, SLOT(handleDirectoryChanged(QString)));
    QObject::connect(m_watcher, SIGNAL(fileChanged(QString)), this, SLOT(handleFileChanged(QString)));
}

/**
 * @brief Handles inclusions or exclusions of files in the Test dir
 * @param dir
 */
void MTUnitTestsCompiler::handleDirectoryChanged(QString dir)
{
    Q_UNUSED(dir)
    QDir rootDir = QDir::currentPath();
    rootDir.cdUp();
    foreach(QString testFile, findTestFiles(dir))
        m_watcher->addPath(dir + "/" + testFile);
    start(rootDir.path());
}

/**
 * @brief Handle content changes in files
 * @param file
 */
void MTUnitTestsCompiler::handleFileChanged(QString file)
{
    QFileInfo fileChecker(file);
    if(!fileChecker.exists())
        return;
    QDir rootDir = QDir::currentPath();
    rootDir.cdUp();
    start(rootDir.path());
}

/**
 * @brief Init the tests compilation and outputs a file called MTUnitAllTests.mqh
 * @param rootDir
 * @return The result of the procedure. (-1 = failure, 1 = success)
 */
int MTUnitTestsCompiler::start(QString rootDir)
{

    QFile mtUnitAllTestsFile(rootDir + "/Include/MTUnitAllTests.mqh");
    if(mtUnitAllTestsFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Creating MTUnitAllTests.mqh file...";
        QTextStream outMTUnitFile(&mtUnitAllTestsFile);

//Writing the Header
        writeHeader(outMTUnitFile);

//Logic to #include the test files...
        QDir testsDir(rootDir + "/Test");
        QStringList testsFound = findTestFiles(testsDir.path());
        writeIncludes(outMTUnitFile, testsFound);

//Creating the MTUnitAllTests class...
        writeMTUnitClass(outMTUnitFile);

//Methods extraction...
        QStringList testFiles;
        foreach(QString testFileName, testsFound)
            testFiles.append(testsDir.path() + "/" + testFileName);
        QMap<QString, QStringList> mapClassXTestCases = testsExtractor(testFiles);
        printMapClassXTestCases(mapClassXTestCases);

//Write RunAllTests method
        writeRunAllTests(outMTUnitFile, mapClassXTestCases);

//Finishing up...
        writeEnd(outMTUnitFile);
        mtUnitAllTestsFile.close();
    }
    else
    {
        qDebug() << "Error creating MTUnitAllTests.mqh file.";
        return -1;
    }
    qDebug() << "MTUnitAllTests.mqh generated successfully!";
    return 1;
}

/**
 * @brief Find all test files inside the Test folder
 * @param testFolderPath
 * @return The list of test files
 */
QStringList MTUnitTestsCompiler::findTestFiles(QString testFolderPath)
{
    QDir testFolder(testFolderPath);
    testFolder.setNameFilters(QStringList() << "*.mqh");
    QStringList testsFound = testFolder.entryList();
    return testsFound;
}

/**
 * @brief Reads the test file looking for test cases
 * @param testFiles
 * @return A map of Test Suites and their respective Test Cases
 */
QMap<QString, QStringList> MTUnitTestsCompiler::testsExtractor(QStringList testFiles)
{
    QMap<QString, QStringList> mapClassXTestCases;
    foreach(QString testFile, testFiles)
    {
        QFile foundMTUnitFile(testFile);
        if(foundMTUnitFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
//            qDebug() << "Extracting test cases...";
            QTextStream inUTFile(&foundMTUnitFile);
            while(!inUTFile.atEnd())
            {
                QString line = inUTFile.readLine();

                static bool commentMode = false;
                line = clearComments(line, commentMode);
                if(commentMode)
                    continue;
                //After here, the line should be clear of any kind of comments... just as the compiler sees it
//                    qDebug() << "Line: " << line;

                QString currentClass = classChecker(line);
                bool foundTestCase = false;
                QString testCaseName = testCaseChecker(line, foundTestCase);
                if(foundTestCase)
                {
                    mapClassXTestCases[currentClass].append(testCaseName);
//                    qDebug() << "Extracted Test Case: " << currentClass << "::" << testCaseName;
                }
            }
            foreach(QStringList testCases, mapClassXTestCases)
                testCases.removeDuplicates(); //In case of double inclusions
            foundMTUnitFile.close();
        }
    }
    mapClassXTestCases.remove("NoClass"); //All methods within "NoClass" are not valid
    return mapClassXTestCases;
}

/**
 * @brief Remove comment lines
 * @param line
 * @param commentMode
 * @return The output should be what the compiler sees
 */
QString MTUnitTestsCompiler::clearComments(QString line, bool &commentMode)
{
    if(line.contains("//"))
    {
        int commentStartPos = line.indexOf("//");
        line.remove(commentStartPos, line.size() - commentStartPos);
    }
    if(line.contains("/*") && !commentMode)
    {
        commentMode = true;
        int commentStartPos = line.indexOf("/*");
        int commentEndPos = line.size() - commentStartPos;
        if(line.contains("*/"))
        {
            commentMode = false;
            commentEndPos = line.indexOf("*/")+2;
        }
        line.remove(commentStartPos, commentEndPos - commentStartPos);
    }
    if(commentMode && line.contains("*/"))
    {
        commentMode = false;
        int commentEndPos = line.indexOf("*/");
        line.remove(0, commentEndPos + 2);
    }
    return line;
}

/**
 * @brief Checks wheter the line represents a begin of a class
 * @param line
 * @return NoClass or "ClassName"
 */
QString MTUnitTestsCompiler::classChecker(QString line)
{
    if(line.trimmed().isEmpty())
        return "NoClass";
    static int classBracketCloseContext = 0;
    static QString className = "NoClass";

    if(line.trimmed().startsWith("class", Qt::CaseSensitive)) //A class was found
    {
        int classPos = line.indexOf("class") + 5;
        if(line.contains(":")) //Removes the inheritance
        {
            int inheritanceStartPos = line.indexOf(":");
            line.remove(inheritanceStartPos, line.size() - inheritanceStartPos);
        }
        className = line.mid(classPos, line.size() - classPos).trimmed();
        if(className.contains(" "))
        {
            qDebug() << "Error: Possible Class name contains spaces: " << className;
            return "NoClass";
        }
    }
    else
    {
        if(line.contains("void") && line.contains("::") && line.contains("()")) //Outside class declaration
        {
            int classPos = line.indexOf("void ") + 5;
            className = line.mid(classPos, line.indexOf("::") - classPos).trimmed();
        }
    }
    //We might be inside the class,
    //so we need to check the brackets openings and closings..
    if(line.contains("{"))
        classBracketCloseContext++;
    if(line.contains("}"))
        classBracketCloseContext--;

    if(!line.contains("class", Qt::CaseSensitive) && !line.contains("::") && classBracketCloseContext == 0)
        className = "NoClass"; //We are out of the class

    return className;
}

/**
 * @brief Check if the line represents a Test Case
 * @param line
 * @param found
 * @return The name of the Test Case found
 */
QString MTUnitTestsCompiler::testCaseChecker(QString line, bool &found)
{
    found = false;

    if(!line.contains("void")) //All tests return void...
        return "";
    if(!line.contains("(") || !line.contains(")")) //The method should have "()"
        return "";
    int startPos = line.indexOf("void ") + 5;
    if(line.contains("::"))
        startPos = line.indexOf("::") + 2;
    int initParenthesisPos = line.indexOf("(");
    QString testCaseName = line.mid(startPos, initParenthesisPos - startPos).trimmed();

    if(testCaseName != "setUp" && testCaseName != "tearDown")
        found = true;
    return testCaseName;
}

/**
 * @brief Output the list of Test Suites and Test Cases found
 * @param map
 */
void MTUnitTestsCompiler::printMapClassXTestCases(QMap<QString, QStringList> map)
{
    qDebug() << "Test Suites and Test Cases found: " << endl;
    for(QMap<QString, QStringList>::iterator it = map.begin(); it != map.end(); it++)
    {
        qDebug() << "MTUnit: " << it.key();
        foreach(QString testCase, it.value())
            qDebug() << "\tTestCase: " << testCase;
    }
}

/**
 * @brief Write the method RunAllTests() of the MTUnitAllTests file
 * @param outMTUnitFile
 * @param mapClassXTestCases
 */
void MTUnitTestsCompiler::writeRunAllTests(QTextStream &outMTUnitFile, QMap<QString, QStringList> mapClassXTestCases)
{
    outMTUnitFile << "    void runAllTests()\n    {\n";
    if(!mapClassXTestCases.isEmpty())
        outMTUnitFile << "        g_mtUnit.initTests();\n";
    for(QMap<QString, QStringList>::iterator it = mapClassXTestCases.begin(); it != mapClassXTestCases.end(); it++)
    {
        outMTUnitFile << "\n        //Auto generated tests for " << it.key() << " Class:\n";
        QString objName = it.key();
        objName[0] = objName[0].toLower();
        outMTUnitFile << "        " << it.key() << "* " << objName << " = new " << it.key() << "();\n";
        outMTUnitFile << "        g_mtUnit.initTestSuite(\"" << it.key() << "\");\n";
        foreach(QString testCase, it.value())
        {
            outMTUnitFile << "        g_mtUnit.initTestCase(\"" << testCase << "\"); " << objName << ".setUp(); " <<
                               objName << "." << testCase << "(); " << objName << ".tearDown(); " << "g_mtUnit.endTestCase();\n";
        }
        outMTUnitFile << "        g_mtUnit.endTestSuite();\n";
        outMTUnitFile << "        delete " << objName << ";\n";
    }
    if(!mapClassXTestCases.isEmpty())
        outMTUnitFile << "        g_mtUnit.endTests();\n\n";
    outMTUnitFile << "    }\n";
}

/**
 * @brief Write the header of the MTUnitAllTests file
 * @param outMTUnitFile
 */
void MTUnitTestsCompiler::writeHeader(QTextStream &outMTUnitFile)
{
    outMTUnitFile << \
"/**\n\
* @file MTUnitAllTests.mqh\n\
* @author Rodrigo Haller\n\
* @date " << QDateTime::currentDateTime().toString("dd/MM/yyyy") << "\n\
* @brief This file is auto generated. It contains all tests that the\n\
* unit test will run.\n\
*/\n\
\n\
#property copyright \"Copyright © 2018, Rodrigo Haller\"\n\
#property link      \"https://www.linkedin.com/in/rodrigohaller/\"\n\
#property version   \"1.00\"\n\
#property strict\n\
\n\
#include \"../Include/MTUnit.mqh\"\n\
#include \"../Include/MTUnitCfg.mqh\"\n\
\n\
//Includes will be added here automatically (from Test folder)\n";
}

/**
* @brief Write the class snippet
* @param outMTUnitFile
*/
void MTUnitTestsCompiler::writeMTUnitClass(QTextStream &outMTUnitFile)
{
   outMTUnitFile << \
"\nclass MTUnitAllTests\n\
{\n\
public:\n\
    MTUnitAllTests() {}\n\
    ~MTUnitAllTests() {}\n";
}

/**
* @brief Write the include files of all Test Suites found
* @param outMTUnitFile
* @param testsFound
*/
void MTUnitTestsCompiler::writeIncludes(QTextStream &outMTUnitFile, QStringList testsFound)
{
    foreach(QString testName, testsFound)
        outMTUnitFile << "#include \"../Test/" << testName << "\"\n";
}

/**
* @brief Write the end of the MTUnitAllTests file
* @param outMTUnitFile
*/
void MTUnitTestsCompiler::writeEnd(QTextStream &outMTUnitFile)
{
    outMTUnitFile << "};\n";
    outMTUnitFile << "//This file is auto generated!";
}
