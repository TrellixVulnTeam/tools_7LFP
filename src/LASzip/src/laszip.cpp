/*
===============================================================================

  FILE:  laszip.cpp
  
  CONTENTS:
  
    see corresponding header file
  
  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2013, martin isenburg, rapidlasso - tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    see corresponding header file
  
===============================================================================
*/
#include "laszip.hpp"
#include "mydefs.hpp"
#include <assert.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

LASzip::LASzip()
{
  compressor = LASZIP_COMPRESSOR_DEFAULT;
  coder = LASZIP_CODER_ARITHMETIC;
  version_major = LASZIP_VERSION_MAJOR;
  version_minor = LASZIP_VERSION_MINOR;
  version_revision = LASZIP_VERSION_REVISION;
  options = 0;
  num_items = 0;
  chunk_size = LASZIP_CHUNK_SIZE_DEFAULT;
  num_points = -1;
  num_bytes = -1;
  error_string = 0;
  items = 0;
  bytes = 0;
}

LASzip::~LASzip()
{
  if (error_string) free(error_string);
  if (items) delete [] items;
  if (bytes) delete [] bytes;
}

// the data of the LASzip VLR
//     U16  compressor         2 bytes 
//     U16  coder              2 bytes 
//     U8   version_major      1 byte 
//     U8   version_minor      1 byte
//     U16  version_revision   2 bytes
//     U32  options            4 bytes 
//     U32  chunk_size         4 bytes
//     I64  num_points         8 bytes
//     I64  num_bytes          8 bytes
//     U16  num_items          2 bytes
//        U16 type                2 bytes * num_items
//        U16 size                2 bytes * num_items
//        U16 version             2 bytes * num_items
// which totals 34+6*num_items

// unpack from VLR data
bool LASzip::unpack(const U8* bytes, const I32 num)
{
  // check input
  if (num < 34) return return_error("too few bytes to unpack");
  if (((num - 34) % 6) != 0) return return_error("wrong number bytes to unpack"); 
  if (((num - 34) / 6) == 0) return return_error("zero items to unpack");
  num_items = (num - 34) / 6;

  // create item list
  if (items) delete [] items;
  items = new LASitem[num_items];

  // do the unpacking
  U16 i;
  const U8* b = bytes;
  compressor = *((U16*)b);
  b += 2;
  coder = *((U16*)b);
  b += 2;
  version_major = *((U8*)b);
  b += 1;
  version_minor = *((U8*)b);
  b += 1;
  version_revision = *((U16*)b);
  b += 2;
  options = *((U32*)b);
  b += 4;
  chunk_size = *((U32*)b);
  b += 4;
  num_points = *((I64*)b);
  b += 8;
  num_bytes = *((I64*)b);
  b += 8;
  num_items = *((U16*)b);
  b += 2;
  for (i = 0; i < num_items; i++)
  {
    items[i].type = (LASitem::Type)*((U16*)b);
    b += 2;
    items[i].size = *((U16*)b);
    b += 2;
    items[i].version = *((U16*)b);
    b += 2;
  }
  assert((bytes + num) == b);

  // check if we support the contents

  for (i = 0; i < num_items; i++)
  {
    if (!check_item(&items[i])) return false;
  }
  return true;
}

// pack to VLR data
bool LASzip::pack(U8*& bytes, I32& num)
{
  // check if we support the contents
  if (!check()) return false;

  // prepare output
  num = 34 + 6*num_items;
  if (this->bytes) delete [] this->bytes;
  this->bytes = bytes = new U8[num];

  // pack
  U16 i;
  U8* b = bytes;
  *((U16*)b) = compressor;
  b += 2;
  *((U16*)b) = coder;
  b += 2;
  *((U8*)b) = version_major;
  b += 1;
  *((U8*)b) = version_minor;
  b += 1;
  *((U16*)b) = version_revision;
  b += 2;
  *((U32*)b) = options;
  b += 4;
  *((U32*)b) = chunk_size;
  b += 4;
  *((I64*)b) = num_points;
  b += 8;
  *((I64*)b) = num_bytes;
  b += 8;
  *((U16*)b) = num_items;
  b += 2;
  for (i = 0; i < num_items; i++)
  {
    *((U16*)b) = (U16)items[i].type;
    b += 2;
    *((U16*)b) = items[i].size;
    b += 2;
    *((U16*)b) = items[i].version;
    b += 2;
  }
  assert((bytes + num) == b);
  return true;
}

