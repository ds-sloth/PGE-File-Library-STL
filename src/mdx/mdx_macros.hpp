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
#ifndef MDX_MACROS_HPP
#define MDX_MACROS_HPP

/*! \file mdx_macros.hpp
 *
 *  \brief Unpleasant macros used for declarative syntax
 *
 * This is a new implementation but supports precisely the same format as PGE-X
 *
 */

#define MDX_FIELD_NAME(MEMBER_NAME) field_ ## MEMBER_NAME
#define MDX_FIELD(NAME, MEMBER_NAME) field<decltype(obj_t::MEMBER_NAME)> MDX_FIELD_NAME(MEMBER_NAME){this, NAME, &obj_t::MEMBER_NAME}

#define MDX_UNIQUE_FIELD_NAME(LOAD_FUNC) unique_field_ ## LOAD_FUNC
#define MDX_UNIQUE_FIELD(NAME, LOAD_FUNC) MDX_UniqueField<obj_t> MDX_UNIQUE_FIELD_NAME(LOAD_FUNC){this, NAME, LOAD_FUNC}

#define MDX_CALLBACK_LOAD(CALLBACK_NAME) load_ ## CALLBACK_NAME
#define MDX_CALLBACK_SAVE(CALLBACK_NAME) save_ ## CALLBACK_NAME
#define MDX_SECTION_NAME(OBJ_T) section_ ## OBJ_T
#define MDX_SECTION(NAME, OBJ_T, CALLBACK_NAME) section<OBJ_T> MDX_SECTION_NAME(OBJ_T){this, NAME, &load_callbacks_t::MDX_CALLBACK_LOAD(CALLBACK_NAME), &save_callbacks_t::MDX_CALLBACK_SAVE(CALLBACK_NAME)}

#define MDX_SETUP_OBJECT(OBJ_T, BODY) template<> \
struct MDX_Object<OBJ_T> : MDX_BaseObject<OBJ_T> \
{ \
    BODY \
}; \

#define MDX_ENABLE_SUB_LIST(OBJ_T) template<> \
struct MDX_FieldType<PGELIST<OBJ_T>> : public MDX_FieldType_ObjectList<MDX_Object<OBJ_T>> {}; \
\
template<> \
const MDX_Object<OBJ_T> MDX_FieldType_ObjectList<MDX_Object<OBJ_T>>::s_obj_loader{}; \

#define MDX_ENABLE_SUB_STRUCT(OBJ_T) template<> \
struct MDX_FieldType<OBJ_T> : public MDX_FieldType_Object<MDX_Object<OBJ_T>> {}; \
\
template<> \
const MDX_Object<OBJ_T> MDX_FieldType_Object<MDX_Object<OBJ_T>>::s_obj_loader{} \

#endif // #ifndef MDX_MACROS_HPP
