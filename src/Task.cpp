#include "Task.h"
#include <sstream>
#include <vector>

// CSV column order:
// id,title,description,priority,status,category,tags,createdAt,dueDate,completedAt,reminderSet,reminderMinutesBefore

static std::string escField(const std::string& s) {
    // Wrap in quotes if contains comma, quote, or newline
    if (s.find_first_of(",\"\n\r") == std::string::npos) return s;
    std::string out = "\"";
    for (char c : s) {
        if (c == '"') out += "\"\"";
        else          out += c;
    }
    out += '"';
    return out;
}

static std::vector<std::string> splitLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string cur;
    bool inQuote = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (inQuote) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i+1] == '"') { cur += '"'; ++i; }
                else inQuote = false;
            } else { cur += c; }
        } else {
            if (c == '"') { inQuote = true; }
            else if (c == ',') { fields.push_back(cur); cur.clear(); }
            else { cur += c; }
        }
    }
    fields.push_back(cur);
    return fields;
}

std::string Task::serialize() const {
    std::ostringstream os;
    os << id << ','
       << escField(title) << ','
       << escField(description) << ','
       << priorityToString(priority) << ','
       << statusToString(status) << ','
       << escField(category) << ','
       << escField(tags) << ','
       << (long long)createdAt << ','
       << (long long)dueDate << ','
       << (long long)completedAt << ','
       << (reminderSet ? 1 : 0) << ','
       << reminderMinutesBefore;
    return os.str();
}

Task Task::deserialize(const std::string& line) {
    auto f = splitLine(line);
    Task t;
    if (f.size() < 12) return t;
    t.id                    = std::stoi(f[0]);
    t.title                 = f[1];
    t.description           = f[2];
    t.priority              = stringToPriority(f[3]);
    t.status                = stringToStatus(f[4]);
    t.category              = f[5];
    t.tags                  = f[6];
    t.createdAt             = (std::time_t)std::stoll(f[7]);
    t.dueDate               = (std::time_t)std::stoll(f[8]);
    t.completedAt           = (std::time_t)std::stoll(f[9]);
    t.reminderSet           = (f[10] == "1");
    t.reminderMinutesBefore = std::stoi(f[11]);
    return t;
}
