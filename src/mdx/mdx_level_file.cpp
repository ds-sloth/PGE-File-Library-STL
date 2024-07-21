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

/*! \file mdx_level_file.cpp
 *
 *  \brief Implements defines PGE-X2 structures for the level objects and file
 *
 * This is a new implementation but supports precisely the same format as PGE-X
 *
 */

#include "lvl_filedata.h"
#include "pge_file_lib_private.h"

#include "mdx/mdx_base_file.hpp"
#include "mdx/mdx_macros.hpp"
#include "mdx/mdx_level_file.h"

MDX_SETUP_OBJECT(LevelHead,
    MDX_FIELD("TL", LevelName); //Level Title
    MDX_FIELD("SZ", stars); //Starz number
    MDX_FIELD("DL", open_level_on_fail); //Open level on fail
    MDX_FIELD("DE", open_level_on_fail_warpID); //Open level's warpID on fail
    MDX_FIELD("NO", player_names_overrides); //Overrides of player names
    MDX_FIELD("XTRA", custom_params); //Level-wide Extra settings
    MDX_FIELD("CPID", configPackId); //Config pack ID string
    MDX_FIELD("MUS", music_files); // Level-wide list of external music files
);

MDX_SETUP_OBJECT(Bookmark,
    MDX_FIELD("BM", bookmarkName); //Bookmark name
    MDX_FIELD("X", x); // Position X
    MDX_FIELD("Y", y); // Position Y
);

MDX_SETUP_OBJECT(CrashData,
    MDX_FIELD("UT", untitled); //Untitled
    MDX_FIELD("MD", modifyed); //Modyfied
    MDX_FIELD("FF", fmtID); //Recent File format
    MDX_FIELD("FV", fmtVer); //Recent File format version
    MDX_FIELD("N",  filename);  //Filename
    MDX_FIELD("P",  path);  //Path
    MDX_FIELD("FP", fullPath);  //Full file Path
);

MDX_SETUP_OBJECT(LevelSection,
    MDX_FIELD("SC", id); //Section ID
    MDX_FIELD("L",  size_left); //Left side
    MDX_FIELD("R",  size_right);//Right side
    MDX_FIELD("T",  size_top); //Top side
    MDX_FIELD("B",  size_bottom);//Bottom side
    MDX_FIELD("MZ", music_id);//Built-in music ID
    MDX_FIELD("BG", background);//Built-in background ID
    MDX_FIELD("LT", lighting_value);//Lighting value
    MDX_FIELD("MF", music_file); //External music file path
    MDX_FIELD("ME", music_file_idx); //External music entry from level list
    MDX_FIELD("CS", wrap_h);//Connect sides horizontally
    MDX_FIELD("CSV", wrap_v);//Connect sides vertically
    MDX_FIELD("OE", OffScreenEn);//Offscreen exit
    MDX_FIELD("SR", lock_left_scroll);//Right-way scroll only (No Turn-back)
    MDX_FIELD("SL", lock_right_scroll);//Left-way scroll only (No Turn-forward)
    MDX_FIELD("SD", lock_up_scroll);//Down-way scroll only (No Turn-forward)
    MDX_FIELD("SU", lock_down_scroll);//Up-way scroll only (No Turn-forward)
    MDX_FIELD("UW", underwater);//Underwater bit
    MDX_FIELD("XTRA", custom_params);//Custom JSON data tree
);

MDX_SETUP_OBJECT(PlayerPoint,
    MDX_FIELD("ID", id); //ID of player point
    MDX_FIELD("X", x);
    MDX_FIELD("Y", y);
    MDX_FIELD("D",  direction);
);

