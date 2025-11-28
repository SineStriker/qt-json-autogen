# Qt Json Autogen

A build-time tool that automatically generate implementation for C++ class serialization/deserialization to/from json.

If your project is based on Qt, this tool may help a lot.

## Introduction

`qasc`(Qt Auto Serialization Compiler) is a code generator based on `moc` in Qt 5.15.2 source, it will generate the implementation of conversions between classes/enumerations and `QJsonObject`/`QString`.

In order to have as little association with the QObject framework as possible, `qasc` implements the conversion between enumeration and string itself.

### Enumeration

```c++
// Source
enum Direction {
    __qas_attr__("west")
    West,

    East,

    North,
    __qas_exclude__
    South,
}

QAS_JSON_NS(Direction)

int main() {
    qDebug() << qAsEnumToJson(Direction::West);
    qDebug() << qAsEnumToJson(Direction::East);
    qDebug() << qAsEnumToJson(Direction::North);
    qDebug() << qAsEnumToJson(Direction::South);
    return 0;
}
```
```sh
# Output
"west"
"East"
"North"
""
```
+ `QAS_JSON` tells `qasc` to generate the conversion implementations for class `Direction`, it also declare two stream operator functions in current scope. If the current scope is a class, use `QAS_JSON` instead, which will declare two friend functions.

+ `__qas_attr__("west")` tells `qasc` that the string form of enum value `West` should be `west`. The string form of enum values without this annotation are the original token in your code.

+ `__qas_exclude__` tells `qasc` that enum `South` won't be processed in conversion.

### Class/Struct
```c++
// Source
class Class {
public:
    class Student {
    public:
        enum Gender {
            Male,
            Female,
        };
        QAS_JSON(Gender);

        QString name;
        Gender gender;
        int age;
    };
    QAS_JSON(Student);

    __qas_attr__("Class")
    QString className;

    QMap<QString, Student> students; // id -> student

    __qas_exclude__
    QString otherInfo;
};
QAS_JSON_NS(Class)

int main() {
    Class cls;
    cls.className = "F114514";
    cls.students = {
        {"1", {"alice", Class::Student::Female, 18}},
        {"2", {"bob",   Class::Student::Male,   17}},
        {"3", {"mark",  Class::Student::Male,   19}},
    };
    cls.otherInfo = "PHP is the best programming language.";
    qDebug().noquote() << QJsonDocument(qAsClassToJson(cls)).toJson();
    return 0;
}
```

```sh
# Output
{
    "Class": "F114514",
    "students": {
        "1": {
            "age": 18,
            "gender": "Female",
            "name": "alice"
        },
        "2": {
            "age": 17,
            "gender": "Male",
            "name": "bob"
        },
        "3": {
            "age": 19,
            "gender": "Male",
            "name": "mark"
        }
    }
}
```
+ The usage of class is similar to enum.

+ Add a `__qas_attr__("XXX")` annotation before a declaration of a member to specify key when insert to a json object;

+ By default, the public members will participate in serialization/deserialization while the protected or private members won't.
    + Add a `__qas_exclude__` annotation before a public member to exclude it.
    + Add a `__qas_include__` annotation before a protected or private member to include it. When a non-public member partipates in serialization/deserialization, the `QAS_JSON` declaration must be right in the scope of the class, because only friend functions can access the non-public members.
        ```c++
        class Sample {
        public:
            QString foo;

        private:
            __qas_include__
            QString bar;
        
            QAS_JSON(Sample) // This declaration cannot be outside
        };
        ```

+ For a derived class, all its public derived super classes will participate in serialization/deserialization while the protected or private ones won't.
    + Add a `__qas_exclude__` annotation before a public super class name to exclude it.
    + Add a `__qas_include__` annotation before a protected or private super class name to include it. For same reason, you need to declare `QAS_JSON` in the derived class scope.
        ```c++
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
        ```

+ A class/struct can be serializable by `qasc` if
    + All its included super classes can be serializd into a json object
    + All its included members meets the following conditions:
        1. A serializable class
        2. A supported container of serialzable classes
            + List and map types in STL or Qt are supported
            + Use macro defined in `qas_json_types.h` to register your own container
        3. A serializable enumeration