const char* LASzip::get_error() const
{
  return error_string;
}

bool LASzip::return_error(const char* error)
{
  char err[256];
  sprintf(err, "%s (LASzip v%d.%dr%d)", error, LASZIP_VERSION_MAJOR, LASZIP_VERSION_MINOR, LASZIP_VERSION_REVISION);
  if (error_string) free(error_string);
  error_string = _strdup(err);
  return false;
}

bool LASzip::check_compressor(const U16 compressor)
{
  if (compressor < LASZIP_COMPRESSOR_TOTAL_NUMBER_OF) return true;
  char error[64];
  sprintf(error, "compressor %d not supported", compressor);
  return return_error(error);
}

bool LASzip::check_coder(const U16 coder)
{
  if (coder < LASZIP_CODER_TOTAL_NUMBER_OF) return true;
  char error[64];
  sprintf(error, "coder %d not supported", coder);
  return return_error(error);
}

bool LASzip::check_item(const LASitem* item)
{
  switch (item->type)
  {
  case LASitem::POINT10:
    if (item->size != 20) return return_error("POINT10 has size != 20");
    if (item->version > 2) return return_error("POINT10 has version > 2");
    break;
  case LASitem::GPSTIME11:
    if (item->size != 8) return return_error("GPSTIME11 has size != 8");
    if (item->version > 2) return return_error("GPSTIME11 has version > 2");
    break;
  case LASitem::RGB12:
    if (item->size != 6) return return_error("RGB12 has size != 6");
    if (item->version > 2) return return_error("RGB12 has version > 2");
    break;
  case LASitem::WAVEPACKET13:
    if (item->size != 29) return return_error("WAVEPACKET13 has size != 29");
    if (item->version > 1) return return_error("WAVEPACKET13 has version > 1");
    break;
  case LASitem::BYTE:
    if (item->size < 1) return return_error("BYTE has size < 1");
    if (item->version > 2) return return_error("BYTE has version > 2");
    break;
  case LASitem::POINT14:
    if (item->size != 30) return return_error("POINT14 has size != 30");
    if (item->version > 0) return return_error("POINT14 has version > 0");
    break;
  case LASitem::RGBNIR14:
    if (item->size != 8) return return_error("RGBNIR14 has size != 8");
    if (item->version > 0) return return_error("RGBNIR14 has version > 0");
    break;
  default:
    if (1)
    {
      char error[64];
      sprintf(error, "item unknown (%d,%d,%d)", item->type, item->size, item->version);
      return return_error(error);
    }
  }
  return true;
}

bool LASzip::check_items(const U16 num_items, const LASitem* items)
{
  if (num_items == 0) return return_error("number of items cannot be zero");
  if (items == 0) return return_error("items pointer cannot be NULL");
  U16 i;
  for (i = 0; i < num_items; i++)
  {
    if (!check_item(&items[i])) return false;
  }
  return true;
}

bool LASzip::check()
{
  if (!check_compressor(compressor)) return false;
  if (!check_coder(coder)) return false;
  if (!check_items(num_items, items)) return false;
  return true;
}

bool LASzip::setup(const U8 point_type, const U16 point_size, const U16 compressor)
{
  if (!check_compressor(compressor)) return false;
  this->num_items = 0;
  if (this->items) delete [] this->items;
  this->items = 0;
  if (!setup(&num_items, &items, point_type, point_size, compressor)) return false;
  this->compressor = compressor;
  if (this->compressor == LASZIP_COMPRESSOR_POINTWISE_CHUNKED)
  {
    if (chunk_size == 0) chunk_size = LASZIP_CHUNK_SIZE_DEFAULT;
  }
  return true;
}

bool LASzip::setup(const U16 num_items, const LASitem* items, const U16 compressor)
{
  // check input
  if (!check_compressor(compressor)) return false;
  if (!check_items(num_items, items)) return false;

  // setup compressor
  this->compressor = compressor;
  if (this->compressor == LASZIP_COMPRESSOR_POINTWISE_CHUNKED)
  {
    if (chunk_size == 0) chunk_size = LASZIP_CHUNK_SIZE_DEFAULT;
  }

  // prepare items
  this->num_items = 0;
  if (this->items) delete [] this->items;
  this->items = 0;
  this->num_items = num_items;
  this->items = new LASitem[num_items];

  // setup items
  U16 i;
  for (i = 0; i < num_items; i++)
  {
    this->items[i] = items[i];
  }

  return true;
}

