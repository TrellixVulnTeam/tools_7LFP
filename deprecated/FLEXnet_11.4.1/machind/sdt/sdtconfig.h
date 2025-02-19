/***************************************************************************************************
 *
 * $Archive: /Eng/Common/Library/Security/Sdt/include/Sdt/SdtConfig.h $     
 *
 * $Revision: 64 $
 *
 * Description:         
 *        
 ***************************************************************************************************
 * Copyright 2005 - 2006 Macrovision Europe Ltd. All Rights Reserved. 
 * 
 * This software has been provided pursuant to a License Agreement containing
 * restrictions on its use. This software contains valuable trade secrets 
 * and proprietary information of Macrovision Europe Ltd. and is protected 
 * by law. It may not be copied or distributed in any form or medium, 
 * disclosed to third parties, reverse engineered or used in any manner not 
 * provided for in said License Agreement except with the prior written 
 * authorization from Macrovision Europe Ltd. 
 ***************************************************************************************************
 *
 * $NoKeywords: $
 *
 **************************************************************************************************/

#ifndef __SDT_CONFIG_H__
#define __SDT_CONFIG_H__

/* Prevent copious warnings caused by the hashing functions */
#ifdef _MSC_VER
#pragma warning( disable : 4307 4308 )
#endif

/* Change these keys for EVERY release of the product that goes out */
#define SDT_CONFIG_SEED_0       0x00000055
#define SDT_CONFIG_SEED_1       0x00000066
#define SDT_CONFIG_SEED_2       0x00000077
#define SDT_CONFIG_SEED_3       0x00000088
#define SDT_CONFIG_SEED_4       0x000000a9
#define SDT_CONFIG_SEED_5       0x000000ba
#define SDT_CONFIG_SEED_6       0x000000cb
#define SDT_CONFIG_SEED_7       0x000000dc

/* Change this to remove encoding (for debugging / testing) */
#ifndef SDT_CONFIG_ENCODING
#define SDT_CONFIG_ENCODING     SDT_API_ENCODING_EXCLUDE_NONE
#endif

/* Change this to put code into plugins in Sdt code */
#ifndef SDT_DEFAULT_PLUGIN
#define SDT_DEFAULT_PLUGIN  Sdt::CDefaultPlugins
#endif

/* Change this to change the number of encoders available */
/* Must be a multiple of 64 and not more than 1024 */
#ifndef SDT_ENCODER_USER_LIMIT
#define SDT_ENCODER_USER_LIMIT 64
#endif

#endif /* __SDT_CONFIG_H__ */
