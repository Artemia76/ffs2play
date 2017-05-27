#ifndef FFS2CORE_GLOBAL_H
#define FFS2CORE_GLOBAL_H

#include <QtCore/QtGlobal>

#if defined(FFS2CORE_DLL)
#  define FFS2CORE_EXPORT Q_DECL_EXPORT
#else
#  define FFS2CORE_EXPORT Q_DECL_IMPORT
#endif

#endif // FFS2CORE_GLOBAL_H