bool LASzip::setup(U16* num_items, LASitem** items, const U8 point_type, const U16 point_size, const U16 compressor)
{
  BOOL have_point14 = FALSE;
  BOOL have_gps_time = FALSE;
  BOOL have_rgb = FALSE;
  BOOL have_nir = FALSE;
  BOOL have_wavepacket = FALSE;
  I32 extra_bytes_number = 0;

  // switch over the point types we know
  switch (point_type)
  {
  case 0:
    extra_bytes_number = (I32)point_size - 20;
    break;
  case 1:
    have_gps_time = TRUE;
    extra_bytes_number = (I32)point_size - 28;
    break;
  case 2:
    have_rgb = TRUE;
    extra_bytes_number = (I32)point_size - 26;
    break;
  case 3:
    have_gps_time = TRUE;
    have_rgb = TRUE;
    extra_bytes_number = (I32)point_size - 34;
    break;
  case 4:
    have_gps_time = TRUE;
    have_wavepacket = TRUE;
    extra_bytes_number = (I32)point_size - 57;
    break;
  case 5:
    have_gps_time = TRUE;
    have_rgb = TRUE;
    have_wavepacket = TRUE;
    extra_bytes_number = (I32)point_size - 63;
    break;
  case 6:
    have_point14 = TRUE;
    extra_bytes_number = (I32)point_size - 30;
    break;
  case 7:
    have_point14 = TRUE;
    have_rgb = TRUE;
    extra_bytes_number = (I32)point_size - 36;
    break;
  case 8:
    have_point14 = TRUE;
    have_rgb = TRUE;
    have_nir = TRUE;
    extra_bytes_number = (I32)point_size - 38;
    break;
  case 9:
    have_point14 = TRUE;
    have_wavepacket = TRUE;
    extra_bytes_number = (I32)point_size - 59;
    break;
  case 10:
    have_point14 = TRUE;
    have_rgb = TRUE;
    have_nir = TRUE;
    have_wavepacket = TRUE;
    extra_bytes_number = (I32)point_size - 67;
    break;
  default:
    if (1)
    {
      char error[64];
      sprintf(error, "point type %d unknown", point_type);
      return return_error(error);
    }
  }

  if (extra_bytes_number < 0)
  {
    char error[64];
    sprintf(error, "point size %d too small for point type %d by %d bytes", point_size, point_type, -extra_bytes_number);
    return return_error(error);
  }

  // create item description

  (*num_items) = 1 + !!(have_gps_time) + !!(have_rgb) + !!(have_wavepacket) + !!(extra_bytes_number);
  (*items) = new LASitem[*num_items];

  U16 i = 1;
  if (have_point14)
  {
    (*items)[0].type = LASitem::POINT14;
    (*items)[0].size = 30;
    (*items)[0].version = 0;
  }
  else
  {
    (*items)[0].type = LASitem::POINT10;
    (*items)[0].size = 20;
    (*items)[0].version = 0;
  }
  if (have_gps_time)
  {
    (*items)[i].type = LASitem::GPSTIME11;
    (*items)[i].size = 8;
    (*items)[i].version = 0;
    i++;
  }
  if (have_rgb)
  {
    if (have_nir)
    {
      (*items)[i].type = LASitem::RGBNIR14;
      (*items)[i].size = 8;
      (*items)[i].version = 0;
    }
    else
    {
      (*items)[i].type = LASitem::RGB12;
      (*items)[i].size = 6;
      (*items)[i].version = 0;
    }
    i++;
  }
  if (have_wavepacket)
  {
    (*items)[i].type = LASitem::WAVEPACKET13;
    (*items)[i].size = 29;
    (*items)[i].version = 0;
    i++;
  }
  if (extra_bytes_number)
  {
    (*items)[i].type = LASitem::BYTE;
    (*items)[i].size = extra_bytes_number;
    (*items)[i].version = 0;
    i++;
  }
  if (compressor) request_version(2);
  assert(i == *num_items);
  return true;
}

