#include "CLI.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <regex>
#include <cctype>
#include <ctime>

// ─── ANSI colours ─────────────────────────────────────────────────────────────
const char* CLI::RESET   = "\033[0m";
const char* CLI::BOLD    = "\033[1m";
const char* CLI::RED     = "\033[91m";
const char* CLI::GREEN   = "\033[92m";
const char* CLI::YELLOW  = "\033[93m";
const char* CLI::BLUE    = "\033[94m";
const char* CLI::MAGENTA = "\033[95m";
const char* CLI::CYAN    = "\033[96m";
const char* CLI::WHITE   = "\033[97m";
const char* CLI::DIM     = "\033[2m";

// ─── Constructor ──────────────────────────────────────────────────────────────
CLI::CLI(TaskManager& mgr) : mgr_(mgr), running_(false), autoSaveEnabled_(true) {
    registerCommands();
}

void CLI::registerCommands() {
    commands_["add"]      = [this](auto& a){ cmdAdd(a); };
    commands_["list"]     = [this](auto& a){ cmdList(a); };
    commands_["view"]     = [this](auto& a){ cmdView(a); };
    commands_["update"]   = [this](auto& a){ cmdUpdate(a); };
    commands_["delete"]   = [this](auto& a){ cmdDelete(a); };
    commands_["done"]     = [this](auto& a){ cmdDone(a); };
    commands_["search"]   = [this](auto& a){ cmdSearch(a); };
    commands_["filter"]   = [this](auto& a){ cmdFilter(a); };
    commands_["sort"]     = [this](auto& a){ cmdSort(a); };
    commands_["report"]   = [this](auto& a){ cmdReport(a); };
    commands_["stats"]    = [this](auto& a){ cmdStats(a); };
    commands_["reminder"] = [this](auto& a){ cmdReminder(a); };
    commands_["save"]     = [this](auto& a){ cmdSave(a); };
    commands_["help"]     = [this](auto& a){ cmdHelp(a); };
    commands_["quit"]     = [this](auto& a){ cmdQuit(a); };
    commands_["exit"]     = [this](auto& a){ cmdQuit(a); };
    commands_["q"]        = [this](auto& a){ cmdQuit(a); };

    helpText_["add"]      = "add                      — Interactive wizard to create a new task";
    helpText_["list"]     = "list [all|pending|done]  — Show task table (default: all)";
    helpText_["view"]     = "view <id>                — Show full details for a task";
    helpText_["update"]   = "update <id>              — Interactive wizard to edit a task";
    helpText_["delete"]   = "delete <id>              — Remove a task permanently";
    helpText_["done"]     = "done <id>                — Mark task as COMPLETED";
    helpText_["search"]   = "search <pattern>         — Regex/substring search across all fields";
    helpText_["filter"]   = "filter status|priority|category <value>";
    helpText_["sort"]     = "sort id|title|priority|status|due|created|category [asc|desc]";
    helpText_["report"]   = "report [filename.md]     — Generate Markdown report";
    helpText_["stats"]    = "stats                    — Show summary statistics";
    helpText_["reminder"] = "reminder <id>            — Toggle / configure reminder for a task";
    helpText_["save"]     = "save                     — Force-save to CSV";
    helpText_["help"]     = "help [command]           — Show this help";
    helpText_["quit"]     = "quit / exit / q          — Exit the application";
}

// ─── Run loop ─────────────────────────────────────────────────────────────────
void CLI::run() {
    printBanner();
    printReminderAlerts();
    running_ = true;
    while (running_) {
        std::cout << CYAN << BOLD << "task" << RESET
                  << CYAN << "> " << RESET;
        std::string line;
        if (!std::getline(std::cin, line)) break;  // EOF / Ctrl-D
        if (line.empty()) continue;
        dispatch(line);
    }
    std::cout << YELLOW << "\nGoodbye! All tasks saved.\n" << RESET;
}

// ─── Dispatch ─────────────────────────────────────────────────────────────────
void CLI::dispatch(const std::string& line) {
    auto tokens = tokenize(line);
    if (tokens.empty()) return;
    std::string cmd = tokens[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());
    auto it = commands_.find(cmd);
    if (it == commands_.end()) {
        std::cout << RED << "Unknown command: " << cmd
                  << "  (type 'help' for a list)\n" << RESET;
        return;
    }
    it->second(args);
}

std::vector<std::string> CLI::tokenize(const std::string& line) const {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string tok;
    while (iss >> tok) tokens.push_back(tok);
    return tokens;
}

