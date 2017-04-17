#include "tools/version.h"
#include "tools/getappversion.h"

sVersion GetAppVersion()
{
    sVersion Version;
    Version.Major = APP_MAJOR;
    Version.Minor = APP_MINOR;
    Version.Build = APP_BUILD;
    Version.Name = APP_NAME;
    QTextStream out(&Version.FullVersion);
    out << Version.Name
       << "_"
       << Version.Major
       << "_"
       << Version.Minor
       << "_"
       << Version.Build;
    return Version;
}