MDX_SETUP_OBJECT(LevelBlock,
    MDX_FIELD("ID", id); //Block ID
    MDX_FIELD("X", x); // Position X
    MDX_FIELD("Y", y); //Position Y
    MDX_FIELD("W", w); //Width
    MDX_FIELD("H", h); //Height
    MDX_FIELD("AS", autoscale);//Enable auto-Scaling
    MDX_FIELD("GXN", gfx_name); //38A GFX-Name
    MDX_FIELD("GXX", gfx_dx); //38A graphics extend x
    MDX_FIELD("GXY", gfx_dy); //38A graphics extend y
    MDX_FIELD("CN", npc_id); //Contains (coins/NPC)
    MDX_FIELD("CS", npc_special_value); //Special value for contained NPC
    MDX_FIELD("IV", invisible); //Invisible
    MDX_FIELD("SL", slippery); //Slippery
    MDX_FIELD("MA", motion_ai_id); //Motion AI type
    MDX_FIELD("S1", special_data); //Special value 1
    MDX_FIELD("S2", special_data2); //Special value 2
    MDX_FIELD("LR", layer); //Layer name
    MDX_FIELD("ED", event_destroy); //Destroy event slot
    MDX_FIELD("EH", event_hit); //Hit event slot
    MDX_FIELD("EE", event_emptylayer); //Hit event slot
    MDX_FIELD_XTRA();//Custom JSON data tree
);

MDX_SETUP_OBJECT(LevelBGO,
    MDX_FIELD("ID", id);  //BGO ID
    MDX_FIELD("X",  x);  //X Position
    MDX_FIELD("Y",  y);  //Y Position
    MDX_FIELD("GXX", gfx_dx); //38A graphics extend x
    MDX_FIELD("GXY", gfx_dy); //38A graphics extend y
    MDX_FIELD("ZO", z_offset); //Z Offset
    MDX_FIELD("ZP", z_mode);  //Z Position
    MDX_FIELD("SP", smbx64_sp);  //SMBX64 Sorting priority
    MDX_FIELD("LR", layer);   //Layer name
    MDX_FIELD_XTRA();//Custom JSON data tree
);

MDX_SETUP_OBJECT(LevelNPC,
    MDX_FIELD("ID", id); //NPC ID
    MDX_FIELD("X", x); //X position
    MDX_FIELD("Y", y); //Y position
    MDX_FIELD("GXN", gfx_name); //38A GFX-Name
    MDX_FIELD("GXX", gfx_dx); //38A graphics extend x
    MDX_FIELD("GXY", gfx_dy); //38A graphics extend y
    MDX_FIELD("OW", override_width); //Override width
    MDX_FIELD("OH", override_height); //Override height
    MDX_FIELD("GAS", gfx_autoscale); //Autoscale GFX on size override
    MDX_FIELD("WGT", wings_type); //38A: Wings type
    MDX_FIELD("WGS", wings_style); //38A: Wings style
    MDX_FIELD("D", direct); //Direction
    MDX_FIELD("CN", contents); //Contents of container-NPC
    MDX_FIELD("S1", special_data); //Special value 1
    MDX_FIELD("S2", special_data2); //Special value 2
    MDX_FIELD("GE", generator); //Generator
    MDX_FIELD("GT", generator_type); //Generator type
    MDX_FIELD("GD", generator_direct); //Generator direction
    MDX_FIELD("GM", generator_period); //Generator period
    MDX_FIELD("GA", generator_custom_angle); //Generator custom angle
    MDX_FIELD("GB",  generator_branches); //Generator number of branches
    MDX_FIELD("GR", generator_angle_range); //Generator angle range
    MDX_FIELD("GS", generator_initial_speed); //Generator custom initial speed
    MDX_FIELD("MG", msg); //Message
    MDX_FIELD("FD", friendly); //Friendly
    MDX_FIELD("NM", nomove); //Don't move
    MDX_FIELD("BS", is_boss); //Enable boss mode!
    MDX_FIELD("LR", layer); //Layer
    MDX_FIELD("LA", attach_layer); //Attach Layer
    MDX_FIELD("SV", send_id_to_variable); //Send ID to variable
    MDX_FIELD("EA", event_activate); //Event slot "Activated"
    MDX_FIELD("ED", event_die); //Event slot "Death/Take/Destroy"
    MDX_FIELD("ET", event_talk); //Event slot "Talk"
    MDX_FIELD("EE", event_emptylayer); //Event slot "Layer is empty"
    MDX_FIELD("EG", event_grab);//Event slot "On grab"
    MDX_FIELD("EO", event_touch);//Event slot "On touch"
    MDX_FIELD("EF", event_nextframe);//Evemt slot "Trigger every frame"
    MDX_FIELD_XTRA();//Custom JSON data tree
);

