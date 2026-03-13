#pragma once
#include <string>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

enum class Priority { LOW = 1, MEDIUM = 2, HIGH = 3, CRITICAL = 4 };
enum class Status   { PENDING, IN_PROGRESS, COMPLETED, CANCELLED };

inline std::string priorityToString(Priority p) {
    switch (p) {
        case Priority::LOW:      return "LOW";
        case Priority::MEDIUM:   return "MEDIUM";
        case Priority::HIGH:     return "HIGH";
        case Priority::CRITICAL: return "CRITICAL";
    }
    return "UNKNOWN";
}

inline std::string statusToString(Status s) {
    switch (s) {
        case Status::PENDING:     return "PENDING";
        case Status::IN_PROGRESS: return "IN_PROGRESS";
        case Status::COMPLETED:   return "COMPLETED";
        case Status::CANCELLED:   return "CANCELLED";
    }
    return "UNKNOWN";
}

inline Priority stringToPriority(const std::string& s) {
    if (s == "LOW")      return Priority::LOW;
    if (s == "HIGH")     return Priority::HIGH;
    if (s == "CRITICAL") return Priority::CRITICAL;
    return Priority::MEDIUM;
}

inline Status stringToStatus(const std::string& s) {
    if (s == "IN_PROGRESS") return Status::IN_PROGRESS;
    if (s == "COMPLETED")   return Status::COMPLETED;
    if (s == "CANCELLED")   return Status::CANCELLED;
    return Status::PENDING;
}

inline std::string formatTime(std::time_t t) {
    if (t == 0) return "N/A";
    std::tm* tm_ptr = std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(tm_ptr, "%Y-%m-%d %H:%M");
    return oss.str();
}

inline std::time_t parseTime(const std::string& s) {
    if (s.empty() || s == "N/A" || s == "0") return 0;
    std::tm tm = {};
    std::istringstream ss(s);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M");
    if (ss.fail()) return 0;
    return std::mktime(&tm);
}

struct Task {
    int         id;
    std::string title;
    std::string description;
    Priority    priority;
    Status      status;
    std::string category;
    std::string tags;        // comma-separated
    std::time_t createdAt;
    std::time_t dueDate;     // 0 = no due date
    std::time_t completedAt; // 0 = not completed
    bool        reminderSet;
    int         reminderMinutesBefore;

    Task()
        : id(0), priority(Priority::MEDIUM), status(Status::PENDING),
          createdAt(std::time(nullptr)), dueDate(0), completedAt(0),
          reminderSet(false), reminderMinutesBefore(30) {}

    std::string serialize() const;
    static Task deserialize(const std::string& line);

    bool isOverdue() const {
        if (dueDate == 0 || status == Status::COMPLETED || status == Status::CANCELLED)
            return false;
        return std::time(nullptr) > dueDate;
    }

    bool isDueSoon(int withinMinutes = 60) const {
        if (dueDate == 0 || status == Status::COMPLETED || status == Status::CANCELLED)
            return false;
        std::time_t now = std::time(nullptr);
        return (dueDate > now) && ((dueDate - now) <= withinMinutes * 60);
    }
};
