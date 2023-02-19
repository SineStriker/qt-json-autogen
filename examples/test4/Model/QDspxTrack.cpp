#include "QDspxTrack.h"

QAS::JsonStream &QDspx::operator>>(QAS::JsonStream &_stream, QDspx::ClipRef &_var) {
    const QJsonValue &_data = _stream.data();
    if (!_data.isObject()) {
        qAsDbg() << typeid(_var).name() << ": expect object, but get " << _data.type();
        _stream.setStatus(QAS::JsonStream::TypeNotMatch);
        return _stream;
    }

    QJsonObject _obj = _data.toObject();
    QDspx::Clip _tmpVar{};

    QAS::JsonStream _tmpStream;
    if (!(_tmpStream = QAS::JsonStreamUtils::parseObjectMember(_obj, "type", "type", typeid(_tmpVar).name(),
                                                               _tmpVar.type)).good()) {
        _stream.setStatus(_tmpStream.status());
        return _stream;
    }

    switch (_tmpVar.type) {
        case QDspx::Clip::Singing: {
            auto _realVar = QDspx::SingingClipRef::create();
            _var = _realVar;
            return _stream >> *_realVar.data();
        }
        case QDspx::Clip::Audio: {
            auto _realVar = QDspx::AudioClipRef::create();
            _var = _realVar;
            return _stream >> *_realVar.data();
        }
    }

    return _stream;
}

QAS::JsonStream &QDspx::operator<<(QAS::JsonStream &_stream, const QDspx::ClipRef &_var) {
    if (_var.isNull()) {
        return _stream;
    }

    return _var->type == QDspx::Clip::Singing ? (_stream << *_var.dynamicCast<QDspx::SingingClip>()) :
           (_stream << *_var.dynamicCast<QDspx::AudioClip>());
}