/*********** Preprocessed module -- do not edit ***************/
/*********** Preprocessed module -- do not edit ***************/
/*********** Preprocessed module -- do not edit ***************/
/*********** Preprocessed module -- do not edit ***************/
/*********** Preprocessed module -- do not edit ***************/
/***************** gpre version WI-V2.5.7.27050 Firebird 2.5 **********************/
/*
 *	PROGRAM:	JRD Access Method
 *	MODULE:		stats.epp
 *	DESCRIPTION:	Record statistics manager
 *
 * The contents of this file are subject to the Interbase Public
 * License Version 1.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy
 * of the License at http://www.Inprise.com/IPL.html
 *
 * Software distributed under the License is distributed on an
 * "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express
 * or implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code was created by Inprise Corporation
 * and its predecessors. Portions created by Inprise Corporation are
 * Copyright (C) Inprise Corporation.
 *
 * All Rights Reserved.
 * Contributor(s): ______________________________________.
 */

#include "firebird.h"
#include "../jrd/ibase.h"

/*DATABASE DB = "yachts.lnk";*/
/**** GDS Preprocessor Definitions ****/
#ifndef JRD_IBASE_H
#include <ibase.h>
#endif

static const ISC_QUAD
   isc_blob_null = {0, 0};	/* initializer for blobs */
isc_db_handle
   DB = 0;		/* database handle */

isc_tr_handle
   gds_trans = 0;		/* default transaction handle */
ISC_STATUS
   isc_status [20],	/* status vector */
   isc_status2 [20];	/* status vector */
ISC_LONG
   isc_array_length, 	/* array return size */
   SQLCODE;		/* SQL status code */
static const short
   isc_0l = 124;
static const char
   isc_0 [] = {
   4,2,4,1,2,0,41,3,0,32,0,7,0,4,0,1,0,7,0,12,0,2,7,'C',1,'J',13,
   'R','D','B','$','R','E','L','A','T','I','O','N','S',0,'G',47,
   23,0,15,'R','D','B','$','R','E','L','A','T','I','O','N','_',
   'I','D',25,0,0,0,-1,14,1,2,1,23,0,17,'R','D','B','$','R','E',
   'L','A','T','I','O','N','_','N','A','M','E',25,1,0,0,1,21,8,
   0,1,0,0,0,25,1,1,0,-1,14,1,1,21,8,0,0,0,0,0,25,1,1,0,-1,-1,'L'
   
   };	/* end of blr string for request isc_0 */


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


extern void* gds__alloc();
extern SLONG gds__vax_integer();

const SSHORT ITEM_seq_reads	= 0;
const SSHORT ITEM_idx_reads	= 1;
const SSHORT ITEM_inserts	= 2;
const SSHORT ITEM_updates	= 3;
const SSHORT ITEM_deletes	= 4;
const SSHORT ITEM_backouts	= 5;
const SSHORT ITEM_purges	= 6;
const SSHORT ITEM_expunges	= 7;
const SSHORT ITEM_count		= 8;

struct fb_stats
{
	SSHORT stats_count;
	SSHORT stats_items;			// Number of item per relation
	SLONG stats_counts[1];
};

typedef int (print_callback)(SCHAR*, SSHORT, SSHORT, const SCHAR* const*, const SLONG*);


static fb_stats* expand_stats(fb_stats** ptr, SSHORT count);
static int get_counts(ISC_STATUS* status_vector, const SCHAR* info, SSHORT length,
		fb_stats** stats_ptr, SSHORT item);
static int print_line(SCHAR* arg, SSHORT relation_id, SSHORT count,
				const SCHAR* const* headers, const SLONG* counts);

static const SCHAR info_request[] =
{
	isc_info_read_seq_count,
	isc_info_read_idx_count,
	isc_info_insert_count,
	isc_info_update_count,
	isc_info_delete_count,
	isc_info_backout_count,
	isc_info_purge_count,
	isc_info_expunge_count,
	isc_info_end
};

static const SCHAR* headers[] =
{
	"S-Reads",
	"I-Reads",
	"Inserts",
	"Updates",
	"Deletes",
	"Backouts",
	"Purges",
	"Expunges"
};

