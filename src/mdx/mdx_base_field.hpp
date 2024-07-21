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
#ifndef MDX_BASE_FIELD_HPP
#define MDX_BASE_FIELD_HPP

#include "pge_file_lib_globs.h"
#include "mdx/mdx_exception.hpp"

#include <string>

/*! \file mdx_base_field.hpp
 *
 *  \brief Contains templates for individual fields
 *
 * This is a new implementation but supports precisely the same format as PGE-X
 *
 */

const char* MDX_skip_term(const char* line);

inline const char* MDX_finish_term(const char* line)
{
    if(*line != ';')
        throw MDX_missing_delimiter(';');

    return line + 1;
}

inline const char* MDX_finish_list_item(const char* line)
{
    if(*line == ']')
        return line;
    else if(*line == ',')
        return line + 1;
    else
        throw MDX_missing_delimiter(',');
}

template<class obj_t>
struct MDX_BaseField
{
protected:
    const char* m_field_name;

    MDX_BaseField(const char* field_name)
        : m_field_name(field_name) {}

    /* attempts to load the matched field to the destination, and returns the new load pointer following the ';'. */
    virtual const char* do_load(obj_t& dest, const char* field_data) const = 0;

public:
    /* attempts to match the field name. if successful, returns true and modifies the load pointer. */
    inline bool try_load(obj_t& dest, const char*& field_name) const
    {
        int i;

        for(i = 0; m_field_name[i] != '\0'; i++)
        {
            if(field_name[i] != m_field_name[i])
                return false;
        }

        if(field_name[i] == ':')
        {
            field_name = do_load(dest, field_name + i + 1);
            return true;
        }

        return false;
    }
};

template<class field_t>
struct MDX_FieldType
{
    static const char* load(field_t& dest, const char* field_data);
};

template<class subtype_t>
struct MDX_FieldType<PGELIST<subtype_t>>
{
    static const char* load(PGELIST<subtype_t>& dest, const char* field_data);
};

template<>
struct MDX_FieldType<PGELIST<bool>>
{
    static const char* load(PGELIST<bool>& dest, const char* field_data);
};

template<class subtype_t>
const char* MDX_FieldType<PGELIST<subtype_t>>::load(PGELIST<subtype_t>& dest, const char* field_data)
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
            cur_pos = MDX_FieldType<subtype_t>::load(dest.back(), cur_pos);
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

template<class obj_loader_t>
struct MDX_FieldType_Object
{
    static const obj_loader_t s_obj_loader;
    static const char* load(typename obj_loader_t::obj_t& dest, const char* field_data);
};

template<class obj_loader_t>
const char* MDX_FieldType_Object<obj_loader_t>::load(typename obj_loader_t::obj_t& dest, const char* field_data)
{
    dest = typename obj_loader_t::obj_t();

    std::string object_string;

    const char* next = MDX_FieldType<std::string>::load(object_string, field_data);

    s_obj_loader.load_object(dest, object_string.c_str());

    return next;
}

template<class obj_loader_t>
struct MDX_FieldType_ObjectList
{
    static const obj_loader_t s_obj_loader;
    static const char* load(PGELIST<typename obj_loader_t::obj_t>& dest, const char* field_data);
};

template<class obj_loader_t>
const char* MDX_FieldType_ObjectList<obj_loader_t>::load(PGELIST<typename obj_loader_t::obj_t>& dest, const char* field_data)
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
            cur_pos = MDX_FieldType<std::string>::load(object_string, cur_pos);
            cur_pos = MDX_finish_list_item(cur_pos);

            s_obj_loader.load_object(dest.back(), object_string.c_str());
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

template<class obj_t, class field_t>
struct MDX_Field : public MDX_BaseField<obj_t>
{
    field_t obj_t::* m_field = nullptr;

    template<class parent_t>
    MDX_Field(parent_t* parent, const char* field_name, field_t obj_t::* field)
        : MDX_BaseField<obj_t>(field_name), m_field(field)
    {
        parent->m_fields.push_back(this);
    }

    virtual const char* do_load(obj_t& dest, const char* field_data) const
    {
        try
        {
            return MDX_finish_term(MDX_FieldType<field_t>::load(dest.*m_field, field_data));
        }
        catch(const MDX_parse_error&)
        {
            std::throw_with_nested(MDX_bad_field(MDX_BaseField<obj_t>::m_field_name));
        }
    }
};

template<class obj_t>
struct MDX_UniqueField : public MDX_BaseField<obj_t>
{
    using load_func_t = const char* (*)(obj_t& dest, const char* field_data);
    // using save_func_t = const char* (*)(obj_t& dest, const char* field_data);

    load_func_t m_load_func = nullptr;
    // save_func_t m_save_func = nullptr;

    template<class parent_t>
    MDX_UniqueField(parent_t* parent, const char* field_name, load_func_t load_func)
        : MDX_BaseField<obj_t>(field_name), m_load_func(load_func)
    {
        parent->m_fields.push_back(this);
    }

    virtual const char* do_load(obj_t& dest, const char* field_data) const
    {
        if(!m_load_func)
            return field_data;

        try
        {
            return m_load_func(dest, field_data);
        }
        catch(const MDX_parse_error&)
        {
            std::throw_with_nested(MDX_bad_field(MDX_BaseField<obj_t>::m_field_name));
        }
    }
};

template<class obj_t, class substruct_t, class field_t>
struct MDX_NestedField : public MDX_BaseField<obj_t>
{
    substruct_t obj_t::* m_substruct = nullptr;
    field_t substruct_t::* m_field = nullptr;

    template<class parent_t>
    MDX_NestedField(parent_t* parent, const char* field_name, substruct_t obj_t::* substruct, field_t substruct_t::* field)
        : MDX_BaseField<obj_t>(field_name), m_substruct(substruct), m_field(field)
    {
        parent->m_fields.push_back(this);
    }

    virtual const char* do_load(obj_t& dest, const char* field_data) const
    {
        try
        {
            return MDX_finish_term(MDX_FieldType<field_t>::load(dest.*m_substruct.*m_field, field_data));
        }
        catch(const MDX_parse_error&)
        {
            std::throw_with_nested(MDX_bad_field(MDX_BaseField<obj_t>::m_field_name));
        }
    }
};

template<class obj_t>
struct MDX_FieldXtra : public MDX_BaseField<obj_t>
{
    template<class parent_t>
    MDX_FieldXtra(parent_t* parent)
        : MDX_BaseField<obj_t>("XTRA")
    {
        parent->m_fields.push_back(this);
    }

    virtual const char* do_load(obj_t& dest, const char* field_data) const
    {
        try
        {
            return MDX_finish_term(MDX_FieldType<PGESTRING>::load(dest.meta.custom_params, field_data));
        }
        catch(const MDX_parse_error&)
        {
            std::throw_with_nested(MDX_bad_field(MDX_BaseField<obj_t>::m_field_name));
        }
    }
};

#endif // #ifndef MDX_BASE_FIELD_HPP
