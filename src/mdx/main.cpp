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

/*! \file main.cpp
 *
 *  \brief Implements defines MDX functions for loading a level object
 *
 * This is a new implementation but supports precisely the same format as PGE-X
 *
 */

#include <SDL2/SDL_rwops.h>

#include "mdx_level_file.h"
#include "mdx_world_file.h"
#include "mdx_gamesave_file.h"
#include "../../file_formats.h"
#include "../../pge_file_lib_private.h"
#include "../../pge_file_lib_globs.h"


#ifdef PGE_FILES_QT
inline const char* c_str(const PGESTRING& s)
{
    return qPrintable(s);
}
#else
inline const char* c_str(const PGESTRING& s)
{
    return s.c_str();
}
#endif

#ifdef WORLD_TEST
#define XData WorldData
#define MDX_load_x MDX_load_world
#define MDX_save_x MDX_save_world
#define OpenXFileT OpenWorldFileT
#define ReadExtendedXFile ReadExtendedWldFile
#define ReadExtendedXFileRaw ReadExtendedWldFileRaw
#define SaveXFile SaveWorldFile
#define SaveXData SaveWorldData
#define X_PGEX WLD_PGEX
#define X_SMBX64 WLD_SMBX64
#elif defined(SAVE_TEST)
#define XData GamesaveData
#define MDX_load_x MDX_load_gamesave
#define MDX_save_x MDX_save_gamesave
#define OpenXFileT ReadSMBX64SavFile
#define ReadExtendedXFile ReadExtendedSaveFile
#define ReadExtendedXFileRaw ReadExtendedSaveFileRaw
#define SaveXFile WriteExtendedSaveFileF
#define SaveXData WriteExtendedSaveFileRaw
#else
#define XData LevelData
#define MDX_load_x MDX_load_level
#define MDX_save_x MDX_save_level
#define OpenXFileT OpenLevelFileT
#define ReadExtendedXFile ReadExtendedLvlFile
#define ReadExtendedXFileRaw ReadExtendedLvlFileRaw
#define SaveXFile SaveLevelFile
#define SaveXData SaveLevelData
#define X_PGEX LVL_PGEX
#define X_SMBX64 LVL_SMBX64
#endif

