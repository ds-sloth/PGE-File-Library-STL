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
#include <limits>
#include <cmath>
#include "mdx/mdx_base_field.hpp"

#include "pge_file_lib_globs.h"

/*! \file mdx_field_impl.cpp
 *
 *  \brief Contains code to load specific field types
 *
 */

const char* MDX_skip_term(const char* line)
{
    bool escape = false;
    const char* tag_begin = line;
    const char* tag_end = nullptr;

    try
    {
        while(true)
        {
            if(*line == '\0')
            {
                if(tag_end)
                    throw MDX_missing_delimiter(';');
                else
                    throw MDX_missing_delimiter(':');
            }
            else if(escape)
            {
                // ignore character
                escape = false;
            }
            else if(*line == ';')
            {
                if(!tag_end)
                    throw MDX_missing_delimiter(':');
                else
                    return line + 1;
            }
            else if(*line == ':')
            {
                if(!tag_end)
                    tag_end = line;
                else
                    throw MDX_unexpected_character(':');
            }
            else if(*line == '\\')
            {
                if(!tag_end)
                    throw MDX_unexpected_character('\\');
                else
                    escape = true;
            }

            line++;
        }
    }
    catch(...)
    {
        if(tag_end)
            std::throw_with_nested(MDX_bad_field(tag_begin, tag_end - tag_begin));
        else
            std::throw_with_nested(MDX_bad_field(tag_begin, line - tag_begin));
    }
}

template<class uint_t>
static const char* s_load_uint(uint_t& dest, const char* field_data)
{
    const char* const ret_error = field_data;

    uint_t value = 0;

    while(true)
    {
        char c = *field_data;

        if(c < '0' || c > '9')
        {
            dest = value;
            return field_data;
        }

        field_data++;

        uint_t digit = c - '0';

        if(value >= std::numeric_limits<uint_t>::max() / 10) [[unlikely]]
        {
            if(value > std::numeric_limits<uint_t>::max() / 10 || digit > std::numeric_limits<uint_t>::max() % 10) [[likely]]
                return ret_error;
        }

        value *= 10;
        value += digit;
    }
}

template<class int_t, int sign>
static const char* s_load_int(int_t& dest, const char* field_data)
{
    const char* const ret_error = field_data;

    if(sign == 1 && *field_data == '-')
        return s_load_int<int_t, -1>(dest, field_data);

    if(sign == -1)
    {
        field_data++;
        if(*field_data < '0' || *field_data > '9')
            return ret_error;
    }

    int_t value = 0;

    while(true)
    {
        char c = *field_data;

        if(c < '0' || c > '9')
        {
            dest = value;
            return field_data;
        }

        field_data++;

        int_t digit = c - '0';

        if(sign == 1)
        {
            if(value >= std::numeric_limits<int_t>::max() / 10) [[unlikely]]
            {
                if(value > std::numeric_limits<int_t>::max() / 10 || digit > std::numeric_limits<int_t>::max() % 10) [[likely]]
                    return ret_error;
            }
        }
        else
        {
            if(value <= std::numeric_limits<int_t>::min() / 10) [[unlikely]]
            {
                if(value < std::numeric_limits<int_t>::min() / 10 || digit > -(std::numeric_limits<int_t>::min() % 10)) [[likely]]
                    return ret_error;
            }
        }

        value *= 10;
        if(sign == 1)
            value += digit;
        else
            value -= digit;
    }
}

