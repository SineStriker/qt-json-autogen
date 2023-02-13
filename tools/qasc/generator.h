#ifndef GENERATOR_H
#define GENERATOR_H

#include "qasc.h"

class Generator {
    Environment *rootEnv;
    FILE *fp;

public:
    explicit Generator(Environment *env, FILE *outfile) : rootEnv(env), fp(outfile){};

    void generateCode();

private:
    void generateEnums(const QByteArray &qualified, const EnumDef &def);

    void generateClass(const QByteArray &qualified, const ClassDef &def);
};



#endif // GENERATOR_H
