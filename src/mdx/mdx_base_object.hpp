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
#ifndef MDX_BASE_OBJECT_HPP
#define MDX_BASE_OBJECT_HPP

#include <vector>
#include "mdx/mdx_base_field.hpp"

/*! \file mdx_base_object.hpp
 *
 *  \brief Contains templates for objects
 *
 * This is a new implementation but supports precisely the same format as PGE-X
 *
 */

template<class _obj_t>
struct MDX_BaseObject
{
    template<class obj_loader_t> friend struct MDX_FieldType_ObjectList;
    template<class obj_loader_t> friend struct MDX_FieldType_Object;

    using obj_t = _obj_t;

    template<class field_t> using field = MDX_Field<obj_t, field_t>;
    std::vector<MDX_BaseField<obj_t>*> m_fields;

protected:
    void load_object(obj_t& dest, const char* line) const
    {
        const char* cur_data = line;
        size_t next_field = 0;

        while(*cur_data != '\0')
        {
            size_t try_field = next_field;
            for(; try_field < m_fields.size(); try_field++)
            {
                if(m_fields[try_field]->try_load(dest, cur_data))
                {
                    if(try_field == next_field)
                        next_field++;

                    break;
                }
            }

            if(try_field >= m_fields.size())
                cur_data = MDX_skip_term(cur_data);
        }
    }
};

template<class _obj_t>
struct MDX_Object;

#endif // #ifndef MDX_BASE_OBJECT_HPP
