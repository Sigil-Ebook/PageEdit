/************************************************************************
**
**  Copyright (C) 2019  Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2015  John Schember <john@nachtimwald.com>
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
**
**  This file is part of PageEdit.
**
**  PageEdit is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  PageEdit is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with PageEdit.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#pragma once
#ifndef PE_EXCEPTION_H
#define PE_EXCEPTION_H

#include <exception>
#include <string>

/**
* Thrown when a file does not exist.
*/
class FileDoesNotExist : public std::runtime_error
{
public:
    FileDoesNotExist(const std::string &msg) : std::runtime_error(msg) { };
};

/**
 * Thrown when a file cannot be read.
 */
class CannotReadFile : public std::runtime_error {
public:
    CannotReadFile(const std::string &msg) : std::runtime_error(msg) { };
};

/**
 * Thrown when a file cannot be opened.
 */
class CannotOpenFile : public std::runtime_error {
public:
    CannotOpenFile(const std::string &msg) : std::runtime_error(msg) { };
};

/**
 * Thrown when a file cannot be copied.
 */
class CannotCopyFile : public std::runtime_error {
public:
    CannotCopyFile(const std::string &msg) : std::runtime_error(msg) { };
};

/**
 * Thrown when a file cannot be written.
 */
class CannotWriteFile : public std::runtime_error {
public:
    CannotWriteFile (const std::string &msg) : std::runtime_error(msg) { };
};

#endif // PE_EXCEPTION_H