static int* database_handle;
static int* request_handle;


void stats_analyze(const fb_stats* before, const fb_stats* after, print_callback callback, SCHAR* arg)
{
/**************************************
 *
 *	s t a t s _ a n a l y z e
 *
 **************************************
 *
 * Functional description
 *
 **************************************/
	SLONG delta[ITEM_count];

	if (!after)
		return;

	const SLONG* const end = delta + ITEM_count;

	const SLONG* tail2 = 0;
	if (before) {
		tail2 = before->stats_counts;
	}

	SSHORT relation_id = 0
	for (const SLONG* tail = after->stats_counts; relation_id < after->stats_count; ++relation_id)
	{
		SLONG total = 0;
		for (SLONG* p = delta; p < end;)
		{
			total += *tail;
			*p++ = *tail++;
		}
		if (before && relation_id < before->stats_count)
			for (SLONG* p = delta; p < end;)
			{
				total -= *tail2;
				*p++ -= *tail2++;
			}
		if (total) {
			(*callback) (arg, relation_id, ITEM_count, headers, delta);
		}
	}
}


int stats_fetch(SLONG *status_vector, int **db_handle, fb_stats** stats_ptr)
{
/**************************************
 *
 *	s t a t s _ f e t c h
 *
 **************************************
 *
 * Functional description
 *	Gather statistics.
 *
 **************************************/
	SCHAR info_buffer[4096];

	if (isc_database_info(status_vector, db_handle, sizeof(info_request), info_request,
							sizeof(info_buffer), info_buffer))
	{
		return status_vector[1];
	}

	fb_stats* stats = *stats_ptr;
	if (stats)
		zap_longs(stats->stats_counts, stats->stats_count * ITEM_count);
	else
		stats = expand_stats(stats_ptr, 64);

	if (!stats)
	{
		status_vector[0] = isc_arg_gds;
		status_vector[1] = isc_virmemexh;
		status_vector[2] = isc_arg_end;
		return status_vector[1];
	}

	const SCHAR* const end = info_buffer + sizeof(info_buffer);
	for (const SCHAR* p = info_buffer; p < end && *p != isc_info_end;)
	{
		const SSHORT length = gds__vax_integer(p + 1, 2);
		SSHORT item = -1;
		switch (*p)
		{
		case isc_info_read_seq_count:
			item = ITEM_seq_reads;
			break;
		case isc_info_read_idx_count:
			item = ITEM_idx_reads;
			break;
		case isc_info_insert_count:
			item = ITEM_inserts;
			break;
		case isc_info_update_count:
			item = ITEM_updates;
			break;
		case isc_info_delete_count:
			item = ITEM_deletes;
			break;
		case isc_info_backout_count:
			item = ITEM_backouts;
			break;
		case isc_info_purge_count:
			item = ITEM_purges;
			break;
		case isc_info_expunge_count:
			item = ITEM_expunges;
			break;
		}
		if (item >= 0)
		{
			if (get_counts(status_vector, p + 3, length, stats_ptr, item))
				break;
			p += 3 + length;
		}

	}
	// MOD I have added this just to compile - it matches the for loop
	// but will need to be checkout out before actually being used

    return status_vector[1];

}


int stats_print(int **db_handle, int **tr_handle, const fb_stats* before, const fb_stats* after)
{
/**************************************
 *
 *	s t a t s _ p r i n t
 *
 **************************************
 *
 * Functional description
 *
 **************************************/
	if (request_handle && database_handle != *db_handle)
		isc_release_request(gds__status, &request_handle);

	db = database_handle = *db_handle;
	gds__trans = *tr_handle;
	SLONG init = 0;
	stats_analyze(before, after, print_line, &init);
	return 0;
}


