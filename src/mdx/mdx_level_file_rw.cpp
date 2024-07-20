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

/*! \file mdx_level_file_rw.cpp
 *
 *  \brief Implements defines PGE-X2 functions for loading a level object
 *
 * This is a new implementation but supports precisely the same format as PGE-X
 *
 */

#include "mdx/mdx_level_file.h"
#include "mdx/mdx_exception.hpp"
#include "file_formats.h"
#include "pge_file_lib_private.h"
#include "pge_file_lib_globs.h"

static void s_on_error(void* _FileData, FileFormatsError& err)
{
    LevelData& FileData = *reinterpret_cast<LevelData*>(_FileData);

    static_cast<FileFormatsError&>(FileData.meta) = std::move(err);
    FileData.meta.ReadFileValid = false;
}

static bool s_load_head(void* _FileData, LevelHead& dest)
{
    LevelData& FileData = *reinterpret_cast<LevelData*>(_FileData);

    FileData.LevelName = std::move(dest.LevelName);
    FileData.stars = dest.stars;
    FileData.open_level_on_fail = std::move(dest.open_level_on_fail);
    FileData.open_level_on_fail_warpID = dest.open_level_on_fail_warpID;
    FileData.player_names_overrides = std::move(dest.player_names_overrides);
    FileData.custom_params = std::move(dest.custom_params);
    FileData.meta.configPackId = std::move(dest.configPackId);
    FileData.music_files = std::move(dest.music_files);

    return true;
}

static bool s_load_bookmark(void* _FileData, Bookmark& dest)
{
    LevelData& FileData = *reinterpret_cast<LevelData*>(_FileData);
    FileData.metaData.bookmarks.push_back(std::move(dest));

    return true;
}

static bool s_load_crash_data(void* _FileData, CrashData& dest)
{
    LevelData& FileData = *reinterpret_cast<LevelData*>(_FileData);
    FileData.metaData.crash = std::move(dest);
    FileData.metaData.crash.used = true;

    return true;
}

static bool s_load_section(void* _FileData, LevelSection& dest)
{
    LevelData& FileData = *reinterpret_cast<LevelData*>(_FileData);

    dest.PositionX = dest.size_left - 10;
    dest.PositionY = dest.size_top - 10;

    //add captured value into array
    pge_size_t sections_count = FileData.sections.size();

    if(dest.id < 0 || dest.id > 10000)
        throw MDX_callback_error("Negative section ID");

    if(dest.id >= static_cast<int>(sections_count))
    {
        pge_size_t needToAdd = static_cast<pge_size_t>(dest.id) - (FileData.sections.size() - 1);
        while(needToAdd > 0)
        {
            LevelSection dummySct = FileFormats::CreateLvlSection();
            dummySct.id = (int)FileData.sections.size();
            FileData.sections.push_back(std::move(dummySct));
            needToAdd--;
        }
    }

    FileData.sections[static_cast<pge_size_t>(dest.id)] = std::move(dest);

    return true;
}

static bool s_load_startpoint(void* _FileData, PlayerPoint& player)
{
    LevelData& FileData = *reinterpret_cast<LevelData*>(_FileData);

    //add captured value into array
    bool found = false;
    pge_size_t q = 0;
    pge_size_t playersCount = FileData.players.size();
    for(q = 0; q < playersCount; q++)
    {
        if(FileData.players[q].id == player.id)
        {
            found = true;
            break;
        }
    }

    PlayerPoint sz = FileFormats::CreateLvlPlayerPoint(player.id);
    player.w = sz.w;
    player.h = sz.h;

    if(found)
        FileData.players[q] = std::move(player);
    else
        FileData.players.push_back(std::move(player));

    return true;
}

static bool s_load_block(void* _FileData, LevelBlock& block)
{
    LevelData& FileData = *reinterpret_cast<LevelData*>(_FileData);

    block.meta.array_id = FileData.blocks_array_id++;
    block.meta.index = static_cast<unsigned int>(FileData.blocks.size());
    FileData.blocks.push_back(std::move(block));

    return true;
}

static bool s_load_bgo(void* _FileData, LevelBGO& bgodata)
{
    LevelData& FileData = *reinterpret_cast<LevelData*>(_FileData);

    bgodata.meta.array_id = FileData.bgo_array_id++;
    bgodata.meta.index = static_cast<unsigned int>(FileData.bgo.size());
    FileData.bgo.push_back(std::move(bgodata));

    return true;
}

static bool s_load_npc(void* _FileData, LevelNPC& npcdata)
{
    LevelData& FileData = *reinterpret_cast<LevelData*>(_FileData);

    npcdata.meta.array_id = FileData.npc_array_id++;
    npcdata.meta.index = static_cast<unsigned int>(FileData.npc.size());
    FileData.npc.push_back(std::move(npcdata));

    return true;
}

static bool s_load_phys(void* _FileData, LevelPhysEnv& physiczone)
{
    LevelData& FileData = *reinterpret_cast<LevelData*>(_FileData);

    physiczone.meta.array_id = FileData.physenv_array_id++;
    physiczone.meta.index = static_cast<unsigned int>(FileData.physez.size());
    FileData.physez.push_back(std::move(physiczone));

    return true;
}

