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
 *  \file pgex2_level_file.h
 *  \brief Contains data structure definitions for the PGEX2 level loader
 */

#pragma once
#ifndef PGEX2_LEVEL_FILE_H
#define PGEX2_LEVEL_FILE_H

#include "pgex2_globals.h"

#include "../pge_file_lib_globs.h"
#include "../lvl_filedata.h"

struct PGEX2_LevelHead
{
    PGESTRING LevelName;
    unsigned stars = 0;
    PGESTRING open_level_on_fail;
    unsigned open_level_on_fail_warpID = 0;
    PGELIST<PGESTRING> player_names_overrides;
    PGESTRING custom_params;
    PGESTRING configPackId;
    PGELIST<PGESTRING> music_files;
};

struct PGEX2_LevelCallbacks : PGEX2_BaseCallbacks
{
    load_callback<PGEX2_LevelHead> load_head = nullptr;
    save_callback<PGEX2_LevelHead> save_head = nullptr;

    load_callback<Bookmark> load_bookmark = nullptr;
    save_callback<Bookmark> save_bookmark = nullptr;

    load_callback<CrashData> load_crash_data = nullptr;
    save_callback<CrashData> save_crash_data = nullptr;

    load_callback<LevelSection> load_section = nullptr;
    save_callback<LevelSection> save_section = nullptr;

    load_callback<PlayerPoint> load_startpoint = nullptr;
    save_callback<PlayerPoint> save_startpoint = nullptr;

    load_callback<LevelBlock> load_block = nullptr;
    save_callback<LevelBlock> save_block = nullptr;

    load_callback<LevelBGO> load_bgo = nullptr;
    save_callback<LevelBGO> save_bgo = nullptr;

    load_callback<LevelNPC> load_npc = nullptr;
    save_callback<LevelNPC> save_npc = nullptr;

    load_callback<LevelPhysEnv> load_phys = nullptr;
    save_callback<LevelPhysEnv> save_phys = nullptr;

    load_callback<LevelDoor> load_warp = nullptr;
    save_callback<LevelDoor> save_warp = nullptr;

    load_callback<LevelLayer> load_layer = nullptr;
    save_callback<LevelLayer> save_layer = nullptr;

    load_callback<LevelSMBX64Event> load_event = nullptr;
    save_callback<LevelSMBX64Event> save_event = nullptr;

    load_callback<LevelVariable> load_var = nullptr;
    save_callback<LevelVariable> save_var = nullptr;

    load_callback<LevelArray> load_arr = nullptr;
    save_callback<LevelArray> save_arr = nullptr;

    load_callback<LevelScript> load_script = nullptr;
    save_callback<LevelScript> save_script = nullptr;

    load_callback<LevelItemSetup38A> load_levelitem38a = nullptr;
    save_callback<LevelItemSetup38A> save_levelitem38a = nullptr;
};

void PGEX2_load_level(PGE_FileFormats_misc::TextInput& input, PGEX2_LevelCallbacks& callbacks);

bool PGEX2_load_level(PGE_FileFormats_misc::TextInput &file, LevelData &FileData);

#endif // #ifndef PGEX2_LEVEL_FILE_H
