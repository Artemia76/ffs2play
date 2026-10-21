/****************************************************************************
**
** Copyright (C) 2026 FSFranceSimulateur team.
** Contact: https://github.com/ffs2/ffs2play
**
** FFS2Play is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** FFS2Play is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** The license is as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this software. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
****************************************************************************/

/****************************************************************************
** fgpropertylist.cpp is part of FF2Play project
**
** Reader of FlightGear PropertyList XML files (aircraft -set.xml)
****************************************************************************/

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>

#include "fgpropertylist.h"

///
/// \brief CFGPropertyList::read
/// \param pFile PropertyList file
/// \param pRoots FlightGear data folders (FG_ROOT) used to resolve the includes
/// \return false if the file is not a readable PropertyList
///
bool CFGPropertyList::read(const QString& pFile, const QStringList& pRoots)
{
    m_Props.clear();
    m_Aliases.clear();
    m_Roots = pRoots;
    return readFile(pFile, QString(), QString(), 0);
}

///
/// \brief CFGPropertyList::value
/// \param pPath property path without leading slash, ex "sim/model/path"
/// \return the last value read for this path, empty if none
///
QString CFGPropertyList::value(const QString& pPath) const
{
    const auto it = m_Props.constFind(pPath);
    if (it == m_Props.constEnd() || it->isEmpty()) return QString();
    return it->last();
}

///
/// \brief CFGPropertyList::values
/// \param pPath property path of a repeated element, ex "sim/tags/tag"
/// \return all values read for this path
///
QStringList CFGPropertyList::values(const QString& pPath) const
{
    return m_Props.value(pPath);
}

///
/// \brief CFGPropertyList::aliases
/// \return the alias attributes, by indexed property path without leading slash
/// ("sim/multiplay/generic/int[12]", index 0 not written) and target path without leading slash
///
QHash<QString, QString> CFGPropertyList::aliases() const
{
    return m_Aliases;
}

///
/// \brief CFGPropertyList::readFile
/// \param pFile
/// \param pPrefix property path where the content of the file is merged
/// \param pIndexedPrefix same path with the indexes of the elements
/// \param pDepth
/// \return
///
bool CFGPropertyList::readFile(const QString& pFile, const QString& pPrefix, const QString& pIndexedPrefix, int pDepth)
{
    if (pDepth > MaxIncludeDepth) return false;
    QFile File(pFile);
    if (!File.open(QIODevice::ReadOnly)) return false;
    QXmlStreamReader Xml(&File);
    if (!Xml.readNextStartElement() || Xml.name() != QLatin1String("PropertyList")) return false;
    readNode(Xml, pFile, pPrefix, pIndexedPrefix, pDepth);
    return !Xml.hasError();
}

///
/// \brief CFGPropertyList::readNode
/// Reads the element just started up to its end
/// \param pXml
/// \param pFile file being read, base of the relative includes
/// \param pPath property path of the element
/// \param pIndexedPath same path with the indexes : attribute n, else rank among the
/// elements of the same name (counted in each file)
/// \param pDepth
///
void CFGPropertyList::readNode(QXmlStreamReader& pXml, const QString& pFile, const QString& pPath, const QString& pIndexedPath, int pDepth)
{
    // The included content comes first : the element's own content overrides it
    const QString Include = pXml.attributes().value(QLatin1String("include")).toString();
    if (!Include.isEmpty())
    {
        const QString IncludeFile = resolveInclude(pFile, Include);
        if (!IncludeFile.isEmpty()) readFile(IncludeFile, pPath, pIndexedPath, pDepth + 1);
    }
    const QString Alias = pXml.attributes().value(QLatin1String("alias")).toString();
    if (!Alias.isEmpty() && !pIndexedPath.isEmpty())
    {
        QString Target = Alias;
        while (Target.startsWith('/')) Target.remove(0, 1);
        m_Aliases.insert(pIndexedPath, Target);
    }
    QString Text;
    bool HasChildren = false;
    QHash<QString, int> NextIndex;
    while (!pXml.atEnd())
    {
        pXml.readNext();
        if (pXml.isStartElement())
        {
            HasChildren = true;
            const QString Name = pXml.name().toString();
            bool Ok = false;
            int Index = pXml.attributes().value(QLatin1String("n")).toInt(&Ok);
            if (!Ok) Index = NextIndex.value(Name);
            NextIndex[Name] = Index + 1;
            const QString IndexedName = (Index > 0) ? QString("%1[%2]").arg(Name).arg(Index) : Name;
            readNode(pXml, pFile, pPath.isEmpty() ? Name : pPath + "/" + Name,
                     pIndexedPath.isEmpty() ? IndexedName : pIndexedPath + "/" + IndexedName, pDepth);
        }
        else if (pXml.isCharacters())
        {
            Text += pXml.text();
        }
        else if (pXml.isEndElement())
        {
            break;
        }
    }
    Text = Text.trimmed();
    if (!HasChildren && !pPath.isEmpty() && !Text.isEmpty()) m_Props[pPath].append(Text);
}

///
/// \brief CFGPropertyList::resolveInclude
/// FlightGear resolves an include against the folder of the including file, then
/// against FG_ROOT ("Aircraft/Generic/..."). An aircraft installed outside FG_ROOT
/// can also use a path relative to one of its parent folders ("Aircraft/c172p/...").
/// \param pFile including file
/// \param pInclude
/// \return the path of the included file, empty if not found
///
QString CFGPropertyList::resolveInclude(const QString& pFile, const QString& pInclude) const
{
    QDir Dir = QFileInfo(pFile).absoluteDir();
    if (Dir.exists(pInclude)) return Dir.absoluteFilePath(pInclude);
    while (Dir.cdUp())
    {
        if (Dir.exists(pInclude)) return Dir.absoluteFilePath(pInclude);
    }
    for (const QString& Root : m_Roots)
    {
        const QDir RootDir(Root);
        if (RootDir.exists(pInclude)) return RootDir.absoluteFilePath(pInclude);
    }
    return QString();
}
