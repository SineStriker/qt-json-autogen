#ifndef QAS_ENUM_TYPES_H
#define QAS_ENUM_TYPES_H

#define QAS_ENUM_TYPE_SERIALIZER

#define QAS_ENUM_TYPE_DESERIALIZER

template <class T>
struct QASEnumTypeDefault {
    enum {
        Defined = 0,
    };
};

template <class T>
struct QASEnumType : public QASEnumTypeDefault<T> //
{                                                 //
};

#endif // QAS_ENUM_TYPES_H
