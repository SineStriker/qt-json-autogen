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

#ifndef QJSONSTREAM_H
#define QJSONSTREAM_H

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QRegularExpression>

#include <QHash>
#include <QList>
#include <QMap>
#include <QSet>
#include <QVector>

#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

#include "qasglobal.h"

QAS_BEGIN_NAMESPACE

class JsonStream {
public:
    enum Status {
        Ok = 1,
        KeyNotFound = 2,
        TypeNotMatch = 4,
        UnlistedValue = 8,
        ConstraintViolation = 16,  // Value violates constraint rules
        Success = Ok,
        Failed = KeyNotFound | UnlistedValue | TypeNotMatch | ConstraintViolation,
    };

    JsonStream() : q_status(Ok){};
    ~JsonStream() = default;
    JsonStream(qint8 sc);
    JsonStream(quint8 c);
    JsonStream(qint16 s);
    JsonStream(quint16 us);
    JsonStream(qint32 i);
    JsonStream(quint32 u);
    JsonStream(qint64 l);
    JsonStream(quint64 ul);
    JsonStream(bool b);
    JsonStream(float f);
    JsonStream(double d);
    JsonStream(const char *s, int size);
    JsonStream(const QString &s);
    JsonStream(const QJsonValue &val);
    JsonStream(const QJsonArray &arr);
    JsonStream(const QJsonObject &obj);

    inline QJsonValue data() const {
        return q_val;
    };

    inline QJsonObject object() const {
        return q_val.toObject();
    }

    inline QJsonArray array() const {
        return q_val.toArray();
    }

    inline QString str() const {
        return q_val.toString();
    }

    Status status() const;
    void setStatus(Status status);
    void resetStatus();

    inline bool failed() const {
        return q_status & Failed;
    }

    inline bool good() const {
        return q_status & Success;
    }

public:
    JsonStream &operator>>(qint8 &sc);
    JsonStream &operator>>(quint8 &c);
    JsonStream &operator>>(qint16 &s);
    JsonStream &operator>>(quint16 &us);
    JsonStream &operator>>(qint32 &i);
    JsonStream &operator>>(quint32 &u);
    JsonStream &operator>>(qint64 &l);
    JsonStream &operator>>(quint64 &ul);
    JsonStream &operator>>(bool &b);
    JsonStream &operator>>(float &f);
    JsonStream &operator>>(double &d);
    JsonStream &operator>>(QString &s);
    JsonStream &operator>>(QJsonValue &val);
    JsonStream &operator>>(QJsonArray &arr);
    JsonStream &operator>>(QJsonObject &obj);
    JsonStream &operator<<(qint8 sc);
    JsonStream &operator<<(quint8 c);
    JsonStream &operator<<(qint16 s);
    JsonStream &operator<<(quint16 us);
    JsonStream &operator<<(qint32 i);
    JsonStream &operator<<(quint32 u);
    JsonStream &operator<<(qint64 l);
    JsonStream &operator<<(quint64 ul);
    JsonStream &operator<<(bool b);
    JsonStream &operator<<(float f);
    JsonStream &operator<<(double d);
    JsonStream &operator<<(const QString &s);
    JsonStream &operator<<(const QJsonValue &val);
    JsonStream &operator<<(const QJsonArray &arr);
    JsonStream &operator<<(const QJsonObject &obj);

    template <class T>
    static JsonStream fromValue(const T &val) {
        JsonStream tmpStream;
        return tmpStream << val;
    }

    /* Convert and return without changing status */
    template <class T>
    bool convert(T *val) const {
        JsonStream copy = *this;
        return (copy >> (*val)).good();
    }

    template <class T>
    T value(bool *ok = nullptr) const {
        T tmp{};
        JsonStream copy = *this;
        QAS_SET_OK(ok, copy.convert(&tmp));
        return tmp;
    }

private:
    QJsonValue q_val;
    Status q_status;
};

// ----------------------------------
// Implementations
// ----------------------------------

