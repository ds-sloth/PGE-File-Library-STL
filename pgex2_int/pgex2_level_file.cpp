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

/*! \file pgex2_level_file.cpp
 *
 *  \brief Implements defines PGE-X2 structures for the level objects and file
 *
 * This is a new implementation but supports precisely the same format as PGE-X
 *
 */

#include "../lvl_filedata.h"
#include "../pge_file_lib_private.h"

#include "pgex2_base_file.hpp"
#include "pgex2_macros.hpp"
#include "pgex2_level_file.h"

PGEX2_SETUP_OBJECT(PGEX2_LevelHead,
    PGEX2_FIELD("TL", LevelName); //Level Title
    PGEX2_FIELD("SZ", stars); //Starz number
    PGEX2_FIELD("DL", open_level_on_fail); //Open level on fail
    PGEX2_FIELD("DE", open_level_on_fail_warpID); //Open level's warpID on fail
    PGEX2_FIELD("NO", player_names_overrides); //Overrides of player names
    PGEX2_FIELD("XTRA", custom_params); //Level-wide Extra settings
    PGEX2_FIELD("CPID", configPackId); //Config pack ID string
    PGEX2_FIELD("MUS", music_files); // Level-wide list of external music files
);

PGEX2_SETUP_OBJECT(Bookmark,
    PGEX2_FIELD("BM", bookmarkName); //Bookmark name
    PGEX2_FIELD("X", x); // Position X
    PGEX2_FIELD("Y", y); // Position Y
);

PGEX2_SETUP_OBJECT(CrashData,
    PGEX2_FIELD("UT", untitled); //Untitled
    PGEX2_FIELD("MD", modifyed); //Modyfied
    PGEX2_FIELD("FF", fmtID); //Recent File format
    PGEX2_FIELD("FV", fmtVer); //Recent File format version
    PGEX2_FIELD("N",  filename);  //Filename
    PGEX2_FIELD("P",  path);  //Path
    PGEX2_FIELD("FP", fullPath);  //Full file Path
);

PGEX2_SETUP_OBJECT(LevelSection,
    PGEX2_FIELD("SC", id); //Section ID
    PGEX2_FIELD("L",  size_left); //Left side
    PGEX2_FIELD("R",  size_right);//Right side
    PGEX2_FIELD("T",  size_top); //Top side
    PGEX2_FIELD("B",  size_bottom);//Bottom side
    PGEX2_FIELD("MZ", music_id);//Built-in music ID
    PGEX2_FIELD("BG", background);//Built-in background ID
    PGEX2_FIELD("LT", lighting_value);//Lighting value
    PGEX2_FIELD("MF", music_file); //External music file path
    PGEX2_FIELD("ME", music_file_idx); //External music entry from level list
    PGEX2_FIELD("CS", wrap_h);//Connect sides horizontally
    PGEX2_FIELD("CSV", wrap_v);//Connect sides vertically
    PGEX2_FIELD("OE", OffScreenEn);//Offscreen exit
    PGEX2_FIELD("SR", lock_left_scroll);//Right-way scroll only (No Turn-back)
    PGEX2_FIELD("SL", lock_right_scroll);//Left-way scroll only (No Turn-forward)
    PGEX2_FIELD("SD", lock_up_scroll);//Down-way scroll only (No Turn-forward)
    PGEX2_FIELD("SU", lock_down_scroll);//Up-way scroll only (No Turn-forward)
    PGEX2_FIELD("UW", underwater);//Underwater bit
    PGEX2_FIELD("XTRA", custom_params);//Custom JSON data tree
);

PGEX2_SETUP_OBJECT(PlayerPoint,
    PGEX2_FIELD("ID", id); //ID of player point
    PGEX2_FIELD("X", x);
    PGEX2_FIELD("Y", y);
    PGEX2_FIELD("D",  direction);
);

PGEX2_SETUP_OBJECT(LevelBlock,
    PGEX2_FIELD("ID", id);
    PGEX2_FIELD("X", x);
    PGEX2_FIELD("Y", y);
    PGEX2_FIELD("W", w);
    PGEX2_FIELD("H", h);
    PGEX2_FIELD("CN", npc_id);
    PGEX2_FIELD("CS", npc_special_value);
    PGEX2_FIELD("IV", invisible);
    PGEX2_FIELD("SL", slippery);
    PGEX2_FIELD("MA", motion_ai_id);
    PGEX2_FIELD("S1", special_data);
    PGEX2_FIELD("S2", special_data2);
    PGEX2_FIELD("LR", layer);
    PGEX2_FIELD("ED", event_destroy);
    PGEX2_FIELD("EH", event_hit);
    PGEX2_FIELD("EE", event_emptylayer);
    // PGEX2_FIELD("XTRA", meta.custom_params); // need to move out of meta
    PGEX2_FIELD("GXN", gfx_name);
    PGEX2_FIELD("GXX", gfx_dx);
    PGEX2_FIELD("GXY", gfx_dy);
);

