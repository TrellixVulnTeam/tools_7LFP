/*********** Preprocessed module -- do not edit ***************/
/*********** Preprocessed module -- do not edit ***************/
/*********** Preprocessed module -- do not edit ***************/
/*********** Preprocessed module -- do not edit ***************/
/*********** Preprocessed module -- do not edit ***************/
/***************** gpre version WI-V2.5.7.27050 Firebird 2.5 **********************/
/*
 *____________________________________________________________
 *
 *		PROGRAM:	Alice (All Else) Utility
 *		MODULE:		alice_meta.epp
 *		DESCRIPTION:	Metadata lookup routines
 *
 *  The contents of this file are subject to the Interbase Public
 *  License Version 1.0 (the "License"); you may not use this file
 *  except in compliance with the License. You may obtain a copy
 *  of the License at http://www.Inprise.com/IPL.html
 *
 *  Software distributed under the License is distributed on an
 *  "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express
 *  or implied. See the License for the specific language governing
 *  rights and limitations under the License.
 *
 *  The Original Code was created by Inprise Corporation
 *  and its predecessors. Portions created by Inprise Corporation are
 *  Copyright (C) Inprise Corporation.
 *
 *  All Rights Reserved.
 *  Contributor(s): ______________________________________.
 *
 *
 *____________________________________________________________
 *
 */

#include "firebird.h"
#include <stdio.h>
#include "../jrd/common.h"
#include "../jrd/ibase.h"
//#include "../jrd/license.h"
#include "../alice/alice.h"
#include "../alice/alice_meta.h"
#include "../jrd/gds_proto.h"
#include "../include/fb_exception.h"
#include "../common/classes/alloc.h"
#include "../common/classes/array.h"
#include "../common/classes/UserBlob.h"
#include "../alice/alice_proto.h"

#include <string.h>

//  For service APIs the follow DB handle is a value stored
//  in thread data.  This is also done for other statics generated by
//  GPRE.  This is to avoid multiple threading problems with module
//  level statics.

/*DATABASE DB = STATIC FILENAME "yachts.lnk";*/
/**** GDS Preprocessor Definitions ****/
#ifndef JRD_IBASE_H
#include <ibase.h>
#endif

static const ISC_QUAD
   isc_blob_null = {0, 0};	/* initializer for blobs */
static isc_db_handle
   DB = 0;		/* database handle */

static isc_tr_handle
   gds_trans = 0;		/* default transaction handle */
static ISC_STATUS
   isc_status [20],	/* status vector */
   isc_status2 [20];	/* status vector */
static ISC_LONG
   isc_array_length, 	/* array return size */
   SQLCODE;		/* SQL status code */
static const short
   isc_0l = 133;
static const char
   isc_0 [] = {
   4,2,4,1,1,0,7,0,4,0,2,0,41,3,0,32,0,41,3,0,32,0,12,0,2,7,'C',
   1,'J',19,'R','D','B','$','R','E','L','A','T','I','O','N','_',
   'F','I','E','L','D','S',0,'G',58,47,23,0,17,'R','D','B','$',
   'R','E','L','A','T','I','O','N','_','N','A','M','E',25,0,1,0,
   47,23,0,14,'R','D','B','$','F','I','E','L','D','_','N','A','M',
   'E',25,0,0,0,-1,14,1,2,1,21,8,0,1,0,0,0,25,1,0,0,-1,14,1,1,21,
   8,0,0,0,0,0,25,1,0,0,-1,-1,'L'
   };	/* end of blr string for request isc_0 */

static const short
   isc_6l = 170;
static const char
   isc_6 [] = {
   4,2,4,1,2,0,9,0,7,0,4,0,1,0,8,0,12,0,2,7,'C',1,'J',16,'R','D',
   'B','$','T','R','A','N','S','A','C','T','I','O','N','S',0,'G',
   58,47,23,0,18,'R','D','B','$','T','R','A','N','S','A','C','T',
   'I','O','N','_','I','D',25,0,0,0,59,61,23,0,27,'R','D','B','$',
   'T','R','A','N','S','A','C','T','I','O','N','_','D','E','S',
   'C','R','I','P','T','I','O','N',-1,14,1,2,1,23,0,27,'R','D',
   'B','$','T','R','A','N','S','A','C','T','I','O','N','_','D',
   'E','S','C','R','I','P','T','I','O','N',25,1,0,0,1,21,8,0,1,
   0,0,0,25,1,1,0,-1,14,1,1,21,8,0,0,0,0,0,25,1,1,0,-1,-1,'L'
   };	/* end of blr string for request isc_6 */

