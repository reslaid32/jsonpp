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
        if (indent_size > 0) {
            oss << std::string(indent_level * indent_size, ' ');
        }
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
        
        if (indent_size > 0) {
            oss << std::string(indent_level * indent_size, ' ');
        }

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
            oss << values_.at(key)->serialize(indent_level, indent_size);
        }
        if (indent_size > 0) {
            oss << "\n" << std::string(indent_level * indent_size, ' ');
        }
        oss << "}";
        return oss.str();
    }

    std::shared_ptr<JsonValue> parseValue(std::istringstream& iss);

    std::shared_ptr<JsonValue> parse(const std::string& json) {
        std::istringstream iss(json);
        return parseValue(iss);
    }

    std::shared_ptr<JsonValue> parseValue(std::istringstream& iss) {
        char ch;
        while (iss >> ch) {
            if (ch == 'n') return std::make_shared<JsonNull>();
            else if (ch == 't') return std::make_shared<JsonBoolean>(true);
            else if (ch == 'f') return std::make_shared<JsonBoolean>(false);
            else if (ch == '"') {
                std::string str;
                while (iss.get(ch) && ch != '"') str += ch;
                return std::make_shared<JsonString>(str);
            } else if (std::isdigit(ch) || ch == '-') {
                iss.putback(ch);
                double num;
                iss >> num;
                return std::make_shared<JsonNumber>(num);
            } else if (ch == '[') {
                auto arr = std::make_shared<JsonArray>();
                while (iss >> ch && ch != ']') {
                    iss.putback(ch);
                    arr->add(parseValue(iss));
                    iss >> ch; // Consume ',' or ']'
                    if (ch == ']') break;
                }
                return arr;
            } else if (ch == '{') {
                auto obj = std::make_shared<JsonObject>();
                while (iss >> ch && ch != '}') {
                    if (ch == '"') {
                        std::string key;
                        while (iss.get(ch) && ch != '"') key += ch;
                        iss >> ch; // Consume ':'
                        obj->add(key, parseValue(iss));
                        iss >> ch; // Consume ',' or '}'
                        if (ch == '}') break;
                    }
                }
                return obj;
            }
        }
        throw std::runtime_error("Invalid JSON format");
    }
}
