#include "QDspxTrack.h"

QDspx::ClipRef QASJsonType<QDspx::ClipRef>::fromObject(const QJsonObject &obj, bool *ok) {
    auto it = obj.find("type");
    if (it == obj.end()) {
        return {};
    }
    QString type = it->toString().toLower();
    if (type == "singing") {
        return QDspx::SingingClipRef::create(QASJsonType<QDspx::SingingClip>::fromObject(obj, ok));
    } else if (type == "audio") {
        return QDspx::AudioClipRef::create(QASJsonType<QDspx::AudioClip>::fromObject(obj, ok));
    }
    return {};
}

QJsonObject QASJsonType<QDspx::ClipRef>::toObject(const QDspx::ClipRef &cls) {
    if (cls.isNull()) {
        return {};
    }
    return cls->type == QDspx::Clip::Singing
               ? QASJsonType<QDspx::SingingClip>::toObject(*cls.dynamicCast<QDspx::SingingClip>())
               : QASJsonType<QDspx::AudioClip>::toObject(*cls.dynamicCast<QDspx::AudioClip>());
}