MDX_SETUP_OBJECT(LevelPhysEnv,
    MDX_FIELD("ET", env_type); //Environment type
    MDX_FIELD("X",  x); //X position
    MDX_FIELD("Y",  y); //Y position
    MDX_FIELD("W",  w); //Width
    MDX_FIELD("H",  h); //Height
    MDX_FIELD("LR", layer);  //Layer
    MDX_FIELD("FR", friction); //Friction
    MDX_FIELD("AD", accel_direct); //Custom acceleration direction
    MDX_FIELD("AC", accel); //Custom acceleration
    MDX_FIELD("MV", max_velocity); //Maximal velocity
    MDX_FIELD("EO",  touch_event); //Touch event/script
    MDX_FIELD_XTRA();//Custom JSON data tree
);

MDX_SETUP_OBJECT(LevelDoor,
    MDX_FIELD("IX", ix); //Input point
    MDX_FIELD("IY", iy); //Input point
    MDX_FIELD("OX", ox); //Output point
    MDX_FIELD("OY", oy); //Output point
    MDX_FIELD("IL", length_i); //Length of entrance (input) point
    MDX_FIELD("OL", length_o); //Length of exit (output) point
    MDX_FIELD("DT", type); //Input point
    MDX_FIELD("ID", idirect); //Input direction
    MDX_FIELD("OD", odirect); //Output direction
    MDX_FIELD("WX", world_x); //Target world map point
    MDX_FIELD("WY", world_y); //Target world map point
    MDX_FIELD("LF", lname);  //Target level file
    MDX_FIELD("LI", warpto); //Target level file's input warp
    MDX_FIELD("ET", lvl_i); //Level Entrance
    MDX_FIELD("EX", lvl_o); //Level exit
    MDX_FIELD("SL", stars); //Stars limit
    MDX_FIELD("SM", stars_msg);  //Message about stars/leeks
    MDX_FIELD("NV", novehicles); //No Vehicles
    MDX_FIELD("SH", star_num_hide); //Don't show stars number
    MDX_FIELD("AI", allownpc); //Allow grabbed items
    MDX_FIELD("LC", locked); //Door is locked
    MDX_FIELD("LB", need_a_bomb); //Door is blocked, need bomb to unlock
    MDX_FIELD("HS", hide_entering_scene); //Don't show entering scene
    MDX_FIELD("AL", allownpc_interlevel); //Allow NPC's inter-level
    MDX_FIELD("SR", special_state_required); //Required a special state to enter
    MDX_FIELD("STR", stood_state_required); //Required a stood state to enter
    MDX_FIELD("TE", transition_effect); //Transition effect
    MDX_FIELD("PT", cannon_exit); //Cannon exit
    MDX_FIELD("PS", cannon_exit_speed); //Cannon exit speed
    MDX_FIELD("LR", layer);  //Layer
    MDX_FIELD("EE", event_enter);  //On-Enter event slot
    MDX_FIELD("TW", two_way); //Two-way warp
    MDX_FIELD_XTRA();//Custom JSON data tree
);

MDX_SETUP_OBJECT(LevelLayer,
    MDX_FIELD("LR", name);  //Layer name
    MDX_FIELD("HD", hidden); //Hidden
    MDX_FIELD("LC", locked); //Locked
);