// ─── Display helpers ──────────────────────────────────────────────────────────
std::string CLI::colorPriority(Priority p) const {
    switch (p) {
        case Priority::LOW:      return std::string(DIM) + "LOW" + RESET;
        case Priority::MEDIUM:   return std::string(BLUE) + "MEDIUM" + RESET;
        case Priority::HIGH:     return std::string(YELLOW) + "HIGH" + RESET;
        case Priority::CRITICAL: return std::string(RED) + BOLD + "CRITICAL" + RESET;
    }
    return "?";
}

std::string CLI::colorStatus(Status s) const {
    switch (s) {
        case Status::PENDING:     return std::string(WHITE) + "PENDING" + RESET;
        case Status::IN_PROGRESS: return std::string(CYAN) + "IN_PROGRESS" + RESET;
        case Status::COMPLETED:   return std::string(GREEN) + "COMPLETED" + RESET;
        case Status::CANCELLED:   return std::string(DIM) + "CANCELLED" + RESET;
    }
    return "?";
}

void CLI::printBanner() const {
    std::cout << CYAN << BOLD;
    std::cout << "╔═══════════════════════════════════════════════════╗\n";
    std::cout << "║         CLI Task Manager  •  C++ Edition          ║\n";
    std::cout << "║         Type 'help' to see available commands      ║\n";
    std::cout << "╚═══════════════════════════════════════════════════╝\n";
    std::cout << RESET;
    std::cout << DIM << "  Total: " << mgr_.totalCount()
              << "  Pending: " << mgr_.pendingCount()
              << "  Overdue: " << mgr_.overdueCount()
              << "\n\n" << RESET;
}

void CLI::printTableHeader() const {
    std::cout << BOLD
              << std::left
              << std::setw(5)  << "ID"
              << std::setw(30) << "Title"
              << std::setw(10) << "Priority"
              << std::setw(13) << "Status"
              << std::setw(15) << "Category"
              << std::setw(18) << "Due Date"
              << "\n" << RESET;
    std::cout << std::string(91, '-') << '\n';
}

void CLI::printTaskRow(const Task& t) const {
    std::string title = t.title;
    if (title.size() > 28) title = title.substr(0, 25) + "...";

    std::string cat = t.category.empty() ? "—" : t.category;
    if (cat.size() > 13) cat = cat.substr(0, 10) + "...";

    // Overdue indicator
    std::string idStr = std::to_string(t.id);
    if (t.isOverdue())    idStr += RED + std::string("!") + RESET;
    else if (t.isDueSoon(60)) idStr += YELLOW + std::string("~") + RESET;

    std::cout << std::left
              << std::setw(5)  << idStr
              << std::setw(30) << title
              << std::setw(20) << colorPriority(t.priority)
              << std::setw(23) << colorStatus(t.status)
              << std::setw(15) << cat
              << std::setw(18) << formatTime(t.dueDate)
              << '\n';
}

void CLI::printTaskTable(const std::vector<Task>& tasks) const {
    if (tasks.empty()) {
        std::cout << DIM << "  (no tasks)\n" << RESET;
        return;
    }
    printTableHeader();
    for (const auto& t : tasks) printTaskRow(t);
    std::cout << DIM << "\n  " << tasks.size() << " task(s) shown\n" << RESET;
}

void CLI::printTask(const Task& t) const {
    std::cout << '\n'
              << BOLD << CYAN << "┌─ Task #" << t.id << " ─────────────────────────────────\n" << RESET
              << BOLD << "  Title:       " << RESET << t.title << '\n'
              << BOLD << "  Priority:    " << RESET << colorPriority(t.priority) << '\n'
              << BOLD << "  Status:      " << RESET << colorStatus(t.status) << '\n'
              << BOLD << "  Category:    " << RESET << (t.category.empty() ? "—" : t.category) << '\n'
              << BOLD << "  Tags:        " << RESET << (t.tags.empty() ? "—" : t.tags) << '\n'
              << BOLD << "  Created:     " << RESET << formatTime(t.createdAt) << '\n'
              << BOLD << "  Due:         " << RESET << formatTime(t.dueDate);
    if (t.isOverdue())    std::cout << RED << "  ⚠ OVERDUE" << RESET;
    else if (t.isDueSoon(60)) std::cout << YELLOW << "  ⏰ DUE SOON" << RESET;
    std::cout << '\n';
    if (t.completedAt)
        std::cout << BOLD << "  Completed:   " << RESET << formatTime(t.completedAt) << '\n';
    if (t.reminderSet)
        std::cout << BOLD << "  Reminder:    " << RESET << t.reminderMinutesBefore << " min before due\n";
    if (!t.description.empty())
        std::cout << BOLD << "  Description:\n" << RESET << "    " << t.description << '\n';
    std::cout << CYAN << "└────────────────────────────────────────────\n\n" << RESET;
}

