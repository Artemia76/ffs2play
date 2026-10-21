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
** fgpropertylist.h is part of FF2Play project
**
** Reader of FlightGear PropertyList XML files (aircraft -set.xml)
****************************************************************************/

#ifndef FGPROPERTYLIST_H
#define FGPROPERTYLIST_H

#include <QHash>
#include <QString>
#include <QStringList>

class QXmlStreamReader;

///
/// \brief The CFGPropertyList class
/// Flattens a PropertyList file into property paths ("sim/model/path") and
/// their values, following the include attributes
///
class CFGPropertyList
{
public:
    /// Maximum nesting of included files
    static constexpr int MaxIncludeDepth = 4;

    bool                    read(const QString& pFile, const QStringList& pRoots = QStringList());
    QString                 value(const QString& pPath) const;
    QStringList             values(const QString& pPath) const;

private:
    /// Values in reading order : an included value is overridden by the including file
    QHash<QString, QStringList> m_Props;
    QStringList             m_Roots;

    bool                    readFile(const QString& pFile, const QString& pPrefix, int pDepth);
    void                    readNode(QXmlStreamReader& pXml, const QString& pFile, const QString& pPath, int pDepth);
    QString                 resolveInclude(const QString& pFile, const QString& pInclude) const;
};

#endif // FGPROPERTYLIST_H
