# Qt Auto Serialization

A compile time tool to generate serialization source codes for class serialization.

## Introduction

### Supported Types

+ Enumeration
+ Class

### Libraries Used

+ [moc](https://github.com/qt/qtbase/tree/dev/src/tools/moc)
    + Qt Meta Object Compiler (Qt 5.15.2)

## Functionalities

### Json

Generate json serializer and deserializer based on QJsonObject

### Enumeration

Generate enumeration converter (to and from QString)

## Usage

### Add Into CMake Project

+ Build `qasc`

+ Get `QasBatch.cmake` and `QasUtils.cmake` from the repository

+ Get `include` directory from the repository

+ Edit the header of `QasUtils.cmake`
    ```sh
    set(QASC_COMMAND <command> CACHE STRING "Qasc command" FORCE)
    set(QAS_BATCH_SCRIPT <script> CACHE STRING "Qas batch script" FORCE)
    ```
    + Set `<command>` as the absolute path of `qasc`
    + Set `<script>` as the absolute path of `QasBatch.cmake`

+ Edit your `CMakeLists.txt`
    ```cmake
    include(/path/to/QasUtils.cmake)

    include_directories(/path/to/include)

    # Add target

    qas_auto_gen(${YOUR_TARGET})
    ```

### Enumeration Serialization

```c++
enum Name {
    QAS_ATTRIBUTE("alice")
    Alice,

    Bob,

    QAS_IGNORE
    Mark,
};

QAS_ENUM_DECLARE(Name)
```
+ Use `QAS_ENUM_DECLARE` to define the serializer and deserializer of the enumeration, then it becomes serializable.
    ```c++
    Name QASEnumType<Name>::fromString(const QString &s, bool *ok = nullptr);
    QString QASEnumType<Name>::toString(Name e);
    ```

+ The string format of a enumeration value is same as the one in definition, use `QAS_ATTRIBUTE` to override it.

+ If a enumeration value is marked with `QAS_IGNORE`, then it will be transparent during serialization.

+ These macros also tell the `qasc` compiler to generate the implementation of the two required functions automatically.

```c++
qDebug() << QASEnumType<Name>::toString(Name::Alice);
qDebug() << QASEnumType<Name>::toString(Name::Bob);
```
```sh
Alice
bob
```

### Class Serialization

```c++
class Classroom {
public:
    class Student {
    public:
        enum Gender {
            Male,
            Female,
        };

        QString name;
        Gender gender;
        int height;
    };

    QString className;
    QString slogan;
    QList<Student> students;
};

QAS_ENUM_DECLARE(Classroom::Student::Gender)
QAS_JSON_DECLARE(Classroom::Student)
QAS_JSON_DECLARE(Classroom)
```
+ Use `QAS_JSON_DECLARE` to define the serializer and deserializer of the class, then it becomes serializable.
    + Template classes or classes in the scope of a template class with more than one type are not supported.

+ If the class is a derived class, only super classes specified after the class will participate in serialization and deserialization. The super class name should be in complete form in global scope.
    ```c++
    namespace NS {
        class Base { ... }
        class Derived : public Base { ... }
    }
    QAS_JSON_DECLARE(NS::Derived, NS::Base)
    ```
    + Note that serializable super class should be publicly inherited.

+ Use `QAS_ATTRIBUTE` to specify the key of a member in json object ans `QAS_IGNORE` to ignore a member. Only public members participate in serialization and deserialization.

+ The member to participating in serialization and deserialization should be one of the following 3:
    1. A serializable class
    2. A supported container
        + List and map types in STL or Qt are supported
        + Use macro defined in `qas_json_types.h` to register your own container
    3. A serializable enumeration

+ The class must have a default constructor.

+ The `QAS_ENUM_DECLARE` and `QAS_JSON_DECLARE` must be used in the global scope, and the class name or enumeration name in the paren must be the original name after macro expansion, it cannot be a alias defined by `using` or `typedef`.

+ If there's a class having a conditional serialization method which is too complex for `qasc` to resolve, you need to use `QAS_JSON_DECLARE_IMPL` to define the serializer and deserializer and implement them yourself.

    ```c++
    Classroom cr;
    cr.className = "201";
    cr.slogan = "Keep calm and carry on.";

    Classroom::Student alice;
    alice.name = "Alice";
    alice.gender = Classroom::Student::Female;
    alice.height = 165;

    Classroom::Student bob;
    bob.name = "Bob";
    bob.gender = Classroom::Student::Male;
    bob.height = 180;

    cr.students = {alice, bob};

    QJsonDocument doc(QASJsonType<Classroom>::toObject(cr));
    qDebug().noquote() << doc.toJson();
    ```
    ```json
    {
        "className": "201",
        "slogan": "Keep calm and carry on.",
        "students": [
            {
                "height": 165,
                "gender": "Female",
                "name": "Alice"
            },
            {
                "height": 180,
                "gender": "Male",
                "name": "Bob"
            }
        ]
    }
    ```