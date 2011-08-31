#ifndef COLORIZEPHOTOEFFECT_GLOBAL_H
#define COLORIZEPHOTOEFFECT_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QtPlugin>

#if defined(COLORIZEPHOTOEFFECT_LIBRARY)
#  define COLORIZEPHOTOEFFECTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define COLORIZEPHOTOEFFECTSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // COLORIZEPHOTOEFFECT_GLOBAL_H