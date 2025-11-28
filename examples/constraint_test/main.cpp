#include <QCoreApplication>
#include <QJsonDocument>
#include <QDebug>

#include "constraint_test.h"

void printSeparator(const QString& title) {
    qDebug() << QString("=== %1 ===").arg(title);
}

void testEnumWithAttributes() {
    printSeparator("Testing Enum with Attributes");
    
    // Test enum serialization
    qDebug() << "UserRole::Administrator =>" << qAsEnumToJson(UserRole::Administrator);  // Should be "admin"
    qDebug() << "UserRole::RegularUser =>" << qAsEnumToJson(UserRole::RegularUser);      // Should be "user"
    qDebug() << "UserRole::Manager =>" << qAsEnumToJson(UserRole::Manager);              // Should be "Manager"
    
    qDebug() << "Gender::Male =>" << qAsEnumToJson(Gender::Male);        // Should be "Male"
    qDebug() << "Gender::Female =>" << qAsEnumToJson(Gender::Female);    // Should be "Female"
    qDebug() << "Gender::Other =>" << qAsEnumToJson(Gender::Other);      // Should be "other"
}

void testBasicInheritance() {
    printSeparator("Testing Base Class with Include/Exclude");
    
    BaseInfo baseObj;
    baseObj.createdAt = "2023-10-16T10:30:00Z";  // Must match ISO pattern
    baseObj.id = 123;  // Must be >= 1
    // Note: protected and private fields can't be set directly from outside
    
    QJsonObject jsonObj = qAsClassToJson(baseObj);
    qDebug() << "BaseInfo JSON:" << QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);
    
    // Should contain: created_at, id, internal_code, version
    // Should NOT contain: excludedData
}

void testComplexStructure() {
    printSeparator("Testing Complex Structure with All Annotations");
    
    TestStruct obj;
    // Set BaseInfo fields (inherited)
    obj.createdAt = "2023-10-16T10:30:00Z";  // Must match ISO pattern
    obj.id = 123;  // Must be >= 1
    obj.setInternalCode("CODE123");
    // Set TestStruct fields
    obj.score = 85;  // Must be 0-100
    obj.name = "Alice Smith";  // Must be 3-20 chars
    obj.email = "alice.smith@example.com";  // Must match email pattern
    obj.status = "active";  // Must be in ["active", "inactive", "pending"]
    obj.version = "1.0.0";  // Must be exactly "1.0.0"
    obj.age = 25;  // Must be >= 18 or == -1
    obj.probability = 0.75;  // Must be > 0.0 and < 1.0
    obj.userRole = UserRole::Administrator;
    obj.gender = Gender::Female;
    obj.someValue = 42;
    obj.publicButExcluded = "This should not appear";
    obj.setAccessToken("abcdef1234567890abcdef1234567890");
    
    QJsonObject jsonObj = qAsClassToJson(obj);
    qDebug() << "TestStruct JSON:";
    qDebug().noquote() << QJsonDocument(jsonObj).toJson();
    
    // Verify custom attribute names are used
    if (jsonObj.contains("user_score") && !jsonObj.contains("score")) {
        qDebug() << "[OK] Custom attribute name 'user_score' is used";
    } else {
        qDebug() << "[FAIL] Custom attribute name 'user_score' not used correctly";
    }
    
    if (jsonObj.contains("email_address") && !jsonObj.contains("email")) {
        qDebug() << "[OK] Custom attribute name 'email_address' is used";
    } else {
        qDebug() << "[FAIL] Custom attribute name 'email_address' not used correctly";
    }
    
    if (jsonObj.contains("role")) {
        QString roleValue = jsonObj["role"].toString();
        if (roleValue == "admin") {
            qDebug() << "[OK] Enum attribute mapping works correctly";
        } else {
            qDebug() << "[FAIL] Enum attribute mapping failed, got:" << roleValue;
        }
    }
    
    if (!jsonObj.contains("publicButExcluded")) {
        qDebug() << "[OK] Excluded public field not serialized";
    } else {
        qDebug() << "[FAIL] Excluded public field was serialized";
    }
}