int main(int argc, char** argv)
{
    XData FileData;
    XData FileData_Backup;

    bool rw = false;
    bool val = false;
    bool from_old = false;

    if(strcmp(argv[argc - 1], "-rw") == 0)
    {
        rw = true;
        argc -= 1;
    }

    if(strcmp(argv[argc - 1], "-val") == 0)
    {
        val = true;
        argc -= 1;
    }

    if(strcmp(argv[argc - 1], "-old") == 0)
    {
        from_old = true;
        argc -= 1;
    }

    PGE_FileFormats_misc::RWopsTextInput input(SDL_RWFromFile(argv[1], "rb"), argv[1]);

    if(from_old)
    {
        if(!FileFormats::OpenXFileT(input, FileData))
        {
            printf("LVL->MDX: input invalid at %ld\n", FileData.meta.ERROR_linenum);
            printf("[%s]\n", c_str(FileData.meta.ERROR_linedata));
            printf("%s\n", c_str(FileData.meta.ERROR_info));
            *(int*)(nullptr) = -1;
        }

        PGESTRING out_mdx_1_raw;

        auto outf = PGE_FileFormats_misc::RawTextOutput(&out_mdx_1_raw);
        MDX_save_x(outf, FileData);

        PGE_FileFormats_misc::RawTextInput input2(&out_mdx_1_raw, argv[1]);

        if(!MDX_load_x(input2, FileData))
        {
            printf("MDX->MDX: input2 invalid at %ld\n", FileData.meta.ERROR_linenum);
            printf("[%s]\n", c_str(FileData.meta.ERROR_linedata));
            printf("%s\n", c_str(FileData.meta.ERROR_info));
            *(int*)(nullptr) = -1;
        }

        PGESTRING out_mdx_2_raw;

        auto outf2 = PGE_FileFormats_misc::RawTextOutput(&out_mdx_2_raw);
        MDX_save_x(outf2, FileData);

        if(out_mdx_1_raw != out_mdx_2_raw)
            *(int*)(nullptr) = -1;

        return 0;
    }

    bool mdx_succ = true;
    if(!MDX_load_x(input, FileData))
    {
        printf("There was an error on line %ld\n", FileData.meta.ERROR_linenum);
        printf("[%s]\n", c_str(FileData.meta.ERROR_linedata));
        printf("%s\n", c_str(FileData.meta.ERROR_info));
        mdx_succ = false;
    }

    input.seek(0, PGE_FileFormats_misc::TextInput::begin);

    if(argc > 2)
    {
        if(mdx_succ)
        {
            PGESTRING arg2 = argv[2];
#ifndef SAVE_TEST
            auto out_format = (arg2[arg2.size() - 1] == 'x') ? FileFormats::X_PGEX : FileFormats::X_SMBX64;

            if(out_format == FileFormats::X_PGEX)
#else
            if(true)
#endif
            {
                PGE_FileFormats_misc::TextFileOutput outf(arg2);
                MDX_save_x(outf, FileData);
            }
#ifndef SAVE_TEST
            else
                FileFormats::SaveXFile(FileData, arg2, out_format);
#endif
        }
    }

    if(argc > 3)
    {
        PGESTRING arg3 = argv[3];
#ifndef SAVE_TEST
        auto out_format = (arg3[arg3.size() - 1] == 'x') ? FileFormats::X_PGEX : FileFormats::X_SMBX64;

        if(FileFormats::ReadExtendedXFile(input, FileData))
            FileFormats::SaveXFile(FileData, arg3, out_format);
#else
        if(FileFormats::ReadExtendedXFile(input, FileData))
            FileFormats::SaveXFile(arg3, FileData);
#endif
    }

    if(argc == 2)
    {
        PGESTRING out_mdx_raw;
        PGESTRING out_pgex_raw;

        if(mdx_succ)
        {
            if(rw)
            {
                auto outf = PGE_FileFormats_misc::RawTextOutput(&out_mdx_raw);
                MDX_save_x(outf, FileData);

                mdx_succ = false;

                FileData_Backup = std::move(FileData);

                try
                {
                    mdx_succ = FileFormats::ReadExtendedXFileRaw(out_mdx_raw, argv[1], FileData);

                    if(!mdx_succ)
                    {
                        printf("MDX->PGEX: There was an error on line %ld\n", FileData.meta.ERROR_linenum);
                        printf("[%s]\n", c_str(FileData.meta.ERROR_linedata));
                        printf("%s\n", c_str(FileData.meta.ERROR_info));
                    }
                }
                catch(const std::exception& e)
                {
                    printf("MDX->PGEX: PGE-X had an uncaught exc: %s\n", e.what());
                }
            }

            if(mdx_succ)
#ifndef SAVE_TEST
                FileFormats::SaveXData(FileData, out_mdx_raw, FileFormats::X_PGEX);
#else
                FileFormats::SaveXData(FileData, out_mdx_raw);
#endif
            else
                out_mdx_raw = "";
        }

        bool pgex_succ = false;
        if(rw)
        {
            FileData = std::move(FileData_Backup);
            pgex_succ = mdx_succ;
        }
        else
        {
            try
            {
                pgex_succ = FileFormats::ReadExtendedXFile(input, FileData);

                if(!pgex_succ)
                {
                    printf("There was an error on line %ld\n", FileData.meta.ERROR_linenum);
                    printf("[%s]\n", c_str(FileData.meta.ERROR_linedata));
                    printf("%s\n", c_str(FileData.meta.ERROR_info));
                }
            }
            catch(const std::exception& e)
            {
                printf("PGE-X had an uncaught exc: %s\n", e.what());
            }
        }

        if(pgex_succ)
#ifndef SAVE_TEST
                FileFormats::SaveXData(FileData, out_pgex_raw, FileFormats::X_PGEX);
#else
                FileFormats::SaveXData(FileData, out_pgex_raw);
#endif

#ifdef PGE_FILES_QT
        std::string out_mdx = out_mdx_raw.toStdString();
        std::string out_pgex = out_pgex_raw.toStdString();
#else
        std::string& out_mdx = out_mdx_raw;
        std::string& out_pgex = out_pgex_raw;
#endif

        if(val && !mdx_succ)
            *(int*)(nullptr) = 7;

        if(out_mdx != out_pgex)
        {
            auto out_its = std::mismatch(out_mdx.begin(), (out_mdx.size() < out_pgex.size()) ? (out_mdx.end()) : (out_mdx.begin() + out_pgex.size()), out_pgex.begin());

            int context_max = 20;
            int context = context_max;

            auto mdx_begin = out_mdx.begin();
            if(out_its.first - out_mdx.begin() > context_max)
                mdx_begin = out_its.first - context_max;
            else
                context = out_its.first - out_mdx.begin();

            if(out_mdx.end() - out_its.first > context_max)
                *(out_its.first + context_max) = '\0';

            auto pgex_begin = out_pgex.begin();
            if(out_its.second - out_pgex.begin() > context_max)
                pgex_begin = out_its.second - context_max;

            if(out_pgex.end() - out_its.second > context_max)
                *(out_its.second + context_max) = '\0';

            printf(" MDX %s\n", &*mdx_begin);
            for(int i = 0; i < context + 5; i++)
                printf(" ");
            printf("|");
            printf("\n");
            printf("PGEX %s\n", &*pgex_begin);

            *(int*)(nullptr) = 3;
        }
    }

    return 0;
}
