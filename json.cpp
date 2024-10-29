#include "json.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace Json {

    std::string JsonNumber::serialize(int indent_level, int indent_size) const {
        std::ostringstream oss;
        oss << value_;
        return oss.str();
    }

    std::string JsonString::serialize(int indent_level, int indent_size) const {
        std::ostringstream oss;
        oss << "\"" << value_ << "\"";
        return oss.str();
    }

    std::shared_ptr<JsonValue> JsonArray::get(size_t index) const {
        if (index < values_.size()) {
            return values_[index];
        }
        throw std::out_of_range("Index out of range");
    }

    std::string JsonArray::serialize(int indent_level, int indent_size) const {
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < values_.size(); ++i) {
            if (i > 0) oss << ",";
            if (indent_size > 0) oss << "\n";
            if (indent_size > 0) {
                oss << std::string((indent_level + 1) * indent_size, ' ');
            }
            oss << values_[i]->serialize(indent_level + 1, indent_size);
        }
        if (indent_size > 0) {
            oss << "\n" << std::string(indent_level * indent_size, ' ');
        }
        oss << "]";
        return oss.str();
    }

    void JsonObject::add(const std::string& key, std::shared_ptr<JsonValue> value) {
        if (values_.find(key) == values_.end()) {
            keys_.push_back(key);
        }
        values_[key] = std::move(value);
    }

    std::shared_ptr<JsonValue> JsonObject::get(const std::string& key) {
        auto it = values_.find(key);
        return (it != values_.end()) ? it->second : nullptr;
    }

    std::string JsonObject::serialize(int indent_level, int indent_size) const {
        std::ostringstream oss;
        oss << "{";
        for (size_t i = 0; i < keys_.size(); ++i) {
            if (i > 0) oss << ",";
            if (indent_size > 0) oss << "\n";
            if (indent_size > 0) {
                oss << std::string((indent_level + 1) * indent_size, ' ');
            }
            const std::string& key = keys_[i];
            oss << "\"" << key << "\":";
            if (indent_size > 0) oss << " ";
            oss << values_.at(key)->serialize(indent_level + 1, indent_size);
        }
        if (indent_size > 0) {
            oss << "\n" << std::string(indent_level * indent_size, ' ');
        }
        oss << "}";
        return oss.str();
    }

    Parser::Parser(const std::string& json) : json_(json), index_(0) {}

    std::shared_ptr<JsonValue> Parser::parse() {
        skipWhitespace();
        auto value = parseValue();
        skipWhitespace();
        if (index_ < json_.size()) {
            throw std::runtime_error("Unexpected characters after JSON value");
        }
        return value;
    }

    void Parser::skipWhitespace() {
        while (index_ < json_.size() && std::isspace(json_[index_])) {
            index_++;
        }
    }

    std::shared_ptr<JsonValue> Parser::parseValue() {
        skipWhitespace();
        if (index_ >= json_.size()) throw std::runtime_error("Unexpected end of input");

        char c = json_[index_];
        if (c == '{') {
            return parseObject();
        } else if (c == '[') {
            return parseArray();
        } else if (c == '"') {
            return std::make_shared<JsonString>(parseString());
        } else if (std::isdigit(c) || c == '-' || c == '+') {
            return std::make_shared<JsonNumber>(parseNumber());
        } else if (c == 't' || c == 'f') {
            return std::make_shared<JsonBoolean>(parseBoolean());
        } else if (c == 'n') {
            parseNull();
            return std::make_shared<JsonNull>();
        }

        throw std::runtime_error("Unexpected character in JSON input");
    }

    std::shared_ptr<JsonObject> Parser::parseObject() {
        index_++;
        auto obj = std::make_shared<JsonObject>();
        skipWhitespace();

        while (index_ < json_.size() && json_[index_] != '}') {
            std::string key = parseString();
            skipWhitespace();

            if (json_[index_] != ':') {
                throw std::runtime_error("Expected ':' after key");
            }
            index_++;
            auto value = parseValue();
            obj->add(key, value);

            skipWhitespace();
            if (json_[index_] == ',') {
                index_++;
            } else if (json_[index_] != '}') {
                throw std::runtime_error("Expected ',' or '}' in object");
            }
            skipWhitespace();
        }

        if (index_ >= json_.size() || json_[index_] != '}') {
            throw std::runtime_error("Expected '}' at end of object");
        }
        index_++;
        return obj;
    }

    std::shared_ptr<JsonArray> Parser::parseArray() {
        index_++;
        auto array = std::make_shared<JsonArray>();
        skipWhitespace();

        while (index_ < json_.size() && json_[index_] != ']') {
            auto value = parseValue();
            array->add(value);

            skipWhitespace();
            if (json_[index_] == ',') {
                index_++;
            } else if (json_[index_] != ']') {
                throw std::runtime_error("Expected ',' or ']' in array");
            }
            skipWhitespace();
        }

        if (index_ >= json_.size() || json_[index_] != ']') {
            throw std::runtime_error("Expected ']' at end of array");
        }
        index_++;
        return array;
    }

    std::string Parser::parseString() {
        if (json_[index_] != '"') {
            throw std::runtime_error("Expected '\"' at start of string");
        }
        index_++;
        std::ostringstream oss;

        while (index_ < json_.size()) {
            if (json_[index_] == '"') {
                index_++;
                return oss.str();
            }
            if (json_[index_] == '\\') {
                index_++;
                if (index_ < json_.size()) {
                    char escapeChar = json_[index_];
                    if (escapeChar == 'n') oss << '\n';
                    else if (escapeChar == 't') oss << '\t';
                    else if (escapeChar == 'r') oss << '\r';
                    else if (escapeChar == 'b') oss << '\b';
                    else if (escapeChar == 'f') oss << '\f';
                    else oss << escapeChar;
                }
            } else {
                oss << json_[index_];
            }
            index_++;
        }
        throw std::runtime_error("Unexpected end of string");
    }

    double Parser::parseNumber() {
        size_t start = index_;
        while (index_ < json_.size() && (std::isdigit(json_[index_]) || json_[index_] == '.' || json_[index_] == '-' || json_[index_] == '+')) {
            index_++;
        }
        std::string numberStr = json_.substr(start, index_ - start);
        return std::stod(numberStr);
    }

    bool Parser::parseBoolean() {
        if (json_.substr(index_, 4) == "true") {
            index_ += 4;
            return true;
        } else if (json_.substr(index_, 5) == "false") {
            index_ += 5;
            return false;
        }
        throw std::runtime_error("Expected 'true' or 'false'");
    }

    void Parser::parseNull() {
        if (json_.substr(index_, 4) == "null") {
            index_ += 4;
        } else {
            throw std::runtime_error("Expected 'null'");
        }
    }
}