void CLI::printReminderAlerts() const {
    auto alerts = mgr_.checkReminders();
    for (const auto& t : alerts) {
        std::cout << YELLOW << BOLD
                  << "  ⏰  REMINDER: [" << t.id << "] " << t.title
                  << "  due " << formatTime(t.dueDate)
                  << '\n' << RESET;
    }
}

// ─── Input helpers ────────────────────────────────────────────────────────────
std::string CLI::promptLine(const std::string& prompt) const {
    std::cout << YELLOW << "  " << prompt << ": " << RESET;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

int CLI::promptInt(const std::string& prompt, int def) const {
    std::string s = promptLine(prompt + " [" + std::to_string(def) + "]");
    if (s.empty()) return def;
    try { return std::stoi(s); } catch (...) { return def; }
}

bool CLI::promptBool(const std::string& prompt, bool def) const {
    std::string s = promptLine(prompt + " [" + (def ? "Y/n" : "y/N") + "]");
    if (s.empty()) return def;
    return (s[0] == 'y' || s[0] == 'Y');
}

std::string CLI::sanitize(const std::string& s) const {
    // Remove control characters
    std::string out;
    for (char c : s)
        if (c >= 0x20 || c == '\t') out += c;
    // Trim
    size_t start = out.find_first_not_of(" \t");
    size_t end   = out.find_last_not_of(" \t");
    if (start == std::string::npos) return "";
    return out.substr(start, end - start + 1);
}

// ─── Command implementations ──────────────────────────────────────────────────

void CLI::cmdAdd(const std::vector<std::string>&) {
    std::cout << CYAN << "\n  ── New Task ──────────────────────────\n" << RESET;
    Task t;
    t.title = sanitize(promptLine("Title (required)"));
    if (t.title.empty()) { std::cout << RED << "  Title cannot be empty.\n" << RESET; return; }
    t.description = sanitize(promptLine("Description"));
    t.category    = sanitize(promptLine("Category"));
    t.tags        = sanitize(promptLine("Tags (comma-separated)"));

    std::cout << "  Priority: 1=LOW  2=MEDIUM  3=HIGH  4=CRITICAL\n";
    int pri = promptInt("Priority", 2);
    if (pri < 1 || pri > 4) pri = 2;
    t.priority = static_cast<Priority>(pri);

    std::cout << "  Status: 1=PENDING  2=IN_PROGRESS\n";
    int sts = promptInt("Status", 1);
    t.status = (sts == 2) ? Status::IN_PROGRESS : Status::PENDING;

    std::string dueStr = sanitize(promptLine("Due date (YYYY-MM-DD HH:MM or blank)"));
    t.dueDate = parseTime(dueStr);

    if (t.dueDate > 0) {
        t.reminderSet = promptBool("Set reminder?", true);
        if (t.reminderSet)
            t.reminderMinutesBefore = promptInt("Minutes before due to remind", 30);
    }

    int id = mgr_.addTask(t);
    std::cout << GREEN << "  ✓ Task #" << id << " created: " << t.title << "\n\n" << RESET;
}

void CLI::cmdList(const std::vector<std::string>& args) {
    std::string filter = args.empty() ? "all" : args[0];
    std::vector<Task> tasks;
    if      (filter == "pending")   tasks = mgr_.filterByStatus(Status::PENDING);
    else if (filter == "progress")  tasks = mgr_.filterByStatus(Status::IN_PROGRESS);
    else if (filter == "done")      tasks = mgr_.filterByStatus(Status::COMPLETED);
    else if (filter == "overdue")   tasks = mgr_.getOverdueTasks();
    else                            tasks = mgr_.getAllTasks();

    std::cout << '\n';
    printTaskTable(tasks);
    std::cout << '\n';
    printReminderAlerts();
}

void CLI::cmdView(const std::vector<std::string>& args) {
    if (args.empty()) { std::cout << RED << "  Usage: view <id>\n" << RESET; return; }
    int id = std::stoi(args[0]);
    auto t = mgr_.getTask(id);
    if (!t) { std::cout << RED << "  Task #" << id << " not found.\n" << RESET; return; }
    printTask(*t);
}

void CLI::cmdUpdate(const std::vector<std::string>& args) {
    if (args.empty()) { std::cout << RED << "  Usage: update <id>\n" << RESET; return; }
    int id = std::stoi(args[0]);
    auto opt = mgr_.getTask(id);
    if (!opt) { std::cout << RED << "  Task #" << id << " not found.\n" << RESET; return; }
    Task t = *opt;

    std::cout << CYAN << "\n  ── Update Task #" << id << " (leave blank to keep current) ──\n" << RESET;

    auto inp = sanitize(promptLine("Title [" + t.title + "]"));
    if (!inp.empty()) t.title = inp;

    inp = sanitize(promptLine("Description"));
    if (!inp.empty()) t.description = inp;

    inp = sanitize(promptLine("Category [" + t.category + "]"));
    if (!inp.empty()) t.category = inp;

    inp = sanitize(promptLine("Tags [" + t.tags + "]"));
    if (!inp.empty()) t.tags = inp;

    std::cout << "  Priority: 1=LOW  2=MEDIUM  3=HIGH  4=CRITICAL  [" << (int)t.priority << "]\n";
    inp = sanitize(promptLine("Priority"));
    if (!inp.empty()) { int p = std::stoi(inp); if (p >= 1 && p <= 4) t.priority = static_cast<Priority>(p); }

    std::cout << "  Status: 1=PENDING  2=IN_PROGRESS  3=COMPLETED  4=CANCELLED  [" << (int)t.status << "]\n";
    inp = sanitize(promptLine("Status"));
    if (!inp.empty()) {
        int s = std::stoi(inp);
        if      (s == 1) t.status = Status::PENDING;
        else if (s == 2) t.status = Status::IN_PROGRESS;
        else if (s == 3) { t.status = Status::COMPLETED; t.completedAt = std::time(nullptr); }
        else if (s == 4) t.status = Status::CANCELLED;
    }

    inp = sanitize(promptLine("Due date (YYYY-MM-DD HH:MM) [" + formatTime(t.dueDate) + "]"));
    if (!inp.empty()) t.dueDate = parseTime(inp);

    if (mgr_.updateTask(id, t))
        std::cout << GREEN << "  ✓ Task #" << id << " updated.\n\n" << RESET;
    else
        std::cout << RED << "  Update failed.\n" << RESET;
}

void CLI::cmdDelete(const std::vector<std::string>& args) {
    if (args.empty()) { std::cout << RED << "  Usage: delete <id>\n" << RESET; return; }
    int id = std::stoi(args[0]);
    if (!promptBool("Delete task #" + std::to_string(id) + "?", false)) return;
    if (mgr_.deleteTask(id))
        std::cout << GREEN << "  ✓ Task #" << id << " deleted.\n\n" << RESET;
    else
        std::cout << RED << "  Task #" << id << " not found.\n" << RESET;
}

void CLI::cmdDone(const std::vector<std::string>& args) {
    if (args.empty()) { std::cout << RED << "  Usage: done <id>\n" << RESET; return; }
    int id = std::stoi(args[0]);
    auto opt = mgr_.getTask(id);
    if (!opt) { std::cout << RED << "  Task #" << id << " not found.\n" << RESET; return; }
    Task t = *opt;
    t.status      = Status::COMPLETED;
    t.completedAt = std::time(nullptr);
    if (mgr_.updateTask(id, t))
        std::cout << GREEN << "  ✓ Task #" << id << " marked as COMPLETED.\n\n" << RESET;
}

void CLI::cmdSearch(const std::vector<std::string>& args) {
    if (args.empty()) { std::cout << RED << "  Usage: search <pattern>\n" << RESET; return; }
    std::string pattern = args[0];
    for (size_t i = 1; i < args.size(); ++i) pattern += " " + args[i];
    auto results = mgr_.searchTasks(pattern);
    std::cout << CYAN << "\n  Search: \"" << pattern << "\"\n\n" << RESET;
    printTaskTable(results);
    std::cout << '\n';
}

void CLI::cmdFilter(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << RED << "  Usage: filter status|priority|category <value>\n" << RESET;
        return;
    }
    std::string field = args[0], val = args[1];
    std::transform(field.begin(), field.end(), field.begin(), ::tolower);
    std::vector<Task> tasks;
    if      (field == "status")   tasks = mgr_.filterByStatus(stringToStatus(val));
    else if (field == "priority") tasks = mgr_.filterByPriority(stringToPriority(val));
    else if (field == "category") tasks = mgr_.filterByCategory(val);
    else { std::cout << RED << "  Unknown filter field: " << field << "\n" << RESET; return; }
    std::cout << CYAN << "\n  Filter: " << field << "=" << val << "\n\n" << RESET;
    printTaskTable(tasks);
    std::cout << '\n';
}

