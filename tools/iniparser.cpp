/****************************************************************************
**
** Copyright (C) 2018 FSFranceSimulateur team.
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

#include "iniparser.h"

namespace ffs2play
{
	IniParser::IniParser(const QString& pFilename, const SIM_TYPE pSimType)
	{
		m_simType = pSimType;
		QFile m_inputFile(pFilename);
		if (m_inputFile.open(QIODevice::ReadOnly))
		{
			QTextStream in(&m_inputFile);
			QString Group;
			QStringList Keys;
			while (!in.atEnd())
			{
				QString NewGroup;
				QString Line = in.readLine();
				bool is_group=false;
				if (m_simType==SIM_TYPE::FSX)
				{
					QRegExp rx("\\[(.*)\\]");
					rx.setMinimal(true);
					if (rx.indexIn(Line)>=0)
					{
						is_group = true;
						NewGroup=rx.cap(1).toLower();
					}
				}
				else if (m_simType==SIM_TYPE::XPLANE)
				{
					if (Line.toLower().contains("_begin"))
					{
						is_group = true;
						NewGroup=Line.toLower().remove("_begin");
					}
				}
				if (is_group)
				{
					if ((!Group.isEmpty())&&(Keys.count()>0))
					{
						m_GroupMap.insert(Group,Keys);
					}
					Keys.clear();
					Group=NewGroup;
				}
				if ((!is_group) && (!Group.isEmpty()))
				{
					if (Line.startsWith("//") || Line.startsWith(";"))
					{

					}
					else
					{
						//Clean space, tabs char
						//Line.remove(" ");
						Line.remove("\t");
						Keys.append(Line);
					}
				}
			}
		}
		else
		{
			IniParserException ex;
			ex.Message=tr("Unable to open ") + pFilename;
			throw ex;
		}
	}

	IniParser::~IniParser()
	{
	}

	///
	/// \brief IniParser::value
	/// \param pGroup
	/// \param pParameter
	/// \param pDefaultValue
	/// \return
	///

	QVariant IniParser::value(const QString& pGroup, const QString& pParameter, const QVariant& pDefaultValue) const
	{
		QVariant Result;
		Result.setValue(QString(""));
		QString Parameter = pParameter.toLower();
		QString Group = pGroup.toLower();
		QStringList Keys = m_GroupMap[Group];
		for (QString Key : Keys)
		{
			if (m_simType==SIM_TYPE::FSX)
			{
				QStringList parser = Key.split('=');

				if (parser.count()>1)
				{
					parser[0].remove(" ");
					parser[0].remove("\t");
					if (parser[0].compare(Parameter,Qt::CaseInsensitive)==0)
					{
						Result.setValue(parser[1]);
						break;
					}
				}
			}
			else if (m_simType==SIM_TYPE::XPLANE)
			{
				QStringList parser = Key.split(' ');
				if (parser.count()>2)
				{
					if (parser[0] == 'P')
					{
						if (parser[1].compare(Parameter,Qt::CaseInsensitive)==0)
						{
							Result.setValue(Key.mid(parser[1].length()+3));
							break;
						}
					}
				}
			}
		}
		return Result;
	}
}
