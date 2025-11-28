/*

   Copyright 2022-2023 Sine Striker

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

#ifndef QASGLOBAL_H
#define QASGLOBAL_H

#include <QString>
#include <QTypeInfo>

// ----------------------------------
// Utils Macros
// ----------------------------------
#ifndef QAS_DISABLE_DEBUG
#    define qAsDbg() qDebug().noquote().nospace()
#else
#    define qAsDbg() QT_NO_QDEBUG_MACRO()
#endif

#define QAS_SET_OK(OK, VALUE) OK ? (*OK = VALUE) : (VALUE, false)


// ----------------------------------
// Namespace Macros
// ----------------------------------
#define QAS_BEGIN_NAMESPACE      namespace QAS {
#define QAS_END_NAMESPACE        }
#define QAS_PREPEND_NAMESPACE(T) QAS::T
#define QAS_USING_NAMESPACE      using namespace QAS;


// ----------------------------------
// Compiler Macros
// ----------------------------------
#ifdef QAS_QASC_RUN
#    define __qas_attr__(T) __qas_attr__(T)
#    define __qas_exclude__ __qas_exclude__
#    define __qas_include__ __qas_include__
#    define __qas_constraint__(T) __qas_constraint__(T)
#else
#    define __qas_attr__(T)
#    define __qas_exclude__
#    define __qas_include__
#    define __qas_constraint__(...)
#endif


// ----------------------------------
// Functions
// ----------------------------------


#endif // QASGLOBAL_H