void testValidConstraints() {
    printSeparator("Testing Valid Constraints with Custom Attributes");
    
    TestStruct validObj;
    // Set BaseInfo fields (inherited)
    validObj.createdAt = "2023-10-16T10:30:00Z";  // Must match ISO pattern
    validObj.id = 123;  // Must be >= 1
    validObj.setInternalCode("CODE123");
    // Set TestStruct fields
    validObj.score = 85;  // Must be 0-100
    validObj.name = "Alice";  // Must be 3-20 chars
    validObj.email = "alice@example.com";  // Must match email pattern
    validObj.status = "active";  // Must be in ["active", "inactive", "pending"]
    validObj.version = "1.0.0";  // Must be exactly "1.0.0"
    validObj.age = 25;  // Must be >= 18 or == -1
    validObj.probability = 0.75;  // Must be > 0.0 and < 1.0
    validObj.userRole = UserRole::RegularUser;
    validObj.gender = Gender::Female;
    validObj.someValue = 42;
    validObj.setAccessToken("abcdef1234567890abcdef1234567890");
    
    QJsonObject jsonObj = qAsClassToJson(validObj);
    qDebug() << "Valid object JSON:";
    qDebug().noquote() << QJsonDocument(jsonObj).toJson();
    
    TestStruct deserializedObj;
    QAS::JsonStream stream(jsonObj);
    stream >> deserializedObj;
    
    if (stream.good()) {
        qDebug() << "[OK] Valid constraints test passed";
        qDebug() << "Deserialized - score:" << deserializedObj.score << ", name:" << deserializedObj.name;
    } else {
        qDebug() << "[FAIL] Valid constraints test failed with status:" << stream.status();
    }
}

