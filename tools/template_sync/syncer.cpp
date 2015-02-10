#include <QtCore>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <iostream>

#include "syncer.h"

void Syncer::syncTemplates()
{
    qDebug() << ">>> Reading source template <<<";
    QFile f(sourceTemplate);
    if ( f.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        QTextStream ts(&f);
        QString section, option, description;
        int startIndex, endIndex;
        while ( !ts.atEnd() ) {
            QString line = ts.readLine().trimmed();
            if ( line.startsWith("<section ") ) {
                startIndex = line.indexOf("name=\"") + 6;
                endIndex = line.indexOf("\"", startIndex);
                section = line.mid(startIndex, endIndex - startIndex);
                option.clear();
                description.clear();
            } else if ( line.startsWith("<option ") ) {
                startIndex = line.indexOf("name=\"") + 6;
                endIndex = line.indexOf("\"", startIndex);
                option = line.mid(startIndex, endIndex - startIndex);
                section.clear();
                description.clear();
            } else if ( line.startsWith(QString("<description lang=\"%1\"").arg(language)) ) {
                startIndex = line.indexOf("text=\"") + 6;
                endIndex = line.indexOf("\"", startIndex);
                description = line.mid(startIndex, endIndex - startIndex);
            }
            if ( !description.isEmpty() ) {
                if ( !section.isEmpty() )
                    sectionDescriptions[section] = description;
                else if ( !option.isEmpty())
                    optionDescriptions[option] = description;
            }
        }
        f.close();
        qDebug() << ">>> Done (reading source template) <<<\n";
        qDebug() << ">>> Source section descriptions <<<";
        QMapIterator<QString, QString> itSections(sectionDescriptions);
        while ( itSections.hasNext() ) {
            itSections.next();
            qDebug() << itSections.key() << "=>" << itSections.value();
        }
        qDebug() << ">>> Source option descriptions <<<";
        QMapIterator<QString, QString> itOptions(optionDescriptions);
        while ( itOptions.hasNext() ) {
            itOptions.next();
            qDebug() << itOptions.key() << "=>" << itOptions.value();
        }

        qDebug() << "\n>>> Reading target template <<<";
        f.setFileName(targetTemplate);
        if ( f.open(QIODevice::ReadOnly	| QIODevice::Text) ) {
            ts.setDevice(&f);
            section.clear();
            option.clear();
            description.clear();
            while ( !ts.atEnd() ) {
                QString line = ts.readLine().trimmed();
                if ( line.startsWith("<section ") ) {
                    startIndex = line.indexOf("name=\"") + 6;
                    endIndex = line.indexOf("\"", startIndex);
                    section = line.mid(startIndex, endIndex - startIndex);
                    option.clear();
                    description.clear();
                } else if ( line.startsWith("<option ") ) {
                    startIndex = line.indexOf("name=\"") + 6;
                    endIndex = line.indexOf("\"", startIndex);
                    option = line.mid(startIndex, endIndex - startIndex);
                    section.clear();
                    description.clear();
                } else if ( line.startsWith(QString("<description lang=\"%1\"").arg(language)) ) {
                    startIndex = line.indexOf("text=\"") + 6;
                    endIndex = line.indexOf("\"", startIndex);
                    description = line.mid(startIndex, endIndex - startIndex);
                }
                if ( !description.isEmpty() ) {
                    if ( !section.isEmpty() )
                        targetSectionDescriptions[section] = description;
                    else if ( !option.isEmpty())
                        targetOptionDescriptions[option] = description;
                }
            }
            qDebug() << ">>> Done (reading target template) <<<\n";
            qDebug() << ">>> Target section descriptions <<<";
            QMapIterator<QString, QString> itTgtSections(targetSectionDescriptions);
            while ( itTgtSections.hasNext() ) {
                itTgtSections.next();
                qDebug() << itTgtSections.key() << "=>" << itTgtSections.value();
            }
            qDebug() << ">>> Target option descriptions <<<";
            QMapIterator<QString, QString> itTgtOptions(targetOptionDescriptions);
            while ( itTgtOptions.hasNext() ) {
                itTgtOptions.next();
                qDebug() << itTgtOptions.key() << "=>" << itTgtOptions.value();
            }

            qDebug() << "\n>>> Merging source & target templates <<<";
            f.seek(0);
            section.clear();
            option.clear();
            description.clear();
            while ( !ts.atEnd() ) {
                QString lineCopy = ts.readLine();
                QString line = lineCopy.trimmed();
                if ( line.startsWith("<section ") ) {
                    startIndex = line.indexOf("name=\"") + 6;
                    endIndex = line.indexOf("\"", startIndex);
                    section = line.mid(startIndex, endIndex - startIndex);
                    option.clear();
                    std::cout << lineCopy.toLocal8Bit().constData() << "\n";
                } else if ( line.startsWith("<option ") ) {
                    startIndex = line.indexOf("name=\"") + 6;
                    endIndex = line.indexOf("\"", startIndex);
                    option = line.mid(startIndex, endIndex - startIndex);
                    section.clear();
                    std::cout << lineCopy.toLocal8Bit().constData() << "\n";
                }
                if ( !section.isEmpty() ) {
                    if ( targetSectionDescriptions[section].isEmpty() ) {
                        if ( !sectionDescriptions[section].isEmpty() )
                            std::cout << QString("\t\t<description lang=\"%1\" text=\"%2\"/>\n").arg(language).arg(sectionDescriptions[section]).toLocal8Bit().constData();
                    }
                    section.clear();
                } else if ( !option.isEmpty() ) {
                    if ( targetOptionDescriptions[option].isEmpty() ) {
                        if ( !optionDescriptions[option].isEmpty() )
                            std::cout << QString("\t\t\t<description lang=\"%1\" text=\"%2\"/>\n").arg(language).arg(optionDescriptions[option]).toLocal8Bit().constData();
                    }
                    option.clear();
                } else
                    std::cout << lineCopy.toLocal8Bit().constData() << "\n";
            }
            f.close();
            qDebug() << ">>> Done (merging source & target templates) <<<";
        } else
            qDebug() << ">>> Failed (reading target template) <<<";
    } else
        qDebug() << ">>> Failed (reading source template) <<<";

    qApp->quit();
}
