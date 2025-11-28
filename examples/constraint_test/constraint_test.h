#ifndef CONSTRAINT_TEST_H
#define CONSTRAINT_TEST_H

#include <utility>

#include "qjsonstream.h"

// Test enum with attributes
enum class UserRole {
    __qas_attr__("admin")
    Administrator,
    
    __qas_attr__("user") 
    RegularUser,
    
    Manager,  // Uses original name
    
    __qas_exclude__
    SystemAdmin  // This won't be serialized
};

// Test enum for Gender
enum Gender {
    Male,
    Female,
    __qas_attr__("other")
    Other
};

// Base class with some fields
class BaseInfo {
public:
    __qas_attr__("created_at")
    __qas_constraint__(PATTERN "^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}Z$")
    QString createdAt;
    
    __qas_constraint__(MINIMUM 1)
    int id;

protected:
    __qas_include__
    __qas_attr__("internal_code")
    __qas_constraint__(MIN_LENGTH 5 MAX_LENGTH 10)
    QString internalCode;

private:
    __qas_include__
    __qas_constraint__(ENUM ["v1", "v2", "v3"])
    QString version = "v1";
    
    QString excludedData;  // Private, not included
    
    QAS_JSON(BaseInfo)  // Must be in class scope for private/protected members
};

// Test struct with various constraint and attribute combinations
class TestStruct : public BaseInfo {
public:
    // Numeric range constraints with custom attribute name
    __qas_attr__("user_score")
    __qas_constraint__(MINIMUM 0 MAXIMUM 100)
    int score;
    
    // String length constraints with custom attribute
    __qas_attr__("full_name")
    __qas_constraint__(MIN_LENGTH 3 MAX_LENGTH 20)
    QString name;
    
    // Pattern constraint for email with custom attribute
    __qas_attr__("email_address")
    __qas_constraint__(PATTERN "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$")
    QString email;
    
    // Enum constraint for status with custom attribute
    __qas_attr__("account_status")
    __qas_constraint__(ENUM ["active", "inactive", "pending"])
    QString status;
    
    // Const constraint for version with custom attribute
    __qas_attr__("api_version")
    __qas_constraint__(CONST "1.0.0")
    QString version;
    
    // Multiple constraint groups (OR relationship) with custom attribute
    __qas_attr__("user_age")
    __qas_constraint__(MINIMUM 18)
    __qas_constraint__(CONST -1)  // Special case for unknown age
    int age;
    
    // Exclusive range constraints with custom attribute
    __qas_attr__("success_rate")
    __qas_constraint__(EXCLUSIVE_MINIMUM 0.0 EXCLUSIVE_MAXIMUM 1.0)
    double probability;

    // Enum field with constraint and attribute
    __qas_attr__("role")
    UserRole userRole;
    
    // Gender enum without constraint but with attribute
    __qas_attr__("user_gender")
    Gender gender;
    
    // Public field marked for exclusion
    __qas_exclude__
    QString publicButExcluded = "should not appear in JSON";
    
    // No constraint, no custom attribute
    int someValue;

    void setInternalCode(QString code) { internalCode = std::move(code); }
    void setAccessToken(QString token) { accessToken = std::move(token); }

protected:
    // Protected field included with constraint and attribute
    __qas_include__
    __qas_attr__("secret_level")
    __qas_constraint__(MINIMUM 1 MAXIMUM 5)
    int securityLevel = 1;

private:
    // Private field included with multiple constraints
    __qas_include__
    __qas_attr__("access_token")
    __qas_constraint__(MIN_LENGTH 32 MAX_LENGTH 64)
    __qas_constraint__(PATTERN "^[a-zA-Z0-9]+$")  // OR relationship with above
    QString accessToken;
    
    // Private field not included
    QString privateData = "secret";
    
    QAS_JSON(TestStruct)  // Must be in class scope for private/protected members
};

// Test class with inheritance constraints
class DerivedTest : public __qas_exclude__ BaseInfo  // Exclude base class
{
public:
    __qas_attr__("derived_field")
    __qas_constraint__(CONST "derived")
    QString derivedField;
};

// Test class with multiple inheritance
class MultipleBase1 {
public:
    __qas_attr__("base1_field")
    QString field1;
};

class MultipleBase2 {
public:
    __qas_attr__("base2_field")
    int field2;
};

class MultipleInheritanceTest : public MultipleBase1, 
                               protected __qas_include__ MultipleBase2
{
public:
    __qas_attr__("multi_field")
    __qas_constraint__(MIN_LENGTH 1)
    QString multiField;

    inline void setField2(int val) { field2 = val; }  // Accessor for protected base field
    
    QAS_JSON(MultipleInheritanceTest)  // Need this for protected inheritance access
};

// Nested class test
class OuterClass {
public:
    class NestedClass {
    public:
        __qas_attr__("nested_value")
        __qas_constraint__(MINIMUM 0)
        int value;
    };
    
    QAS_JSON(NestedClass)  // For nested class, placed in outer class
    
    __qas_attr__("nested_obj")
    NestedClass nested;
};

QAS_JSON_NS(UserRole)
QAS_JSON_NS(Gender)
QAS_JSON_NS(DerivedTest)
QAS_JSON_NS(MultipleBase1)
QAS_JSON_NS(MultipleBase2)
QAS_JSON_NS(OuterClass)

#endif // CONSTRAINT_TEST_H