void testInvalidConstraints() {
    printSeparator("Testing Invalid Constraints");
    
    struct TestCase {
        QJsonObject json;
        QString description;
    };
    
    TestCase testCases[] = {
        // Score out of range (using custom attribute name)
        {QJsonObject{
            {"created_at", "2023-10-16T10:30:00Z"},
            {"id", 123},
            {"user_score", 150},  // Invalid: > 100
            {"full_name", "Alice"},
            {"email_address", "alice@example.com"},
            {"account_status", "active"},
            {"api_version", "1.0.0"},
            {"user_age", 25},
            {"success_rate", 0.75},
            {"role", "admin"},
            {"user_gender", "Female"},
            {"someValue", 42},
            {"secret_level", 3},
            {"access_token", "abcdef1234567890abcdef1234567890"},
            {"internal_code", "CODE123"},
            {"version", "v1"}
        }, "Score out of range"},
        
        // Name too short (using custom attribute name)
        {QJsonObject{
            {"created_at", "2023-10-16T10:30:00Z"},
            {"id", 123},
            {"user_score", 85},
            {"full_name", "Al"},  // Invalid: length < 3
            {"email_address", "alice@example.com"},
            {"account_status", "active"},
            {"api_version", "1.0.0"},
            {"user_age", 25},
            {"success_rate", 0.75},
            {"role", "admin"},
            {"user_gender", "Female"},
            {"someValue", 42},
            {"secret_level", 3},
            {"access_token", "abcdef1234567890abcdef1234567890"},
            {"internal_code", "CODE123"},
            {"version", "v1"}
        }, "Name too short"},
        
        // Invalid email pattern (using custom attribute name)
        {QJsonObject{
            {"created_at", "2023-10-16T10:30:00Z"},
            {"id", 123},
            {"user_score", 85},
            {"full_name", "Alice"},
            {"email_address", "invalid-email"},  // Invalid: doesn't match pattern
            {"account_status", "active"},
            {"api_version", "1.0.0"},
            {"user_age", 25},
            {"success_rate", 0.75},
            {"role", "admin"},
            {"user_gender", "Female"},
            {"someValue", 42},
            {"secret_level", 3},
            {"access_token", "abcdef1234567890abcdef1234567890"},
            {"internal_code", "CODE123"},
            {"version", "v1"}
        }, "Invalid email pattern"},
        
        // Invalid status enum (using custom attribute name)
        {QJsonObject{
            {"created_at", "2023-10-16T10:30:00Z"},
            {"id", 123},
            {"user_score", 85},
            {"full_name", "Alice"},
            {"email_address", "alice@example.com"},
            {"account_status", "unknown"},  // Invalid: not in enum
            {"api_version", "1.0.0"},
            {"user_age", 25},
            {"success_rate", 0.75},
            {"role", "admin"},
            {"user_gender", "Female"},
            {"someValue", 42},
            {"secret_level", 3},
            {"access_token", "abcdef1234567890abcdef1234567890"},
            {"internal_code", "CODE123"},
            {"version", "v1"}
        }, "Invalid status enum"},
        
        // Wrong version constant (using custom attribute name)
        {QJsonObject{
            {"created_at", "2023-10-16T10:30:00Z"},
            {"id", 123},
            {"user_score", 85},
            {"full_name", "Alice"},
            {"email_address", "alice@example.com"},
            {"account_status", "active"},
            {"api_version", "2.0.0"},  // Invalid: doesn't match const
            {"user_age", 25},
            {"success_rate", 0.75},
            {"role", "admin"},
            {"user_gender", "Female"},
            {"someValue", 42},
            {"secret_level", 3},
            {"access_token", "abcdef1234567890abcdef1234567890"},
            {"internal_code", "CODE123"},
            {"version", "v1"}
        }, "Wrong version constant"},
        
        // Invalid role enum
        {QJsonObject{
            {"created_at", "2023-10-16T10:30:00Z"},
            {"id", 123},
            {"user_score", 85},
            {"full_name", "Alice"},
            {"email_address", "alice@example.com"},
            {"account_status", "active"},
            {"api_version", "1.0.0"},
            {"user_age", 25},
            {"success_rate", 0.75},
            {"role", "invalid_role"},  // Invalid: not in constraint enum
            {"user_gender", "Female"},
            {"someValue", 42},
            {"secret_level", 3},
            {"access_token", "abcdef1234567890abcdef1234567890"},
            {"internal_code", "CODE123"},
            {"version", "v1"}
        }, "Invalid role enum"},
        
        // Age too young (not -1)
        {QJsonObject{
            {"created_at", "2023-10-16T10:30:00Z"},
            {"id", 123},
            {"user_score", 85},
            {"full_name", "Alice"},
            {"email_address", "alice@example.com"},
            {"account_status", "active"},
            {"api_version", "1.0.0"},
            {"user_age", 16},  // Invalid: < 18 and != -1
            {"success_rate", 0.75},
            {"role", "admin"},
            {"user_gender", "Female"},
            {"someValue", 42},
            {"secret_level", 3},
            {"access_token", "abcdef1234567890abcdef1234567890"},
            {"internal_code", "CODE123"},
            {"version", "v1"}
        }, "Age too young"},
        
        // Probability out of exclusive range
        {QJsonObject{
            {"created_at", "2023-10-16T10:30:00Z"},
            {"id", 123},
            {"user_score", 85},
            {"full_name", "Alice"},
            {"email_address", "alice@example.com"},
            {"account_status", "active"},
            {"api_version", "1.0.0"},
            {"user_age", 25},
            {"success_rate", 1.0},  // Invalid: not < 1.0 (exclusive)
            {"role", "admin"},
            {"user_gender", "Female"},
            {"someValue", 42},
            {"secret_level", 3},
            {"access_token", "abcdef1234567890abcdef1234567890"},
            {"internal_code", "CODE123"},
            {"version", "v1"}
        }, "Probability out of range"}
    };
    
    for (const auto& testCase : testCases) {
        TestStruct testObj;
        QAS::JsonStream stream(testCase.json);
        stream >> testObj;
        if ((stream.status() & QAS::JsonStream::ConstraintViolation) || (stream.status() & QAS::JsonStream::UnlistedValue)) {
            qDebug() << "[OK]" << testCase.description << "correctly failed constraint validation";
        } else {
            qDebug() << "[FAIL]" << testCase.description << "should have failed but didn't, status:" << stream.status();
        }
    }
}