PGEX2_SETUP_OBJECT(LevelBGO,
    PGEX2_FIELD("ID", id);  //BGO ID
    PGEX2_FIELD("X",  x);  //X Position
    PGEX2_FIELD("Y",  y);  //Y Position
    PGEX2_FIELD("GXX", gfx_dx); //38A graphics extend x
    PGEX2_FIELD("GXY", gfx_dy); //38A graphics extend y
    PGEX2_FIELD("ZO", z_offset); //Z Offset
    PGEX2_FIELD("ZP", z_mode);  //Z Position
    PGEX2_FIELD("SP", smbx64_sp);  //SMBX64 Sorting priority
    PGEX2_FIELD("LR", layer);   //Layer name
    // PGEX2_FIELD("XTRA", meta.custom_params);//Custom JSON data tree
);

PGEX2_SETUP_OBJECT(LevelNPC,
    PGEX2_FIELD("ID", id); //NPC ID
    PGEX2_FIELD("X", x); //X position
    PGEX2_FIELD("Y", y); //Y position
    PGEX2_FIELD("GXN", gfx_name); //38A GFX-Name
    PGEX2_FIELD("GXX", gfx_dx); //38A graphics extend x
    PGEX2_FIELD("GXY", gfx_dy); //38A graphics extend y
    PGEX2_FIELD("OW", override_width); //Override width
    PGEX2_FIELD("OH", override_height); //Override height
    PGEX2_FIELD("GAS", gfx_autoscale); //Autoscale GFX on size override
    PGEX2_FIELD("WGT", wings_type); //38A: Wings type
    PGEX2_FIELD("WGS", wings_style); //38A: Wings style
    PGEX2_FIELD("D", direct); //Direction
    PGEX2_FIELD("CN", contents); //Contents of container-NPC
    PGEX2_FIELD("S1", special_data); //Special value 1
    PGEX2_FIELD("S2", special_data2); //Special value 2
    PGEX2_FIELD("GE", generator); //Generator
    PGEX2_FIELD("GT", generator_type); //Generator type
    PGEX2_FIELD("GD", generator_direct); //Generator direction
    PGEX2_FIELD("GM", generator_period); //Generator period
    PGEX2_FIELD("GA", generator_custom_angle); //Generator custom angle
    PGEX2_FIELD("GB",  generator_branches); //Generator number of branches
    PGEX2_FIELD("GR", generator_angle_range); //Generator angle range
    PGEX2_FIELD("GS", generator_initial_speed); //Generator custom initial speed
    PGEX2_FIELD("MG", msg); //Message
    PGEX2_FIELD("FD", friendly); //Friendly
    PGEX2_FIELD("NM", nomove); //Don't move
    PGEX2_FIELD("BS", is_boss); //Enable boss mode!
    PGEX2_FIELD("LR", layer); //Layer
    PGEX2_FIELD("LA", attach_layer); //Attach Layer
    PGEX2_FIELD("SV", send_id_to_variable); //Send ID to variable
    PGEX2_FIELD("EA", event_activate); //Event slot "Activated"
    PGEX2_FIELD("ED", event_die); //Event slot "Death/Take/Destroy"
    PGEX2_FIELD("ET", event_talk); //Event slot "Talk"
    PGEX2_FIELD("EE", event_emptylayer); //Event slot "Layer is empty"
    PGEX2_FIELD("EG", event_grab);//Event slot "On grab"
    PGEX2_FIELD("EO", event_touch);//Event slot "On touch"
    PGEX2_FIELD("EF", event_nextframe);//Evemt slot "Trigger every frame"
    // PGEX2_FIELD("XTRA", meta.custom_params);//Custom JSON data tree
);

PGEX2_SETUP_OBJECT(LevelPhysEnv,
    PGEX2_FIELD("ET", env_type); //Environment type
    PGEX2_FIELD("X",  x); //X position
    PGEX2_FIELD("Y",  y); //Y position
    PGEX2_FIELD("W",  w); //Width
    PGEX2_FIELD("H",  h); //Height
    PGEX2_FIELD("LR", layer);  //Layer
    PGEX2_FIELD("FR", friction); //Friction
    PGEX2_FIELD("AD", accel_direct); //Custom acceleration direction
    PGEX2_FIELD("AC", accel); //Custom acceleration
    PGEX2_FIELD("MV", max_velocity); //Maximal velocity
    PGEX2_FIELD("EO",  touch_event); //Touch event/script
    // PGEX2_FIELD("XTRA", meta.custom_params);//Custom JSON data tree
);

