#ifndef FOO_H
#define FOO_H

#include "qjsonstream.h"

class Foo {
public:
    int foo = 1;
};

class Bar {
public:
    int bar = 2;
};

class Baz {
public:
    int baz = 3;
};

class Qux {
public:
    int qux = 4;
};

QAS_JSON_NS(Foo)
QAS_JSON_NS(Bar)
QAS_JSON_NS(Baz)
QAS_JSON_NS(Qux)

class Messed :  public                    Foo,
                protected __qas_include__ Bar,
                public    __qas_exclude__ Baz,
                protected                 Qux
{
public:
    QString token = "what";

    QAS_JSON(Messed)
};


#endif // FOO_H
