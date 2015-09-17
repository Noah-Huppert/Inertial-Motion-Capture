#ifndef CSV_LOG_H
#define CSV_LOG_H

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>

#include "log.h"
#include "status.h"

class CSVLog {
public:
    CSVLog(std::string file_path): file_path(file_path) {}
    CSVLog(const CSVLog& csv_log) {
        file_open = csv_log.file_open;
        columns_locked = csv_log.columns_locked;
        columns_written = csv_log.columns_written;

        file_path = csv_log.file_path;

        columns = csv_log.columns;
        line = csv_log.line;
    }

    int open();
    int close();

    int add_column(std::string key);
    int lock_columns();

    int add_to_line(std::string key, std::string value);
    int add_to_line(std::string key, double value);

    int finish_line();

    static std::string double_to_str(double value);

private:
    bool file_open = false;
    bool columns_locked = false;
    bool columns_written = false;

    std::string file_path;
    std::ofstream file;

    std::vector<std::string> columns;
    std::map<std::string, std::string> line;
};

#endif