const char* MDX_LevelEvent_load_controls(LevelSMBX64Event& event, const char* field_data)
{
    PGELIST<bool> controls;

    const char* next = MDX_finish_term(MDX_FieldType<PGELIST<bool>>::load(controls, field_data));

#ifndef PGE_FILES_QT
    controls.resize(12);
#else
    if(controls.size() > 12)
        controls.erase(controls.begin() + 12, controls.end());
    while(controls.size() < 12)
        controls.push_back(false);
#endif

    // SMBX64-only
    event.ctrl_up = controls[0];
    event.ctrl_down = controls[1];
    event.ctrl_left = controls[2];
    event.ctrl_right = controls[3]; //-V112
    event.ctrl_run = controls[4];
    event.ctrl_jump = controls[5];
    event.ctrl_drop = controls[6];
    event.ctrl_start = controls[7];
    event.ctrl_altrun = controls[8];
    event.ctrl_altjump = controls[9];
    // SMBX64-only end
    // SMBX-38A begin
    event.ctrls_enable = controls[10];
    event.ctrl_lock_keyboard = controls[11];
    // SMBX-38A end

    return next;
}

const char* MDX_LevelEvent_load_autoscroll_path(LevelEvent_Sets& set, const char* field_data)
{
    PGELIST<long> arr;

    const char* next = MDX_finish_term(MDX_FieldType<PGELIST<long>>::load(arr, field_data));

    if(arr.size() % 4)
        throw MDX_bad_term("Invalid Section Autoscroll path data contains non-multiple 4 entries");

    set.autoscroll_path.clear();

    for(pge_size_t pe = 0; pe < arr.size(); pe += 4)
    {
        LevelEvent_Sets::AutoScrollStopPoint stop;
        stop.x =     arr[pe + 0];
        stop.y =     arr[pe + 1];
        stop.type =  (int)arr[pe + 2];
        stop.speed = arr[pe + 3];
        set.autoscroll_path.push_back(stop);
    }

    return next;
}

MDX_SETUP_OBJECT(LevelEvent_Sets,
    MDX_FIELD("ID", id);
    MDX_FIELD("SL", position_left);
    MDX_FIELD("ST", position_top);
    MDX_FIELD("SB", position_bottom);
    MDX_FIELD("SR", position_right);
    MDX_FIELD("SXX", expression_pos_x);
    MDX_FIELD("SYX", expression_pos_y);
    MDX_FIELD("SWX", expression_pos_w);
    MDX_FIELD("SHX", expression_pos_h);
    MDX_FIELD("MI", music_id);
    MDX_FIELD("MF", music_file);
    MDX_FIELD("ME", music_file_idx);
    MDX_FIELD("BG", background_id);
    MDX_FIELD("AS", autoscrol);
    MDX_FIELD("AST", autoscroll_style);
    MDX_UNIQUE_FIELD("ASP", MDX_LevelEvent_load_autoscroll_path);
    MDX_FIELD("AX", autoscrol_x);
    MDX_FIELD("AY", autoscrol_y);
    MDX_FIELD("AXX", expression_autoscrool_x);
    MDX_FIELD("AYX", expression_autoscrool_y);
);
MDX_ENABLE_SUB_LIST(LevelEvent_Sets);

MDX_SETUP_OBJECT(LevelEvent_MoveLayer,
    MDX_FIELD("LN", name);
    MDX_FIELD("SX", speed_x);
    MDX_FIELD("SXX", expression_x);
    MDX_FIELD("SY", speed_y);
    MDX_FIELD("SYX", expression_y);
    MDX_FIELD("MW", way);
);
MDX_ENABLE_SUB_STRUCT(LevelEvent_MoveLayer);