inline JsonStream::JsonStream(qint8 sc) {
    *this << sc;
}

inline JsonStream::JsonStream(quint8 c) {
    *this << c;
}

inline JsonStream::JsonStream(qint16 s) {
    *this << s;
}

inline JsonStream::JsonStream(quint16 us) {
    *this << us;
}

inline JsonStream::JsonStream(qint32 i) {
    *this << i;
}

inline JsonStream::JsonStream(quint32 u) {
    *this << u;
}

inline JsonStream::JsonStream(qint64 l) {
    *this << l;
}

inline JsonStream::JsonStream(quint64 ul) {
    *this << ul;
}

inline JsonStream::JsonStream(bool b) {
    *this << b;
}

inline JsonStream::JsonStream(float f) {
    *this << f;
}

inline JsonStream::JsonStream(double d) {
    *this << d;
}

inline JsonStream::JsonStream(const char *s, int size) {
    *this << QString::fromUtf8(s, size);
}

inline JsonStream::JsonStream(const QString &s) {
    *this << s;
}

inline JsonStream::JsonStream(const QJsonValue &val) {
    *this << val;
}

inline JsonStream::JsonStream(const QJsonArray &arr) {
    *this << arr;
}

inline JsonStream::JsonStream(const QJsonObject &obj) {
    *this << obj;
}

inline JsonStream::Status JsonStream::status() const {
    return q_status;
}

inline void JsonStream::setStatus(JsonStream::Status status) {
    if (q_status == Ok)
        q_status = status;
}

inline void JsonStream::resetStatus() {
    q_status = Ok;
}

#define QJSONSTREAM_OUTPUT(VAL, TYPE) setStatus(q_val.is##TYPE() ? (VAL = q_val.to##TYPE(), Ok) : TypeNotMatch)

inline JsonStream &JsonStream::operator>>(qint8 &sc) {
    QJSONSTREAM_OUTPUT(sc, Double);
    return *this;
}

inline JsonStream &JsonStream::operator>>(quint8 &c) {
    QJSONSTREAM_OUTPUT(c, Double);
    return *this;
}

inline JsonStream &JsonStream::operator>>(qint16 &s) {
    QJSONSTREAM_OUTPUT(s, Double);
    return *this;
}

inline JsonStream &JsonStream::operator>>(quint16 &us) {
    QJSONSTREAM_OUTPUT(us, Double);
    return *this;
}

inline JsonStream &JsonStream::operator>>(qint32 &i) {
    QJSONSTREAM_OUTPUT(i, Double);
    return *this;
}

inline JsonStream &JsonStream::operator>>(quint32 &u) {
    QJSONSTREAM_OUTPUT(u, Double);
    return *this;
}

inline JsonStream &JsonStream::operator>>(qint64 &l) {
    QJSONSTREAM_OUTPUT(l, Double);
    return *this;
}

inline JsonStream &JsonStream::operator>>(quint64 &ul) {
    QJSONSTREAM_OUTPUT(ul, Double);
    return *this;
}

inline JsonStream &JsonStream::operator>>(bool &b) {
    QJSONSTREAM_OUTPUT(b, Bool);
    return *this;
}

inline JsonStream &JsonStream::operator>>(float &f) {
    QJSONSTREAM_OUTPUT(f, Double);
    return *this;
}

inline JsonStream &JsonStream::operator>>(double &d) {
    QJSONSTREAM_OUTPUT(d, Double);
    return *this;
}

inline JsonStream &JsonStream::operator>>(QString &s) {
    QJSONSTREAM_OUTPUT(s, String);
    return *this;
}

inline JsonStream &JsonStream::operator>>(QJsonValue &val) {
    q_val = val;
    return *this;
}

inline JsonStream &JsonStream::operator>>(QJsonArray &arr) {
    QJSONSTREAM_OUTPUT(arr, Array);
    return *this;
}

inline JsonStream &JsonStream::operator>>(QJsonObject &obj) {
    QJSONSTREAM_OUTPUT(obj, Object);
    return *this;
}

