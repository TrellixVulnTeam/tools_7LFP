//----------------------------------------------------------------------------
//
// File: ossimLasPointRecord2.h
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description: Container class for LAS point record type 2.
//
//----------------------------------------------------------------------------
// $Id$
#ifndef ossimLasPointRecord2_HEADER
#define ossimLasPointRecord2_HEADER 1

#include "ossimLasPointRecordInterface.h"

class ossimLasPointRecord2 : public ossimLasPointRecordInterface
{
public:

   /** @brief default constructor */
   ossimLasPointRecord2();

   /* @brief copy constructor */
   ossimLasPointRecord2(const ossimLasPointRecord2& obj);

   /* @brief assignment operator= */
   const ossimLasPointRecord2& operator=(const ossimLasPointRecord2& copy_this);

   /** @brief destructor */
   virtual ~ossimLasPointRecord2();

   /** @brief Initialize record from stream. */
   virtual void readStream(std::istream& in);

   /** @brief Writes record to stream. */
   virtual void writeStream(std::ostream& out);

   /** @return x */
   virtual ossim_int32 getX() const;

   /** @return y */
   virtual ossim_int32 getY() const;
   
   /** @return z */
   virtual ossim_int32 getZ() const;

   /** @return return number */
   virtual ossim_uint8 getReturnNumber() const;

   virtual ossim_uint8 getNumberOfReturns() const;
   
   virtual ossim_uint8 getEdgeFlag() const;
      
   virtual std::ostream& print(std::ostream& out) const;

private:

   /** @brief Performs a swap if system byte order is not little endian. */
   void swap();

   union BitFlags
   {
      BitFlags(const BitFlags& obj):m_bits(obj.m_bits){}
      BitFlags(ossim_uint8 value):m_bits((ossim_uint8)value){}
      const BitFlags& operator =(const BitFlags& obj){m_bits = obj.m_bits;return *this;}
      struct
      {
         ossim_uint8 m_returnNumber     : 3;
         ossim_uint8 m_numberOfReturns  : 3;
         ossim_uint8 m_scanDirection    : 1;
         ossim_uint8 m_edgeOfFlightLine : 1;
      };
      ossim_uint8 m_bits;
   };

   //---
   // Private structure so we can do a single read/write in the readStream
   // and writeStream methods.  This sped the code up 5X when doing an ossim-icp
   // from a LAS file to a tif.
   //---
   struct ossimLasPointRecord2Data
   {
      ossimLasPointRecord2Data();
      ossimLasPointRecord2Data(const ossimLasPointRecord2Data& obj);
      const ossimLasPointRecord2Data& operator=(const ossimLasPointRecord2Data& obj);
      ossim_int32   m_x;
      ossim_int32   m_y;
      ossim_int32   m_z;
      ossim_uint16  m_intensity;
      BitFlags      m_bitFlags;
      ossim_uint8   m_classification;
      ossim_int8    m_scanAngleRank;
      ossim_uint8   m_userData;
      ossim_uint16  m_pointSourceId;
      ossim_uint16  m_red;
      ossim_uint16  m_green;
      ossim_uint16  m_blue;
   };
   
   ossimLasPointRecord2Data m_record;
};

#endif /* #ifndef ossimLasPointRecord2_HEADER */