void testSpecialCases() {
    printSeparator("Testing Special Cases (OR constraints)");
    
    // Test age = -1 (should be valid due to OR constraint)
    QJsonObject specialAgeObj{
        {"created_at", "2023-10-16T10:30:00Z"},
        {"id", 123},
        {"user_score", 85},
        {"full_name", "Alice"},
        {"email_address", "alice@example.com"},
        {"account_status", "active"},
        {"api_version", "1.0.0"},
        {"user_age", -1},  // Valid: matches CONST -1 constraint
        {"success_rate", 0.75},
        {"role", "admin"},
        {"user_gender", "Female"},
        {"someValue", 42},
        {"secret_level", 3},
        {"access_token", "abcdef1234567890abcdef1234567890"},
        {"internal_code", "CODE123"},
        {"version", "v1"}
    };
    
    TestStruct testObj;
    QAS::JsonStream stream(specialAgeObj);
    stream >> testObj;
    
    if (stream.good()) {
        qDebug() << "[OK] Special age case (-1) correctly passed validation";
    } else {
        qDebug() << "[FAIL] Special age case (-1) should have passed, status:" << stream.status();
    }
}

void testDerivedClass() {
    printSeparator("Testing Derived Class with Excluded Base");
    
    DerivedTest derived;
    derived.derivedField = "derived";
    
    QJsonObject jsonObj = qAsClassToJson(derived);
    qDebug() << "DerivedTest JSON:" << QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);
    
    // Should only contain derivedField, not base class fields
    if (jsonObj.contains("derived_field") && !jsonObj.contains("created_at") && !jsonObj.contains("id")) {
        qDebug() << "[OK] Base class correctly excluded from derived class";
    } else {
        qDebug() << "[FAIL] Base class exclusion not working properly";
    }
}

void testMultipleInheritance() {
    printSeparator("Testing Multiple Inheritance");
    
    MultipleInheritanceTest multi;
    multi.field1 = "base1_value";
    multi.setField2(100);
    multi.multiField = "multi_value";
    
    QJsonObject jsonObj = qAsClassToJson(multi);
    qDebug() << "MultipleInheritanceTest JSON:" << QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);
    
    // Should contain field1 (public base), field2 (protected but included), and multiField
    bool hasBase1 = jsonObj.contains("base1_field");
    bool hasBase2 = jsonObj.contains("base2_field");
    bool hasMulti = jsonObj.contains("multi_field");
    
    if (hasBase1 && hasBase2 && hasMulti) {
        qDebug() << "[OK] Multiple inheritance with include/exclude works correctly";
    } else {
        qDebug() << "[FAIL] Multiple inheritance not working properly - base1:" << hasBase1 << "base2:" << hasBase2 << "multi:" << hasMulti;
    }
}

void testNestedClass() {
    printSeparator("Testing Nested Class");
    
    OuterClass outer;
    outer.nested.value = 42;
    
    QJsonObject jsonObj = qAsClassToJson(outer);
    qDebug() << "OuterClass JSON:" << QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);
    
    if (jsonObj.contains("nested_obj")) {
        QJsonObject nestedObj = jsonObj["nested_obj"].toObject();
        if (nestedObj.contains("nested_value") && nestedObj["nested_value"].toInt() == 42) {
            qDebug() << "[OK] Nested class serialization works correctly";
        } else {
            qDebug() << "[FAIL] Nested class serialization failed";
        }
    } else {
        qDebug() << "[FAIL] Nested object not found in JSON";
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "Qt JSON Autogen Comprehensive Test";
    qDebug() << "===================================";
    
    testEnumWithAttributes();
    testBasicInheritance();
    testComplexStructure();
    testValidConstraints();
    testInvalidConstraints();
    testSpecialCases();
    testDerivedClass();
    testMultipleInheritance();
    testNestedClass();
    
    qDebug() << "\nAll tests completed.";
    
    return 0;
}