#undef QJSONSTREAM_OUTPUT

#define QJSONSTREAM_INPUT(VALUE)                                                                                       \
    q_val = QJsonValue(VALUE);                                                                                         \
    q_status = Ok;

inline JsonStream &JsonStream::operator<<(qint8 sc) {
    QJSONSTREAM_INPUT(sc);
    return *this;
}

inline JsonStream &JsonStream::operator<<(quint8 c) {
    QJSONSTREAM_INPUT(c);
    return *this;
}

inline JsonStream &JsonStream::operator<<(qint16 s) {
    QJSONSTREAM_INPUT(s);
    return *this;
}

inline JsonStream &JsonStream::operator<<(quint16 us) {
    QJSONSTREAM_INPUT(us);
    return *this;
}

inline JsonStream &JsonStream::operator<<(qint32 i) {
    QJSONSTREAM_INPUT(i);
    return *this;
}

inline JsonStream &JsonStream::operator<<(quint32 u) {
    QJSONSTREAM_INPUT((qint64) u);
    return *this;
}

inline JsonStream &JsonStream::operator<<(qint64 l) {
    QJSONSTREAM_INPUT(l);
    return *this;
}

inline JsonStream &JsonStream::operator<<(quint64 ul) {
    QJSONSTREAM_INPUT((qint64) ul);
    return *this;
}

inline JsonStream &JsonStream::operator<<(bool b) {
    QJSONSTREAM_INPUT(b);
    return *this;
}

inline JsonStream &JsonStream::operator<<(float f) {
    QJSONSTREAM_INPUT(f);
    return *this;
}

inline JsonStream &JsonStream::operator<<(double d) {
    QJSONSTREAM_INPUT(d);
    return *this;
}

inline JsonStream &JsonStream::operator<<(const QString &s) {
    QJSONSTREAM_INPUT(s);
    return *this;
}

inline JsonStream &JsonStream::operator<<(const QJsonValue &val) {
    QJSONSTREAM_INPUT(val);
    return *this;
}

inline JsonStream &JsonStream::operator<<(const QJsonArray &arr) {
    QJSONSTREAM_INPUT(arr);
    return *this;
}

inline JsonStream &JsonStream::operator<<(const QJsonObject &obj) {
    QJSONSTREAM_INPUT(obj);
    return *this;
}

#ifdef QAS_JSON_ENABLE_DECLARE_EVERYWHERE
// ----------------------------------
// User Implementation Part
// ----------------------------------
template <class T>
char __qas_json_get_read_writer(const T &) noexcept;

template <class T>
void __qas_json_check_impl() {
    Q_STATIC_ASSERT_X(sizeof(__qas_json_get_read_writer(T())) > sizeof(char),
                      "QAS::JsonStream only works with classes declared as "
                      "QAS_JSON, QAS_JSON_NS");
};

template <class T>
JsonStream &operator<<(JsonStream &stream, const T &var) {
    __qas_json_check_impl<T>();

    auto maybeStreamImpl = __qas_json_get_read_writer(var);
    maybeStreamImpl.template stream = stream;
    maybeStreamImpl.template read(var);
    return stream;
}

template <class T>
JsonStream &operator>>(JsonStream &stream, T &var) {
    __qas_json_check_impl<T>();

    auto maybeStreamImpl = __qas_json_get_read_writer(var);
    maybeStreamImpl.template stream = stream;
    maybeStreamImpl.template write(var);

    return stream;
}

template <class T>
struct JsonStreamImpl {
    JsonStream *stream;

    static void read(const T &var);
    static void write(const T &var);
};
#endif

// ----------------------------------
// Private Part
// ----------------------------------

/**
 * Notification : Do not use anything from this namespace directly in your code.
 *
 */