static bool s_load_warp(void* _FileData, LevelDoor& door)
{
    LevelData& FileData = *reinterpret_cast<LevelData*>(_FileData);

    door.isSetIn = (!door.lvl_i);
    door.isSetOut = (!door.lvl_o || (door.lvl_i));

    if(!door.isSetIn && door.isSetOut)
    {
        door.ix = door.ox;
        door.iy = door.oy;
    }

    if(!door.isSetOut && door.isSetIn)
    {
        door.ox = door.ix;
        door.oy = door.iy;
    }

    door.meta.array_id = FileData.doors_array_id++;
    door.meta.index = static_cast<unsigned int>(FileData.doors.size());
    FileData.doors.push_back(std::move(door));

    return true;
}

static bool s_load_layer(void* _FileData, LevelLayer& layer)
{
    LevelData& FileData = *reinterpret_cast<LevelData*>(_FileData);

    //add captured value into array
    bool found = false;
    pge_size_t q = 0;
    for(q = 0; q < FileData.layers.size(); q++)
    {
        if(FileData.layers[q].name == layer.name)
        {
            found = true;
            break;
        }
    }

    if(found)
    {
        layer.meta.array_id = FileData.layers[q].meta.array_id;
        FileData.layers[q] = std::move(layer);
    }
    else
    {
        layer.meta.array_id = FileData.layers_array_id++;
        FileData.layers.push_back(std::move(layer));
    }

    return true;
}

static bool s_load_event(void* _FileData, LevelSMBX64Event& event)
{
    LevelData& FileData = *reinterpret_cast<LevelData*>(_FileData);

    // FIXME: do something to add padding to event.sets based on the IDs of the section settings
    // Note: this causes a hang in extreme cases
    for(const auto& set : event.sets)
    {
        if(set.id < 0 || set.id > 10000)
            throw MDX_callback_error("Negative section ID");
    }

    //add captured value into array
    bool found = false;
    pge_size_t q = 0;

    for(q = 0; q < FileData.events.size(); q++)
    {
        if(FileData.events[q].name == event.name)
        {
            found = true;
            break;
        }
    }

    if(found)
    {
        event.meta.array_id = FileData.events[q].meta.array_id;
        FileData.events[q] = std::move(event);
    }
    else
    {
        event.meta.array_id = FileData.events_array_id++;
        FileData.events.push_back(std::move(event));
    }

    return true;
}

static bool s_load_var(void* _FileData, LevelVariable& v)
{
    LevelData& FileData = *reinterpret_cast<LevelData*>(_FileData);
    FileData.variables.push_back(std::move(v));

    return true;
}

static bool s_load_arr(void* _FileData, LevelArray& a)
{
    LevelData& FileData = *reinterpret_cast<LevelData*>(_FileData);
    FileData.arrays.push_back(std::move(a));

    return true;
}

static bool s_load_script(void* _FileData, LevelScript& s)
{
    LevelData& FileData = *reinterpret_cast<LevelData*>(_FileData);
    FileData.scripts.push_back(std::move(s));

    return true;
}

static bool s_load_levelitem38a(void* _FileData, LevelItemSetup38A& customcfg38A)
{
    LevelData& FileData = *reinterpret_cast<LevelData*>(_FileData);
    FileData.custom38A_configs.push_back(std::move(customcfg38A));

    return true;
}

bool MDX_load_level(PGE_FileFormats_misc::TextInput &file, LevelData &FileData)
{
    FileFormats::CreateLevelData(FileData);
    FileData.meta.RecentFormat = LevelData::PGEX;

    //Add path data
    PGESTRING filePath = file.getFilePath();
    if(!IsEmpty(filePath))
    {
        PGE_FileFormats_misc::FileInfo  in_1(filePath);
        FileData.meta.filename = in_1.basename();
        FileData.meta.path = in_1.dirpath();
    }

    LevelLoadCallbacks callbacks;

    callbacks.on_error = s_on_error;

    callbacks.load_head = s_load_head;
    callbacks.load_bookmark = s_load_bookmark;
    callbacks.load_crash_data = s_load_crash_data;
    callbacks.load_section = s_load_section;
    callbacks.load_startpoint = s_load_startpoint;
    callbacks.load_block = s_load_block;
    callbacks.load_bgo = s_load_bgo;
    callbacks.load_npc = s_load_npc;
    callbacks.load_phys = s_load_phys;
    callbacks.load_warp = s_load_warp;
    callbacks.load_layer = s_load_layer;
    callbacks.load_event = s_load_event;
    callbacks.load_var = s_load_var;
    callbacks.load_arr = s_load_arr;
    callbacks.load_script = s_load_script;
    callbacks.load_levelitem38a = s_load_levelitem38a;

    callbacks.userdata = reinterpret_cast<void*>(&FileData);

    return MDX_load_level(file, callbacks);
}
