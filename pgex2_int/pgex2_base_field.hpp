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
#ifndef PGEX2_BASE_FIELD_HPP
#define PGEX2_BASE_FIELD_HPP

#include "../pge_file_lib_globs.h"

/*! \file pgex2_base_field.hpp
 *
 *  \brief Contains templates for individual fields
 *
 * This is a new implementation but supports precisely the same format as PGE-X
 *
 */

const char* PGEX2_find_next_term(const char* line);
const char* PGEX2_find_next_list_item(const char* line);

template<class obj_t>
struct PGEX2_BaseField
{
protected:
    const char* m_field_name;

    PGEX2_BaseField(const char* field_name)
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
struct PGEX2_FieldType
{
    static const char* load(field_t& dest, const char* field_data);
};

template<class subtype_t>
struct PGEX2_FieldType<PGELIST<subtype_t>>
{
    static const char* load(PGELIST<subtype_t>& dest, const char* field_data);
};

template<>
struct PGEX2_FieldType<PGELIST<bool>>
{
    static const char* load(PGELIST<bool>& dest, const char* field_data);
};

template<class subtype_t>
const char* PGEX2_FieldType<PGELIST<subtype_t>>::load(PGELIST<subtype_t>& dest, const char* field_data)
{
    dest.clear();

    const char* cur_pos = field_data;
    if(*cur_pos != '[')
    {
        // this is an error
        return cur_pos;
    }

    cur_pos++;

    while(*cur_pos != ']' && *cur_pos != '\0')
    {
        dest.emplace_back();
        cur_pos = PGEX2_FieldType<subtype_t>::load(dest.back(), cur_pos);
        cur_pos = PGEX2_find_next_list_item(cur_pos);
    }

    if(*cur_pos != ']')
    {
        // this is an error
        return cur_pos;
    }

    cur_pos++;

    return cur_pos;
}

template<class obj_loader_t>
struct PGEX2_FieldType_ObjectList
{
    static const obj_loader_t s_obj_loader;
    static const char* load(PGELIST<typename obj_loader_t::obj_t>& dest, const char* field_data);
};

template<class obj_loader_t>
const char* PGEX2_FieldType_ObjectList<obj_loader_t>::load(PGELIST<typename obj_loader_t::obj_t>& dest, const char* field_data)
{
    dest.clear();

    const char* cur_pos = field_data;
    if(*cur_pos != '[')
    {
        // this is an error
        return cur_pos;
    }

    cur_pos++;

    PGESTRING object_string;

    while(*cur_pos != ']' && *cur_pos != '\0')
    {
        dest.emplace_back();

        cur_pos = PGEX2_FieldType<PGESTRING>::load(object_string, cur_pos);
        cur_pos = PGEX2_find_next_list_item(cur_pos);

        s_obj_loader.load_object(dest.back(), object_string.c_str());
    }

    if(*cur_pos != ']')
    {
        // this is an error
        return cur_pos;
    }

    cur_pos++;

    return cur_pos;
}

template<class obj_t, class field_t>
struct PGEX2_Field : public PGEX2_BaseField<obj_t>
{
    field_t obj_t::* m_field = nullptr;

    template<class parent_t>
    PGEX2_Field(parent_t* parent, const char* field_name, field_t obj_t::* field)
        : PGEX2_BaseField<obj_t>(field_name), m_field(field)
    {
        parent->m_fields.push_back(this);
    }

    virtual const char* do_load(obj_t& dest, const char* field_data) const
    {
        return PGEX2_find_next_term(PGEX2_FieldType<field_t>::load(dest.*m_field, field_data));
    }
};

template<class obj_t>
struct PGEX2_UniqueField : public PGEX2_BaseField<obj_t>
{
    using load_func_t = const char* (*)(obj_t& dest, const char* field_data);
    // using save_func_t = const char* (*)(obj_t& dest, const char* field_data);

    load_func_t m_load_func = nullptr;
    // save_func_t m_save_func = nullptr;

    template<class parent_t>
    PGEX2_UniqueField(parent_t* parent, const char* field_name, load_func_t load_func)
        : PGEX2_BaseField<obj_t>(field_name), m_load_func(load_func)
    {
        parent->m_fields.push_back(this);
    }

    virtual const char* do_load(obj_t& dest, const char* field_data) const
    {
        if(!m_load_func)
            return field_data;

        return m_load_func(dest, field_data);
    }
};

#endif // #ifndef PGEX2_BASE_FIELD_HPP
