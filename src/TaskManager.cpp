#include "TaskManager.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <iostream>
#include <iomanip>
#include <ctime>

TaskManager::TaskManager(const std::string& csvFile)
    : csvFile_(csvFile), nextId_(1), dirty_(false) {
    loadFromCSV();
}

TaskManager::~TaskManager() {
    if (dirty_) saveToCSV();
}

// ─── CRUD ──────────────────────────────────────────────────────────────────────

int TaskManager::addTask(const Task& t) {
    Task newTask = t;
    newTask.id = generateId();
    newTask.createdAt = std::time(nullptr);
    tasks_.push_back(newTask);
    dirty_ = true;
    autoSave();
    return newTask.id;
}

bool TaskManager::updateTask(int id, const Task& updated) {
    for (auto& t : tasks_) {
        if (t.id == id) {
            Task tmp   = updated;
            tmp.id     = id;
            tmp.createdAt = t.createdAt;
            t = tmp;
            dirty_ = true;
            autoSave();
            return true;
        }
    }
    return false;
}

bool TaskManager::deleteTask(int id) {
    auto it = std::remove_if(tasks_.begin(), tasks_.end(),
                             [id](const Task& t){ return t.id == id; });
    if (it == tasks_.end()) return false;
    tasks_.erase(it, tasks_.end());
    dirty_ = true;
    autoSave();
    return true;
}

std::optional<Task> TaskManager::getTask(int id) const {
    for (const auto& t : tasks_)
        if (t.id == id) return t;
    return std::nullopt;
}

// ─── QUERIES ──────────────────────────────────────────────────────────────────

std::vector<Task> TaskManager::getAllTasks() const { return tasks_; }

std::vector<Task> TaskManager::searchTasks(const std::string& pattern) const {
    std::vector<Task> results;
    try {
        std::regex re(pattern, std::regex_constants::icase);
        for (const auto& t : tasks_) {
            if (std::regex_search(t.title,       re) ||
                std::regex_search(t.description, re) ||
                std::regex_search(t.category,    re) ||
                std::regex_search(t.tags,        re))
                results.push_back(t);
        }
    } catch (...) {
        // Fallback: plain substring search
        std::string lp = pattern;
        std::transform(lp.begin(), lp.end(), lp.begin(), ::tolower);
        for (const auto& t : tasks_) {
            auto check = [&](const std::string& s){
                std::string ls = s;
                std::transform(ls.begin(), ls.end(), ls.begin(), ::tolower);
                return ls.find(lp) != std::string::npos;
            };
            if (check(t.title) || check(t.description) || check(t.category) || check(t.tags))
                results.push_back(t);
        }
    }
    return results;
}

std::vector<Task> TaskManager::filterByStatus(Status s) const {
    std::vector<Task> r;
    for (const auto& t : tasks_) if (t.status == s) r.push_back(t);
    return r;
}

std::vector<Task> TaskManager::filterByPriority(Priority p) const {
    std::vector<Task> r;
    for (const auto& t : tasks_) if (t.priority == p) r.push_back(t);
    return r;
}

std::vector<Task> TaskManager::filterByCategory(const std::string& cat) const {
    std::vector<Task> r;
    std::string lc = cat;
    std::transform(lc.begin(), lc.end(), lc.begin(), ::tolower);
    for (const auto& t : tasks_) {
        std::string lt = t.category;
        std::transform(lt.begin(), lt.end(), lt.begin(), ::tolower);
        if (lt == lc) r.push_back(t);
    }
    return r;
}

std::vector<Task> TaskManager::getOverdueTasks() const {
    std::vector<Task> r;
    for (const auto& t : tasks_) if (t.isOverdue()) r.push_back(t);
    return r;
}

std::vector<Task> TaskManager::getDueSoonTasks(int withinMinutes) const {
    std::vector<Task> r;
    for (const auto& t : tasks_) if (t.isDueSoon(withinMinutes)) r.push_back(t);
    return r;
}

std::vector<Task> TaskManager::checkReminders() const {
    std::vector<Task> alerts;
    std::time_t now = std::time(nullptr);
    for (const auto& t : tasks_) {
        if (!t.reminderSet || t.dueDate == 0) continue;
        if (t.status == Status::COMPLETED || t.status == Status::CANCELLED) continue;
        std::time_t alertTime = t.dueDate - (std::time_t)(t.reminderMinutesBefore * 60);
        // Trigger if within the last 5 minutes window
        if (now >= alertTime && now <= alertTime + 300) alerts.push_back(t);
    }
    return alerts;
}

// ─── SORTING ──────────────────────────────────────────────────────────────────