void CLI::cmdSort(const std::vector<std::string>& args) {
    std::string field = args.empty() ? "id" : args[0];
    bool asc = (args.size() < 2 || args[1] != "desc");
    mgr_.sortBy(field, asc);
    std::cout << GREEN << "  ✓ Sorted by " << field << " (" << (asc ? "asc" : "desc") << ")\n\n" << RESET;
    cmdList({});
}

void CLI::cmdReport(const std::vector<std::string>& args) {
    std::string fname = args.empty() ? "" : args[0];
    if (mgr_.generateMarkdownReport(fname)) {
        std::cout << GREEN << "  ✓ Report generated";
        if (!fname.empty()) std::cout << ": " << fname;
        std::cout << "\n\n" << RESET;
    } else {
        std::cout << RED << "  Failed to generate report.\n" << RESET;
    }
}

void CLI::cmdStats(const std::vector<std::string>&) {
    auto all  = mgr_.getAllTasks();
    int total = mgr_.totalCount();
    int done  = mgr_.completedCount();

    std::cout << '\n' << BOLD << CYAN << "  ── Statistics ─────────────────────\n" << RESET;
    std::cout << "  " << BOLD << std::setw(22) << std::left << "Total tasks:" << RESET << total << '\n';
    std::cout << "  " << BOLD << std::setw(22) << std::left << "Pending/In-progress:" << RESET << mgr_.pendingCount() << '\n';
    std::cout << "  " << BOLD << std::setw(22) << std::left << "Completed:" << RESET << done << '\n';
    std::cout << "  " << BOLD << std::setw(22) << std::left << "Overdue:" << RESET << RED << mgr_.overdueCount() << RESET << '\n';
    std::cout << "  " << BOLD << std::setw(22) << std::left << "Due soon (1h):" << RESET << YELLOW << mgr_.getDueSoonTasks(60).size() << RESET << '\n';

    // Completion %
    if (total > 0) {
        int pct = (done * 100) / total;
        std::cout << "\n  Progress: [";
        int filled = pct / 5;
        std::cout << GREEN;
        for (int i = 0; i < 20; ++i) std::cout << (i < filled ? "█" : "░");
        std::cout << RESET << "] " << pct << "%\n";
    }

    // Priority breakdown
    std::cout << "\n  " << BOLD << "By Priority:\n" << RESET;
    std::map<Priority,int> priCnt;
    for (const auto& t : all) priCnt[t.priority]++;
    for (auto& [p, c] : priCnt)
        std::cout << "    " << std::setw(10) << priorityToString(p) << " : " << c << '\n';

    // Category breakdown
    std::map<std::string,int> catCnt;
    for (const auto& t : all)
        catCnt[t.category.empty() ? "(none)" : t.category]++;
    std::cout << "\n  " << BOLD << "By Category:\n" << RESET;
    for (auto& [c, n] : catCnt)
        std::cout << "    " << std::setw(15) << c << " : " << n << '\n';
    std::cout << '\n';
}