MDX_SETUP_OBJECT(LevelEvent_SpawnNPC,
    MDX_FIELD("ID", id);
    MDX_FIELD("SX", x);
    MDX_FIELD("SXX", expression_x);
    MDX_FIELD("SY", y);
    MDX_FIELD("SYX", expression_y);
    MDX_FIELD("SSX", speed_x);
    MDX_FIELD("SSXX", expression_sx);
    MDX_FIELD("SSY", speed_y);
    MDX_FIELD("SSYX", expression_sy);
    MDX_FIELD("SSS", special);
);
MDX_ENABLE_SUB_LIST(LevelEvent_SpawnNPC);

MDX_SETUP_OBJECT(LevelEvent_SpawnEffect,
    MDX_FIELD("ID", id);
    MDX_FIELD("SX", x);
    MDX_FIELD("SXX", expression_x);
    MDX_FIELD("SY", y);
    MDX_FIELD("SYX", expression_y);
    MDX_FIELD("SSX", speed_x);
    MDX_FIELD("SSXX", expression_sx);
    MDX_FIELD("SSY", speed_y);
    MDX_FIELD("SSYX", expression_sy);
    MDX_FIELD("FP", fps);
    MDX_FIELD("TTL", max_life_time);
    MDX_FIELD("GT", gravity);
);
MDX_ENABLE_SUB_LIST(LevelEvent_SpawnEffect);

MDX_SETUP_OBJECT(LevelEvent_UpdateVariable,
    MDX_FIELD("N", name);
    MDX_FIELD("V", newval);
);
MDX_ENABLE_SUB_LIST(LevelEvent_UpdateVariable);

MDX_SETUP_OBJECT(LevelSMBX64Event,
    MDX_FIELD("ET", name);  //Event Title
    MDX_FIELD("MG", msg);  //Event Message
    MDX_FIELD("SD", sound_id); //Play Sound ID
    MDX_FIELD("EG", end_game); //End game algorithm
    MDX_FIELD("LH", layers_hide); //Hide layers
    MDX_FIELD("LS", layers_show); //Show layers
    MDX_FIELD("LT", layers_toggle); //Toggle layers
    //Legacy values (without SMBX-38A values support)
    // MDX_FIELD("SM", musicSets)  //Switch music
    // MDX_FIELD("SB", bgSets)     //Switch background
    // MDX_FIELD("SS", ssSets)     //Section Size
    //-------------------
    //New values (with SMBX-38A values support)
    MDX_FIELD("SSS", sets); //Section settings in new format
    //-------------------
    //---SMBX-38A entries-----
    MDX_FIELD("MLA",  moving_layers);       //NPC's to spawn
    MDX_FIELD("SNPC", spawn_npc);       //NPC's to spawn
    MDX_FIELD("SEF",  spawn_effects);    //Effects to spawn
    MDX_FIELD("UV",   update_variable); //Variables to update
    MDX_FIELD("TSCR", trigger_script); //Trigger script
    MDX_FIELD("TAPI", trigger_api_id); //Trigger script
    MDX_NESTED_FIELD("TMR", timer_def, enable); //Enable timer
    MDX_NESTED_FIELD("TMC", timer_def, count); //Count of timer units
    MDX_NESTED_FIELD("TMI", timer_def, interval); //Interval of timer tick
    MDX_NESTED_FIELD("TMD", timer_def, count_dir); //Direction of count
    MDX_NESTED_FIELD("TMV", timer_def, show); //Show timer on screen
    //-------------------
    MDX_FIELD("TE", trigger); //Trigger event
    MDX_FIELD("TD", trigger_timer); //Trigger delay
    MDX_FIELD("DS", nosmoke); //Disable smoke
    MDX_FIELD("AU", autostart); //Auto start
    MDX_FIELD("AUC", autostart_condition); //Auto start condition
    MDX_UNIQUE_FIELD("PC", MDX_LevelEvent_load_controls);
    MDX_FIELD("ML", movelayer);   //Move layer
    MDX_FIELD("MX", layer_speed_x); //Layer motion speed X
    MDX_FIELD("MY", layer_speed_y); //Layer motion speed Y
    MDX_FIELD("AS", scroll_section); //Autoscroll section ID
    MDX_FIELD("AX", move_camera_x); //Autoscroll speed X
    MDX_FIELD("AY", move_camera_y); //Autoscroll speed Y
);

