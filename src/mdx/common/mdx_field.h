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
#include "mdx/common/mdx_exception.h"

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

struct MDX_BaseObject;

struct MDX_BaseField
{
public:
    enum class SaveMode
    {
        normal,   //!< store if can_save returns true
        no_skip,  //!< always store
        not_only, //!< if only these values are non-default, the object will be ignored
    } m_save_mode = SaveMode::normal;

protected:
    const char* const m_field_name;

    MDX_BaseField(MDX_BaseObject* parent, const char* field_name, SaveMode save_mode = SaveMode::normal);

    /* attempts to load the matched field to the destination, and returns the new load pointer following the ';'. */
    virtual const char* do_load(void* dest, const char* field_data) const = 0;
    /* confirms whether the field is non-default. */
    virtual bool can_save(const void* src, const void* /* obj_t */ ref) const = 0;
    /* tries to write the field, and returns false if this is impossible. */
    virtual bool do_save(std::string& out, const void* src) const = 0;

public:
    /* attempts to match the field name. if successful, returns true and modifies the load pointer. */
    inline bool try_load(void* dest, const char*& field_name) const
    {
        int i;

        for(i = 0; m_field_name[i] != '\0'; i++)
        {
            if(field_name[i] != m_field_name[i])
                return false;
        }

        if(field_name[i] == ':')
        {
            try
            {
                field_name = MDX_finish_term(do_load(dest, field_name + i + 1));
            }
            catch(const MDX_parse_error&)
            {
                std::throw_with_nested(MDX_bad_field(m_field_name));
            }
            return true;
        }

        return false;
    }

    /* confirms whether the field is non-default, and writes it to out if so. */
    inline bool try_save(std::string& out, const void* src, const void* ref) const
    {
        if(m_save_mode != SaveMode::no_skip && !can_save(src, ref))
            return false;

        auto old_size = out.size();

        out += m_field_name;
        out += ':';

        bool do_skip = !do_save(out, src);
        if(do_skip)
        {
            out.resize(old_size);
            return false;
        }

        out += ';';

        return true;
    }
};

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

template<class obj_t, class field_t>
struct MDX_Field : public MDX_BaseField /* <obj_t> */
{
    using MDX_BaseField/*<obj_t>*/::m_field_name;
    using SaveMode = typename MDX_BaseField/*<obj_t>*/::SaveMode;

    field_t obj_t::* const m_field = nullptr;

    MDX_Field(MDX_BaseObject* parent, const char* field_name, field_t obj_t::* field, SaveMode save_mode = SaveMode::normal)
        : MDX_BaseField(parent, field_name, save_mode), m_field(field) {}

    virtual const char* do_load(void* _dest, const char* field_data) const
    {
        obj_t& dest = *reinterpret_cast<obj_t*>(_dest);

        return MDX_Value<field_t>::load(dest.*m_field, field_data);
    }

    virtual bool can_save(const void* _src, const void* _ref) const
    {
        const obj_t& src = *reinterpret_cast<const obj_t*>(_src);
        const obj_t& ref = *reinterpret_cast<const obj_t*>(_ref);

        return !MDX_Value<field_t>::is_ref(src.*m_field, ref.*m_field);
    }

    virtual bool do_save(std::string& out, const void* _src) const
    {
        const obj_t& src = *reinterpret_cast<const obj_t*>(_src);

        return MDX_Value<field_t>::save(out, src.*m_field);
    }
};

template<class obj_t, class field_t>
struct MDX_NonNegField : public MDX_Field<obj_t, field_t>
{
    using MDX_Field<obj_t, field_t>::MDX_Field;
    using MDX_Field<obj_t, field_t>::m_field;

    virtual const char* do_load(void* _dest, const char* field_data) const
    {
        obj_t& dest = *reinterpret_cast<obj_t*>(_dest);

        const char* ret = MDX_Value<field_t>::load(dest.*m_field, field_data);

        if(*field_data == '-' || dest.*m_field < 0)
            throw(MDX_bad_term("Negative value"));

        return ret;
    }
};

template<class obj_t>
struct MDX_UniqueField : public MDX_BaseField /* <obj_t> */
{
    using MDX_BaseField::m_field_name;