PGEX2_SETUP_OBJECT(LevelDoor,
    PGEX2_FIELD("IX", ix); //Input point
    PGEX2_FIELD("IY", iy); //Input point
    PGEX2_FIELD("OX", ox); //Output point
    PGEX2_FIELD("OY", oy); //Output point
    PGEX2_FIELD("IL", length_i); //Length of entrance (input) point
    PGEX2_FIELD("OL", length_o); //Length of exit (output) point
    PGEX2_FIELD("DT", type); //Input point
    PGEX2_FIELD("ID", idirect); //Input direction
    PGEX2_FIELD("OD", odirect); //Output direction
    PGEX2_FIELD("WX", world_x); //Target world map point
    PGEX2_FIELD("WY", world_y); //Target world map point
    PGEX2_FIELD("LF", lname);  //Target level file
    PGEX2_FIELD("LI", warpto); //Target level file's input warp
    PGEX2_FIELD("ET", lvl_i); //Level Entrance
    PGEX2_FIELD("EX", lvl_o); //Level exit
    PGEX2_FIELD("SL", stars); //Stars limit
    PGEX2_FIELD("SM", stars_msg);  //Message about stars/leeks
    PGEX2_FIELD("NV", novehicles); //No Vehicles
    PGEX2_FIELD("SH", star_num_hide); //Don't show stars number
    PGEX2_FIELD("AI", allownpc); //Allow grabbed items
    PGEX2_FIELD("LC", locked); //Door is locked
    PGEX2_FIELD("LB", need_a_bomb); //Door is blocked, need bomb to unlock
    PGEX2_FIELD("HS", hide_entering_scene); //Don't show entering scene
    PGEX2_FIELD("AL", allownpc_interlevel); //Allow NPC's inter-level
    PGEX2_FIELD("SR", special_state_required); //Required a special state to enter
    PGEX2_FIELD("STR", stood_state_required); //Required a stood state to enter
    PGEX2_FIELD("TE", transition_effect); //Transition effect
    PGEX2_FIELD("PT", cannon_exit); //Cannon exit
    PGEX2_FIELD("PS", cannon_exit_speed); //Cannon exit speed
    PGEX2_FIELD("LR", layer);  //Layer
    PGEX2_FIELD("EE", event_enter);  //On-Enter event slot
    PGEX2_FIELD("TW", two_way); //Two-way warp
    // PGEX2_FIELD("XTRA", meta.custom_params);//Custom JSON data tree
);

PGEX2_SETUP_OBJECT(LevelLayer,
    PGEX2_FIELD("LR", name);  //Layer name
    PGEX2_FIELD("HD", hidden); //Hidden
    PGEX2_FIELD("LC", locked); //Locked
);

const char* PGEX2_LevelEvent_load_controls(LevelSMBX64Event& event, const char* field_data)
{
    PGELIST<bool> controls;

    const char* next = PGEX2_find_next_term(PGEX2_FieldType<PGELIST<bool>>::load(controls, field_data));

    const auto cs = controls.size();

    // SMBX64-only
    if(cs >= 1)  event.ctrl_up = controls[0];
    if(cs >= 2)  event.ctrl_down = controls[1];
    if(cs >= 3)  event.ctrl_left = controls[2];
    if(cs >= 4)  event.ctrl_right = controls[3]; //-V112
    if(cs >= 5)  event.ctrl_run = controls[4];
    if(cs >= 6)  event.ctrl_jump = controls[5];
    if(cs >= 7)  event.ctrl_drop = controls[6];
    if(cs >= 8)  event.ctrl_start = controls[7];
    if(cs >= 9)  event.ctrl_altrun = controls[8];
    if(cs >= 10) event.ctrl_altjump = controls[9];
    // SMBX64-only end
    // SMBX-38A begin
    if(cs >= 11) event.ctrls_enable = controls[10];
    if(cs >= 12) event.ctrl_lock_keyboard = controls[11];
    // SMBX-38A end

    return next;
}

