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
    // void generateUsing(const QByteArray &qualified);

    void generateEnums(const QByteArray &ns, const QByteArray &qualified, const EnumDef &def);

    void generateClass(const QByteArray &ns, const QByteArray &qualified,
                       const QByteArrayList &supers, const ClassDef &def);
                       
    // Generate constraint validation code
    void generateConstraintValidation(const QByteArray &fieldName, 
                                     const QVector<ConstraintGroup> &constraintGroups);
    void generateSingleConstraintCheck(const Constraint &constraint);
};

#endif // GENERATOR_H