namespace JsonStreamPrivate {
#define QAS_JSON_PRIVATE_DECLARE_CONVERTER(TYPE)                                                                       \
    class TYPE##Converter {                                                                                            \
    public:                                                                                                            \
        explicit TYPE##Converter(const Q##TYPE &val) : stream(val) {                                                   \
        }                                                                                                              \
                                                                                                                       \
        template <class T>                                                                                             \
        operator T() const {                                                                                           \
            return stream.value<T>();                                                                                  \
        }                                                                                                              \
                                                                                                                       \
    private:                                                                                                           \
        JsonStream stream;                                                                                             \
        Q_DISABLE_COPY_MOVE(TYPE##Converter)                                                                           \
    };

    QAS_JSON_PRIVATE_DECLARE_CONVERTER(JsonValue)
    QAS_JSON_PRIVATE_DECLARE_CONVERTER(JsonObject)
    QAS_JSON_PRIVATE_DECLARE_CONVERTER(String)

#undef QAS_JSON_PRIVATE_DECLARE_CONVERTER
}

#define QAS_JSON_FROM_VALUE_IMPL(VAL)  QAS::JsonStreamPrivate::JsonValueConverter(VAL)
#define QAS_JSON_FROM_OBJECT_IMPL(OBJ) QAS::JsonStreamPrivate::JsonObjectConverter(OBJ)
#define QAS_JSON_FROM_STRING_IMPL(STR) QAS::JsonStreamPrivate::StringConverter(STR)

namespace JsonStreamUtils {

    inline JsonStream &parseAsArray(JsonStream &stream, const QByteArray &typeName, QJsonArray *out) {
        stream.resetStatus();
        const QJsonValue &_data = stream.data();
        if (!_data.isArray()) {
            qAsDbg() << typeName << ": expect array, but get " << _data.type();
            stream.setStatus(QAS::JsonStream::TypeNotMatch);
        } else {
            *out = _data.toArray();
        }
        return stream;
    }

    inline JsonStream &parseAsObject(JsonStream &stream, const QByteArray &typeName, QJsonObject *out) {
        stream.resetStatus();
        const QJsonValue &_data = stream.data();
        if (!_data.isObject()) {
            qAsDbg() << typeName << ": expect object, but get " << _data.type();
            stream.setStatus(QAS::JsonStream::TypeNotMatch);
        } else {
            *out = _data.toObject();
        }
        return stream;
    }

    inline JsonStream &parseAsString(JsonStream &stream, const QByteArray &typeName, QString *out) {
        stream.resetStatus();
        const QJsonValue &_data = stream.data();
        if (!_data.isString()) {
            qAsDbg() << typeName << ": expect string, but get " << _data.type();
            stream.setStatus(QAS::JsonStream::TypeNotMatch);
        } else {
            *out = _data.toString();
        }
        return stream;
    }

    template <class T>
    JsonStream parseObjectMember(const QJsonObject &obj, const QByteArray &key, const QByteArray &typeName, T *out) {
        auto it = obj.find(key);
        QAS::JsonStream tmpStream;
        if (it != obj.end()) {
            tmpStream << it.value();
            tmpStream >> *out;

            // If failed
            if (!tmpStream.good()) {
                qAsDbg() << typeName << ": fail at key " << key;
                tmpStream.setStatus(tmpStream.status());
            }
        } else {
            tmpStream.setStatus(JsonStream::KeyNotFound);
        }
        return tmpStream;
    };

}

// ----------------------------------
// Supported Containers
// ----------------------------------

namespace JsonStreamContainers {

    // List Implementations
    template <class LIST>
    JsonStream &writeList(JsonStream &stream, LIST &list) {
        // Check type
        QJsonArray arr;
        if (!JsonStreamUtils::parseAsArray(stream, typeid(list).name(), &arr).good()) {
            return stream;
        }

        // Write
        LIST tmpList;
        for (const auto &item : qAsConst(arr)) {
            JsonStream tmpStream(item);
            typename LIST::value_type tmp;

            tmpStream >> tmp;
            if (!tmpStream.good()) {
                qAsDbg() << typeid(list).name() << ": fail at index " << list.size();
                stream.setStatus(tmpStream.status());
                return stream;
            }

            tmpList.append(tmp);
        }
        list = std::move(tmpList);
        return stream;
    }