static const short
   isc_12l = 131;
static const char
   isc_12 [] = {
   4,2,4,1,2,0,7,0,7,0,4,0,1,0,8,0,12,0,2,7,'C',1,'J',16,'R','D',
   'B','$','T','R','A','N','S','A','C','T','I','O','N','S',0,'G',
   47,23,0,18,'R','D','B','$','T','R','A','N','S','A','C','T','I',
   'O','N','_','I','D',25,0,0,0,-1,14,1,2,1,21,8,0,1,0,0,0,25,1,
   0,0,1,23,0,21,'R','D','B','$','T','R','A','N','S','A','C','T',
   'I','O','N','_','S','T','A','T','E',25,1,1,0,-1,14,1,1,21,8,
   0,0,0,0,0,25,1,0,0,-1,-1,'L'
   };	/* end of blr string for request isc_12 */

static const short
   isc_18l = 89;
static const char
   isc_18 [] = {
   4,2,4,2,1,0,7,0,4,1,1,0,7,0,4,0,1,0,7,0,2,7,'C',1,'J',13,'R',
   'D','B','$','L','O','G','_','F','I','L','E','S',0,-1,2,14,0,
   2,1,21,8,0,1,0,0,0,25,0,0,0,-1,17,0,9,13,12,2,18,0,12,1,5,0,
   -1,-1,14,0,1,21,8,0,0,0,0,0,25,0,0,0,-1,-1,'L'
   };	/* end of blr string for request isc_18 */


#define gds_blob_null	isc_blob_null	/* compatibility symbols */
#define gds_status	isc_status
#define gds_status2	isc_status2
#define gds_array_length	isc_array_length
#define gds_count	isc_count
#define gds_slack	isc_slack
#define gds_utility	isc_utility	/* end of compatibility symbols */

#ifndef isc_version4
    Generate a compile-time error.
    Picking up a V3 include file after preprocessing with V4 GPRE.
#endif

/**** end of GPRE definitions ****/


#define DB			tdgbl->db_handle
#define gds_trans	tdgbl->tr_handle

typedef Firebird::HalfStaticArray<TEXT, 1024> TextBuffer;

static alice_str* alloc_string(const TEXT**);
static USHORT get_capabilities(ISC_STATUS*);
static tdr* get_description(ISC_QUAD*);
static void parse_fullpath(tdr*);
static bool snarf_blob(ISC_QUAD*, TextBuffer&);


// table used to determine capabilities, checking for specific
// fields in system relations

struct rfr_tab_t
{
	const TEXT* relation;
	const TEXT* field;
	int bit_mask;
};

static const rfr_tab_t rfr_table[] =
{
	{ "RDB$TRANSACTIONS", "RDB$TRANSACTION_DESCRIPTION", CAP_transactions },
	{ 0, 0, 0 }
};

static inline void return_error(const ISC_STATUS* /*user_status*/)
{
	ALICE_print_status(true, gds_status);
	Firebird::LongJump::raise();
}

/*____________________________________________________________
 *
 *		Disable WAL for the database forcibly.
 *		Drop all the entries from RDB$LOG_FILES
 */

