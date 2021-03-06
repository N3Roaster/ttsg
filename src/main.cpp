// ttsg
// Typica .ts Generator
// This is a program that scrapes configuration files for Typica
// and generates a .ts file translators can use so that user
// visible text in a configuration can be translated into another
// language.
//
// This is very slapdash with a minimal feature set focusing on
// having something usable for initial translations starting with
// the Typica 1.7 release. There are certain valid Typica
// configurations that this will fail on. This is very much
// tailored to use with the currently provided example
// configuration. If you're using Typica for something strange
// this tool might need to be altered.
//
// Copyright 2016, Neal Evan Wilson
// See LICENSE for more details.

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QXmlStreamWriter>
#include <QtDebug>
#include <QHash>
#include <QHashIterator>

struct Context
{
    QString name;
    QString filename;
    QStringList messages;
    QStringList locations;
    QString reportDirectory;
};

struct Locations
{
    QStringList filenames;
    QStringList linenumbers;
};

QHash<QString, Locations*> globalContext;

void addMessage(QString source, QString file, int line);
Context* buildContext(QString filename, QString basedir);
int generate(QString configfile, QString transfile);
int main(int argc, char **argv);

// Add a message to the context.
void addMessage(QString source, QString file, int line)
{
    Locations *location;
    if(globalContext.contains(source))
    {
        location = globalContext.value(source);
    }
    else
    {
        location = new Locations;
    }
    location->filenames.append(file);
    location->linenumbers.append(QString("%1").arg(line));
    globalContext.insert(source, location);
}

// Create a Context* with messages from a file.
Context* buildContext(QString filename, QString basedir)
{
    Context *context = new Context;
    context->filename = filename;
    QFile source(QString("%1/%2").arg(basedir).arg(filename));
    if(source.open(QIODevice::ReadOnly))
    {
        QDomDocument document;
        document.setContent(&source, true);
        QDomElement root = document.documentElement();
        context->name = root.attribute("id");
        QDomNodeList currentSet = document.elementsByTagName("button");
        QDomNode currentNode;
        QDomElement currentElement;
        for(int i = 0; i < currentSet.length(); i++)
        {
            currentNode = currentSet.at(i);
            currentElement = currentNode.toElement();
            addMessage(currentElement.attribute("name"), filename, currentNode.lineNumber());
        }
        currentSet = document.elementsByTagName("reporttitle");
        for(int i = 0; i < currentSet.length(); i++)
        {
            currentNode = currentSet.at(i);
            currentElement = currentNode.toElement();
            addMessage(currentElement.text(), filename, currentNode.lineNumber());
        }
        currentSet = document.elementsByTagName("menu");
        for(int i = 0; i < currentSet.length(); i++)
        {
            currentNode = currentSet.at(i);
            currentElement = currentNode.toElement();
            addMessage(currentElement.attribute("name"), filename, currentNode.lineNumber());
            if(currentElement.attribute("type") == "reports")
            {
                context->reportDirectory = currentElement.attribute("src");
            }
        }
        currentSet = document.elementsByTagName("item");
        for(int i = 0; i < currentSet.length(); i++)
        {
            currentNode = currentSet.at(i);
            currentElement = currentNode.toElement();
            addMessage(currentElement.text(), filename, currentNode.lineNumber());
        }
        currentSet = document.elementsByTagName("label");
        for(int i = 0; i < currentSet.length(); i++)
        {
            currentNode = currentSet.at(i);
            currentElement = currentNode.toElement();
            addMessage(currentElement.text(), filename, currentNode.lineNumber());
        }
        currentSet = document.elementsByTagName("decoration");
        for(int i = 0; i < currentSet.length(); i++)
        {
            currentNode = currentSet.at(i);
            currentElement = currentNode.toElement();
            addMessage(currentElement.attribute("name"), filename, currentNode.lineNumber());
        }
        currentSet = document.elementsByTagName("spinbox");
        for(int i = 0; i < currentSet.length(); i++)
        {
            currentNode = currentSet.at(i);
            currentElement = currentNode.toElement();
            if(currentElement.hasAttribute("pretext"))
            {
                addMessage(currentElement.attribute("pretext"), filename, currentNode.lineNumber());
            }
            if(currentElement.hasAttribute("posttext"))
            {
                addMessage(currentElement.attribute("posttext"), filename, currentNode.lineNumber());
            }
        }
        currentSet = document.elementsByTagName("measurementtable");
        for(int i = 0; i < currentSet.length(); i++)
        {
            currentNode = currentSet.at(i);
            currentElement = currentNode.toElement();
            if(currentElement.hasChildNodes())
            {
                for(int j = 0; j < currentElement.childNodes().count(); j++)
                {
                    QDomNode childNode = currentElement.childNodes().at(j);
                    if(childNode.isElement())
                    {
                        QDomElement childElement = childNode.toElement();
                        if(childElement.tagName() == "column")
                        {
                            addMessage(childElement.text(), filename, childNode.lineNumber());
                        }
                    }
                }
            }
        }
        currentSet = document.elementsByTagName("sqltablearray");
        for(int i = 0; i < currentSet.length(); i++)
        {
            currentNode = currentSet.at(i);
            currentElement = currentNode.toElement();
            if(currentElement.hasChildNodes())
            {
                for(int j = 0; j < currentElement.childNodes().count(); j++)
                {
                    QDomNode childNode = currentElement.childNodes().at(j);
                    if(childNode.isElement())
                    {
                        QDomElement childElement = childNode.toElement();
                        if(childElement.tagName() == "column")
                        {
                            addMessage(childElement.attribute("name"), filename, childNode.lineNumber());
                        }
                    }
                }
            }
        }
        currentSet = document.elementsByTagName("program");
        for(int i = 0; i < currentSet.length(); i++)
        {
            currentNode = currentSet.at(i);
            currentElement = currentNode.toElement();
            QString script = currentElement.text();
            QStringList scriptLines = script.split("\n");
            for(int j = 0; j < scriptLines.length(); j++)
            {
                if(scriptLines.at(j).contains("TTR"))
                {
                    QString currentLine = scriptLines.at(j);
                    currentLine = currentLine.remove(0, currentLine.indexOf("TTR"));
                    currentLine = currentLine.remove(0, currentLine.indexOf("\"")+1);
                    currentLine = currentLine.remove(0, currentLine.indexOf("\"")+1);
                    currentLine = currentLine.remove(0, currentLine.indexOf("\"")+1);
                    currentLine = currentLine.remove(currentLine.indexOf("\""), currentLine.length());
                    addMessage(currentLine, filename, currentNode.lineNumber() + j + 1);
                }
            }
        }
    }
    source.close();
    return context;
}

