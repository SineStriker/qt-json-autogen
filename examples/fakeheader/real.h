#ifndef REAL_H
#define REAL_H

#include "common.h"

namespace NS1 {


    struct Foo : Bar {
        Foo &operator<<(int n);
    };


}

#endif // REAL_H