void MET_disable_wal(ISC_STATUS* user_status, isc_db_handle handle)
{
   struct isc_23_struct {
          short isc_24;	/* isc_utility */
   } isc_23;
   struct isc_21_struct {
          short isc_22;	/* isc_utility */
   } isc_21;
   struct isc_19_struct {
          short isc_20;	/* isc_utility */
   } isc_19;
	FB_API_HANDLE request = 0;
	AliceGlobals* tdgbl = AliceGlobals::getSpecific();

	if (!(DB = handle))
		return;

	/*START_TRANSACTION*/
	{
	{
	isc_start_transaction (isc_status, (FB_API_HANDLE*) &gds_trans, (short) 1, &DB, (short) 0, (char*) 0);
	};
	/*ON_ERROR*/
	if (isc_status [1])
	   {
		return_error(user_status);
	/*END_ERROR;*/
	   }
	}

	/*FOR(REQUEST_HANDLE request)
	X IN RDB$LOG_FILES*/
	{
        if (!request)
           isc_compile_request (NULL, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &request, (short) sizeof(isc_18), (char*) isc_18);
        isc_start_request (NULL, (FB_API_HANDLE*) &request, (FB_API_HANDLE*) &gds_trans, (short) 0);
	while (1)
	   {
           isc_receive (NULL, (FB_API_HANDLE*) &request, (short) 0, (short) 2, &isc_19, (short) 0);
	   if (!isc_19.isc_20) break;
		/*ERASE X;*/
                isc_send (NULL, (FB_API_HANDLE*) &request, (short) 1, (short) 2, &isc_21, (short) 0);
	/*END_FOR*/
           isc_send (NULL, (FB_API_HANDLE*) &request, (short) 2, (short) 2, &isc_23, (short) 0);
	   }
	}
	/*COMMIT*/
	{
	isc_commit_transaction (isc_status, (FB_API_HANDLE*) &gds_trans);;
	/*ON_ERROR*/
	if (isc_status [1])
	   {
		return_error(user_status);
	/*END_ERROR;*/
	   }
	}
}


/*____________________________________________________________
 *
 *		Get the state of a transaction,
 *		assuming that the database has
 *		already been attached.
 */

void MET_get_state(ISC_STATUS* user_status, tdr* trans)
{
   struct isc_15_struct {
          short isc_16;	/* isc_utility */
          short isc_17;	/* RDB$TRANSACTION_STATE */
   } isc_15;
   struct isc_13_struct {
          ISC_LONG isc_14;	/* RDB$TRANSACTION_ID */
   } isc_13;
	FB_API_HANDLE request = 0;
	AliceGlobals* tdgbl = AliceGlobals::getSpecific();

	if (!(DB = trans->tdr_db_handle) || !(trans->tdr_db_caps & CAP_transactions))
	{
		trans->tdr_state = TRA_unknown;
		return;
	}

	/*START_TRANSACTION*/
	{
	{
	isc_start_transaction (isc_status, (FB_API_HANDLE*) &gds_trans, (short) 1, &DB, (short) 0, (char*) 0);
	};
	/*ON_ERROR*/
	if (isc_status [1])
	   {
		return_error(user_status);
	/*END_ERROR;*/
	   }
	}

	/*FOR(REQUEST_HANDLE request)
	TRA IN RDB$TRANSACTIONS WITH
		TRA.RDB$TRANSACTION_ID = trans->tdr_id*/
	{
        if (!request)
           isc_compile_request (isc_status, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &request, (short) sizeof(isc_12), (char*) isc_12);
	isc_13.isc_14 = trans->tdr_id;
	if (request)
	   {
           isc_start_and_send (isc_status, (FB_API_HANDLE*) &request, (FB_API_HANDLE*) &gds_trans, (short) 0, (short) 4, &isc_13, (short) 0);
	   }
	if (!isc_status [1]) {
	while (1)
	   {
           isc_receive (isc_status, (FB_API_HANDLE*) &request, (short) 1, (short) 4, &isc_15, (short) 0);
	   if (!isc_15.isc_16 || isc_status [1]) break;
		trans->tdr_state = /*TRA.RDB$TRANSACTION_STATE*/
				   isc_15.isc_17;
	/*END_FOR*/
	   }
	   };
	/*ON_ERROR*/
	if (isc_status [1])
	   {
		return_error(user_status);
	/*END_ERROR;*/
	   }
	}

	isc_release_request(gds_status, &request);
	if (gds_status[1]) {
		return_error(user_status);
	}

	/*ROLLBACK*/
	{
	isc_rollback_transaction (isc_status, (FB_API_HANDLE*) &gds_trans);;
	/*ON_ERROR*/
	if (isc_status [1])
	   {
		return_error(user_status);
	/*END_ERROR;*/
	   }
	}
}


