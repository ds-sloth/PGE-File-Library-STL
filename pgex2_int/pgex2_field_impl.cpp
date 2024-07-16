/*
 * PGE File Library - a library to process file formats, part of Moondust project
 *
 * Copyright (c) 2014-2024 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cstdlib>
#include <climits>
#include "pgex2_base_field.hpp"

#include "../pge_file_lib_globs.h"

/*! \file pgex2_field_impl.cpp
 *
 *  \brief Contains code to load specific field types
 *
 */

const char* PGEX2_find_next_term(const char* line)
{
    bool escape = false;
    while(true)
    {
        if(*line == ';')
        {
            if(!escape)
                return line + 1;

            escape = false;
        }
        else if(*line == '\0')
            return line;
        else if(*line == '\\')
            escape = true;
        else
            escape = false;

        line++;
    }
}

const char* PGEX2_find_next_list_item(const char* line)
{
    bool escape = false;
    while(true)
    {
        if(*line == ',')
        {
            if(!escape)
                return line + 1;

            escape = false;
        }
        else if(*line == ']')
        {
            if(!escape)
                return line;

            escape = false;
        }
        else if(*line == '\0')
            return line;
        else if(*line == '\\')
            escape = true;
        else
            escape = false;

        line++;
    }
}

template<>
const char* PGEX2_FieldType<int>::load(int& dest, const char* field_data)
{
    char* str_end;
#if (INT_MAX == LONG_MAX) && (INT_MIN == LONG_MIN)
    dest = strtol(field_data, &str_end, 10);
#else
    long out = strtol(field_data, &str_end, 10);
    if(out > INT_MAX)
        dest = UINT_MAX;
    else if(out < INT_MIN)
        dest = INT_MIN;
    else
        dest = out;
#endif

    return str_end;
}

template<>
const char* PGEX2_FieldType<unsigned>::load(unsigned& dest, const char* field_data)
{
    char* str_end;
#if (UINT_MAX == ULONG_MAX)
    dest = strtoul(field_data, &str_end, 10);
#else
    unsigned long out = strtoul(field_data, &str_end, 10);
    if(out > UINT_MAX)
        dest = UINT_MAX;
    else
        dest = out;
#endif

    return str_end;
}

template<>
const char* PGEX2_FieldType<bool>::load(bool& dest, const char* field_data)
{
    if(*field_data == '1')
        dest = true;
    else
        dest = false;

    return field_data + 1;
}

template<>
const char* PGEX2_FieldType<long>::load(long& dest, const char* field_data)
{
    char* str_end;
    dest = strtol(field_data, &str_end, 10);
    return str_end;
}

template<>
const char* PGEX2_FieldType<unsigned long>::load(unsigned long& dest, const char* field_data)
{
    char* str_end;
    dest = strtoul(field_data, &str_end, 10);
    return str_end;
}

template<>
const char* PGEX2_FieldType<long long>::load(long long& dest, const char* field_data)
{
    char* str_end;
    dest = strtoll(field_data, &str_end, 10);
    return str_end;
}

template<>
const char* PGEX2_FieldType<unsigned long long>::load(unsigned long long& dest, const char* field_data)
{
    char* str_end;
    dest = strtoull(field_data, &str_end, 10);
    return str_end;
}

template<>
const char* PGEX2_FieldType<float>::load(float& dest, const char* field_data)
{
    char* str_end;
    dest = strtof(field_data, &str_end);
    return str_end;
}

template<>
const char* PGEX2_FieldType<double>::load(double& dest, const char* field_data)
{
    char* str_end;
    dest = strtod(field_data, &str_end);
    return str_end;
}

template<>
const char* PGEX2_FieldType<PGESTRING>::load(PGESTRING& dest, const char* field_data)
{
    dest.clear();

    const char* cur_pos = field_data;
    if(*cur_pos != '\"')
    {
        // this is an error
        return cur_pos;
    }

    cur_pos++;

    bool escape = false;

    while(true)
    {
        const char cur_byte = *cur_pos++;

        if(cur_byte == '\0')
            break;

        if(escape)
        {
            char escaped;
            if(cur_byte == 'n')
                escaped = '\n';
            else if(cur_byte == 'r')
                escaped = '\r';
            // something like \ " [ , etc
            else
                escaped = cur_byte;

            dest.push_back(escaped);

            escape = false;
            continue;
        }
        else if(cur_byte == '\\')
        {
            escape = true;
            continue;
        }
        else if(cur_byte == '"')
            break;
        else
        {
            escape = false;
            dest.push_back(cur_byte);
        }
    }

    if(*cur_pos != '"')
    {
        // this is an error
        return cur_pos;
    }

    cur_pos++;

    return cur_pos;
}

const char* PGEX2_FieldType<PGELIST<bool>>::load(PGELIST<bool>& dest, const char* field_data)
{
    dest.clear();

    const char* cur_pos = field_data;

    while(*cur_pos != ';' && *cur_pos != '\0')
    {
        if(*cur_pos == '1')
            dest.push_back(true);
        else if(*cur_pos == '0')
            dest.push_back(false);
        else
        {
            // this is an error
            break;
        }
        cur_pos++;
    }

    return cur_pos;
}
