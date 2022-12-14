/**
 * This file holds utility functions that do not have a good home anywhere else 
 * in the program source code.
 * 
 * @file utilities.h
 * @author Dalton Caron
 */
#ifndef UTILITIES_H__
#define UTILITIES_H__

#include <memory>
#include <string>

#include <assertions.h>
#include <logging.h>

/** Reads CSV files. */
class CsvReader {
    public:
        CsvReader(const std::string path, const char separator=',');
        virtual ~CsvReader();

        std::string get(const int row, const int col) const;
        int getRows() const { return this->rows; };
        int getColumns() const { return this->columns; };
    private:
        int rows;
        int columns;
        std::unique_ptr<std::string[]> data;
};

#endif