/*____________________________________________________________
 *
 *		Get the description of a transaction in
 *		limbo, including all associated transactions
 *		in other databases.
 */

tdr* MET_get_transaction(ISC_STATUS* user_status, isc_db_handle handle, SLONG id)
{
   struct isc_9_struct {
          ISC_QUAD isc_10;	/* RDB$TRANSACTION_DESCRIPTION */
          short isc_11;	/* isc_utility */
   } isc_9;
   struct isc_7_struct {
          ISC_LONG isc_8;	/* RDB$TRANSACTION_ID */
   } isc_7;
	FB_API_HANDLE request = 0;
	tdr* trans = NULL;
	AliceGlobals* tdgbl = AliceGlobals::getSpecific();

	if (!(DB = handle))
		return 0;

	/*START_TRANSACTION*/
	{
	{
	isc_start_transaction (isc_status, (FB_API_HANDLE*) &gds_trans, (short) 1, &DB, (short) 0, (char*) 0);
	};
	/*ON_ERROR*/
	if (isc_status [1])
	   {
		return_error(user_status);
	/*END_ERROR;*/
	   }
	}

	const USHORT capabilities = get_capabilities(user_status);

	if (capabilities & CAP_transactions)
	{
		/*FOR(REQUEST_HANDLE request)
		TRA IN RDB$TRANSACTIONS WITH
			TRA.RDB$TRANSACTION_ID = id AND
			TRA.RDB$TRANSACTION_DESCRIPTION NOT MISSING*/
		{
                if (!request)
                   isc_compile_request (isc_status, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &request, (short) sizeof(isc_6), (char*) isc_6);
		isc_7.isc_8 = id;
		if (request)
		   {
                   isc_start_and_send (isc_status, (FB_API_HANDLE*) &request, (FB_API_HANDLE*) &gds_trans, (short) 0, (short) 4, &isc_7, (short) 0);
		   }
		if (!isc_status [1]) {
		while (1)
		   {
                   isc_receive (isc_status, (FB_API_HANDLE*) &request, (short) 1, (short) 10, &isc_9, (short) 0);
		   if (!isc_9.isc_11 || isc_status [1]) break;
			trans = get_description(&/*TRA.RDB$TRANSACTION_DESCRIPTION*/
						 isc_9.isc_10);
		/*END_FOR*/
		   }
		   };
		/*ON_ERROR*/
		if (isc_status [1])
		   {
			return_error(user_status);
		/*END_ERROR;*/
		   }
		}

		isc_release_request(gds_status, &request);
		if (gds_status[1]) {
			return_error(user_status);
		}
	}

	/*ROLLBACK*/
	{
	isc_rollback_transaction (isc_status, (FB_API_HANDLE*) &gds_trans);;
	/*ON_ERROR*/
	if (isc_status [1])
	   {
		return_error(user_status);
	/*END_ERROR;*/
	   }
	}

	if (trans)
		trans->tdr_db_caps = capabilities;

	return trans;
}


/*____________________________________________________________
 *
 *		Get the capabilities associated with
 *		the database for a particular transaction.
 */

void MET_set_capabilities(ISC_STATUS* user_status, tdr* trans)
{
	AliceGlobals* tdgbl = AliceGlobals::getSpecific();

	if (!(DB = trans->tdr_db_handle))
		return;

	/*START_TRANSACTION*/
	{
	{
	isc_start_transaction (isc_status, (FB_API_HANDLE*) &gds_trans, (short) 1, &DB, (short) 0, (char*) 0);
	};
	/*ON_ERROR*/
	if (isc_status [1])
	   {
		return_error(user_status);
	/*END_ERROR;*/
	   }
	}

	trans->tdr_db_caps = get_capabilities(user_status);

	/*ROLLBACK*/
	{
	isc_rollback_transaction (isc_status, (FB_API_HANDLE*) &gds_trans);;
	/*ON_ERROR*/
	if (isc_status [1])
	   {
		return_error(user_status);
	/*END_ERROR;*/
	   }
	}
}