void TaskManager::sortBy(const std::string& field, bool ascending) {
    auto cmp = [&](const Task& a, const Task& b) -> bool {
        bool result = false;
        if      (field == "id")       result = a.id < b.id;
        else if (field == "title")    result = a.title < b.title;
        else if (field == "priority") result = (int)a.priority < (int)b.priority;
        else if (field == "status")   result = (int)a.status < (int)b.status;
        else if (field == "due")      result = (a.dueDate == 0 ? INT64_MAX : a.dueDate) < (b.dueDate == 0 ? INT64_MAX : b.dueDate);
        else if (field == "created")  result = a.createdAt < b.createdAt;
        else if (field == "category") result = a.category < b.category;
        else                          result = a.id < b.id;
        return ascending ? result : !result;
    };
    std::sort(tasks_.begin(), tasks_.end(), cmp);
}

// ─── PERSISTENCE ──────────────────────────────────────────────────────────────

bool TaskManager::loadFromCSV() {
    std::ifstream f(csvFile_);
    if (!f.is_open()) return false;
    tasks_.clear();
    nextId_ = 1;
    std::string line;
    std::getline(f, line); // skip header
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        Task t = Task::deserialize(line);
        if (t.id > 0) {
            tasks_.push_back(t);
            if (t.id >= nextId_) nextId_ = t.id + 1;
        }
    }
    dirty_ = false;
    return true;
}

bool TaskManager::saveToCSV() const {
    std::ofstream f(csvFile_);
    if (!f.is_open()) return false;
    f << "id,title,description,priority,status,category,tags,"
         "createdAt,dueDate,completedAt,reminderSet,reminderMinutesBefore\n";
    for (const auto& t : tasks_) f << t.serialize() << '\n';
    dirty_ = false;
    return true;
}

void TaskManager::autoSave() { saveToCSV(); }

// ─── STATS ────────────────────────────────────────────────────────────────────

int TaskManager::pendingCount() const {
    int c = 0;
    for (const auto& t : tasks_) if (t.status == Status::PENDING || t.status == Status::IN_PROGRESS) ++c;
    return c;
}

int TaskManager::completedCount() const {
    int c = 0;
    for (const auto& t : tasks_) if (t.status == Status::COMPLETED) ++c;
    return c;
}

int TaskManager::overdueCount() const {
    int c = 0;
    for (const auto& t : tasks_) if (t.isOverdue()) ++c;
    return c;
}

// ─── MARKDOWN REPORT ──────────────────────────────────────────────────────────

bool TaskManager::generateMarkdownReport(const std::string& filename) const {
    std::time_t now = std::time(nullptr);
    std::string fname = filename.empty()
        ? "report_" + std::to_string(now) + ".md"
        : filename;

    std::ofstream f(fname);
    if (!f.is_open()) return false;

    auto tm_now = std::localtime(&now);
    f << "# Task Manager Report\n\n";
    f << "**Generated:** " << std::put_time(tm_now, "%Y-%m-%d %H:%M:%S") << "\n\n";

    f << "## Summary\n\n";
    f << "| Metric | Value |\n";
    f << "|--------|-------|\n";
    f << "| Total Tasks | " << totalCount() << " |\n";
    f << "| Pending / In-Progress | " << pendingCount() << " |\n";
    f << "| Completed | " << completedCount() << " |\n";
    f << "| Overdue | " << overdueCount() << " |\n\n";

    auto printSection = [&](const std::string& heading, const std::vector<Task>& list) {
        f << "## " << heading << " (" << list.size() << ")\n\n";
        if (list.empty()) { f << "_None_\n\n"; return; }
        f << "| ID | Title | Priority | Status | Category | Due Date |\n";
        f << "|----|-------|----------|--------|----------|-----------|\n";
        for (const auto& t : list) {
            f << "| " << t.id
              << " | " << t.title
              << " | " << priorityToString(t.priority)
              << " | " << statusToString(t.status)
              << " | " << (t.category.empty() ? "-" : t.category)
              << " | " << formatTime(t.dueDate)
              << " |\n";
        }
        f << '\n';
    };

    printSection("Overdue Tasks",     getOverdueTasks());
    printSection("Due Soon (1 hour)", getDueSoonTasks(60));
    printSection("All Tasks",         tasks_);

    f << "## Detailed Task List\n\n";
    for (const auto& t : tasks_) {
        f << "### [" << t.id << "] " << t.title << "\n\n";
        f << "- **Priority:** " << priorityToString(t.priority) << "\n";
        f << "- **Status:** "   << statusToString(t.status)     << "\n";
        f << "- **Category:** " << (t.category.empty() ? "—" : t.category) << "\n";
        f << "- **Tags:** "     << (t.tags.empty() ? "—" : t.tags) << "\n";
        f << "- **Created:** "  << formatTime(t.createdAt) << "\n";
        f << "- **Due:** "      << formatTime(t.dueDate) << "\n";
        if (t.completedAt) f << "- **Completed:** " << formatTime(t.completedAt) << "\n";
        if (!t.description.empty()) f << "\n> " << t.description << "\n";
        f << "\n---\n\n";
    }

    return true;
}
