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
#ifndef MDX_BASE_FILE_HPP
#define MDX_BASE_FILE_HPP

/*! \file mdx_base_file.hpp
 *
 *  \brief Contains templates for file formats
 *
 * This is a new implementation but supports precisely the same format as PGE-X
 *
 */

#include <vector>
#include <cstddef>
#include <cstring>

#include <string>

#include "pge_file_lib_globs.h"
#include "mdx/mdx_base_field.hpp"
#include "mdx/mdx_base_section.hpp"

template<class _load_callbacks_t, class _save_callbacks_t>
struct MDX_File
{
    using load_callbacks_t = _load_callbacks_t;
    using save_callbacks_t = _save_callbacks_t;
    template<class obj_t> using section = MDX_Section<load_callbacks_t, save_callbacks_t, obj_t>;
    template<class obj_t> using section_single = MDX_Section<load_callbacks_t, save_callbacks_t, obj_t, true>;

    std::vector<MDX_BaseSection<load_callbacks_t, save_callbacks_t>*> m_sections;

    bool load_file(PGE_FileFormats_misc::TextInput& inf, const load_callbacks_t& cb)
    {
#ifdef PGE_FILES_QT
        QString utf16_cur_line;
#endif

        std::string cur_line;

        for(auto* section : m_sections)
            section->reset();

        try
        {
            inf.seek(0, PGE_FileFormats_misc::TextFileInput::begin);

            while(!inf.eof())
            {
#ifndef PGE_FILES_QT
                inf.readLine(cur_line);
#else
                inf.readLine(utf16_cur_line);
                cur_line = utf16_cur_line.toStdString();
#endif

                bool handled = false;
                for(auto* section : m_sections)
                {
                    if(section->try_load(cb, inf, cur_line))
                    {
                        handled = true;
                        break;
                    }
                }

                if(!handled)
                {
                    // ignore line if all spaces
                    bool all_spaces = true;
                    for(const auto c : cur_line)
                    {
                        if(c != ' ')
                            all_spaces = false;
                    }
                    if(all_spaces)
                        continue;

                    // otherwise, treat as unrecognized section and skip it
                    if(cur_line.size() != strlen(cur_line.c_str()))
                        throw MDX_parse_error_misc("Bad section name");
                    else
                    {
                        std::string section_name = cur_line;
                        MDX_skip_section(inf, cur_line, section_name.c_str());
                    }
                }
            }
        }
        catch(const std::exception& e)
        {
            if(!cb.on_error)
                return false;

            FileFormatsError err;

#ifndef PGE_FILES_QT
            err.ERROR_info = "Failed to parse PGEX file (line ";
            err.ERROR_info += std::to_string(inf.getCurrentLineNumber());
            err.ERROR_info += ")\n";

            err.add_exc_info(e, inf.getCurrentLineNumber(), std::move(cur_line));
#else
            err.ERROR_info = "Failed to parse PGEX file (line ";
            err.ERROR_info += QString::number(inf.getCurrentLineNumber());
            err.ERROR_info += ")\n";

            err.add_exc_info(e, inf.getCurrentLineNumber(), std::move(utf16_cur_line));
#endif

            cb.on_error(cb.userdata, err);
            return false;
        }

        return true;
    }
};

#endif // #ifndef MDX_BASE_FILE_HPP