    using load_func_t = const char* (*)(obj_t& dest, const char* field_data);
    using save_func_t = bool (*)(std::string& out, const obj_t& src);

    load_func_t m_load_func = nullptr;
    save_func_t m_save_func = nullptr;

    MDX_UniqueField(MDX_BaseObject* parent, const char* field_name, load_func_t load_func, save_func_t save_func)
        : MDX_BaseField(parent, field_name), m_load_func(load_func), m_save_func(save_func) {}

    virtual const char* do_load(void* _dest, const char* field_data) const
    {
        obj_t& dest = *reinterpret_cast<obj_t*>(_dest);

        if(!m_load_func)
            return field_data;

        return m_load_func(dest, field_data);
    }

    virtual bool can_save(const void* src, const void* ref) const
    {
        (void)src; (void)ref;
        return (bool)(m_save_func);
    }

    virtual bool do_save(std::string& out, const void* _src) const
    {
        const obj_t& src = *reinterpret_cast<const obj_t*>(_src);

        return m_save_func(out, src);
    }
};

template<class obj_t, class substruct_t, class field_t>
struct MDX_NestedField : public MDX_BaseField
{
    using MDX_BaseField::m_field_name;

    substruct_t obj_t::* const m_substruct = nullptr;
    field_t substruct_t::* const m_field = nullptr;

    MDX_NestedField(MDX_BaseObject* parent, const char* field_name, substruct_t obj_t::* substruct, field_t substruct_t::* field)
        : MDX_BaseField(parent, field_name), m_substruct(substruct), m_field(field) {}

    virtual const char* do_load(void* _dest, const char* field_data) const
    {
        obj_t& dest = *reinterpret_cast<obj_t*>(_dest);

        return MDX_Value<field_t>::load(dest.*m_substruct.*m_field, field_data);
    }

    virtual bool can_save(const void* _src, const void* _ref) const
    {
        const obj_t& src = *reinterpret_cast<const obj_t*>(_src);
        const obj_t& ref = *reinterpret_cast<const obj_t*>(_ref);

        return !MDX_Value<field_t>::is_ref(src.*m_substruct.*m_field, ref.*m_substruct.*m_field);
    }

    virtual bool do_save(std::string& out, const void* _src) const
    {
        const obj_t& src = *reinterpret_cast<const obj_t*>(_src);

        return MDX_Value<field_t>::save(out, src.*m_substruct.*m_field);
    }
};

template<class obj_t, class substruct_t, class field_t>
struct MDX_NonNegNestedField : public MDX_NestedField<obj_t, substruct_t, field_t>
{
    using MDX_NestedField<obj_t, substruct_t, field_t>::MDX_NestedField;
    using MDX_NestedField<obj_t, substruct_t, field_t>::m_substruct;
    using MDX_NestedField<obj_t, substruct_t, field_t>::m_field;

    virtual const char* do_load(void* _dest, const char* field_data) const
    {
        obj_t& dest = *reinterpret_cast<obj_t*>(_dest);

        const char* ret = MDX_Value<field_t>::load(dest.*m_substruct.*m_field, field_data);

        if(*field_data == '-' || dest.*m_substruct.*m_field < 0)
            throw(MDX_bad_term("Illegal negative"));

        return ret;
    }
};

template<class obj_t>
struct MDX_FieldXtra : public MDX_BaseField /* <obj_t> */
{
    MDX_FieldXtra(MDX_BaseObject* parent)
        : MDX_BaseField(parent, "XTRA") {}

    virtual const char* do_load(void* _dest, const char* field_data) const
    {
        obj_t& dest = *reinterpret_cast<obj_t*>(_dest);
        return MDX_Value<PGESTRING>::load(dest.meta.custom_params, field_data);
    }

    virtual bool can_save(const void* _src, const void* ref) const
    {
        (void)ref;
        const obj_t& src = *reinterpret_cast<const obj_t*>(_src);

        return src.meta.custom_params != PGESTRING();
    }

    virtual bool do_save(std::string& out, const void* _src) const
    {
        const obj_t& src = *reinterpret_cast<const obj_t*>(_src);

        return MDX_Value<PGESTRING>::save(out, src.meta.custom_params);
    }
};

#endif // #ifndef MDX_BASE_FIELD_HPP
