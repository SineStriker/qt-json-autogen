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