void CLI::cmdReminder(const std::vector<std::string>& args) {
    if (args.empty()) { std::cout << RED << "  Usage: reminder <id>\n" << RESET; return; }
    int id = std::stoi(args[0]);
    auto opt = mgr_.getTask(id);
    if (!opt) { std::cout << RED << "  Task #" << id << " not found.\n" << RESET; return; }
    Task t = *opt;
    if (t.dueDate == 0) { std::cout << RED << "  Task has no due date. Set one first with 'update'.\n" << RESET; return; }
    t.reminderSet = !t.reminderSet;
    if (t.reminderSet) {
        t.reminderMinutesBefore = promptInt("Minutes before due", 30);
        std::cout << GREEN << "  ✓ Reminder enabled (" << t.reminderMinutesBefore << " min before)\n\n" << RESET;
    } else {
        std::cout << YELLOW << "  Reminder disabled.\n\n" << RESET;
    }
    mgr_.updateTask(id, t);
}

void CLI::cmdSave(const std::vector<std::string>&) {
    mgr_.saveToCSV();
    std::cout << GREEN << "  ✓ Saved.\n\n" << RESET;
}

void CLI::cmdHelp(const std::vector<std::string>& args) {
    std::cout << '\n' << BOLD << CYAN << "  ── Commands ───────────────────────────────────────\n" << RESET;
    if (!args.empty()) {
        auto it = helpText_.find(args[0]);
        if (it != helpText_.end())
            std::cout << "  " << it->second << '\n';
        else
            std::cout << RED << "  Unknown command: " << args[0] << '\n' << RESET;
    } else {
        for (auto& [cmd, text] : helpText_)
            std::cout << "  " << text << '\n';
    }
    std::cout << '\n';
}

void CLI::cmdQuit(const std::vector<std::string>&) {
    mgr_.saveToCSV();
    running_ = false;
}
