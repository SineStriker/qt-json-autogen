#include "QDspxParam.h"

QDspx::ParamCurveRef QASJsonType<QDspx::ParamCurveRef>::fromObject(const QJsonObject &obj, bool *ok) {
    auto it = obj.find("type");
    if (it == obj.end()) {
        return {};
    }
    QString type = it->toString().toLower();
    if (type == "anchor") {
        return QDspx::ParamAnchorRef::create(QASJsonType<QDspx::ParamAnchor>::fromObject(obj, ok));
    } else if (type == "free") {
        return QDspx::ParamFreeRef::create(QASJsonType<QDspx::ParamFree>::fromObject(obj, ok));
    }
    return {};
}

QJsonObject QASJsonType<QDspx::ParamCurveRef>::toObject(const QDspx::ParamCurveRef &cls) {
    if (cls.isNull()) {
        return {};
    }
    return cls->type == QDspx::ParamCurve::Anchor
               ? QASJsonType<QDspx::ParamAnchor>::toObject(*cls.dynamicCast<QDspx::ParamAnchor>())
               : QASJsonType<QDspx::ParamFree>::toObject(*cls.dynamicCast<QDspx::ParamFree>());
}