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

#include "lvl_filedata.h"
#include "file_formats.h"
#include "pge_file_lib_private.h"
#include "pge_file_lib_globs.h"

#include "mdx/mdx.h"
#include "pgex/file_rw_pgex.h"
#include "smbx38a/file_rw_38a.h"

namespace PGEFL
{

namespace SAX
{

static int s_detect_level_format(PGE_FileFormats_misc::TextInput &file)
{
    PGESTRING firstLine;

    file.readLine(firstLine);
    file.seek(0, PGE_FileFormats_misc::TextInput::begin);

    if(PGE_StartsWith(firstLine, "SMBXFile"))
        return LevelData::SMBX38A;
    else if(PGE_FileFormats_misc::PGE_DetectSMBXFile(firstLine))
        return LevelData::SMBX64;
    else
        return LevelData::PGEX;
}

bool OpenLevelFile(PGE_FileFormats_misc::TextInput &file, LevelLoadCallbacks &callbacks, int format)
{
    if(format == -1)
        format = s_detect_level_format(file);

    if(format == LevelData::SMBX38A)
        return PGEFL::SMBX38A::ReadSMBX38ALvlFile(file, callbacks);
    else if(format == LevelData::PGEX && !FileFormats::g_use_mdx)
        return PGEFL::PGEX::ReadExtendedLvlFile(file, callbacks);
    else
        return MDX_load_level(file, callbacks);
}

bool OpenLevelFileHeader(PGE_FileFormats_misc::TextInput &file, LevelLoadCallbacks &callbacks, int format)
{
    if(format == -1)
        format = s_detect_level_format(file);

    if(format == LevelData::SMBX38A)
        return PGEFL::SMBX38A::ReadSMBX38ALvlFileHeader(file, callbacks);
    else if(format == LevelData::PGEX && !FileFormats::g_use_mdx)
        return PGEFL::PGEX::ReadExtendedLvlFileHeader(file, callbacks);
    else
        return MDX_load_level(file, callbacks);
}

bool SaveLevelFile(PGE_FileFormats_misc::TextOutput &file, LevelSaveCallbacks &callbacks, int format, int version)
{
    if(format == LevelData::SMBX38A)
        return PGEFL::SMBX38A::WriteSMBX38ALvlFile(file, callbacks, version);
    else if(format == LevelData::PGEX && !FileFormats::g_use_mdx)
        return PGEFL::PGEX::WriteExtendedLvlFile(file, callbacks);
    else
        return MDX_save_level(file, callbacks);
}

} // namespace SAX

} // namespace PGEFL