MDX_SETUP_OBJECT(LevelVariable,
    MDX_FIELD("N", name); //Variable name
    MDX_FIELD("V", value); //Variable value
    MDX_FIELD("G", is_global); //Is global variable
);

MDX_SETUP_OBJECT(LevelArray,
    MDX_FIELD("N", name); //Array name
);

MDX_SETUP_OBJECT(LevelScript,
    MDX_FIELD("N", name);  //Variable name
    MDX_FIELD("L", language);  //Variable name
    MDX_FIELD("S", script); //Script text
);

template<>
const char* MDX_FieldType<LevelItemSetup38A::ItemType>::load(LevelItemSetup38A::ItemType& dest, const char* field_data)
{
    int got = 0;
    const char* ret = MDX_FieldType<int>::load(got, field_data);

    if(got < LevelItemSetup38A::UNKNOWN || got >= LevelItemSetup38A::ITEM_TYPE_MAX)
        throw MDX_bad_term("Bad type");

    dest = (LevelItemSetup38A::ItemType)got;

    return ret;
}

template<>
const char* MDX_FieldType<LevelItemSetup38A::Entry>::load(LevelItemSetup38A::Entry& e, const char* field_data)
{
    std::string got;
    const char* ret = MDX_FieldType<std::string>::load(got, field_data);

    const char* const str_data_start = got.c_str();
    const char* str_data = str_data_start;

    str_data = MDX_FieldType<decltype(LevelItemSetup38A::Entry::key)>::load(e.key, str_data);

    if(*str_data != '=')
        throw MDX_missing_delimiter('=');

    str_data++;

    str_data = MDX_FieldType<decltype(LevelItemSetup38A::Entry::value)>::load(e.value, str_data);

    if((size_t)(str_data - str_data_start) != got.size())
        throw MDX_missing_delimiter('"');

    return ret;
}

MDX_SETUP_OBJECT(LevelItemSetup38A,
    MDX_FIELD("T",  type); //Type of item
    MDX_FIELD("ID", id);
    MDX_FIELD("D", data); //Variable value
);

struct MDX_LevelFile : MDX_File<LevelLoadCallbacks, LevelSaveCallbacks>
{
    MDX_SECTION_SINGLE("HEAD", LevelHead, head);

    MDX_SECTION_SINGLE("META_SYS_CRASH", CrashData, crash_data);

    MDX_SECTION("META_BOOKMARKS", Bookmark, bookmark);

    MDX_SECTION("SECTION", LevelSection, section);

    MDX_SECTION("STARTPOINT", PlayerPoint, startpoint);

    MDX_SECTION("BLOCK", LevelBlock, block);

    MDX_SECTION("BGO", LevelBGO, bgo);

    MDX_SECTION("NPC", LevelNPC, npc);

    MDX_SECTION("PHYSICS", LevelPhysEnv, phys);

    MDX_SECTION("DOORS", LevelDoor, warp);

    MDX_SECTION("LAYERS", LevelLayer, layer);

    MDX_SECTION("EVENTS_CLASSIC", LevelSMBX64Event, event);

    MDX_SECTION("VARIABLES", LevelVariable, var);

    MDX_SECTION("ARRAYS", LevelArray, arr);

    MDX_SECTION("SCRIPTS", LevelScript, script);

    MDX_SECTION("CUSTOM_ITEMS_38A", LevelItemSetup38A, levelitem38a);
};

bool MDX_load_level(PGE_FileFormats_misc::TextInput& input, LevelLoadCallbacks& callbacks)
{
    MDX_LevelFile f;
    return f.load_file(input, callbacks);
}
