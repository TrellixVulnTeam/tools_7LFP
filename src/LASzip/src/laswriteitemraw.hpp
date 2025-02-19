/*
===============================================================================

  FILE:  LASwriteitemraw.hpp
  
  CONTENTS:
  
    Implementation of LASwriteItemRaw for *all* items that compose a point.

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2012, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    10 January 2011 -- licensing change for LGPL release and liblas integration
    7 January 2011 -- introduced swap buffers to reduce number of fwrite calls
    12 December 2010 -- refactored after watching two movies with silke
  
===============================================================================
*/
#ifndef LAS_WRITE_ITEM_RAW_H
#define LAS_WRITE_ITEM_RAW_H

#include "laswriteitem.hpp"

#include <assert.h>

class LASwriteItemRaw_POINT10_LE : public LASwriteItemRaw
{
public:
  LASwriteItemRaw_POINT10_LE(){};
  inline BOOL write(const U8* item)
  {
    return outstream->putBytes(item, 20);
  };
};

class LASwriteItemRaw_POINT10_BE : public LASwriteItemRaw
{
public:
  LASwriteItemRaw_POINT10_BE(){};
  inline BOOL write(const U8* item)
  {
    ENDIAN_SWAP_32(&item[ 0], &swapped[ 0]);    // x
    ENDIAN_SWAP_32(&item[ 4], &swapped[ 4]);    // y
    ENDIAN_SWAP_32(&item[ 8], &swapped[ 8]);    // z
    ENDIAN_SWAP_16(&item[12], &swapped[12]);    // intensity
    *((U32*)&swapped[14]) = *((U32*)&item[14]); // bitfield, classification, scan_angle_rank, user_data
    ENDIAN_SWAP_16(&item[18], &swapped[18]);    // point_source_ID
    return outstream->putBytes(swapped, 20);
  };
private:
  U8 swapped[20];
};

class LASwriteItemRaw_GPSTIME11_LE : public LASwriteItemRaw
{
public:
  LASwriteItemRaw_GPSTIME11_LE() {};
  inline BOOL write(const U8* item)
  {
    return outstream->putBytes(item, 8);
  };
};

class LASwriteItemRaw_GPSTIME11_BE : public LASwriteItemRaw
{
public:
  LASwriteItemRaw_GPSTIME11_BE() {};
  inline BOOL write(const U8* item)
  {
    ENDIAN_SWAP_64(item, swapped);
    return outstream->putBytes(swapped, 8);
  };
private:
  U8 swapped[8];
};

class LASwriteItemRaw_RGB12_LE : public LASwriteItemRaw
{
public:
  LASwriteItemRaw_RGB12_LE(){}
  inline BOOL write(const U8* item)
  {
    return outstream->putBytes(item, 6);
  };
};

class LASwriteItemRaw_RGB12_BE : public LASwriteItemRaw
{
public:
  LASwriteItemRaw_RGB12_BE(){}
  inline BOOL write(const U8* item)
  {
    ENDIAN_SWAP_32(&item[ 0], &swapped[ 0]); // R
    ENDIAN_SWAP_32(&item[ 2], &swapped[ 2]); // G
    ENDIAN_SWAP_32(&item[ 4], &swapped[ 4]); // B
    return outstream->putBytes(swapped, 6);
  };
private:
  U8 swapped[6];
};

class LASwriteItemRaw_WAVEPACKET13_LE : public LASwriteItemRaw
{
public:
  LASwriteItemRaw_WAVEPACKET13_LE(){}
  inline BOOL write(const U8* item)
  {
    return outstream->putBytes(item, 29);
  };
};

class LASwriteItemRaw_WAVEPACKET13_BE : public LASwriteItemRaw
{
public:
  LASwriteItemRaw_WAVEPACKET13_BE(){}
  inline BOOL write(const U8* item)
  {
    swapped[0] = item[0];                    // wavepacket descriptor index
    ENDIAN_SWAP_64(&item[ 1], &swapped[ 1]); // byte offset to waveform data
    ENDIAN_SWAP_32(&item[ 9], &swapped[ 9]); // waveform packet size in bytes
    ENDIAN_SWAP_32(&item[13], &swapped[13]); // return point waveform location
    ENDIAN_SWAP_32(&item[17], &swapped[17]); // X(t)
    ENDIAN_SWAP_32(&item[21], &swapped[21]); // Y(t)
    ENDIAN_SWAP_32(&item[25], &swapped[25]); // Z(t)
    return outstream->putBytes(swapped, 29);
  };
private:
  U8 swapped[29];
};

class LASwriteItemRaw_BYTE : public LASwriteItemRaw
{
public:
  LASwriteItemRaw_BYTE(U32 number)
  {
    this->number = number;
  }
  inline BOOL write(const U8* item)
  {
    return outstream->putBytes(item, number);
  };
private:
  U32 number;
};

