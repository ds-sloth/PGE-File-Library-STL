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

/*!
 *  \file file_rw_38a.h
 *  \brief File read/write functions exported by the 38A parser
 */

#pragma once
#ifndef FILE_RW_38A_H
#define FILE_RW_38A_H

namespace PGEFL
{

namespace SMBX38A
{

bool ReadSMBX38ALvlFileHeader(PGE_FileFormats_misc::TextInput &inf, LevelLoadCallbacks &cb);
bool ReadSMBX38ALvlFile(PGE_FileFormats_misc::TextInput &inf, LevelLoadCallbacks &cb);
bool WriteSMBX38ALvlFile(PGE_FileFormats_misc::TextOutput &out, LevelSaveCallbacks &cb, unsigned int format_version);

} // namespace SMBX38A

} // namespace PGEFL

#endif // #ifndef FILE_RW_38A_H
