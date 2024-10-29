#include "json.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace Json {

    std::string JsonNumber::serialize() const {
        std::ostringstream oss;
        oss << value_;
        return oss.str();
    }

    std::string JsonString::serialize() const {
        std::ostringstream oss;
        oss << "\"" << value_ << "\"";
        return oss.str();
    }

    std::string JsonArray::serialize() const {
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < values_.size(); ++i) {
            if (i > 0) oss << ",";
            oss << values_[i]->serialize();
        }
        oss << "]";
        return oss.str();
    }

    std::string JsonObject::serialize() const {
        std::ostringstream oss;
        oss << "{";
        for (size_t i = 0; i < keys_.size(); ++i) {
            if (i > 0) oss << ",";
            const std::string& key = keys_[i];
            oss << "\"" << key << "\":" << values_.at(key)->serialize();
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