static const char* s_load_double(double& dest, const char* field_data)
{
    const char* const ret_error = field_data;

    int sign = 1;

    if(*field_data == '-')
    {
        sign = -1;
        field_data++;
    }

    if(*field_data < '0' || *field_data > '9')
        return ret_error;

    double value = 0;
    double divisor = 0.1;

    while(true)
    {
        char c = *field_data;

        if(c == '.')
        {
            break;
        }
        else if(c < '0' || c > '9')
        {
            if(c == 'e')
                goto exponent;

            dest = sign * value;
            return field_data;
        }

        field_data++;

        value *= 10;
        value += static_cast<double>(c - '0');
    }

    field_data++;

    while(true)
    {
        char c = *field_data;

        if(c < '0' || c > '9')
        {
            // don't allow "." and "-."
            if(field_data == ret_error + 1 || (field_data == ret_error + 2 && sign == -1))
                return ret_error;

            if(c == 'e')
                goto exponent;

            dest = sign * value;
            return field_data;
        }

        field_data++;

        value += divisor * static_cast<double>(c - '0');
        divisor *= 0.1;
    }

exponent:
    field_data++;
    const char* exp_start = field_data;

    int exponent;
    field_data = s_load_int<int, 1>(exponent, field_data);
    if(field_data == exp_start)
        return ret_error;

    dest = sign * value * std::pow(10, exponent);
    return field_data;
}

template<>
const char* MDX_FieldType<int>::load(int& dest, const char* field_data)
{
    const char* str_end = s_load_int<int, 1>(dest, field_data);

    if(str_end == field_data)
        throw MDX_bad_term("Bad int");

    return str_end;
}

template<>
const char* MDX_FieldType<unsigned>::load(unsigned& dest, const char* field_data)
{
    const char* str_end = s_load_uint(dest, field_data);

    if(str_end == field_data)
        throw MDX_bad_term("Bad uint");

    return str_end;
}

template<>
const char* MDX_FieldType<bool>::load(bool& dest, const char* field_data)
{
    if(*field_data == '1')
        dest = true;
    else if(*field_data == '0')
        dest = false;
    else
        throw MDX_bad_term("Bad bool");

    return field_data + 1;
}

template<>
const char* MDX_FieldType<long>::load(long& dest, const char* field_data)
{
    const char* str_end = s_load_int<long, 1>(dest, field_data);

    if(str_end == field_data)
        throw MDX_bad_term("Bad long");

    return str_end;
}

template<>
const char* MDX_FieldType<unsigned long>::load(unsigned long& dest, const char* field_data)
{
    const char* str_end = s_load_uint(dest, field_data);

    if(str_end == field_data)
        throw MDX_bad_term("Bad ulong");

    return str_end;
}

template<>
const char* MDX_FieldType<long long>::load(long long& dest, const char* field_data)
{
    const char* str_end = s_load_int<long long, 1>(dest, field_data);

    if(str_end == field_data)
        throw MDX_bad_term("Bad llong");

    return str_end;
}

template<>
const char* MDX_FieldType<unsigned long long>::load(unsigned long long& dest, const char* field_data)
{
    const char* str_end = s_load_uint(dest, field_data);

    if(str_end == field_data)
        throw MDX_bad_term("Bad ullong");

    return str_end;
}

template<>
const char* MDX_FieldType<float>::load(float& dest, const char* field_data)
{
    double ret;
    const char* str_end = s_load_double(ret, field_data);
    dest = ret;

    if(str_end == field_data)
        throw MDX_bad_term("Bad float");

    return str_end;
}

template<>
const char* MDX_FieldType<double>::load(double& dest, const char* field_data)
{
    const char* str_end = s_load_double(dest, field_data);

    if(str_end == field_data)
        throw MDX_bad_term("Bad double");

    return str_end;
}

template<>
const char* MDX_FieldType<PGESTRING>::load(PGESTRING& dest, const char* field_data)
{
    dest.clear();

    const char* cur_pos = field_data;
    if(*cur_pos != '"')
        throw MDX_missing_delimiter('"');

    bool escape = false;

    while(true)
    {
        const char cur_byte = *(++cur_pos);

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
        else if(cur_byte == ';' || cur_byte == ':')
            throw MDX_unexpected_character(cur_byte);
        else if(cur_byte == '"')
            break;
        else
        {
            escape = false;
            dest.push_back(cur_byte);
        }
    }

    if(*cur_pos != '"')
        throw MDX_missing_delimiter('"');

    cur_pos++;

    return cur_pos;
}

const char* MDX_FieldType<PGELIST<bool>>::load(PGELIST<bool>& dest, const char* field_data)
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
            throw(MDX_bad_array(dest.size() + 1));

        cur_pos++;
    }

    return cur_pos;
}