const char* PGEX2_LevelEvent_load_autoscroll_path(LevelEvent_Sets& set, const char* field_data)
{
    PGELIST<long> arr;

    const char* next = PGEX2_find_next_term(PGEX2_FieldType<PGELIST<long>>::load(arr, field_data));

    if(arr.size() % 4)
    {
        // errorString = "Invalid Section Autoscroll path data contains non-multiple 4 entries";
        // goto badfile;
    }

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

PGEX2_SETUP_OBJECT(LevelEvent_Sets,
    PGEX2_FIELD("ID", id);
    PGEX2_FIELD("SL", position_left);
    PGEX2_FIELD("ST", position_top);
    PGEX2_FIELD("SB", position_bottom);
    PGEX2_FIELD("SR", position_right);
    PGEX2_FIELD("SXX", expression_pos_x);
    PGEX2_FIELD("SYX", expression_pos_y);
    PGEX2_FIELD("SWX", expression_pos_w);
    PGEX2_FIELD("SHX", expression_pos_h);
    PGEX2_FIELD("MI", music_id);
    PGEX2_FIELD("MF", music_file);
    PGEX2_FIELD("ME", music_file_idx);
    PGEX2_FIELD("BG", background_id);
    PGEX2_FIELD("AS", autoscrol);
    PGEX2_FIELD("AST", autoscroll_style);
    PGEX2_UNIQUE_FIELD("ASP", PGEX2_LevelEvent_load_autoscroll_path);
    PGEX2_FIELD("AX", autoscrol_x);
    PGEX2_FIELD("AY", autoscrol_y);
    PGEX2_FIELD("AXX", expression_autoscrool_x);
    PGEX2_FIELD("AYX", expression_autoscrool_y);
);

PGEX2_SETUP_OBJECT(LevelEvent_MoveLayer,
    PGEX2_FIELD("LN", name);
    PGEX2_FIELD("SX", speed_x);
    PGEX2_FIELD("SXX", expression_x);
    PGEX2_FIELD("SY", speed_y);
    PGEX2_FIELD("SYX", expression_y);
    PGEX2_FIELD("MW", way);
);

PGEX2_SETUP_OBJECT(LevelEvent_SpawnNPC,
    PGEX2_FIELD("ID", id);
    PGEX2_FIELD("SX", x);
    PGEX2_FIELD("SXX", expression_x);
    PGEX2_FIELD("SY", y);
    PGEX2_FIELD("SYX", expression_y);
    PGEX2_FIELD("SSX", speed_x);
    PGEX2_FIELD("SSXX", expression_sx);
    PGEX2_FIELD("SSY", speed_y);
    PGEX2_FIELD("SSYX", expression_sy);
    PGEX2_FIELD("SSS", special);
);

PGEX2_SETUP_OBJECT(LevelEvent_SpawnEffect,
    PGEX2_FIELD("ID", id);
    PGEX2_FIELD("SX", x);
    PGEX2_FIELD("SXX", expression_x);
    PGEX2_FIELD("SY", y);
    PGEX2_FIELD("SYX", expression_y);
    PGEX2_FIELD("SSX", speed_x);
    PGEX2_FIELD("SSXX", expression_sx);
    PGEX2_FIELD("SSY", speed_y);
    PGEX2_FIELD("SSYX", expression_sy);
    PGEX2_FIELD("FP", fps);
    PGEX2_FIELD("TTL", max_life_time);
    PGEX2_FIELD("GT", gravity);
);

PGEX2_SETUP_OBJECT(LevelEvent_UpdateVariable,
    PGEX2_FIELD("N", name);
    PGEX2_FIELD("V", newval);
);

PGEX2_SETUP_OBJECT(LevelSMBX64Event,
    PGEX2_FIELD("ET", name);  //Event Title
    PGEX2_FIELD("MG", msg);  //Event Message
    PGEX2_FIELD("SD", sound_id); //Play Sound ID
    PGEX2_FIELD("EG", end_game); //End game algorithm
    PGEX2_FIELD("LH", layers_hide); //Hide layers
    PGEX2_FIELD("LS", layers_show); //Show layers
    PGEX2_FIELD("LT", layers_toggle); //Toggle layers
    //Legacy values (without SMBX-38A values support)
    // PGEX2_FIELD("SM", musicSets)  //Switch music
    // PGEX2_FIELD("SB", bgSets)     //Switch background
    // PGEX2_FIELD("SS", ssSets)     //Section Size
    //-------------------
    //New values (with SMBX-38A values support)
    PGEX2_FIELD("SSS", sets); //Section settings in new format
    //-------------------
    //---SMBX-38A entries-----
    PGEX2_FIELD("MLA",  moving_layers);       //NPC's to spawn
    PGEX2_FIELD("SNPC", spawn_npc);       //NPC's to spawn
    PGEX2_FIELD("SEF",  spawn_effects);    //Effects to spawn
    PGEX2_FIELD("UV",   update_variable); //Variables to update
    PGEX2_FIELD("TSCR", trigger_script); //Trigger script
    PGEX2_FIELD("TAPI", trigger_api_id); //Trigger script
    // PGEX2_FIELD("TMR", timer_def.enable); //Enable timer
    // PGEX2_FIELD("TMC", timer_def.count); //Count of timer units
    // PGEX2_FIELD("TMI", timer_def.interval); //Interval of timer tick
    // PGEX2_FIELD("TMD", timer_def.count_dir); //Direction of count
    // PGEX2_FIELD("TMV", timer_def.show); //Show timer on screen
    //-------------------
    PGEX2_FIELD("TE", trigger); //Trigger event
    PGEX2_FIELD("TD", trigger_timer); //Trigger delay
    PGEX2_FIELD("DS", nosmoke); //Disable smoke
    PGEX2_FIELD("AU", autostart); //Auto start
    PGEX2_FIELD("AUC", autostart_condition); //Auto start condition
    PGEX2_UNIQUE_FIELD("PC", PGEX2_LevelEvent_load_controls);
    PGEX2_FIELD("ML", movelayer);   //Move layer
    PGEX2_FIELD("MX", layer_speed_x); //Layer motion speed X
    PGEX2_FIELD("MY", layer_speed_y); //Layer motion speed Y
    PGEX2_FIELD("AS", scroll_section); //Autoscroll section ID
    PGEX2_FIELD("AX", move_camera_x); //Autoscroll speed X
    PGEX2_FIELD("AY", move_camera_y); //Autoscroll speed Y
);

PGEX2_SETUP_OBJECT(LevelVariable,
    PGEX2_FIELD("N", name); //Variable name
    PGEX2_FIELD("V", value); //Variable value
    PGEX2_FIELD("G", is_global); //Is global variable
);

PGEX2_SETUP_OBJECT(LevelArray,
    PGEX2_FIELD("N", name); //Array name
);

PGEX2_SETUP_OBJECT(LevelScript,
    PGEX2_FIELD("N", name);  //Variable name
    PGEX2_FIELD("L", language);  //Variable name
    PGEX2_FIELD("S", script); //Script text
);

template<>
const char* PGEX2_FieldType<LevelItemSetup38A::ItemType>::load(LevelItemSetup38A::ItemType& dest, const char* field_data)
{
    char* str_end;
    long got = strtol(field_data, &str_end, 10);
    if(got < 0 || got > 2)
        dest = LevelItemSetup38A::UNKNOWN;
    else
        dest = (LevelItemSetup38A::ItemType)got;
    return str_end;
}

PGEX2_SETUP_OBJECT(LevelItemSetup38A,
    PGEX2_FIELD("T",  type); //Type of item
    PGEX2_FIELD("ID", id);
    // PGEX2_FIELD("D", data); //Variable value
);

struct PGEX2_LevelFile : PGEX2_File<PGEX2_LevelCallbacks>
{
    PGEX2_SECTION("HEAD", PGEX2_LevelHead, head);

    PGEX2_SECTION("BOOKMARK", Bookmark, bookmark);

    PGEX2_SECTION("META_SYS_CRASH", CrashData, crash_data);

    PGEX2_SECTION("SECTION", LevelSection, section);

    PGEX2_SECTION("STARTPOINT", PlayerPoint, startpoint);

    PGEX2_SECTION("BLOCK", LevelBlock, block);

    PGEX2_SECTION("BGO", LevelBGO, bgo);

    PGEX2_SECTION("NPC", LevelNPC, npc);

    PGEX2_SECTION("PHYSICS", LevelPhysEnv, phys);

    PGEX2_SECTION("DOORS", LevelDoor, warp);

    PGEX2_SECTION("LAYERS", LevelLayer, layer);

    PGEX2_SECTION("EVENTS_CLASSIC", LevelSMBX64Event, event);

    PGEX2_SECTION("VARIABLES", LevelVariable, var);

    PGEX2_SECTION("ARRAYS", LevelArray, arr);

    PGEX2_SECTION("SCRIPTS", LevelScript, script);

    PGEX2_SECTION("CUSTOM_ITEMS_38A", LevelItemSetup38A, levelitem38a);
};

void PGEX2_load_level(PGE_FileFormats_misc::TextInput& input, PGEX2_LevelCallbacks& callbacks)
{
    PGEX2_LevelFile f;
    f.load_file(input, callbacks);
}