/*____________________________________________________________
 *
 *		Eat a string with a byte-encoded length.
 */

static alice_str* alloc_string(const TEXT** ptr)
{
	AliceGlobals* tdgbl = AliceGlobals::getSpecific();

	const TEXT* p = *ptr;

	const USHORT length = (USHORT) *p++;
	alice_str* string = FB_NEW_RPT(*tdgbl->getDefaultPool(), length + 1) alice_str;
	memcpy(string->str_data, p, length);
	string->str_data[length] = 0;
	*ptr = p + length;

	return string;
}



/*____________________________________________________________
 *
 *		Get the capabilities associated with
 *		the database for a particular transaction.
 */

static USHORT get_capabilities(ISC_STATUS* user_status)
{
   struct isc_4_struct {
          short isc_5;	/* isc_utility */
   } isc_4;
   struct isc_1_struct {
          char  isc_2 [32];	/* RDB$FIELD_NAME */
          char  isc_3 [32];	/* RDB$RELATION_NAME */
   } isc_1;
	USHORT capabilities = CAP_none;
	AliceGlobals* tdgbl = AliceGlobals::getSpecific();

	// Look for desired fields in system relations
	FB_API_HANDLE req = 0;

	for (const rfr_tab_t* rel_field_table = rfr_table; rel_field_table->relation; rel_field_table++)
	{
		/*FOR(REQUEST_HANDLE req) x IN DB.RDB$RELATION_FIELDS
			WITH x.RDB$RELATION_NAME = rel_field_table->relation
			AND x.RDB$FIELD_NAME = rel_field_table->field*/
		{
                if (!req)
                   isc_compile_request (isc_status, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &req, (short) sizeof(isc_0), (char*) isc_0);
		isc_vtov ((const char*) rel_field_table->field, (char*) isc_1.isc_2, 32);
		isc_vtov ((const char*) rel_field_table->relation, (char*) isc_1.isc_3, 32);
		if (req)
		   {
                   isc_start_and_send (isc_status, (FB_API_HANDLE*) &req, (FB_API_HANDLE*) &gds_trans, (short) 0, (short) 64, &isc_1, (short) 0);
		   }
		if (!isc_status [1]) {
		while (1)
		   {
                   isc_receive (isc_status, (FB_API_HANDLE*) &req, (short) 1, (short) 2, &isc_4, (short) 0);
		   if (!isc_4.isc_5 || isc_status [1]) break;
			capabilities |= rel_field_table->bit_mask;
		/*END_FOR*/
		   }
		   };
		/*ON_ERROR*/
		if (isc_status [1])
		   {
			return_error(user_status);
		/*END_ERROR;*/
		   }
		}
	}

	isc_release_request(gds_status, &req);
	if (gds_status[1]) {
		return_error(user_status);
	}

	return capabilities;
}


/*____________________________________________________________
 *
 *		Get the description of a transaction in
 *		limbo, including all associated transactions
 *		in other databases.
 */

static tdr* get_description(ISC_QUAD* blob_id)
{
	AliceGlobals* tdgbl = AliceGlobals::getSpecific();
	TextBuffer buffer;

	if (UserBlob::blobIsNull(*blob_id))
		return NULL;

	if (!snarf_blob(blob_id, buffer))
	{
		ALICE_print_status(true, gds_status);
		return NULL;
	}

	if (buffer.getCount() < 2)
		return NULL;

	tdr* trans = NULL;
	alice_str* host_site = NULL;
	alice_str* database_path = NULL;

	const TEXT* p = buffer.begin();

	// skip version number
	++p;

	tdr* ptr = NULL; // silence uninitialized warning
	SLONG id_length, id;

	while (*p)
	{
		switch (*p++)
		{
		case TDR_HOST_SITE:
			host_site = alloc_string(&p);
			break;

		case TDR_DATABASE_PATH:
			database_path = alloc_string(&p);
			break;

		case TDR_TRANSACTION_ID:
			id_length = *p++;
			id = gds__vax_integer(reinterpret_cast<const UCHAR*>(p), id_length);
			p += id_length;
			if (!trans) {
				trans = ptr = FB_NEW(*tdgbl->getDefaultPool()) tdr;
			}
			else
			{
				ptr->tdr_next = FB_NEW(*tdgbl->getDefaultPool()) tdr;
				ptr = ptr->tdr_next;
			}
			ptr->tdr_host_site = host_site;
			ptr->tdr_fullpath = database_path;
			parse_fullpath(ptr);
			ptr->tdr_id = id;
			database_path = NULL;
			break;

		default:
			ALICE_print(108);
			// msg 108: Transaction description item unknown.

			return NULL;
		}
	}

	return trans;
}


