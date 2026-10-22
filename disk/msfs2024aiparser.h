/****************************************************************************
**
** Copyright (C) 2017 FSFranceSimulateur team.
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
** msfs2024aiparser.h is part of FF2Play project
**
** This class fills the AI database with the aircraft listed by MSFS 2024
****************************************************************************/

#ifndef MSFS2024AIPARSER_H
#define MSFS2024AIPARSER_H

#include "disk/aibaseparser.h"

///
/// \brief The MSFS2024AIParser class
/// MSFS 2024 packs its aircraft (.fsarchive, streamed packages) : the list comes from
/// SimConnect instead of the files. One record per (title, livery), the livery is kept
/// in the Path column and reaches createAI through AIResol::path.
///
class MSFS2024AIParser : public AIBaseParser
{
public:
    MSFS2024AIParser( QObject *parent = nullptr);
    ~MSFS2024AIParser() override;

protected:
    void    startScanSimObject() override;
    void    DBFileUpdate(const QString&, const QString&) override {}

private:
    void    onSimObjectList(const QList<QPair<QString,QString>>& pList, bool pComplete);
};

#endif // MSFS2024AIPARSER_H
