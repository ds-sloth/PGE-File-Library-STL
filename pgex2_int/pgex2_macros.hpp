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
#ifndef PGEX2_MACROS_HPP
#define PGEX2_MACROS_HPP

/*! \file pgex2_macros.hpp
 *
 *  \brief Unpleasant macros used for declarative syntax
 *
 * This is a new implementation but supports precisely the same format as PGE-X
 *
 */

#define PGEX2_FIELD_NAME(MEMBER_NAME) field_ ## MEMBER_NAME
#define PGEX2_FIELD(NAME, MEMBER_NAME) field<decltype(obj_t::MEMBER_NAME)> PGEX2_FIELD_NAME(MEMBER_NAME){this, NAME, &obj_t::MEMBER_NAME}

#define PGEX2_UNIQUE_FIELD_NAME(LOAD_FUNC) unique_field_ ## LOAD_FUNC
#define PGEX2_UNIQUE_FIELD(NAME, LOAD_FUNC) PGEX2_UniqueField<obj_t> PGEX2_UNIQUE_FIELD_NAME(LOAD_FUNC){this, NAME, LOAD_FUNC}

#define PGEX2_CALLBACK_LOAD(CALLBACK_NAME) load_ ## CALLBACK_NAME
#define PGEX2_CALLBACK_SAVE(CALLBACK_NAME) save_ ## CALLBACK_NAME
#define PGEX2_SECTION_NAME(OBJ_T) section_ ## OBJ_T
#define PGEX2_SECTION(NAME, OBJ_T, CALLBACK_NAME, BODY) struct : section<OBJ_T> \
{ \
    using section<OBJ_T>::section; \
    BODY \
} PGEX2_SECTION_NAME(OBJ_T){this, NAME, &callback_table_t::PGEX2_CALLBACK_LOAD(CALLBACK_NAME), &callback_table_t::PGEX2_CALLBACK_SAVE(CALLBACK_NAME)}

#define PGEX2_SETUP_OBJECT_LIST_NAME(OBJ_T) PGEX2_Object_ ## OBJ_T
#define PGEX2_SETUP_OBJECT_LIST(OBJ_T, BODY) struct PGEX2_SETUP_OBJECT_LIST_NAME(OBJ_T) : PGEX2_Object<OBJ_T> \
{ \
    BODY \
}; \
\
template<> \
struct PGEX2_FieldType<PGELIST<OBJ_T>> : public PGEX2_FieldType_ObjectList<PGEX2_SETUP_OBJECT_LIST_NAME(OBJ_T)> {}; \
\
template<> \
const PGEX2_SETUP_OBJECT_LIST_NAME(OBJ_T) PGEX2_FieldType_ObjectList<PGEX2_SETUP_OBJECT_LIST_NAME(OBJ_T)>::s_obj_loader{}

#endif // #ifndef PGEX2_MACROS_HPP