/*____________________________________________________________
 *
 *		Take apart a full remote path specification,
 *		finding the filename and the remote site.
 */

static void parse_fullpath(tdr* trans)
{
	AliceGlobals* tdgbl = AliceGlobals::getSpecific();

	// start at the end of the full pathname

 	const TEXT* p = (TEXT*) trans->tdr_fullpath->str_data;
	const TEXT* const start = p;
	while (*p)
		p++;
	const TEXT* const end = p;

	// Check for a named pipes name - \\node\path\db or //node/path/db
	while (p > start && !(*p == '/' && p[-1] == '/') && !(*p == '\\' && p[-1] == '\\'))
	{
		--p;
	}

	if (p > start)
	{
		// Increment p past slash, & search forward for end of node name
		p = p + 1;
		const TEXT* q = p;

		while (*q && *q != '/' && *q != '\\')
			q++;
		if (*q)
		{
			trans->tdr_filename = q + 1;

			trans->tdr_remote_site = FB_NEW_RPT(*tdgbl->getDefaultPool(), q - p + 1) alice_str;
			strncpy((char*) trans->tdr_remote_site->str_data, (char*) p, q - p);
			trans->tdr_remote_site->str_data[q - p] = '\0';
		}
	}
	else
	{
		p = end;

		// If not named pipes, check the other protocols
		// work backwards until we find a remote protocol specifier


		while (p >= start && (*p != '^' && *p != ':' && *p != '@'))
			p--;
		// dimitr:	make sure that the remote path is parsed correctly
		//			for win32 servers, i.e. the drive separator is taken into account
		if ((p - 2 >= start) && p[-2] == ':' && (p[0] == ':'))
			p -= 2;
		trans->tdr_filename = p + 1;

		// now find the last remote node in the chain

		while (p > start && (*p == ':' || *p == '^' || *p == '@'))
			p--;

		USHORT length = 0;
		for (; p >= start && (*p != '^' && *p != ':' && *p != '@'); ++length)
			--p;
		++p;

		if (length)
		{
			trans->tdr_remote_site = FB_NEW_RPT(*tdgbl->getDefaultPool(), length + 1) alice_str;
			TEXT* q = (TEXT *) trans->tdr_remote_site->str_data;
			while (length--)
				*q++ = *p++;
			*q = 0;
		}
	}
}


/*____________________________________________________________
 *
 *		Get a blob into a buffer, returning false if error happens
 */

static bool snarf_blob(ISC_QUAD* blob_id, TextBuffer& buffer)
{
	AliceGlobals* tdgbl = AliceGlobals::getSpecific();

	UserBlob blob(gds_status);
	if (!blob.open(DB, gds_trans, *blob_id))
		return false;

	SLONG blob_size;
	if (!getBlobSize(blob, &blob_size, NULL, NULL))
		return NULL;

	size_t len = blob_size;
	TEXT* ptr = buffer.getBuffer(blob_size + 1);
	while (len)
	{
		size_t real_len;
		if (len && blob.getSegment(len, ptr, real_len))
		{
			len -= real_len;
			ptr += real_len;
		}

		const ISC_STATUS err = blob.getCode();
		if (err == 0 || err == isc_segment)
			continue;

		if (err == isc_segstr_eof)
			break;

		return false;
	}
	blob.close();
	*ptr = 0;

	buffer.resize(ptr - buffer.begin() + 1);

	return true;
}

