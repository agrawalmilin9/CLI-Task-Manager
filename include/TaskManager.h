#pragma once
#include "Task.h"
#include <vector>
#include <string>
#include <functional>
#include <optional>

class TaskManager {
public:
    explicit TaskManager(const std::string& csvFile = "tasks.csv");
    ~TaskManager();

    // CRUD
    int  addTask(const Task& t);
    bool updateTask(int id, const Task& updated);
    bool deleteTask(int id);
    std::optional<Task> getTask(int id) const;

    // Queries
    std::vector<Task> getAllTasks() const;
    std::vector<Task> searchTasks(const std::string& pattern) const;
    std::vector<Task> filterByStatus(Status s) const;
    std::vector<Task> filterByPriority(Priority p) const;
    std::vector<Task> filterByCategory(const std::string& cat) const;
    std::vector<Task> getOverdueTasks() const;
    std::vector<Task> getDueSoonTasks(int withinMinutes = 60) const;

    // Sorting
    void sortBy(const std::string& field, bool ascending = true);

    // Persistence
    bool loadFromCSV();
    bool saveToCSV() const;
    void autoSave();

    // Report
    bool generateMarkdownReport(const std::string& filename = "") const;

    // Stats
    int  totalCount() const   { return (int)tasks_.size(); }
    int  pendingCount() const;
    int  completedCount() const;
    int  overdueCount() const;

    // Reminder check — returns tasks that should alert now
    std::vector<Task> checkReminders() const;

private:
    std::string              csvFile_;
    std::vector<Task>        tasks_;
    int                      nextId_;
    mutable bool             dirty_;

    std::string escapeCsvField(const std::string& s) const;
    std::string unescapeCsvField(const std::string& s) const;
    std::vector<std::string> splitCsvLine(const std::string& line) const;

    int generateId() { return nextId_++; }
};