    template <class LIST>
    JsonStream &readList(JsonStream &stream, const LIST &list) {
        stream.resetStatus();

        QJsonArray arr;
        for (const auto &item : qAsConst(list)) {
            arr += JsonStream::fromValue(item).data();
        }
        stream << arr;

        return stream;
    }

    // Map Implementations
    template <class MAP, class OP>
    JsonStream &writeMap(JsonStream &stream, MAP &map, OP op) {
        // Check type
        QJsonObject obj;
        if (!JsonStreamUtils::parseAsObject(stream, typeid(map).name(), &obj).good()) {
            return stream;
        }
        MAP tmpMap;

        for (auto it = obj.begin(); it != obj.end(); ++it) {
            // Use operator to get key-value pair
            JsonStream tmpStream(it.value());
            typename MAP::mapped_type tmp;

            tmpStream >> tmp;
            if (!tmpStream.good()) {
                qAsDbg() << typeid(map).name() << ": fail at key " << it.key();
                stream.setStatus(tmpStream.status());
                return stream;
            }

            // Use operator to insert
            op.insert(tmpMap, it.key(), tmp);
        }

        map = std::move(tmpMap);
        return stream;
    }

    template <class MAP, class OP>
    JsonStream &readMap(JsonStream &stream, const MAP &map, OP op) {
        stream.resetStatus(); // Reset to Ok

        QJsonObject obj;

        for (auto it = map.begin(); it != map.end(); ++it) {
            // Use operator to get key-value pair
            QPair<QString, typename MAP::mapped_type> pair = op.template KVPair<QString, typename MAP::mapped_type>(it);
            obj.insert(pair.first, JsonStream::fromValue(pair.second).data());
        }

        stream << obj;

        return stream;
    }
}

// std::vector
template <class T>
JsonStream &operator>>(JsonStream &stream, std::vector<T> &list) {
    return QAS::JsonStreamContainers::writeList(stream, list);
}

template <class T>
JsonStream &operator<<(JsonStream &stream, const std::vector<T> &list) {
    return QAS::JsonStreamContainers::readList(stream, list);
}

// std::list
template <class T>
JsonStream &operator>>(JsonStream &stream, std::list<T> &list) {
    return QAS::JsonStreamContainers::writeList(stream, list);
}

template <class T>
JsonStream &operator<<(JsonStream &stream, const std::list<T> &list) {
    return QAS::JsonStreamContainers::readList(stream, list);
}

// std::set
template <class T>
JsonStream &operator>>(JsonStream &stream, std::set<T> &list) {
    return QAS::JsonStreamContainers::writeList(stream, list);
}