// Produce a .ts file from a Typica configuration file.
int generate(QString configfile, QString transfile)
{
    QFile source(configfile);
    QFileInfo info(configfile);
    QDir directory = info.dir();
    if(!source.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open: " << configfile;
        return 1;
    }
    QDomDocument configuration;
    configuration.setContent(&source, true);
    QDomElement root = configuration.documentElement();
    QDomNodeList children = root.childNodes();
    QList<Context *> contexts;
    for(int i = 0; i < children.size(); i++)
    {
        QDomNode currentNode = children.at(i);
        QDomElement currentElement;
        if(currentNode.nodeName() == "include")
        {
            currentElement = currentNode.toElement();
            if(currentElement.hasAttribute("src"))
            {
                Context *context = buildContext(currentElement.attribute("src"), directory.path());
                if(context)
                {
                    contexts.append(context);
                    if(context->reportDirectory.length() > 0)
                    {
                        QDir reportsDirectory(QString("%1/%2").arg(directory.path()).arg(context->reportDirectory));
                        reportsDirectory.setFilter(QDir::Files);
                        reportsDirectory.setSorting(QDir::Name);
                        QStringList nameFilter;
                        nameFilter << "*.xml";
                        reportsDirectory.setNameFilters(nameFilter);
                        QStringList reportFiles = reportsDirectory.entryList();
                        for(int j = 0; j < reportFiles.size(); j++)
                        {
                            Context *reportContext = buildContext(QString("%1/%2").arg(context->reportDirectory).arg(reportFiles.at(j)), directory.path());
                            if(reportContext)
                            {
                                contexts.append(reportContext);
                            }
                        }
                    }
                }
            }
        }
    }
    source.close();
    QFile ts(transfile);
    if(!ts.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open output file: " << transfile;
        return 1;
    }
    QXmlStreamWriter xmlout(&ts);
    xmlout.setAutoFormatting(true);
    xmlout.writeStartDocument("1.0");
    xmlout.writeDTD("<!DOCTYPE TS>");
    xmlout.writeStartElement("TS");
    xmlout.writeAttribute("version", "2.0");
    QHashIterator<QString, Locations*> i(globalContext);
    xmlout.writeStartElement("context");
    xmlout.writeStartElement("name");
    xmlout.writeCharacters("configuration");
    xmlout.writeEndElement();
    while(i.hasNext())
    {
        i.next();
        xmlout.writeStartElement("message");
        xmlout.writeStartElement("source");
        xmlout.writeCharacters(i.key());
        xmlout.writeEndElement();
        for(int j = 0; j < i.value()->filenames.length(); j++)
        {
            xmlout.writeStartElement("location");
            xmlout.writeAttribute("filename", i.value()->filenames.at(j));
            xmlout.writeAttribute("line", i.value()->linenumbers.at(j));
            xmlout.writeEndElement();
        }
        // Uncomment to produce placeholder translations.
        /*
        xmlout.writeStartElement("translation");
        xmlout.writeCharacters("T" + i.key());
        xmlout.writeEndElement();
        */
        xmlout.writeEndElement();
    }
    xmlout.writeEndElement();
    xmlout.writeEndElement();
    xmlout.writeEndDocument();
    ts.close();
    return 0;
}

// Take the source and destination files from the command line arguments and
// pass them to generate.
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QStringList arguments = app.arguments();
    if(arguments.length() != 3) {
        qDebug() << "Usage: ttsg configuration.xml translation.ts";
        return 1;
    }
    QString configfile = arguments.at(1);
    QString transfile = arguments.at(2);
    return generate(configfile, transfile);
}
