#pragma once
#include "TaskManager.h"
#include <string>
#include <map>
#include <functional>

class CLI {
public:
    explicit CLI(TaskManager& mgr);
    void run();

private:
    TaskManager& mgr_;
    bool         running_;
    bool         autoSaveEnabled_;

    using CommandFn = std::function<void(const std::vector<std::string>&)>;
    std::map<std::string, CommandFn>    commands_;
    std::map<std::string, std::string>  helpText_;

    void registerCommands();
    void dispatch(const std::string& line);
    std::vector<std::string> tokenize(const std::string& line) const;

    // Command handlers
    void cmdAdd      (const std::vector<std::string>& args);
    void cmdList     (const std::vector<std::string>& args);
    void cmdView     (const std::vector<std::string>& args);
    void cmdUpdate   (const std::vector<std::string>& args);
    void cmdDelete   (const std::vector<std::string>& args);
    void cmdDone     (const std::vector<std::string>& args);
    void cmdSearch   (const std::vector<std::string>& args);
    void cmdFilter   (const std::vector<std::string>& args);
    void cmdSort     (const std::vector<std::string>& args);
    void cmdReport   (const std::vector<std::string>& args);
    void cmdStats    (const std::vector<std::string>& args);
    void cmdReminder (const std::vector<std::string>& args);
    void cmdSave     (const std::vector<std::string>& args);
    void cmdHelp     (const std::vector<std::string>& args);
    void cmdQuit     (const std::vector<std::string>& args);

    // Display helpers
    void printTask(const Task& t) const;
    void printTaskRow(const Task& t) const;
    void printTableHeader() const;
    void printTaskTable(const std::vector<Task>& tasks) const;
    void printBanner() const;
    void printReminderAlerts() const;

    // Input helpers
    std::string promptLine(const std::string& prompt) const;
    int         promptInt (const std::string& prompt, int def = 0) const;
    bool        promptBool(const std::string& prompt, bool def = false) const;
    std::string sanitize(const std::string& s) const;

    // Colours / formatting
    static const char* RESET;
    static const char* BOLD;
    static const char* RED;
    static const char* GREEN;
    static const char* YELLOW;
    static const char* BLUE;
    static const char* MAGENTA;
    static const char* CYAN;
    static const char* WHITE;
    static const char* DIM;

    std::string colorPriority(Priority p) const;
    std::string colorStatus(Status s) const;
};
