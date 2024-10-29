#ifndef JSON_H
#define JSON_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <memory>

#ifndef Json_Default_IndentLevel
#define Json_Default_IndentLevel 0
#endif

#ifndef Json_Default_IndentSize
#define Json_Default_IndentSize  0
#endif

namespace Json {
    class JsonArray;
    class JsonObject;

    enum class Type {
        Null,
        Boolean,
        Number,
        String,
        Object,
        Array
    };

    class JsonValue {
    public:
        virtual ~JsonValue() = default;
        virtual Type type() const = 0;
        virtual std::string serialize(int indent_level = Json_Default_IndentLevel, int indent_size = Json_Default_IndentSize) const = 0;
        virtual std::string asString() const { throw std::runtime_error("Not a string"); }
        virtual double asNumber() const { throw std::runtime_error("Not a number"); }
        virtual bool asBoolean() const { throw std::runtime_error("Not a boolean"); }
        virtual std::shared_ptr<JsonArray> asArray() const { throw std::runtime_error("Not an array"); }
        virtual std::shared_ptr<JsonObject> asObject() const { throw std::runtime_error("Not an object"); }
    };

    class JsonNull : public JsonValue {
    public:
        Type type() const override { return Type::Null; }
        std::string serialize(int indent_level = Json_Default_IndentLevel, int indent_size = Json_Default_IndentSize) const override { return "null"; }
    };

    class JsonBoolean : public JsonValue {
    public:
        explicit JsonBoolean(bool value) : value_(value) {}
        Type type() const override { return Type::Boolean; }
        std::string serialize(int indent_level = Json_Default_IndentLevel, int indent_size = Json_Default_IndentSize) const override { return value_ ? "true" : "false"; }
        bool asBoolean() const override { return value_; } 
    protected:
        bool value_;
    };

    class JsonNumber : public JsonValue {
    public:
        explicit JsonNumber(double value) : value_(value) {}
        Type type() const override { return Type::Number; }
        std::string serialize(int indent_level = Json_Default_IndentLevel, int indent_size = Json_Default_IndentSize) const override;
        double asNumber() const override { return value_; } 
    protected:
        double value_;
    };

    class JsonString : public JsonValue {
    public:
        explicit JsonString(const std::string& value) : value_(value) {}
        Type type() const override { return Type::String; }
        std::string serialize(int indent_level = Json_Default_IndentLevel, int indent_size = Json_Default_IndentSize) const override;
        std::string asString() const override { return value_; }
    protected:
        std::string value_;
    };

    class JsonArray : public JsonValue, public std::enable_shared_from_this<JsonArray> {
    public:
        JsonArray() = default;
        void add(std::shared_ptr<JsonValue> value) { values_.emplace_back(std::move(value)); }
        Type type() const override { return Type::Array; }
        std::string serialize(int indent_level = Json_Default_IndentLevel, int indent_size = Json_Default_IndentSize) const override;
        std::shared_ptr<JsonArray> asArray() const override { return std::const_pointer_cast<JsonArray>(shared_from_this()); }
        std::shared_ptr<JsonValue> get(size_t index) const;
        const std::vector<std::shared_ptr<JsonValue>>& values() const { return values_; }
    protected:
        std::vector<std::shared_ptr<JsonValue>> values_;
    };

    class JsonObject : public JsonValue {
    public:
        JsonObject() = default;

        void add(const std::string& key, std::shared_ptr<JsonValue> value);

        Type type() const override { return Type::Object; }

        std::string serialize(int indent_level = Json_Default_IndentLevel, int indent_size = Json_Default_IndentSize) const override;

        std::shared_ptr<JsonValue> get(const std::string& key);

    protected:
        std::unordered_map<std::string, std::shared_ptr<JsonValue>> values_;
        std::vector<std::string> keys_;
    };
    
    class Parser {
    public:
        explicit Parser(const std::string& json);

        std::shared_ptr<JsonValue> parse();

    protected:
        std::string json_;
        size_t index_;

        void skipWhitespace();
        std::shared_ptr<JsonValue> parseValue();
        std::shared_ptr<JsonObject> parseObject();
        std::shared_ptr<JsonArray> parseArray();
        std::string parseString();
        double parseNumber();
        bool parseBoolean();
        void parseNull();
    };
}

#endif // JSON_H
