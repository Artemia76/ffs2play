#include "xpcslparser.h"
#include "tools/simple_file_parser.h"

XPCSLParser::XPCSLParser(QObject *parent) : AIBaseParser(parent)
{
    m_AIFileFilter << "xsb_aircraft.txt";
    searchSimPath();
}

XPCSLParser::~XPCSLParser()
{
    stopScan();
}

///
/// \brief XPCSLParser::findXPlaneRoots
/// \return the X-Plane 12 and 11 installations listed by X-Plane in x-plane_install_12.txt / _11.txt
///
QStringList XPCSLParser::findXPlaneRoots()
{
#if defined(Q_OS_WIN)
    const QString Folder = qEnvironmentVariable("LOCALAPPDATA");
#elif defined(Q_OS_MACOS)
    const QString Folder = QDir::homePath() + "/Library/Preferences";
#else
    const QString Folder = QDir::homePath() + "/.x-plane";
#endif
    QStringList Roots;
    for (const QString& Name : {"x-plane_install_12.txt", "x-plane_install_11.txt"})
    {
        QFile File(Folder + "/" + Name);
        if (!File.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
        QTextStream Stream(&File);
        while (!Stream.atEnd())
        {
            const QString Root = QDir::fromNativeSeparators(QDir::cleanPath(Stream.readLine().trimmed()));
            if (!Root.isEmpty() && QDir(Root + "/Resources/plugins").exists() && !Roots.contains(Root, Qt::CaseInsensitive))
                Roots << Root;
        }
    }
    return Roots;
}

///
/// \brief XPCSLParser::hasCSLPackage
/// \return true if the folder or one of its sub-folders is a CSL package (xsb_aircraft.txt)
///
bool XPCSLParser::hasCSLPackage(const QString& pPath)
{
    const QDir Dir(pPath);
    if (!Dir.exists()) return false;
    if (Dir.exists("xsb_aircraft.txt")) return true;
    for (const QString& Sub : Dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        if (QFile::exists(Dir.absoluteFilePath(Sub + "/xsb_aircraft.txt"))) return true;
    }
    return false;
}

///
/// \brief XPCSLParser::searchSimPath
/// Without any folder configured, registers the CSL folders installed in the X-Plane plugins :
/// X-CSL-Updater, xPilot, X-IvAp, LiveTraffic... (<plugin>/Resources/CSL, <plugin>/CSL, or a plugin
/// folder named after CSL). The old CSL package bundled with xffs2play is ignored.
/// The reset of the AI database finds them again.
///
void XPCSLParser::searchSimPath()
{
    if (!m_Initialized || !getPathList().isEmpty()) return;
    QStringList Added;
    for (const QString& Root : findXPlaneRoots())
    {
        const QDir Plugins(Root + "/Resources/plugins");
        for (const QString& Plugin : Plugins.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
        {
            if (Plugin.compare("xffs2play", Qt::CaseInsensitive) == 0) continue;
            QStringList Candidates;
            Candidates << Plugins.absoluteFilePath(Plugin + "/Resources/CSL") << Plugins.absoluteFilePath(Plugin + "/CSL");
            if (Plugin.contains("CSL", Qt::CaseInsensitive)) Candidates << Plugins.absoluteFilePath(Plugin);
            for (const QString& Candidate : std::as_const(Candidates))
            {
                const QString Path = QDir::cleanPath(Candidate);
                if (Added.contains(Path, Qt::CaseInsensitive) || !hasCSLPackage(Path)) continue;
                if (addPath(Path))
                {
                    Added << Path;
                    m_log.log(tr("CSL folder found : ") + Path);
                    break;
                }
            }
        }
    }
    if (Added.isEmpty()) m_log.log(tr("No CSL folder found : add it in the P2P settings"), Qt::darkYellow);
}

///
/// \brief AIMapping::DBFileUpdate
/// \param pKey
/// \param pPath
///
void XPCSLParser::DBFileUpdate(const QString &pKey, const QString &pPath)
{
    if ((!m_Initialized) || m_Table.isEmpty()) return;
    if (!m_Thread_DB.isOpen()) return;
    QSqlQuery query(m_Thread_DB);
    QString Path= pPath;
    xsb_aircraft Content;
    ReadCSL(pPath,Content);
    query.prepare( "SELECT checksum FROM Aircrafts WHERE (checksum = :checksum) and (Sim = :Sim)");
    query.bindValue(":checksum",pKey);
    query.bindValue(":Sim",m_Table);
    if (query.exec())
    {
        if (!query.next())
        {
#ifdef FFS_DEBUG
            m_log.log("AIMapping : No record was found for " + pKey, Qt::darkYellow,LEVEL_VERBOSE);
#endif
#ifdef Q_OS_WIN
            Path = Path.replace("/","\\");
#endif
            QString Editor;
            QString Atc_model;
            QString Atc_type;
            QString Atc_Airline;
            QString Category;
            QString Title;
            int Engine_Type = 0;
            double Static_CG_Height=0.0;
            double Static_Pitch = 0.0;


            for (const auto& csl : std::as_const(Content.ArrCSL))
            {
                Atc_model = csl.ICAO;
                Atc_Airline = csl.AirLine;
                Title = csl.ICAO + ":" + csl.AirLine + ":" + QString::number(csl.Livery);
                Editor = Content.Export_Name;
                //Atc_type = s.value("properties","acf/_manufacturer").toString();
                //Category =  s.value("General","Category").toString();
                //Engine_Type =  s.value("GeneralEngineData","engine_type").toInt();
                Static_CG_Height =  csl.Offset;
                Static_Pitch =  0.0;
#if FFS_DEBUG
                m_log.log("AIMapping : Path = " + Path,Qt::darkYellow,LEVEL_VERBOSE);
                m_log.log("AIMapping : Editor = " + Editor,Qt::darkYellow,LEVEL_VERBOSE);
                m_log.log("AIMapping : ATCModel = " + Atc_model, Qt::darkYellow,LEVEL_VERBOSE);
                m_log.log("AIMapping : ATCType = " + Atc_type, Qt::darkYellow,LEVEL_VERBOSE);
                m_log.log("AIMapping : ATCAirline = " + Atc_Airline, Qt::darkYellow,LEVEL_VERBOSE);
                m_log.log("AIMapping : Category = " + Category, Qt::darkYellow,LEVEL_VERBOSE);
                m_log.log("AIMapping : EngineType = " + QString::number(Engine_Type), Qt::darkYellow,LEVEL_VERBOSE);
                m_log.log("AIMapping : CG Height = " +  QString::number(Static_CG_Height), Qt::darkYellow,LEVEL_VERBOSE);
                m_log.log("AIMapping : Pitch = " +  QString::number(Static_Pitch), Qt::darkYellow,LEVEL_VERBOSE);
#endif
                int nb_moteur = 0;
                int nb_title = 0;
#if FFS_DEBUG
                m_log.log("AIMapping : livery found Title = " + Title,Qt::darkYellow,LEVEL_VERBOSE);
#endif
                nb_title++;
                query.prepare("SELECT * FROM Aircrafts WHERE (Title = :Title) AND (checksum = :checksum) AND (Sim = :Sim)" );
                query.bindValue(":Title",Title);
                query.bindValue(":checksum",pKey);
                query.bindValue(":Sim", m_Table);
                if (query.exec())
                {
                    // If there is no result, we have a new aircraft to register
                    if (!query.next())
                    {
                        query.prepare("INSERT INTO Aircrafts (checksum, Sim, Title, Editor, Type, Model, AirLine, Path, Category, NbEngine, EngineType, CGHeight, Pitch)"
                                      "VALUES (:checksum, :Sim, :Title, :Editor, :Type, :Model, :AirLine, :Path, :Category, :NBEngine, :EngineType, :CGHeight, :Pitch)");
                        query.bindValue(":checksum", pKey);
                        query.bindValue(":Sim",m_Table);
                        query.bindValue(":Title", Title);
                        query.bindValue(":Editor", Editor);
                        query.bindValue(":Type", Atc_type);
                        query.bindValue(":Model", Atc_model);
                        query.bindValue(":AirLine", Atc_Airline);
                        query.bindValue(":Path", Path);
                        query.bindValue(":Category", Category);
                        query.bindValue(":NBEngine", nb_moteur);
                        query.bindValue(":EngineType", Engine_Type);
                        query.bindValue(":CGHeight", Static_CG_Height);
                        query.bindValue(":Pitch", Static_Pitch);
                        query.exec();
                        m_NbAdded++;
                    }
                    else //Update Record
                    {
                        query.prepare("UPDATE Aircrafts SET checksum = :checksum, Editor = :Editor, Type = :Type, Model = :Model, AirLine = :AirLine, Path = :Path, Category = :Category, NbEngine = :NbEngine, EngineType = :EngineType, CGHeight = :CHeight, Pitch = :Pitch WHERE (Title = :Title) and (Sim = :Sim)");
                        query.bindValue(":checksum", pKey);
                        query.bindValue(":Sim",m_Table);
                        query.bindValue(":Title", Title);
                        query.bindValue(":Editor", Editor);
                        query.bindValue(":Type", Atc_type);
                        query.bindValue(":Model", Atc_model);
                        query.bindValue(":AirLine", Atc_Airline);
                        query.bindValue(":Path", Path);
                        query.bindValue(":Category", Category);
                        query.bindValue(":NBEngine", nb_moteur);
                        query.bindValue(":EngineType", Engine_Type);
                        query.bindValue(":CGHeight", Static_CG_Height);
                        query.bindValue(":Pitch", Static_Pitch);
                        query.exec();
                        m_NbUpdated++;
                    }
                }
            }
        }
    }
    else m_log.log(tr("AIMapping : Error = ") + query.lastError().text(), Qt::darkYellow,LEVEL_VERBOSE);
}

void XPCSLParser::ReadCSL (const QString& pPath,  xsb_aircraft& pContent)
{
    CSL csl;
    csl.Offset=0.0;
    simple_file_parser s;
    if (s.open(pPath.toStdString()))
    {
        s.set_single_char_tokens(" :");
        while (s.get_next_line())
        {
            size_t nbToken = s.get_num_tokens();
            if (nbToken>0)
            {
                QString Token1 = QString::fromStdString(s.get_token(0)).toUpper();
                if (Token1=="EXPORT_NAME")
                {
                    pContent.Export_Name = QString::fromStdString(s.get_token(1)).toUpper();
                }
                if (Token1=="ICAO")
                {
                    csl.ICAO = QString::fromStdString(s.get_token(1)).toUpper();
                    csl.AirLine = csl.ICAO;
                    csl.Livery = 0;
                    pContent.ArrCSL.push_back(csl);
                }
                if (Token1=="AIRLINE")
                {
                    csl.ICAO = QString::fromStdString(s.get_token(1)).toUpper();
                    csl.AirLine = QString::fromStdString(s.get_token(2)).toUpper();
                    csl.Livery = 0;
                    pContent.ArrCSL.push_back(csl);
                }
                if (Token1=="LIVERY")
                {
                    csl.ICAO = QString::fromStdString(s.get_token(1)).toUpper();
                    csl.AirLine = QString::fromStdString(s.get_token(2)).toUpper();
                    csl.Livery = QString::fromStdString(s.get_token(3)).toInt();
                    pContent.ArrCSL.push_back(csl);
                }
                if (Token1=="VERT_OFFSET")
                {
                    csl.Offset = QString::fromStdString(s.get_token(1)).toDouble();
                }
            }
        }
        s.close();
    }
    else
    {
        m_log.log(tr("AIMapping : Fail to open file = ") + pPath ,Qt::magenta,LEVEL_NORMAL);
    }
}
