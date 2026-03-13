# 📋 CLI Task Manager — C++17

> A production-grade command-line task manager with CSV persistence, regex search, time-based reminders, and Markdown report generation — built entirely in modern C++17.

---

## GitHub Description
```
Modular C++17 CLI task manager with CSV storage, regex search, SIGINT handling, real-time reminders, and Markdown reports.
```

---

## 🛠 Tech Stack

![C++](https://img.shields.io/badge/C++-17-00599C?style=flat&logo=cplusplus&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.16+-064F8C?style=flat&logo=cmake)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey?style=flat)

`C++17` · `STL` · `std::regex` · `iomanip` · `RAII` · `SIGINT` · `CSV` · `CMake` · `Makefile`

---

## ✨ Highlights

- 🗂 **Modular architecture** — Task, TaskManager, CLI separated into clean layers
- 💾 **CSV serialization** with RFC-4180 escaping and crash-resilient auto-save
- 🔍 **Regex search** with case-insensitive matching and substring fallback
- ⏰ **Real-time reminders** — alerts triggered on startup based on due date proximity
- 🚦 **SIGINT handler** — Ctrl-C guarantees CSV persistence before exit (RAII)
- 📊 **Stats dashboard** — progress bar, per-priority and per-category breakdown
- 📝 **Markdown reports** — auto-generated with task tables and summary
- 🎨 **ANSI color output** — color-coded priority, status, and overdue indicators

---

## 📁 Project Structure

```
task_manager/
├── include/
│   ├── Task.h             # Task struct, enums, time helpers
│   ├── TaskManager.h      # Storage engine interface
│   └── CLI.h              # Command dispatcher interface
├── src/
│   ├── Task.cpp           # CSV serialization logic
│   ├── TaskManager.cpp    # CRUD, search, sort, reminders, reports
│   ├── CLI.cpp            # 15 command handlers + ANSI display
│   └── main.cpp           # Entry point + SIGINT handler
├── tasks.csv              # Sample data file
├── Makefile
└── CMakeLists.txt
```

---

## 🚀 Build & Run

**Linux / macOS**
```bash
make
./task_manager
```

**Windows (MinGW)**
```cmd
mingw32-make
.\task_manager.exe
```

**CMake**
```bash
mkdir build && cd build
cmake ..
make
./task_manager
```

---

## 💻 Commands

| Command | Description |
|---|---|
| `add` | Interactive wizard to create a task |
| `list [pending\|done\|overdue]` | Display task table |
| `view <id>` | Full detail view |
| `update <id>` | Edit any field interactively |
| `delete <id>` | Remove a task permanently |
| `done <id>` | Mark as completed |
| `search <pattern>` | Regex / substring search |
| `filter status\|priority\|category <value>` | Filter tasks |
| `sort <field> [asc\|desc]` | Sort by 7 fields |
| `stats` | Progress bar + breakdown |
| `report [file.md]` | Generate Markdown report |
| `reminder <id>` | Toggle reminder for a task |