class LAStempWritePoint10
{
public:
  I32 x;
  I32 y;
  I32 z;
  U16 intensity;
  U8 return_number : 3;
  U8 number_of_returns_of_given_pulse : 3;
  U8 scan_direction_flag : 1;
  U8 edge_of_flight_line : 1;
  U8 classification;
  I8 scan_angle_rank;
  U8 user_data;
  U16 point_source_ID;
  // LAS 1.4 only
  U8 extended_point_type : 2;
  U8 extended_scanner_channel : 2;
  U8 extended_classification_flags : 4;
  U8 extended_classification;
  U8 extended_return_number : 4;
  U8 extended_number_of_returns_of_given_pulse : 4;
  I16 extended_scan_angle;
};

class LAStempWritePoint14
{
public:
  I32 x;
  I32 y;
  I32 z;
  U16 intensity;
  U8 return_number : 4;
  U8 number_of_returns_of_given_pulse : 4;
  U8 classification_flags : 4;
  U8 scanner_channel : 2;
  U8 scan_direction_flag : 1;
  U8 edge_of_flight_line : 1;
  U8 classification;
  U8 user_data;
  I16 scan_angle;
  U16 point_source_ID;
};

class LASwriteItemRaw_POINT14_LE : public LASwriteItemRaw
{
public:
  LASwriteItemRaw_POINT14_LE(){};
  inline BOOL write(const U8* item)
  {
    ((LAStempWritePoint14*)buffer)->x = ((LAStempWritePoint10*)item)->x;
    ((LAStempWritePoint14*)buffer)->y = ((LAStempWritePoint10*)item)->y;
    ((LAStempWritePoint14*)buffer)->z = ((LAStempWritePoint10*)item)->z;
    ((LAStempWritePoint14*)buffer)->intensity = ((LAStempWritePoint10*)item)->intensity;
    ((LAStempWritePoint14*)buffer)->scan_direction_flag = ((LAStempWritePoint10*)item)->scan_direction_flag;
    ((LAStempWritePoint14*)buffer)->edge_of_flight_line = ((LAStempWritePoint10*)item)->edge_of_flight_line;
    ((LAStempWritePoint14*)buffer)->classification = ((LAStempWritePoint10*)item)->classification & 31;
    ((LAStempWritePoint14*)buffer)->user_data = ((LAStempWritePoint10*)item)->user_data;
    ((LAStempWritePoint14*)buffer)->point_source_ID = ((LAStempWritePoint10*)item)->point_source_ID;

    if (((LAStempWritePoint10*)item)->extended_point_type)
    {
      ((LAStempWritePoint14*)buffer)->classification_flags = ((LAStempWritePoint10*)item)->extended_classification_flags | (((LAStempWritePoint10*)item)->classification >> 5);
      if (((LAStempWritePoint10*)item)->extended_classification > 31) ((LAStempWritePoint14*)buffer)->classification = ((LAStempWritePoint10*)item)->extended_classification;
      ((LAStempWritePoint14*)buffer)->scanner_channel = ((LAStempWritePoint10*)item)->extended_scanner_channel;
      ((LAStempWritePoint14*)buffer)->return_number = ((LAStempWritePoint10*)item)->extended_return_number;
      ((LAStempWritePoint14*)buffer)->number_of_returns_of_given_pulse = ((LAStempWritePoint10*)item)->extended_number_of_returns_of_given_pulse;
      ((LAStempWritePoint14*)buffer)->scan_angle = ((LAStempWritePoint10*)item)->extended_scan_angle;
    }
    else
    {
      ((LAStempWritePoint14*)buffer)->classification_flags = (((LAStempWritePoint10*)item)->classification >> 5);
      ((LAStempWritePoint14*)buffer)->scanner_channel = 0;
      ((LAStempWritePoint14*)buffer)->return_number = ((LAStempWritePoint10*)item)->return_number;
      ((LAStempWritePoint14*)buffer)->number_of_returns_of_given_pulse = ((LAStempWritePoint10*)item)->number_of_returns_of_given_pulse;
      ((LAStempWritePoint14*)buffer)->scan_angle = I16_QUANTIZE(((LAStempWritePoint10*)item)->scan_angle_rank/0.006f);
    }

    *((F64*)&buffer[22]) = *((F64*)&item[24]);
    return outstream->putBytes(buffer, 30);
  }
private:
  U8 buffer[30];
};

class LASwriteItemRaw_RGBNIR14_LE : public LASwriteItemRaw
{
public:
  LASwriteItemRaw_RGBNIR14_LE(){}
  inline BOOL write(const U8* item)
  {
    return outstream->putBytes(item, 8);
  };
};

class LASwriteItemRaw_RGBNIR14_BE : public LASwriteItemRaw
{
public:
  LASwriteItemRaw_RGBNIR14_BE(){}
  inline BOOL write(const U8* item)
  {
    ENDIAN_SWAP_32(&item[ 0], &swapped[ 0]); // R
    ENDIAN_SWAP_32(&item[ 2], &swapped[ 2]); // G
    ENDIAN_SWAP_32(&item[ 4], &swapped[ 4]); // B
    ENDIAN_SWAP_32(&item[ 6], &swapped[ 6]); // NIR
    return outstream->putBytes(swapped, 8);
  };
private:
  U8 swapped[8];
};

#endif