+ Otherwise you may need to use `QAS_JSON_IMPL` or `QAS_JSON_NS_IMPL` to declare the functions(2 `QAS::JsonStream`'s stream operators to be overloaded) and implement them yourself.

+ The serializable class should have a parameterless constructor.

+ Initializing member variables in class definitions is not recommended, you should do it in constructor.
    ```c++
    // Not recommended
    struct Foo {
        int a = 1;
        char b{};
    };
    ```

+ So we can conclude that constant, reference, pointer members may need to be excluded.
+ Some useful util functions are provided in `qjsonstream.h`.

### Constraint Validation

The `__qas_constraint__` feature allows you to define validation rules for JSON deserialization using a declarative syntax. Constraints are automatically validated during JSON to C++ object conversion.

#### Usage Syntax
```cpp
__qas_constraint__(<constraint_type> <value> [<constraint_type> <value>...])
```

+ Multiple constraints within the same `__qas_constraint__` declaration have an AND relationship.
+ Multiple `__qas_constraint__` declarations for the same field have an OR relationship.

#### Supported Constraint Types

**Range Constraints (Numeric Values)**
- `MINIMUM <value>` - Value must be >= value
- `MAXIMUM <value>` - Value must be <= value  
- `EXCLUSIVE_MINIMUM <value>` - Value must be > value
- `EXCLUSIVE_MAXIMUM <value>` - Value must be < value

```cpp
__qas_constraint__(MINIMUM 0 MAXIMUM 100)
int score;
```

**Equality Constraints**
- `CONST <json_value>` - Value must equal the specified constant
- `ENUM [<json_array>]` - Value must be one of the specified values

```cpp
__qas_constraint__(CONST "1.0.0")
QString version;

__qas_constraint__(ENUM ["red", "green", "blue"])
QString color;
```

**String Length Constraints**
- `MIN_LENGTH <integer>` - String length must be >= value
- `MAX_LENGTH <integer>` - String length must be <= value

```cpp
__qas_constraint__(MIN_LENGTH 3 MAX_LENGTH 20)
QString username;
```

**Pattern Constraint**
- `PATTERN "<regex_pattern>"` - String must match the regular expression

```cpp
__qas_constraint__(PATTERN "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$")
QString email;
```

#### Error Handling

When constraint validation fails during deserialization:
- `JsonStream::ConstraintViolation` status is set
- Deserialization stops immediately
- The `Failed` mask includes `ConstraintViolation` for easy error checking

#### Example

```cpp
class UserProfile {
public:
    __qas_constraint__(MIN_LENGTH 3 MAX_LENGTH 20)
    QString username;
    
    __qas_constraint__(MINIMUM 13 MAXIMUM 120)
    int age;
    
    __qas_constraint__(PATTERN "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$")
    QString email;
    
    __qas_constraint__(ENUM ["admin", "user", "guest"])
    QString role;
};
QAS_JSON_NS(UserProfile)

// During deserialization, if any constraint is violated,
// JsonStream::ConstraintViolation status will be set
```

## Supported Types

| C++ Type                                                                     | JSON Type    |
| ---------------------------------------------------------------------------- |:------------:|
| custom structures/classes                                                    | object       |
| `bool`                                                                       | true/false   |
| signed and unsigned integral types                                           | number       |
| `float` and `double`                                                         | number       |
| `enum` and `enum class`                                                      | string       |
| `QString`                                                                    | string       |
| iteratable lists (`QVector`, `QList`, `std::vector`, `std::list`)            | array        |
| sets (`QSet`, `std::set`, `std::unordered_set`)                              | array        |
| map (`QMap`, `QHash`, `std::map`, `std::unordered_map`)                      | object       |

## How To Use
### Add Into CMake Project

+ Example `CMakeLists.txt`
    ```cmake
    find_package(qastool REQUIRED)
    include_directories(${QASTOOL_INCLUDE_DIRS})

    set(_src ...            ) # Add source files
    set(_headers_for_qas ...) # Headers using QAS macros

    # Specify your target, for example an exe
    add_executable(${YOUR_TARGET} ${_src})
    
    # Tell qasc to preprocess your headers
    qas_wrap_cpp(_qasc_src ${_headers_for_qas} TARGET ${YOUR_TARGET})
    
    # Add auto generated sources to target
    target_sources(${YOUR_TARGET} PRIVATE ${_qasc_src})
    ```

+ `qas_wrap_cpp` simply adds a series of command to generate the extra source files containing the implementations, and return the extra sources list to variable `_qasc_src`, you need to add them to the target.

+ The codes in generated source files need a number of functions defined in `qjsonstream.h`.

## Details

### CMake Identifiers Intro

#### qas_wrap_cpp
```
qas_wrap_cpp(<VAR> src_file1 [src_file2 ...]
            [TARGET target]
            [OPTIONS ...]
            [DEPENDS ...])
```
+ This macro is modified from `qt5_wrap_cpp` in Qt cmake modules.
+ Creates rules for calling the Qt Auto Serialization Compiler (qasc) on the given source files. For each input file, an output file is generated in the build directory. The paths of the generated files are added to `<VAR>`.
+ You can set an explicit `TARGET`. This will make sure that the target properties `INCLUDE_DIRECTORIES` and `COMPILE_DEFINITIONS` are also used when scanning the source files with qasc.
+ You can set additional `OPTIONS` that should be added to the qasc calls. You can find possible options in the qasc documentation.
+ `DEPENDS` allows you to add additional dependencies for recreation of the generated files. This is useful when the sources have implicit dependencies.

### QASC Tool

+ `qasc` is a lexical analyzer modified from `moc` in Qt5 tools, it preprocesses source files before building. If any one of the following identifiers appears in a given source file, `qasc` will generate implementations of serializers and deserializers into `stdout` or a new file.
    + `QAS_JSON`
    + `QAS_JSON_NS`

+ `qasc` has been tested when in Qt6 framework, it works fine.

## Acknowledgements

+ [moc](https://github.com/qt/qtbase/tree/dev/src/tools/moc)
    + Qt Meta Object Compiler (Qt 5.15.2)

## License

+ The moc compiler is a part of Qt sources which is released under GNU LGPL v2.1 or later, which `qasc` inherited. If you modify any codes in `qasc`'s source files, you need to make your modification open source as the LGPL requires. (Important!!!)

+ Other codes in this project are released under Apache 2.0 License.