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
#include <cfloat>
#include <limits>
#include <cmath>
#include <cstdarg>
#include "mdx/common/mdx_field.h"
#include "mdx/common/mdx_object.h"
#include "mdx/common/milo_yip/itoa.h"

#include "pge_file_lib_globs.h"

/*! \file mdx_field_impl.cpp
 *
 *  \brief Contains code to load specific field types
 *
 */

MDX_BaseField::MDX_BaseField(MDX_BaseObject* parent, const char* field_name, SaveMode save_mode)
    : m_save_mode(save_mode), m_field_name(field_name)
{
    parent->m_fields.push_back(this);
}

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

const char* MDX_load_int(int& dest, const char* field_data)
{
    return s_load_int<int, 1>(dest, field_data);
}

const char* MDX_load_long(long& dest, const char* field_data)
{
    return s_load_int<long, 1>(dest, field_data);
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

    if((*field_data < '0' || *field_data > '9') && *field_data != '.')
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

        if(value >= std::numeric_limits<double>::max() / 10) [[unlikely]]
            return ret_error;

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

    // allow plus sign (but not followed by minus sign)
    if(field_data[0] == '+' && field_data[1] != '-')
        field_data++;

    const char* exp_start = field_data;
    int allowed_chars = (*exp_start == '-') ? 5 : 4;

    int exponent;
    field_data = s_load_int<int, 1>(exponent, field_data);
    if(field_data == exp_start || field_data - exp_start > allowed_chars)
        return ret_error;

    if(exponent > DBL_MAX_10_EXP)
        return ret_error;

    dest = sign * value * std::pow(10, exponent);
    return field_data;
}

static void s_sprintf_append(std::string& out, const char* format, ...)
{
    size_t old_size = out.size();
    out.resize(old_size + 32);

    va_list values;
    va_start(values, format);
    int printed = vsnprintf(&out[old_size], 32, format, values);
    va_end(values);

    if(printed >= 32)
    {
        // we would have lost something, but none of the numbers should ever take this long
        out.resize(old_size + 31);
    }
    else
        out.resize(old_size + printed);
}

template<>
const char* MDX_Value<int>::load(int& dest, const char* field_data)
{
    const char* str_end = s_load_int<int, 1>(dest, field_data);

    if(str_end == field_data)
        throw MDX_bad_term("Bad int");

    return str_end;
}

template<>
bool MDX_Value<int>::save(std::string& out, const int& src)
{
    out.resize(out.size() + 32);
    char* dest = &out[out.size() - 32];

    char* end = milo_yip::i32toa((int32_t)src, dest);

    out.resize(out.size() - 32 + end - dest);

    return true;
}

template<>
const char* MDX_Value<unsigned>::load(unsigned& dest, const char* field_data)
{
    const char* str_end = s_load_uint(dest, field_data);

    if(str_end == field_data)
        throw MDX_bad_term("Bad uint");

    return str_end;
}

template<>
bool MDX_Value<unsigned>::save(std::string& out, const unsigned& src)
{
    out.resize(out.size() + 32);
    char* dest = &out[out.size() - 32];

    char* end = milo_yip::u32toa((uint32_t)src, dest);

    out.resize(out.size() - 32 + end - dest);

    return true;
}

template<>
const char* MDX_Value<bool>::load(bool& dest, const char* field_data)
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
bool MDX_Value<bool>::save(std::string& out, const bool& src)
{
    if(src)
        out += '1';
    else
        out += '0';

    return true;
}

template<>
const char* MDX_Value<long>::load(long& dest, const char* field_data)
{
    const char* str_end = s_load_int<long, 1>(dest, field_data);

    if(str_end == field_data)
        throw MDX_bad_term("Bad long");

    return str_end;
}

template<>
bool MDX_Value<long>::save(std::string& out, const long& src)
{
    out.resize(out.size() + 32);
    char* dest = &out[out.size() - 32];

    char* end = milo_yip::i64toa((int64_t)src, dest);

    out.resize(out.size() - 32 + end - dest);

    return true;
}

template<>
const char* MDX_Value<unsigned long>::load(unsigned long& dest, const char* field_data)
{
    const char* str_end = s_load_uint(dest, field_data);

    if(str_end == field_data)
        throw MDX_bad_term("Bad ulong");

    return str_end;
}

template<>
bool MDX_Value<unsigned long>::save(std::string& out, const unsigned long& src)
{
    out.resize(out.size() + 32);
    char* dest = &out[out.size() - 32];

    char* end = milo_yip::u64toa((uint64_t)src, dest);

    out.resize(out.size() - 32 + end - dest);

    return true;
}

