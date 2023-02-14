#ifndef QAS_GLOBAL_H
#define QAS_GLOBAL_H

#ifndef QAS_DISABLE_DEBUG
#define qasDebug qDebug
#else
#define qasDebug QT_NO_QDEBUG_MACRO
#endif

#define QAS_SET_OK(OK, VALUE) OK ? (*OK = VALUE) : false

#endif // QAS_GLOBAL_H