bool LASzip::set_chunk_size(const U32 chunk_size)
{
  if (num_items == 0) return return_error("call setup() before setting chunk size");
  if (this->compressor == LASZIP_COMPRESSOR_POINTWISE_CHUNKED)
  {
    this->chunk_size = chunk_size;
    return true;
  }
  return false;
}

bool LASzip::request_version(const U16 requested_version)
{
  if (num_items == 0) return return_error("call setup() before requesting version");
  if (compressor == LASZIP_COMPRESSOR_NONE)
  {
    if (requested_version > 0) return return_error("without compression version is always 0");
  }
  else
  {
    if (requested_version < 1) return return_error("with compression version is at least 1");
    if (requested_version > 2) return return_error("version larger than 2 not supported");
  }
  U16 i;
  for (i = 0; i < num_items; i++)
  {
    switch (items[i].type)
    {
    case LASitem::POINT10:
    case LASitem::GPSTIME11:
    case LASitem::RGB12:
    case LASitem::BYTE:
        items[i].version = requested_version;
        break;
    case LASitem::WAVEPACKET13:
        items[i].version = 1; // no version 2
        break;
    default:
        return return_error("itrm type not supported");
    }
  }
  return true;
}

bool LASzip::is_standard(U8* point_type, U16* record_length)
{
  return is_standard(num_items, items, point_type, record_length);
}

