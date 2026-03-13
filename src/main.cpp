#include "TaskManager.h"
#include "CLI.h"

#include <iostream>
#include <csignal>
#include <cstdlib>
#include <cstdio>   // needed for printf

static TaskManager* g_manager = nullptr;


// Handle Ctrl + C
void handleSIGINT(int)
{
    std::cout << "\n\n[WARNING] Interrupted - saving tasks...\n";

    if (g_manager)
    {
        g_manager->saveToCSV();
        std::cout << "[OK] Tasks saved. Goodbye!\n";
    }

    std::cout.flush();
    std::exit(0);
}


// Print CLI header
void printHeader()
{
    std::printf("========================================\n");
    std::printf("CLI Task Manager - C++ Edition\n");
    std::printf("Type 'help' to see commands\n");
    std::printf("========================================\n\n");
}


int main(int argc, char* argv[])
{
    std::string csvFile = "tasks.csv";

    if (argc >= 2)
        csvFile = argv[1];

    TaskManager manager(csvFile);
    g_manager = &manager;

    std::signal(SIGINT, handleSIGINT);

    printHeader();

    CLI cli(manager);
    cli.run();

    g_manager = nullptr;
    return 0;
}