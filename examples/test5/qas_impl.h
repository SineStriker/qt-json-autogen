#ifndef QAS_IMPL_H
#define QAS_IMPL_H

#include "qjsonstream.h"

enum Gender {
    Male,
    Female,
};

QAS_JSON_NS_IMPL(Gender);

inline QAS::JsonStream &operator>>(QAS::JsonStream &_stream, Gender &_var) {
    QString _str;
    if (!QAS::JsonStreamUtils::parseAsString(_stream, typeid(_var).name(), &_str).good()) {
        return _stream;
    }

    Gender _tmp{};
    if (_str == "Male") {
        _tmp = Male;
    } else if (_str == "Female") {
        _tmp = Female;
    } else {
        _stream.setStatus(QAS::JsonStream::UnlistedValue);
    }
    _var = _tmp;
    return _stream;
}

inline QAS::JsonStream &operator<<(QAS::JsonStream &_stream, const Gender &_var) {
    _stream.resetStatus();

    QString _tmp;
    switch (_var) {
        case Male:
            _tmp = "Male";
            break;
        case Female:
            _tmp = "Female";
            break;
        default:
            break;
    }
    _stream << _tmp;

    return _stream;
}

struct Rectangle {
    int a;
    int b;
};

QAS_JSON_NS_IMPL(Rectangle)

inline QAS::JsonStream &operator>>(QAS::JsonStream &_stream, Rectangle &_var) {
    QJsonObject _obj;
    if (!QAS::JsonStreamUtils::parseAsObject(_stream, typeid(_var).name(), &_obj).good()) {
        return _stream;
    }

    Rectangle _tmpVar{};
    QAS::JsonStream _tmpStream;
    if (!(_tmpStream = QAS::JsonStreamUtils::parseObjectMember(_obj, "a", typeid(_tmpVar).name(),
                                                               &_tmpVar.a)).good()) {
        _stream.setStatus(_tmpStream.status());
        return _stream;
    }
    if (!(_tmpStream = QAS::JsonStreamUtils::parseObjectMember(_obj, "b", typeid(_tmpVar).name(),
                                                               &_tmpVar.b)).good()) {
        _stream.setStatus(_tmpStream.status());
        return _stream;
    }

    _var = std::move(_tmpVar);
    return _stream;
}

inline QAS::JsonStream &operator<<(QAS::JsonStream &_stream, const Rectangle &_var) {
    _stream.resetStatus();

    QJsonObject _obj;
    _obj.insert("a", QAS::JsonStream::fromValue(_var.a).data());
    _obj.insert("b", QAS::JsonStream::fromValue(_var.b).data());
    _stream << _obj;

    return _stream;
}


#endif // QAS_IMPL_H
