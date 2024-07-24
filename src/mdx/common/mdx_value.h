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

#pragma once
#ifndef MDX_VALUE_H
#define MDX_VALUE_H

/*! \file mdx_value.h
 *
 *  \brief Code to support saving/loading values
 *
 * This is a new implementation but supports precisely the same format as PGE-X
 *
 */

template<class field_t>
struct MDX_Value
{
    static const char* load(field_t& dest, const char* field_data);

    /* tries to save the field, and returns false (restoring out to its original state) if this is impossible */
    static bool save(std::string& out, const field_t& src);

    /* checks if src matches a reference (which is assumed to be a default value) */
    static bool is_ref(const field_t& src, const field_t& reference)
    {
        return src == reference;
    }
};

template<class subtype_t>
struct MDX_Value<PGELIST<subtype_t>>
{
    static const char* load(PGELIST<subtype_t>& dest, const char* field_data);
    static bool save(std::string& out, const PGELIST<subtype_t>& src);
    static bool is_ref(const PGELIST<subtype_t>& src, const PGELIST<subtype_t>& /*reference*/)
    {
        return src.size() == 0;
    }
};

template<>
struct MDX_Value<PGELIST<bool>>
{
    static const char* load(PGELIST<bool>& dest, const char* field_data);
    static bool save(std::string& out, const PGELIST<bool>& src);
    static bool is_ref(const PGELIST<bool>& src, const PGELIST<bool>& /*reference*/)
    {
        return src.size() == 0;
    }
};

inline const char* MDX_finish_list_item(const char* line)
{
    if(*line == ']')
        return line;
    else if(*line == ',')
        return line + 1;
    else
        throw MDX_missing_delimiter(',');
}

template<class subtype_t>
const char* MDX_Value<PGELIST<subtype_t>>::load(PGELIST<subtype_t>& dest, const char* field_data)
{
    dest.clear();

    const char* cur_pos = field_data;
    if(*cur_pos != '[')
        throw MDX_missing_delimiter('[');

    cur_pos++;

    while(*cur_pos != ']' && *cur_pos != '\0')
    {
#ifndef PGE_FILES_QT
        dest.emplace_back();
#else
        dest.push_back(subtype_t());
#endif

        try
        {
            cur_pos = MDX_Value<subtype_t>::load(dest.back(), cur_pos);
            cur_pos = MDX_finish_list_item(cur_pos);
        }
        catch(const MDX_parse_error&)
        {
            std::throw_with_nested(MDX_bad_array(dest.size()));
        }
    }

    if(*(cur_pos - 1) == ',')
        throw MDX_unexpected_character(']');

    if(*cur_pos != ']')
        throw MDX_missing_delimiter(']');

    cur_pos++;

    return cur_pos;
}

template<class subtype_t>
bool MDX_Value<PGELIST<subtype_t>>::save(std::string& out, const PGELIST<subtype_t>& src)
{
    out.push_back('[');

    for(const subtype_t& s : src)
    {
        if(MDX_Value<subtype_t>::save(out, s))
            out.push_back(',');
    }

    // close the array
    if(out.back() == ',')
    {
        out.back() = ']';
        return true;
    }
    // nothing was written, remove the '['
    else
    {
        out.pop_back();
        return false;
    }
}

template<class obj_loader_t>
struct MDX_Value_Object
{
    static const obj_loader_t s_obj_loader;
    static const char* load(typename obj_loader_t::obj_t& dest, const char* field_data);
    static bool save(std::string& out, const typename obj_loader_t::obj_t& src);
    static bool is_ref(const PGELIST<bool>& src, const PGELIST<bool>& /*reference*/)
    {
        return false;
    }
};

template<class obj_loader_t>
const char* MDX_Value_Object<obj_loader_t>::load(typename obj_loader_t::obj_t& dest, const char* field_data)
{
    dest = typename obj_loader_t::obj_t();

    std::string object_string;

    const char* next = MDX_Value<std::string>::load(object_string, field_data);

    s_obj_loader.load_object(&dest, object_string.c_str());

    return next;
}

template<class obj_loader_t>
bool MDX_Value_Object<obj_loader_t>::save(std::string& out, const typename obj_loader_t::obj_t& src)
{
    const typename obj_loader_t::obj_t ref;
    std::string object_string;

    if(!s_obj_loader.save_object(object_string, &src, &ref))
        return false;

    MDX_Value<std::string>::save(out, object_string);

    return true;
}

template<class obj_loader_t>
struct MDX_Value_ObjectList
{
    static const obj_loader_t s_obj_loader;
    static const char* load(PGELIST<typename obj_loader_t::obj_t>& dest, const char* field_data);
    static bool save(std::string& out, const PGELIST<typename obj_loader_t::obj_t>& src);
    static bool is_ref(const PGELIST<typename obj_loader_t::obj_t>& src, const PGELIST<typename obj_loader_t::obj_t>& /*reference*/)
    {
        return (src.size() == 0);
    }
};

template<class obj_loader_t>
const char* MDX_Value_ObjectList<obj_loader_t>::load(PGELIST<typename obj_loader_t::obj_t>& dest, const char* field_data)
{
    dest.clear();

    const char* cur_pos = field_data;
    if(*cur_pos != '[')
        throw MDX_missing_delimiter('[');

    cur_pos++;

    std::string object_string;

    while(*cur_pos != ']' && *cur_pos != '\0')
    {
#ifndef PGE_FILES_QT
        dest.emplace_back();
#else
        dest.push_back(typename obj_loader_t::obj_t());
#endif

        try
        {
            cur_pos = MDX_Value<std::string>::load(object_string, cur_pos);
            cur_pos = MDX_finish_list_item(cur_pos);

            s_obj_loader.load_object(&dest.back(), object_string.c_str());
        }
        catch(const MDX_parse_error&)
        {
            std::throw_with_nested(MDX_bad_array(dest.size()));
        }
    }

    if(*(cur_pos - 1) == ',')
        throw MDX_unexpected_character(']');

    if(*cur_pos != ']')
        throw MDX_missing_delimiter(']');

    cur_pos++;

    return cur_pos;
}

template<class obj_loader_t>
bool MDX_Value_ObjectList<obj_loader_t>::save(std::string& out, const PGELIST<typename obj_loader_t::obj_t>& src)
{
    std::string object_string;
    typename obj_loader_t::obj_t ref;

    out.push_back('[');

    for(const auto& s : src)
    {
        object_string.clear();

        if(s_obj_loader.save_object(object_string, &s, &ref))
        {
            MDX_Value<std::string>::save(out, object_string);

            out.push_back(',');
        }
    }

    // close the array
    if(out.back() == ',')
    {
        out.back() = ']';
        return true;
    }
    // nothing was written, remove the '['
    else
    {
        out.pop_back();
        return false;
    }
}

#endif // #ifndef MDX_VALUE_H