template <class T>
JsonStream &operator<<(JsonStream &stream, const std::set<T> &list) {
    return QAS::JsonStreamContainers::readList(stream, list);
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
// QList
template <class T>
JsonStream &operator>>(JsonStream &stream, QList<T> &list) {
    return QAS::JsonStreamContainers::writeList(stream, list);
}

template <class T>
JsonStream &operator<<(JsonStream &stream, const QList<T> &list) {
    return QAS::JsonStreamContainers::readList(stream, list);
}
#endif

// QSet
template <class T>
JsonStream &operator>>(JsonStream &stream, QSet<T> &list) {
    return QAS::JsonStreamContainers::writeList(stream, list);
}

template <class T>
JsonStream &operator<<(JsonStream &stream, const QSet<T> &list) {
    return QAS::JsonStreamContainers::readList(stream, list);
}

// QVector
template <class T>
JsonStream &operator>>(JsonStream &stream, QVector<T> &list) {
    return QAS::JsonStreamContainers::writeList(stream, list);
}

template <class T>
JsonStream &operator<<(JsonStream &stream, const QVector<T> &list) {
    return QAS::JsonStreamContainers::readList(stream, list);
}

// QStringList
inline JsonStream &operator>>(JsonStream &stream, QStringList &list) {
    return QAS::JsonStreamContainers::writeList(stream, list);
}

inline JsonStream &operator<<(JsonStream &stream, const QStringList &list) {
    return QAS::JsonStreamContainers::readList(stream, list);
}

// STL map operators
struct STLMapOps {
    template <class K, class V, class IT>
    QPair<K, V> KVPair(const IT &it) const {
        return qMakePair(it.first, it.second);
    }

    template <class MAP, class K, class V>
    void insert(MAP &map, const K &key, const V &value) const {
        map.insert(std::make_pair(key, value));
    }
};

// std::map
template <class T>
JsonStream &operator>>(JsonStream &stream, std::map<QString, T> &map) {
    return QAS::JsonStreamContainers::writeMap(stream, map, STLMapOps());
}

template <class T>
JsonStream &operator<<(JsonStream &stream, const std::map<QString, T> &map) {
    return QAS::JsonStreamContainers::readMap(stream, map, STLMapOps());
}

// std::unordered_map
template <class T>
JsonStream &operator>>(JsonStream &stream, std::unordered_map<QString, T> &map) {
    return QAS::JsonStreamContainers::writeMap(stream, map, STLMapOps());
}

template <class T>
JsonStream &operator<<(JsonStream &stream, const std::unordered_map<QString, T> &map) {
    return QAS::JsonStreamContainers::readMap(stream, map, STLMapOps());
}

// Qt map operators
struct QtMapOps {
    template <class K, class V, class IT>
    QPair<K, V> KVPair(const IT &it) const {
        return qMakePair(it.key(), it.value());
    }

    template <class MAP, class K, class V>
    void insert(MAP &map, const K &key, const V &value) const {
        map.insert(key, value);
    }
};

// QMap
template <class T>
JsonStream &operator>>(JsonStream &stream, QMap<QString, T> &map) {
    return QAS::JsonStreamContainers::writeMap(stream, map, QtMapOps());
}

template <class T>
JsonStream &operator<<(JsonStream &stream, const QMap<QString, T> &map) {
    return QAS::JsonStreamContainers::readMap(stream, map, QtMapOps());
}

// QHash
template <class T>
JsonStream &operator>>(JsonStream &stream, QHash<QString, T> &map) {
    return QAS::JsonStreamContainers::writeMap(stream, map, QtMapOps());
}

template <class T>
JsonStream &operator<<(JsonStream &stream, const QHash<QString, T> &map) {
    return QAS::JsonStreamContainers::readMap(stream, map, QtMapOps());
}

QAS_END_NAMESPACE

#define QAS_JSON_IMPL(TYPE)                                                                                            \
    friend QAS::JsonStream &operator>>(QAS::JsonStream &stream, TYPE &var);                                            \
    friend QAS::JsonStream &operator<<(QAS::JsonStream &stream, const TYPE &var);

#define QAS_JSON_NS_IMPL(TYPE)                                                                                         \
    QAS::JsonStream &operator>>(QAS::JsonStream &stream, TYPE &var);                                                   \
    QAS::JsonStream &operator<<(QAS::JsonStream &stream, const TYPE &var);

// ----------------------------------
// QASC Macros
// ----------------------------------
#ifdef QAS_QASC_RUN
#    define QAS_JSON(T)    QAS_JSON(T)
#    define QAS_JSON_NS(T) QAS_JSON_NS(T)
#else
#    define QAS_JSON(T)    QAS_JSON_IMPL(T)
#    define QAS_JSON_NS(T) QAS_JSON_NS_IMPL(T)
#endif


// ----------------------------------
// Simplified Macros or Functions
// ----------------------------------

// Any Class -> QJsonValue
template <class T>
QJsonValue qAsAnyToJson(const T &var) {
    return QAS::JsonStream::fromValue(var).data();
}

// QJsonValue -> Any Class
template <class T>
T qAsJsonGetAny(const QJsonValue &val) {
    return QAS::JsonStream(val).value<T>();
}

#define _qAsJsonGetAny(VAL) QAS_JSON_FROM_VALUE_IMPL(VAL) // No need to specify return type

template <class T>
bool qAsJsonTryGetAny(const QJsonValue &val, T *out) {
    return QAS::JsonStream(val).convert(out);
}

// Non-Basic Class -> QJsonObject
template <class T>
QJsonObject qAsClassToJson(const T &var) {
    return QAS::JsonStream::fromValue(var).object();
}

// QJsonObject -> Non-Basic Class
template <class T>
T qAsJsonGetClass(const QJsonObject &obj) {
    return QAS::JsonStream(obj).value<T>();
}

#define _qAsJsonGetClass(OBJ) QAS_JSON_FROM_OBJECT_IMPL(OBJ) // No need to specify return type

template <class T>
bool qAsJsonTryGetClass(const QJsonObject &obj, T *out) {
    return QAS::JsonStream(obj).convert(out);
}

// Enum Class -> QString
template <class T>
QString qAsEnumToJson(const T &var) {
    return QAS::JsonStream::fromValue(var).str();
}

// QString -> Enum Class
template <class T>
T qAsJsonGetEnum(const QString &str) {
    return QAS::JsonStream(str).value<T>();
}

#define _qAsJsonGetEnum(STR) QAS_JSON_FROM_STRING_IMPL(STR) // No need to specify return type

template <class T>
bool qAsJsonTryGetEnum(const QString &str, T *out) {
    return QAS::JsonStream(str).convert(out);
}

// ----------------------------------
// Constraint Validator
// ----------------------------------

QAS_BEGIN_NAMESPACE

class ConstraintValidator {
public:
    // Validate individual constraint types
    static inline bool validateMinimum(const QJsonValue& input, const QJsonValue& constraint, QString* errorMsg = nullptr) {
        if (!isNumeric(input) || !isNumeric(constraint)) {
            if (errorMsg) *errorMsg = "MINIMUM constraint requires numeric values";
            return false;
        }
        
        double inputVal = toDouble(input);
        double constraintVal = toDouble(constraint);
        bool valid = inputVal >= constraintVal;
        
        if (!valid && errorMsg) {
            *errorMsg = QString("Value %1 is less than minimum %2").arg(inputVal).arg(constraintVal);
        }
        
        return valid;
    }
    
    static inline bool validateMaximum(const QJsonValue& input, const QJsonValue& constraint, QString* errorMsg = nullptr) {
        if (!isNumeric(input) || !isNumeric(constraint)) {
            if (errorMsg) *errorMsg = "MAXIMUM constraint requires numeric values";
            return false;
        }
        
        double inputVal = toDouble(input);
        double constraintVal = toDouble(constraint);
        bool valid = inputVal <= constraintVal;
        
        if (!valid && errorMsg) {
            *errorMsg = QString("Value %1 is greater than maximum %2").arg(inputVal).arg(constraintVal);
        }
        
        return valid;
    }
    
    static inline bool validateExclusiveMinimum(const QJsonValue& input, const QJsonValue& constraint, QString* errorMsg = nullptr) {
        if (!isNumeric(input) || !isNumeric(constraint)) {
            if (errorMsg) *errorMsg = "EXCLUSIVE_MINIMUM constraint requires numeric values";
            return false;
        }
        
        double inputVal = toDouble(input);
        double constraintVal = toDouble(constraint);
        bool valid = inputVal > constraintVal;
        
        if (!valid && errorMsg) {
            *errorMsg = QString("Value %1 is not greater than exclusive minimum %2").arg(inputVal).arg(constraintVal);
        }
        
        return valid;
    }
    
    static inline bool validateExclusiveMaximum(const QJsonValue& input, const QJsonValue& constraint, QString* errorMsg = nullptr) {
        if (!isNumeric(input) || !isNumeric(constraint)) {
            if (errorMsg) *errorMsg = "EXCLUSIVE_MAXIMUM constraint requires numeric values";
            return false;
        }
        
        double inputVal = toDouble(input);
        double constraintVal = toDouble(constraint);
        bool valid = inputVal < constraintVal;
        
        if (!valid && errorMsg) {
            *errorMsg = QString("Value %1 is not less than exclusive maximum %2").arg(inputVal).arg(constraintVal);
        }
        
        return valid;
    }
    
    static inline bool validateConst(const QJsonValue& input, const QJsonValue& constraint, QString* errorMsg = nullptr) {
        bool valid = (input == constraint);
        
        if (!valid && errorMsg) {
            *errorMsg = QString("Value does not match constant constraint");
        }
        
        return valid;
    }
    
    static inline bool validateEnum(const QJsonValue& input, const QJsonValue& constraint, QString* errorMsg = nullptr) {
        if (!constraint.isArray()) {
            if (errorMsg) *errorMsg = "ENUM constraint value must be an array";
            return false;
        }
        
        QJsonArray enumArray = constraint.toArray();
        
        for (const auto& enumValue : enumArray) {
            if (input == enumValue) {
                return true; // Found matching value
            }
        }
        
        // No matching value found
        if (errorMsg) {
            *errorMsg = QString("Value is not in the allowed enumeration");
        }
        
        return false;
    }
    
    static inline bool validateMinLength(const QJsonValue& input, const QJsonValue& constraint, QString* errorMsg = nullptr) {
        if (!input.isString()) {
            if (errorMsg) *errorMsg = "MIN_LENGTH constraint requires string input";
            return false;
        }
        
        if (!constraint.isDouble()) {
            if (errorMsg) *errorMsg = "MIN_LENGTH constraint value must be a number";
            return false;
        }
        
        QString inputStr = toString(input);
        int minLength = constraint.toInt();
        bool valid = inputStr.length() >= minLength;
        
        if (!valid && errorMsg) {
            *errorMsg = QString("String length %1 is less than minimum %2").arg(inputStr.length()).arg(minLength);
        }
        
        return valid;
    }
    
    static inline bool validateMaxLength(const QJsonValue& input, const QJsonValue& constraint, QString* errorMsg = nullptr) {
        if (!input.isString()) {
            if (errorMsg) *errorMsg = "MAX_LENGTH constraint requires string input";
            return false;
        }
        
        if (!constraint.isDouble()) {
            if (errorMsg) *errorMsg = "MAX_LENGTH constraint value must be a number";
            return false;
        }
        
        QString inputStr = toString(input);
        int maxLength = constraint.toInt();
        bool valid = inputStr.length() <= maxLength;
        
        if (!valid && errorMsg) {
            *errorMsg = QString("String length %1 is greater than maximum %2").arg(inputStr.length()).arg(maxLength);
        }
        
        return valid;
    }
    
    static inline bool validatePattern(const QJsonValue& input, const QJsonValue& constraint, QString* errorMsg = nullptr) {
        if (!input.isString()) {
            if (errorMsg) *errorMsg = "PATTERN constraint requires string input";
            return false;
        }
        
        if (!constraint.isString()) {
            if (errorMsg) *errorMsg = "PATTERN constraint value must be a string";
            return false;
        }
        
        QString inputStr = toString(input);
        QString pattern = toString(constraint);
        
        QRegularExpression regex(pattern);
        if (!regex.isValid()) {
            if (errorMsg) *errorMsg = QString("Invalid regular expression pattern: %1").arg(pattern);
            return false;
        }
        
        bool valid = regex.match(inputStr).hasMatch();
        
        if (!valid && errorMsg) {
            *errorMsg = QString("String does not match pattern: %1").arg(pattern);
        }
        
        return valid;
    }
    
private:
    // Helper functions for type checking and conversion
    static inline bool isNumeric(const QJsonValue& value) {
        return value.isDouble() || value.isNull() == false;
    }
    
    static inline double toDouble(const QJsonValue& value) {
        return value.toDouble();
    }
    
    static inline QString toString(const QJsonValue& value) {
        return value.toString();
    }
};

QAS_END_NAMESPACE

#endif // QJSONSTREAM_H