static fb_stats* expand_stats(fb_stats** ptr, SSHORT count)
{
/**************************************
 *
 *	e x p a n d _ s t a t s
 *
 **************************************
 *
 * Functional description
 *	Make sure vector is big enough.
 *
 **************************************/

	// If the thing is already big enough, don't do anything
	fb_stats* old = *ptr;
	if (old && old->stats_count < count)
		return old;

	count += 20;
	const SLONG length = sizeof(fb_stats) + (ITEM_count * count - 1) * sizeof(SLONG);
	fb_stats* stats = (fb_stats*) gds__alloc(length);
	// FREE: apparently never freed
	if (!stats)					// NOMEM: out of memory
		return NULL;			// leave *ptr unchanged
	zap_longs(stats->stats_counts, count * ITEM_count);
	stats->stats_count = count;
	stats->stats_items = ITEM_count;

	if (old)
	{
		SLONG* p = stats->stats_counts;
		const SLONG* q = old->stats_counts;
		const SLONG* const end = q + ITEM_count * old->stats_count;
		while (q < end) {
			*p++ = *q++;
		}
		gds__free(old);
	}

	return *ptr = stats;
}


static int get_counts(ISC_STATUS *status_vector, const SCHAR *info, SSHORT length,
		fb_stats** stats_ptr, SSHORT item)
{
/**************************************
 *
 *	g e t _ c o u n t s
 *
 **************************************
 *
 * Functional description
 *	Pick up operation counts from information items.
 *
 **************************************/
	fb_stats* stats = *stats_ptr;

	const SCHAR* p = info;
	for (const SCHAR* const end = p + length; p < end; p += 6)
	{
		const SSHORT relation_id = gds__vax_integer(p, 2);
		if (relation_id >= stats->stats_count)
		{
			if (!expand_stats(&stats, relation_id))
			{
				status_vector[0] = isc_arg_gds;
				status_vector[1] = isc_virmemexh;
				status_vector[2] = isc_arg_end;
				return status_vector[1];
			}
		}
		stats->stats_counts[relation_id * ITEM_count + item] = gds__vax_integer(p + 2, 4);
	}
	// CVC: Maybe this line is missing here? This function may relocate the pointer.
	// Otherwise the caller may be using deallocated memory in the next iteration.
	*stats_ptr = stats;
	return status_vector[1];
}


static int print_line(SCHAR* arg, SSHORT relation_id, SSHORT count,
				const SCHAR* const* headers, const SLONG* counts)
{
   struct isc_3_struct {
          char  isc_4 [32];	/* RDB$RELATION_NAME */
          short isc_5;	/* isc_utility */
   } isc_3;
   struct isc_1_struct {
          short isc_2;	/* RDB$RELATION_ID */
   } isc_1;
/**************************************
 *
 *	p r i n t _ l i n e
 *
 **************************************
 *
 * Functional description
 *	Display data.
 *
 **************************************/
	if (!*arg)
	{
		*arg = 1;
		printf("%32s ", " ");
		for (SSHORT n = count; n; --n)
			printf("%10s", *headers++);
		}
		printf("\n");
	}

	// This didn't work previously! GDML uses uppercased keywords.
	/*FOR (REQUEST_HANDLE request_handle)
		x IN RDB$RELATIONS
		WITH x.RDB$RELATION_ID EQ relation_id*/
	{
        if (!request_handle)
           isc_compile_request (NULL, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &request_handle, (short) sizeof(isc_0), (char*) isc_0);
	isc_1.isc_2 = relation_id;
        isc_start_and_send (NULL, (FB_API_HANDLE*) &request_handle, (FB_API_HANDLE*) &gds_trans, (short) 0, (short) 2, &isc_1, (short) 0);
	while (1)
	   {
           isc_receive (NULL, (FB_API_HANDLE*) &request_handle, (short) 1, (short) 34, &isc_3, (short) 0);
	   if (!isc_3.isc_5) break;
		printf("%32s", /*x.rdb$relation_name*/
			       isc_3.isc_4);
		for (const SLONG* const end = counts + count; counts < end; counts++) {
			printf("%10d", *counts);
		}
		printf("\n");
	/*END_FOR*/
	   }
	}

	return 0;
}


static int zap_longs(SLONG* ptr, SSHORT count)
{
/**************************************
 *
 *	z a p _ l o n g s
 *
 **************************************
 *
 * Functional description
 *	Zero a bunch of SLONGwords.
 *
 **************************************/

	if (count > 0)
	{
		do {
			*ptr++ = 0;
		} while (--count);
	}

	return 0;
}