bool LASzip::is_standard(const U16 num_items, const LASitem* items, U8* point_type, U16* record_length)
{
  if (items == 0) return return_error("LASitem array is zero");

  // this is always true
  if (point_type) *point_type = 127;
  if (record_length)
  {
    U16 i;
    *record_length = 0;
    for (i = 0; i < num_items; i++)
    {
      *record_length += items[i].size;
    }
  }

  // the minimal number of items is 1
  if (num_items < 1) return return_error("less than one LASitem entries");
  // the maximal number of items is 5
  if (num_items > 5) return return_error("more than five LASitem entries");

  if (items[0].is_type(LASitem::POINT10))
  {
    // consider all the POINT10 combinations
    if (num_items == 1)
    {
      if (point_type) *point_type = 0;
      if (record_length) assert(*record_length == 20);
      return true;
    }
    else
    {
      if (items[1].is_type(LASitem::GPSTIME11))
      {
        if (num_items == 2)
        {
          if (point_type) *point_type = 1;
          if (record_length) assert(*record_length == 28);
          return true;
        }
        else
        {
          if (items[2].is_type(LASitem::RGB12))
          {
            if (num_items == 3)
            {
              if (point_type) *point_type = 3;
              if (record_length) assert(*record_length == 34);
              return true;
            }
            else
            {
              if (items[3].is_type(LASitem::WAVEPACKET13))
              {
                if (num_items == 4)
                {
                  if (point_type) *point_type = 5;
                  if (record_length) assert(*record_length == 63);
                  return true;
                }
                else
                {
                  if (items[4].is_type(LASitem::BYTE))
                  {
                    if (num_items == 5)
                    {
                      if (point_type) *point_type = 5;
                      if (record_length) assert(*record_length == (63 + items[4].size));
                      return true;
                    }
                  }
                }
              }
              else if (items[3].is_type(LASitem::BYTE))
              {
                if (num_items == 4)
                {
                  if (point_type) *point_type = 3;
                  if (record_length) assert(*record_length == (34 + items[3].size));
                  return true;
                }
              }
            }
          }
          else if (items[2].is_type(LASitem::WAVEPACKET13))
          {
            if (num_items == 3)
            {
              if (point_type) *point_type = 4;
              if (record_length) assert(*record_length == 57);
              return true;
            }
            else 
            {
              if (items[3].is_type(LASitem::BYTE))
              {
                if (num_items == 4)
                {
                  if (point_type) *point_type = 4;
                  if (record_length) assert(*record_length == (57 + items[3].size));
                  return true;
                }
              }
            }
          }
          else if (items[2].is_type(LASitem::BYTE))
          {
            if (num_items == 3)
            {
              if (point_type) *point_type = 1;
              if (record_length) assert(*record_length == (28 + items[2].size));
              return true;
            }
          }
        }
      }
      else if (items[1].is_type(LASitem::RGB12))
      {
        if (num_items == 2)
        {
          if (point_type) *point_type = 2;
          if (record_length) assert(*record_length == 26);
          return true;
        }
        else
        {
          if (items[2].is_type(LASitem::BYTE))
          {
            if (num_items == 3)
            {
              if (point_type) *point_type = 2;
              if (record_length) assert(*record_length == (26 + items[2].size));
              return true;
            }
          }
        }
      }
      else if (items[1].is_type(LASitem::BYTE))
      {
        if (num_items == 2)
        {
          if (point_type) *point_type = 0;
          if (record_length) assert(*record_length == (20 + items[1].size));
          return true;
        }
      }
    }
  }
  else if (items[0].is_type(LASitem::POINT14))
  {
    // consider all the POINT14 combinations
    if (num_items == 1)
    {
      if (point_type) *point_type = 6;
      if (record_length) assert(*record_length == 30);
      return true;
    }
    else
    {
      if (items[1].is_type(LASitem::RGB12))
      {
        if (num_items == 2)
        {
          if (point_type) *point_type = 7;
          if (record_length) assert(*record_length == 36);
          return true;
        }
        else
        {
          if (items[2].is_type(LASitem::BYTE))
          {
            if (num_items == 3)
            {
              if (point_type) *point_type = 7;
              if (record_length) assert(*record_length == (36 + items[2].size));
              return true;
            }
          }
        }
      }
      else if (items[1].is_type(LASitem::RGBNIR14))
      {
        if (num_items == 2)
        {
          if (point_type) *point_type = 8;
          if (record_length) assert(*record_length == 38);
          return true;
        }
        else
        {
          if (items[2].is_type(LASitem::WAVEPACKET13))
          {
            if (num_items == 3)
            {
              if (point_type) *point_type = 10;
              if (record_length) assert(*record_length == 67);
              return true;
            }
            else 
            {
              if (items[3].is_type(LASitem::BYTE))
              {
                if (num_items == 4)
                {
                  if (point_type) *point_type = 10;
                  if (record_length) assert(*record_length == (67 + items[3].size));
                  return true;
                }
              }
            }
          }
          else if (items[2].is_type(LASitem::BYTE))
          {
            if (num_items == 3)
            {
              if (point_type) *point_type = 8;
              if (record_length) assert(*record_length == (38 + items[2].size));
              return true;
            }
          }
        }
      }
      else if (items[1].is_type(LASitem::WAVEPACKET13))
      {
        if (num_items == 2)
        {
          if (point_type) *point_type = 9;
          if (record_length) assert(*record_length == 59);
          return true;
        }
        else
        {
          if (items[2].is_type(LASitem::BYTE))
          {
            if (num_items == 3)
            {
              if (point_type) *point_type = 9;
              if (record_length) assert(*record_length == (59 + items[2].size));
              return true;
            }
          }
        }
      }
      else if (items[1].is_type(LASitem::BYTE))
      {
        if (num_items == 2)
        {
          if (point_type) *point_type = 6;
          if (record_length) assert(*record_length == (30 + items[1].size));
          return true;
        }
      }
    }
  }
  else
  {
    return_error("first LASitem is neither POINT10 nor POINT14");
  }
  return return_error("LASitem array does not match LAS specification 1.4");
}

bool LASitem::is_type(LASitem::Type t) const
{
  if (t != type) return false;
  switch (t)
  {
  case POINT10:
      if (size != 20) return false;
      break;
  case GPSTIME11:
      if (size != 8) return false;
      break;
  case RGB12:
      if (size != 6) return false;
      break;
  case WAVEPACKET13:
      if (size != 29) return false;
      break;
  case BYTE:
      if (size < 1) return false;
      break;
  default:
      return false;
  }
  return true;
}

const char* LASitem::get_name() const
{
  switch (type)
  {
  case POINT10:
      return "POINT10";
      break;
  case GPSTIME11:
      return "GPSTIME11";
      break;
  case RGB12:
      return "RGB12";
      break;
  case WAVEPACKET13:
      return "WAVEPACKET13";
      break;
  case BYTE:
      return "BYTE";
      break;
  default:
      break;
  }
  return 0;
}

