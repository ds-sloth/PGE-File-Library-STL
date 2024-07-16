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
#ifndef PGEX2_BASE_SECTION_HPP
#define PGEX2_BASE_SECTION_HPP

/*! \file pgex2_base_section.hpp
 *
 *  \brief Contains templates for file sections
 *
 * This is a new implementation but supports precisely the same format as PGE-X
 *
 */

#include <cstddef>
#include <cstdlib>
#include <cstring>

#include "../pge_file_lib_globs.h"
#include "pgex2_base_field.hpp"
#include "pgex2_base_object.hpp"

inline void PGEX2_skip_section(PGE_FileFormats_misc::TextInput& inf, PGESTRING& cur_line);

inline bool PGEX2_line_is_section_end(const PGESTRING& cur_line)
{
    return cur_line.size() > 4 && strncmp(cur_line.c_str() + cur_line.size() - 4, "_END", 4) == 0;
}

template<class callback_table_t>
struct PGEX2_BaseSection
{
    virtual bool try_load(const callback_table_t& table, PGE_FileFormats_misc::TextInput& inf, PGESTRING& cur_line) = 0;
};

template<class callback_table_t, class _obj_t>
struct PGEX2_Section : public PGEX2_Object<_obj_t>, public PGEX2_BaseSection<callback_table_t>
{
    using PGEX2_Object<_obj_t>::load_object;
    using obj_t = _obj_t;

private:
    // private fields for load-time
    obj_t m_obj;

    using load_callback_t = typename callback_table_t::template load_callback<obj_t>;
    using save_callback_t = typename callback_table_t::template save_callback<obj_t>;

    typedef load_callback_t callback_table_t::* load_callback_ptr_t;
    typedef save_callback_t callback_table_t::* save_callback_ptr_t;

    const char* m_section_name = "";

    load_callback_ptr_t m_load_callback = nullptr;
    save_callback_ptr_t m_save_callback = nullptr;

public:
    template<class parent_t>
    PGEX2_Section(parent_t* parent, const char* section_name, load_callback_ptr_t load_callback, save_callback_ptr_t save_callback)
        : m_section_name(section_name), m_load_callback(load_callback), m_save_callback(save_callback)
    {
        parent->m_sections.push_back(this);
    }

    /* attempts to match the field name. if successful, returns true and leaves the file pointer following the end of the section. */
    virtual bool try_load(const callback_table_t& cb, PGE_FileFormats_misc::TextInput& inf, PGESTRING& cur_line)
    {
        // check match
        if(cur_line != m_section_name)
            return false;

        // skip if there is no callback registered
        const load_callback_t callback = cb.*m_load_callback;

        if(!callback)
            return false;

        while(true)
        {
            inf.readLine(cur_line);

            if(cur_line.empty())
            {
                // unexpected EOF
                if(inf.eof())
                    return true;
                // invalid line
                else
                {
                    // complain, eventually...
                }
            }
            // ordinary line
            else if(*(cur_line.end() - 1) == ';')
            {
                m_obj = obj_t();
                load_object(m_obj, cur_line.c_str());

                if(!callback(cb.userdata, m_obj))
                {
                    PGEX2_skip_section(inf, cur_line);
                    return true;
                }
            }
            // section end line
            else if(PGEX2_line_is_section_end(cur_line) && strncmp(cur_line.c_str(), m_section_name, cur_line.size() - 4) == 0)
            {
                return true;
            }
            // invalid line
            else
            {
                // complain, eventually...
            }
        }
    }
};

inline void PGEX2_skip_section(PGE_FileFormats_misc::TextInput& inf, PGESTRING& cur_line)
{
    while(!inf.eof())
    {
        inf.readLine(cur_line);

        if(PGEX2_line_is_section_end(cur_line))
        {
            printf("Skipped until [%s]\n", cur_line.c_str());
            break;
        }
    }
}

#endif // #ifndef PGEX2_BASE_SECTION_HPP