template<>
const char* MDX_Value<long long>::load(long long& dest, const char* field_data)
{
    const char* str_end = s_load_int<long long, 1>(dest, field_data);

    if(str_end == field_data)
        throw MDX_bad_term("Bad llong");

    return str_end;
}

template<>
bool MDX_Value<long long>::save(std::string& out, const long long& src)
{
    s_sprintf_append(out, "%lld", src);
    return true;
}

template<>
const char* MDX_Value<unsigned long long>::load(unsigned long long& dest, const char* field_data)
{
    const char* str_end = s_load_uint(dest, field_data);

    if(str_end == field_data)
        throw MDX_bad_term("Bad ullong");

    return str_end;
}

template<>
bool MDX_Value<unsigned long long>::save(std::string& out, const unsigned long long& src)
{
    s_sprintf_append(out, "%llu", src);
    return true;
}

template<>
const char* MDX_Value<float>::load(float& dest, const char* field_data)
{
    double ret;
    const char* str_end = s_load_double(ret, field_data);

    if(ret > std::numeric_limits<float>::max()
        || ret < -std::numeric_limits<float>::max()
        || str_end == field_data)
    {
        throw MDX_bad_term("Bad float");
    }

    dest = ret;

    return str_end;
}

template<>
bool MDX_Value<float>::save(std::string& out, const float& src)
{
    s_sprintf_append(out, "%.10g", src);
    return true;
}

template<>
const char* MDX_Value<double>::load(double& dest, const char* field_data)
{
    const char* str_end = s_load_double(dest, field_data);

    if(str_end == field_data)
        throw MDX_bad_term("Bad double");

    return str_end;
}

template<>
bool MDX_Value<double>::save(std::string& out, const double& src)
{
    s_sprintf_append(out, "%.10g", src);
    return true;
}

template<>
const char* MDX_Value<std::string>::load(std::string& dest, const char* field_data)
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

template<>
bool MDX_Value<std::string>::save(std::string& out, const std::string& src)
{
    out += '"';

    for(char c : src)
    {
        switch(c)
        {
        case '\n':
            out += '\\';
            out += 'n';
            break;
        case '\r':
            out += '\\';
            out += 'r';
            break;
        case '\"':
        case ';':
        case ':':
        case '[':
        case ']':
        case ',':
        case '%':
        case '\\':
            out += '\\';
            out += c;
            break;
        default:
            out += c;
            break;
        }
    }

    out += '"';

    return true;
}

#ifdef PGE_FILES_QT
template<>
const char* MDX_Value<QString>::load(QString& dest, const char* field_data)
{
    std::string dest_utf8;

    const char* ret = MDX_Value<std::string>::load(dest_utf8, field_data);
    dest = QString::fromStdString(dest_utf8);

    return ret;
}

template<>
bool MDX_Value<QString>::save(std::string& out, const QString& src)
{
    std::string src_utf8 = src.toStdString();

    return MDX_Value<std::string>::save(out, src_utf8);
}

template<>
const char* MDX_Value<QStringList>::load(QStringList& dest, const char* field_data)
{
    dest.clear();

    const char* cur_pos = field_data;
    if(*cur_pos != '[')
        throw MDX_missing_delimiter('[');

    cur_pos++;

    std::string got_utf8;

    while(*cur_pos != ']' && *cur_pos != '\0')
    {
        try
        {
            cur_pos = MDX_Value<std::string>::load(got_utf8, cur_pos);
            cur_pos = MDX_finish_list_item(cur_pos);
        }
        catch(const MDX_parse_error&)
        {
            std::throw_with_nested(MDX_bad_array(dest.size()));
        }

        dest.push_back(QString::fromStdString(got_utf8));
    }

    if(*(cur_pos - 1) == ',')
        throw MDX_unexpected_character(']');

    if(*cur_pos != ']')
        throw MDX_missing_delimiter(']');

    cur_pos++;

    return cur_pos;
}

template<>
bool MDX_Value<QStringList>::save(std::string& out, const QStringList& src)
{
    if(src.size() == 0)
        return false;

    std::string src_i_utf8;

    out.push_back('[');

    for(const auto& s : src)
    {
        src_i_utf8 = s.toStdString();

        MDX_Value<std::string>::save(out, src_i_utf8);
        out.push_back(',');
    }

    out.back() = ']';

    return true;
}
#endif

const char* MDX_Value<PGELIST<bool>>::load(PGELIST<bool>& dest, const char* field_data)
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

bool MDX_Value<PGELIST<bool>>::save(std::string& out, const PGELIST<bool>& src)
{
    for(bool i : src)
        out += (i) ? '1' : '0';

    return true;
}
