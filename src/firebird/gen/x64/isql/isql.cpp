/*********** Preprocessed module -- do not edit ***************/
/*********** Preprocessed module -- do not edit ***************/
/*********** Preprocessed module -- do not edit ***************/
/*********** Preprocessed module -- do not edit ***************/
/*********** Preprocessed module -- do not edit ***************/
/***************** gpre version WI-V2.5.7.27050 Firebird 2.5 **********************/
/*
 *	PROGRAM:	Interactive SQL utility
 *	MODULE:		isql.epp
 *	DESCRIPTION:	Main line routine
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
 *
  Revision 1.5  2000/11/18 16:49:24  fsg
  Increased PRINT_BUFFER_LENGTH to 2048 to show larger plans
  Fixed Bug #122563 in extract.e get_procedure_args
  Apparently this has to be done in show.e also,
  but that is for another day :-)

  2001/05/20  Neil McCalden  add planonly option
  2001.09.09  Claudio Valderrama: put double quotes around identifiers
	in dialect 3 only when needed. Solve mischievous declaration/invocation
	of ISQL_copy_SQL_id that made no sense and caused pointer problems.
  2001/10/03  Neil McCalden  pick up Firebird version from database server
	and display it with client version when -z used.
  2001.10.09 Claudio Valderrama: try to disconnect gracefully in batch mode.
  2001.11.23 Claudio Valderrama: skip any number of -- comments but only at
	the beginning and ignore void statements like block comments followed by
	a semicolon.
  2002-02-24 Sean Leyne - Code Cleanup of old Win 3.1 port (WINDOWS_ONLY)
  2003-08-15 Fred Polizo, Jr. - Fixed print_item() to correctly print
  string types as their hex represention for CHARACTER SET OCTETS.
  2004-11-16 Damyan Ivanov - bail out on error in non-interactive mode.

*/

#include "firebird.h"
#include <stdio.h>
#include "../dsql/keywords.h"
#include "../jrd/gds_proto.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>
#include "../common/utils_proto.h"
#include "../common/classes/array.h"
#include "../common/classes/init.h"
#include "../common/classes/ClumpletWriter.h"
#include "../common/classes/TempFile.h"
#include "../common/classes/FpeControl.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

//#ifdef HAVE_IO_H
//#include <io.h> // mktemp
//#endif

#ifdef HAVE_EDITLINE_H
// This is a local file included in our distribution - but not always
// compiled into the system
#include "editline.h"
#endif

enum literal_string_type
{
	INIT_STR_FLAG			= 0,
	SINGLE_QUOTED_STRING	= 1,
	DOUBLE_QUOTED_STRING	= 2,
	NO_MORE_STRING			= 3,
	INCOMPLETE_STRING		= 4
};

#include "../common/classes/timestamp.h"

#include "../jrd/common.h"
#if defined(WIN_NT)
#include <windows.h>
#endif
#include "../jrd/ibase.h"
#include "../isql/isql.h"
#include "../jrd/perf.h"
#include "../jrd/license.h"
#include "../jrd/constants.h"
#include "../jrd/ods.h"
#include "../jrd/file_params.h"
//#include "../common/stuff.h"
#include "../isql/extra_proto.h"
#include "../isql/isql_proto.h"
#include "../isql/show_proto.h"
#include "../jrd/perf_proto.h"
#include "../jrd/utl_proto.h"
#include "../jrd/why_proto.h"
#include "../jrd/gdsassert.h"

#ifdef SCROLLABLE_CURSORS
#include "../jrd/scroll_cursors.h"
#endif

#include "../isql/Extender.h"
#include "../isql/PtrSentry.h"
#include "../common/classes/UserBlob.h"
#include "../common/classes/MsgPrint.h"

using Firebird::TempFile;
using MsgFormat::SafeArg;

#include "../isql/ColList.h"
#include "../isql/InputDevices.h"
#include "../isql/OptionsBase.h"


/*DATABASE DB = COMPILETIME "yachts.lnk";*/
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
static isc_req_handle
   isc_0 = 0;		/* request handle */

static const short
   isc_1l = 256;
static const char
   isc_1 [] = {
   4,2,4,1,5,0,9,0,7,0,7,0,7,0,7,0,4,0,1,0,41,3,0,32,0,12,0,2,7,
   'C',2,'J',10,'R','D','B','$','F','I','E','L','D','S',0,'J',19,
   'R','D','B','$','R','E','L','A','T','I','O','N','_','F','I',
   'E','L','D','S',1,'G',58,47,23,0,14,'R','D','B','$','F','I',
   'E','L','D','_','N','A','M','E',23,1,16,'R','D','B','$','F',
   'I','E','L','D','_','S','O','U','R','C','E',47,23,1,17,'R','D',
   'B','$','R','E','L','A','T','I','O','N','_','N','A','M','E',
   25,0,0,0,'F',2,'H',23,1,18,'R','D','B','$','F','I','E','L','D',
   '_','P','O','S','I','T','I','O','N','H',23,1,14,'R','D','B',
   '$','F','I','E','L','D','_','N','A','M','E',-1,14,1,2,1,23,0,
   16,'R','D','B','$','C','O','M','P','U','T','E','D','_','B','L',
   'R',41,1,0,0,2,0,1,21,8,0,1,0,0,0,25,1,1,0,1,23,0,14,'R','D',
   'B','$','D','I','M','E','N','S','I','O','N','S',41,1,4,0,3,0,
   -1,14,1,1,21,8,0,0,0,0,0,25,1,1,0,-1,-1,'L'
   };	/* end of blr string for request isc_1 */

static isc_req_handle
   isc_10 = 0;		/* request handle */

static const short
   isc_11l = 145;
static const char
   isc_11 [] = {
   4,2,4,1,3,0,41,3,0,32,0,7,0,7,0,4,0,1,0,41,3,0,32,0,12,0,2,7,
   'C',1,'J',10,'R','D','B','$','F','I','E','L','D','S',0,'G',47,
   23,0,14,'R','D','B','$','F','I','E','L','D','_','N','A','M',
   'E',25,0,0,0,-1,14,1,2,1,23,0,14,'R','D','B','$','F','I','E',
   'L','D','_','N','A','M','E',25,1,0,0,1,21,8,0,1,0,0,0,25,1,1,
   0,1,23,0,15,'R','D','B','$','S','Y','S','T','E','M','_','F',
   'L','A','G',25,1,2,0,-1,14,1,1,21,8,0,0,0,0,0,25,1,1,0,-1,-1,
   'L'
   };	/* end of blr string for request isc_11 */

static isc_req_handle
   isc_18 = 0;		/* request handle */

static const short
   isc_19l = 116;
static const char
   isc_19 [] = {
   4,2,4,1,2,0,7,0,7,0,4,0,1,0,41,3,0,32,0,12,0,2,7,'C',1,'J',10,
   'R','D','B','$','F','I','E','L','D','S',0,'G',47,23,0,14,'R',
   'D','B','$','F','I','E','L','D','_','N','A','M','E',25,0,0,0,
   -1,14,1,2,1,21,8,0,1,0,0,0,25,1,0,0,1,23,0,13,'R','D','B','$',
   'N','U','L','L','_','F','L','A','G',25,1,1,0,-1,14,1,1,21,8,
   0,0,0,0,0,25,1,0,0,-1,-1,'L'
   };	/* end of blr string for request isc_19 */

static isc_req_handle
   isc_25 = 0;		/* request handle */

static const short
   isc_26l = 295;
static const char
   isc_26 [] = {
   4,2,4,1,8,0,41,3,0,32,0,7,0,7,0,7,0,7,0,7,0,7,0,7,0,4,0,2,0,
   41,3,0,32,0,41,3,0,32,0,12,0,2,7,'C',2,'J',10,'R','D','B','$',
   'F','I','E','L','D','S',0,'J',19,'R','D','B','$','R','E','L',
   'A','T','I','O','N','_','F','I','E','L','D','S',1,'G',58,47,
   23,1,16,'R','D','B','$','F','I','E','L','D','_','S','O','U',
   'R','C','E',23,0,14,'R','D','B','$','F','I','E','L','D','_',
   'N','A','M','E',58,47,23,1,17,'R','D','B','$','R','E','L','A',
   'T','I','O','N','_','N','A','M','E',25,0,1,0,47,23,1,14,'R',
   'D','B','$','F','I','E','L','D','_','N','A','M','E',25,0,0,0,
   -1,14,1,2,1,23,1,14,'R','D','B','$','B','A','S','E','_','F',
   'I','E','L','D',41,1,0,0,5,0,1,21,8,0,1,0,0,0,25,1,1,0,1,23,
   1,16,'R','D','B','$','V','I','E','W','_','C','O','N','T','E',
   'X','T',25,1,2,0,1,23,1,13,'R','D','B','$','N','U','L','L','_',
   'F','L','A','G',41,1,4,0,3,0,1,23,0,13,'R','D','B','$','N','U',
   'L','L','_','F','L','A','G',41,1,7,0,6,0,-1,14,1,1,21,8,0,0,
   0,0,0,25,1,1,0,-1,-1,'L'
   };	/* end of blr string for request isc_26 */

static isc_req_handle
   isc_39 = 0;		/* request handle */

static const short
   isc_40l = 294;
static const char
   isc_40 [] = {
   4,2,4,1,3,0,7,0,7,0,7,0,4,0,3,0,41,3,0,32,0,41,3,0,32,0,7,0,
   12,0,2,7,'C',2,'J',18,'R','D','B','$','V','I','E','W','_','R',
   'E','L','A','T','I','O','N','S',0,'J',24,'R','D','B','$','P',
   'R','O','C','E','D','U','R','E','_','P','A','R','A','M','E',
   'T','E','R','S',1,'D',21,8,0,1,0,0,0,'G',58,47,23,0,13,'R','D',
   'B','$','V','I','E','W','_','N','A','M','E',25,0,1,0,58,47,23,
   0,16,'R','D','B','$','V','I','E','W','_','C','O','N','T','E',
   'X','T',25,0,2,0,58,47,23,1,18,'R','D','B','$','P','R','O','C',
   'E','D','U','R','E','_','N','A','M','E',23,0,17,'R','D','B',
   '$','R','E','L','A','T','I','O','N','_','N','A','M','E',58,47,
   23,1,18,'R','D','B','$','P','A','R','A','M','E','T','E','R',
   '_','N','A','M','E',25,0,0,0,47,23,1,18,'R','D','B','$','P',
   'A','R','A','M','E','T','E','R','_','T','Y','P','E',21,8,0,1,
   0,0,0,-1,14,1,2,1,21,8,0,1,0,0,0,25,1,0,0,1,23,1,13,'R','D',
   'B','$','N','U','L','L','_','F','L','A','G',41,1,2,0,1,0,-1,
   14,1,1,21,8,0,0,0,0,0,25,1,0,0,-1,-1,'L'
   };	/* end of blr string for request isc_40 */

static isc_req_handle
   isc_49 = 0;		/* request handle */

static const short
   isc_50l = 341;
static const char
   isc_50 [] = {
   4,2,4,1,7,0,41,3,0,32,0,41,3,0,32,0,7,0,7,0,7,0,7,0,7,0,4,0,
   3,0,41,3,0,32,0,41,3,0,32,0,7,0,12,0,2,7,'C',2,'J',18,'R','D',
   'B','$','V','I','E','W','_','R','E','L','A','T','I','O','N',
   'S',0,'J',19,'R','D','B','$','R','E','L','A','T','I','O','N',
   '_','F','I','E','L','D','S',1,'D',21,8,0,1,0,0,0,'G',58,47,23,
   0,13,'R','D','B','$','V','I','E','W','_','N','A','M','E',25,
   0,1,0,58,47,23,0,16,'R','D','B','$','V','I','E','W','_','C',
   'O','N','T','E','X','T',25,0,2,0,58,47,23,1,17,'R','D','B','$',
   'R','E','L','A','T','I','O','N','_','N','A','M','E',23,0,17,
   'R','D','B','$','R','E','L','A','T','I','O','N','_','N','A',
   'M','E',47,23,1,14,'R','D','B','$','F','I','E','L','D','_','N',
   'A','M','E',25,0,0,0,-1,14,1,2,1,23,1,17,'R','D','B','$','R',
   'E','L','A','T','I','O','N','_','N','A','M','E',25,1,0,0,1,23,
   1,14,'R','D','B','$','B','A','S','E','_','F','I','E','L','D',
   41,1,1,0,6,0,1,21,8,0,1,0,0,0,25,1,2,0,1,23,1,16,'R','D','B',
   '$','V','I','E','W','_','C','O','N','T','E','X','T',25,1,3,0,
   1,23,1,13,'R','D','B','$','N','U','L','L','_','F','L','A','G',
   41,1,5,0,4,0,-1,14,1,1,21,8,0,0,0,0,0,25,1,2,0,-1,-1,'L'
   };	/* end of blr string for request isc_50 */

static isc_req_handle
   isc_63 = 0;		/* request handle */

static const short
   isc_64l = 152;
static const char
   isc_64 [] = {
   4,2,4,1,2,0,41,3,0,32,0,7,0,4,0,1,0,41,3,0,32,0,12,0,2,7,'C',
   1,'J',18,'R','D','B','$','I','N','D','E','X','_','S','E','G',
   'M','E','N','T','S',0,'G',47,23,0,14,'R','D','B','$','I','N',
   'D','E','X','_','N','A','M','E',25,0,0,0,'F',1,'H',23,0,18,'R',
   'D','B','$','F','I','E','L','D','_','P','O','S','I','T','I',
   'O','N',-1,14,1,2,1,23,0,14,'R','D','B','$','F','I','E','L',
   'D','_','N','A','M','E',25,1,0,0,1,21,8,0,1,0,0,0,25,1,1,0,-1,
   14,1,1,21,8,0,0,0,0,0,25,1,1,0,-1,-1,'L'
   };	/* end of blr string for request isc_64 */

static isc_req_handle
   isc_70 = 0;		/* request handle */

static const short
   isc_71l = 125;
static const char
   isc_71 [] = {
   4,2,4,1,3,0,9,0,7,0,7,0,4,0,1,0,41,3,0,32,0,12,0,2,7,'C',1,'J',
   10,'R','D','B','$','F','I','E','L','D','S',0,'G',47,23,0,14,
   'R','D','B','$','F','I','E','L','D','_','N','A','M','E',25,0,
   0,0,-1,14,1,2,1,23,0,18,'R','D','B','$','D','E','F','A','U',
   'L','T','_','S','O','U','R','C','E',41,1,0,0,2,0,1,21,8,0,1,
   0,0,0,25,1,1,0,-1,14,1,1,21,8,0,0,0,0,0,25,1,1,0,-1,-1,'L'
   };	/* end of blr string for request isc_71 */

static isc_req_handle
   isc_78 = 0;		/* request handle */

static const short
   isc_79l = 248;
static const char
   isc_79 [] = {
   4,2,4,1,5,0,9,0,9,0,7,0,7,0,7,0,4,0,2,0,41,3,0,32,0,41,3,0,32,
   0,12,0,2,7,'C',2,'J',10,'R','D','B','$','F','I','E','L','D',
   'S',0,'J',19,'R','D','B','$','R','E','L','A','T','I','O','N',
   '_','F','I','E','L','D','S',1,'G',58,47,23,1,16,'R','D','B',
   '$','F','I','E','L','D','_','S','O','U','R','C','E',23,0,14,
   'R','D','B','$','F','I','E','L','D','_','N','A','M','E',58,47,
   23,1,17,'R','D','B','$','R','E','L','A','T','I','O','N','_',
   'N','A','M','E',25,0,1,0,47,23,0,14,'R','D','B','$','F','I',
   'E','L','D','_','N','A','M','E',25,0,0,0,-1,14,1,2,1,23,0,18,
   'R','D','B','$','D','E','F','A','U','L','T','_','S','O','U',
   'R','C','E',41,1,0,0,3,0,1,23,1,18,'R','D','B','$','D','E','F',
   'A','U','L','T','_','S','O','U','R','C','E',41,1,1,0,4,0,1,21,
   8,0,1,0,0,0,25,1,2,0,-1,14,1,1,21,8,0,0,0,0,0,25,1,2,0,-1,-1,
   'L'
   };	/* end of blr string for request isc_79 */

static isc_req_handle
   isc_89 = 0;		/* request handle */

static const short
   isc_90l = 175;
static const char
   isc_90 [] = {
   4,2,4,1,2,0,41,3,0,32,0,7,0,4,0,1,0,7,0,12,0,2,7,'C',1,'J',18,
   'R','D','B','$','C','H','A','R','A','C','T','E','R','_','S',
   'E','T','S',0,'D',21,8,0,1,0,0,0,'G',47,23,0,20,'R','D','B',
   '$','C','H','A','R','A','C','T','E','R','_','S','E','T','_',
   'I','D',25,0,0,0,'F',1,'H',23,0,22,'R','D','B','$','C','H','A',
   'R','A','C','T','E','R','_','S','E','T','_','N','A','M','E',
   -1,14,1,2,1,23,0,22,'R','D','B','$','C','H','A','R','A','C',
   'T','E','R','_','S','E','T','_','N','A','M','E',25,1,0,0,1,21,
   8,0,1,0,0,0,25,1,1,0,-1,14,1,1,21,8,0,0,0,0,0,25,1,1,0,-1,-1,
   'L'
   };	/* end of blr string for request isc_90 */

static isc_req_handle
   isc_96 = 0;		/* request handle */

static const short
   isc_97l = 357;
static const char
   isc_97 [] = {
   4,2,4,1,4,0,41,3,0,32,0,41,3,0,32,0,41,3,0,32,0,7,0,4,0,2,0,
   7,0,7,0,12,0,2,7,'C',2,'J',14,'R','D','B','$','C','O','L','L',
   'A','T','I','O','N','S',0,'J',18,'R','D','B','$','C','H','A',
   'R','A','C','T','E','R','_','S','E','T','S',1,'D',21,8,0,1,0,
   0,0,'G',58,47,23,0,20,'R','D','B','$','C','H','A','R','A','C',
   'T','E','R','_','S','E','T','_','I','D',23,1,20,'R','D','B',
   '$','C','H','A','R','A','C','T','E','R','_','S','E','T','_',
   'I','D',58,47,23,0,16,'R','D','B','$','C','O','L','L','A','T',
   'I','O','N','_','I','D',25,0,1,0,47,23,1,20,'R','D','B','$',
   'C','H','A','R','A','C','T','E','R','_','S','E','T','_','I',
   'D',25,0,0,0,'F',2,'H',23,0,18,'R','D','B','$','C','O','L','L',
   'A','T','I','O','N','_','N','A','M','E','H',23,1,22,'R','D',
   'B','$','C','H','A','R','A','C','T','E','R','_','S','E','T',
   '_','N','A','M','E',-1,14,1,2,1,23,1,24,'R','D','B','$','D',
   'E','F','A','U','L','T','_','C','O','L','L','A','T','E','_',
   'N','A','M','E',25,1,0,0,1,23,0,18,'R','D','B','$','C','O','L',
   'L','A','T','I','O','N','_','N','A','M','E',25,1,1,0,1,23,1,
   22,'R','D','B','$','C','H','A','R','A','C','T','E','R','_','S',
   'E','T','_','N','A','M','E',25,1,2,0,1,21,8,0,1,0,0,0,25,1,3,
   0,-1,14,1,1,21,8,0,0,0,0,0,25,1,3,0,-1,-1,'L'
   };	/* end of blr string for request isc_97 */

static isc_req_handle
   isc_106 = 0;		/* request handle */

static const short
   isc_107l = 153;
static const char
   isc_107 [] = {
   4,2,4,1,4,0,7,0,7,0,7,0,7,0,4,0,1,0,41,3,0,32,0,12,0,2,7,'C',
   1,'J',10,'R','D','B','$','F','I','E','L','D','S',0,'G',47,23,
   0,14,'R','D','B','$','F','I','E','L','D','_','N','A','M','E',
   25,0,0,0,-1,14,1,2,1,21,8,0,1,0,0,0,25,1,0,0,1,23,0,16,'R','D',
   'B','$','F','I','E','L','D','_','L','E','N','G','T','H',25,1,
   1,0,1,23,0,20,'R','D','B','$','C','H','A','R','A','C','T','E',
   'R','_','L','E','N','G','T','H',41,1,3,0,2,0,-1,14,1,1,21,8,
   0,0,0,0,0,25,1,0,0,-1,-1,'L'
   };	/* end of blr string for request isc_107 */

static isc_req_handle
   isc_115 = 0;		/* request handle */

static const short
   isc_116l = 147;
static const char
   isc_116 [] = {
   4,2,4,1,2,0,7,0,7,0,4,0,1,0,41,3,0,32,0,12,0,2,7,'C',1,'J',18,
   'R','D','B','$','C','H','A','R','A','C','T','E','R','_','S',
   'E','T','S',0,'D',21,8,0,1,0,0,0,'G',47,23,0,22,'R','D','B',
   '$','C','H','A','R','A','C','T','E','R','_','S','E','T','_',
   'N','A','M','E',25,0,0,0,-1,14,1,2,1,21,8,0,1,0,0,0,25,1,0,0,
   1,23,0,20,'R','D','B','$','C','H','A','R','A','C','T','E','R',
   '_','S','E','T','_','I','D',25,1,1,0,-1,14,1,1,21,8,0,0,0,0,
   0,25,1,0,0,-1,-1,'L'
   };	/* end of blr string for request isc_116 */

static isc_req_handle
   isc_122 = 0;		/* request handle */

static const short
   isc_123l = 132;
static const char
   isc_123 [] = {
   4,2,4,0,2,0,41,3,0,32,0,7,0,2,7,'C',1,'J',12,'R','D','B','$',
   'D','A','T','A','B','A','S','E',0,'D',21,8,0,1,0,0,0,'G',59,
   61,23,0,22,'R','D','B','$','C','H','A','R','A','C','T','E','R',
   '_','S','E','T','_','N','A','M','E',-1,14,0,2,1,23,0,22,'R',
   'D','B','$','C','H','A','R','A','C','T','E','R','_','S','E',
   'T','_','N','A','M','E',25,0,0,0,1,21,8,0,1,0,0,0,25,0,1,0,-1,
   14,0,1,21,8,0,0,0,0,0,25,0,1,0,-1,-1,'L'
   };	/* end of blr string for request isc_123 */

static isc_req_handle
   isc_127 = 0;		/* request handle */

static const short
   isc_128l = 195;
static const char
   isc_128 [] = {
   4,2,4,1,4,0,8,0,8,0,7,0,7,0,4,0,1,0,41,3,0,32,0,12,0,2,7,'C',
   1,'J',20,'R','D','B','$','F','I','E','L','D','_','D','I','M',
   'E','N','S','I','O','N','S',0,'G',47,23,0,14,'R','D','B','$',
   'F','I','E','L','D','_','N','A','M','E',25,0,0,0,'F',1,'H',23,
   0,13,'R','D','B','$','D','I','M','E','N','S','I','O','N',-1,
   14,1,2,1,23,0,15,'R','D','B','$','U','P','P','E','R','_','B',
   'O','U','N','D',25,1,0,0,1,23,0,15,'R','D','B','$','L','O','W',
   'E','R','_','B','O','U','N','D',25,1,1,0,1,21,8,0,1,0,0,0,25,
   1,2,0,1,23,0,13,'R','D','B','$','D','I','M','E','N','S','I',
   'O','N',25,1,3,0,-1,14,1,1,21,8,0,0,0,0,0,25,1,2,0,-1,-1,'L'
   
   };	/* end of blr string for request isc_128 */


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


IsqlGlobals isqlGlob;

#define DIGIT(c)		((c) >= '0' && (c) <= '9')
#define INT64_LIMIT	 ((((SINT64) 1) << 62) / 5)	// same as in cvt.cpp

//  Print lengths of numeric values

const size_t MAXCHARSET_SIZE	= 32;	// CHARSET names
const int SHORT_LEN		= 7;	// NUMERIC (4,2) = -327.68
const int LONG_LEN		= 12;	// NUMERIC (9,2) = -21474836.48
const int INT64_LEN		= 21;	// NUMERIC(18,2) = -92233720368547758.08
const int QUAD_LEN		= 19;
const int FLOAT_LEN		= 14;	// -1.2345678E+38
const int DOUBLE_LEN		= 23;	// -1.234567890123456E+300
const int DATE_LEN		= 11;	// 11 for date only
const int DATETIME_LEN		= 25;	// 25 for date-time
const int TIME_ONLY_LEN		= 13;	// 13 for time only
const int DATE_ONLY_LEN		= 11;
const int UNKNOWN_LEN		= 20;	// Unknown type: %d

const int MAX_TERMS		= 10;	// max # of terms in an interactive cmd


enum switch_argtype {
	SWARG_NONE,
	SWARG_STRING,
	SWARG_INTEGER
};

enum switch_id {
	SWITCH_EXTRACTALL,
	SWITCH_BAIL,
	SWITCH_CACHE,
	SWITCH_CHARSET,
	SWITCH_DATABASE,
	SWITCH_ECHO,
	SWITCH_EXTRACT,
	SWITCH_FETCHPASS,
	SWITCH_INPUT,
	SWITCH_MERGE,
	SWITCH_MERGE2,
	SWITCH_NOAUTOCOMMIT,
	SWITCH_NODBTRIGGERS,
	SWITCH_NOWARN,
	SWITCH_OUTPUT,
	SWITCH_PAGE,
	SWITCH_PASSWORD,
	SWITCH_QUIET,
	SWITCH_ROLE,
	SWITCH_ROLE2,
	SWITCH_SQLDIALECT,
	SWITCH_TERM,
#ifdef TRUSTED_AUTH
	SWITCH_TRUSTED,
#endif
	SWITCH_USER,
	SWITCH_VERSION,
#ifdef DEV_BUILD
	SWITCH_EXTRACTTBL,
#endif
	// this id has to be the last
	SWITCH_UNKNOWN
};

struct switch_info
{
	switch_id id;				// switch id
	const char* text;			// canonical switch name
	size_t abbrlen;				// minimum abbreviation length
	switch_argtype argtype;		// argument type (SWARG_NONE if none)
	int usage_id;				// message id for usage text
	//bool allow_dup;				// switch can be used more then once
};

const switch_info switches[] =
{
#ifdef DEV_BUILD
	{ SWITCH_EXTRACTTBL,   "xt",             2, SWARG_NONE,     -1 },
#endif
	{ SWITCH_EXTRACTALL,   "all",            1, SWARG_NONE,     11 },
	{ SWITCH_BAIL,         "bail",           1, SWARG_NONE,    104 },
	{ SWITCH_CACHE,        "cache",          1, SWARG_INTEGER, 111 },
	{ SWITCH_CHARSET,      "charset",        2, SWARG_STRING,  122 },
	{ SWITCH_DATABASE,     "database",       1, SWARG_STRING,  123 },
	{ SWITCH_FETCHPASS,	   "fetch_password", 1, SWARG_STRING,  161 },
	{ SWITCH_ECHO,         "echo",           1, SWARG_NONE,    124 },
	{ SWITCH_EXTRACT,      "extract",        2, SWARG_NONE,    125 },
	{ SWITCH_INPUT,        "input",          1, SWARG_STRING,  126 },
	{ SWITCH_MERGE,        "merge",          1, SWARG_NONE,    127 },
	{ SWITCH_MERGE2,       "m2",             2, SWARG_NONE,    128 },
	{ SWITCH_NOAUTOCOMMIT, "noautocommit",   1, SWARG_NONE,    129 },
	{ SWITCH_NODBTRIGGERS, "nodbtriggers",   3, SWARG_NONE,    154 },
	{ SWITCH_NOWARN,       "nowarnings",     3, SWARG_NONE,    130 },
	{ SWITCH_OUTPUT,       "output",         1, SWARG_STRING,  131 },
	{ SWITCH_PAGE,         "pagelength",     3, SWARG_INTEGER, 132 },
	{ SWITCH_PASSWORD,     "password",       1, SWARG_STRING,  133 },
	{ SWITCH_QUIET,        "quiet",          1, SWARG_NONE,    134 },
	{ SWITCH_ROLE,         "role",           1, SWARG_STRING,  135 },
	{ SWITCH_ROLE2,        "r2",             2, SWARG_STRING,  136 },
	{ SWITCH_SQLDIALECT,   "sqldialect",     1, SWARG_INTEGER, 137 },
	{ SWITCH_SQLDIALECT,   "sql_dialect",    1, SWARG_INTEGER,  -1 },
	{ SWITCH_TERM,         "terminator",     1, SWARG_STRING,  138 },
#ifdef TRUSTED_AUTH
	{ SWITCH_TRUSTED,      "trusted",        2, SWARG_NONE,    155 },
#endif
	{ SWITCH_USER,         "user",           1, SWARG_STRING,  139 },
	{ SWITCH_EXTRACT,      "x",              1, SWARG_NONE,    140 },
	{ SWITCH_VERSION,      "z",              1, SWARG_NONE,    141 }
};


static inline bool commit_trans(isc_tr_handle* x)
{
	if (isc_commit_transaction (isc_status, x)) {
		ISQL_errmsg (isc_status);
		isc_rollback_transaction (isc_status, x);
		return false;
	}
	return true;
}

static inline int fb_isspace(const char c)
{
	return isspace((int)(UCHAR)c);
}

static inline int fb_isspace(const SSHORT c)
{
	return isspace((int)(UCHAR)c);
}

static inline int fb_isdigit(const char c)
{
	return isdigit((int)(UCHAR)c);
}


// I s q l G l o b a l s : : p r i n t f
// Output to the Out stream.
void IsqlGlobals::printf(const char* buffer, ...)
{
	va_list args;
	va_start(args, buffer);
	vfprintf(Out, buffer, args);
	va_end(args);
	fflush(Out); // John's fix.
}

// I s q l G l o b a l s : : p r i n t s
// Output to the Out stream a literal string. No escape characters recognized.
void IsqlGlobals::prints(const char* buffer)
{
	fprintf(Out, "%s", buffer);
	fflush(Out); // John's fix.
}



struct ri_actions
{
	const SCHAR* ri_action_name;
	const SCHAR* ri_action_print_caps;
	const SCHAR* ri_action_print_mixed;
};

static processing_state add_row(TEXT*);
static processing_state blobedit(const TEXT*, const TEXT* const*);
static processing_state bulk_insert_hack(const char* command, XSQLDA** sqldap);
static bool bulk_insert_retriever(const char* prompt);
static bool check_date(const tm& times);
static bool check_time(const tm& times);
static bool check_timestamp(const tm& times, const int msec);
static size_t chop_at(char target[], const size_t size);
static void col_check(const TEXT*, SSHORT*);
static void copy_str(TEXT**, const TEXT**, bool*, const TEXT* const,
	const TEXT* const, literal_string_type);
static processing_state copy_table(TEXT*, TEXT*, TEXT*);
static processing_state create_db(const TEXT*, TEXT*);
static void do_isql();
static processing_state drop_db();
static processing_state edit(const TEXT* const*);
static processing_state end_trans();
static processing_state escape(const TEXT*);
static processing_state frontend(const TEXT*);
static processing_state frontend_set(const char* cmd, const char* const* parms,
	const char* const* lparms, char* const bad_dialect_buf, bool& bad_dialect);
static void frontend_free_parms(TEXT*[], TEXT*[], TEXT parm_defaults[][1]);
static void frontend_load_parms(const TEXT* p, TEXT* parms[], TEXT* lparms[],
	TEXT parm_defaults[][1]);
static processing_state do_set_command(const TEXT*, bool*);
static processing_state get_dialect(const char* const dialect_str,
	char* const bad_dialect_buf, bool& bad_dialect);
static processing_state get_statement(TEXT* const, const size_t, const TEXT*);
static bool get_numeric(const UCHAR*, USHORT, SSHORT*, SINT64*);
static void get_str(const TEXT* const, const TEXT**, const TEXT**,
	literal_string_type*);
static void print_set(const char* str, bool v);
static processing_state print_sets();
static processing_state help(const TEXT*);
static bool isyesno(const TEXT*);
static processing_state newdb(TEXT*, const TEXT*, const TEXT*, int, const TEXT*, bool);
static processing_state newinput(const TEXT*);
static processing_state newoutput(const TEXT*);
static processing_state newsize(const TEXT*, const TEXT*);
static processing_state newRowCount(const TEXT* newRowCountStr);
static processing_state newtrans(const TEXT*);
static processing_state parse_arg(int, SCHAR**, SCHAR*); //, FILE**);
#ifdef DEV_BUILD
static processing_state passthrough(const char* cmd);
#endif
static SSHORT print_item(TEXT**, XSQLVAR*, const int);
static void print_item_numeric(SINT64, int, int, TEXT*);
static processing_state print_line(XSQLDA*, const int pad[], TEXT line[]);
static void print_performance(const perf64* perf_before);
static processing_state print_sqlda_input(const bool is_selectable);
static void print_sqlda_output(const XSQLDA& sqlda);
static void process_header(const XSQLDA* sqlda, const int pad[], TEXT header[], TEXT header2[]);
static void process_plan();
static SINT64 process_record_count(const int statement_type);
static SSHORT process_request_type();
static SLONG* process_sqlda_buffer(XSQLDA* const sqlda, SSHORT nullind[]);
static SLONG process_sqlda_display(XSQLDA* const sqlda, SLONG buffer[], int pad[]);
static int process_statement(const TEXT*, XSQLDA**);
#ifdef WIN_NT
static BOOL CALLBACK query_abort(DWORD);
#else
static int query_abort(const int, const int, void*);
#endif
static bool stdin_redirected();
static void strip_quotes(const TEXT*, TEXT*);
//#ifdef DEV_BUILD
static const char* sqltype_to_string(USHORT);
//#endif

XSQLDA* global_sqlda;
XSQLDA** global_sqldap;
// The dialect spoken by the database, should be 0 when no database is connected.
USHORT global_dialect_spoken = 0;
USHORT requested_SQL_dialect = SQL_DIALECT_V6;
//bool connecting_to_pre_v6_server = false; Not used now.
bool Quiet = false;
#ifdef TRUSTED_AUTH
bool Trusted_auth = false;
#endif
bool Version_info = false;

// Utility transaction handle
static isc_tr_handle D__trans;
static isc_tr_handle M__trans;
static int global_numbufs;	// # of cache buffers on connect
static isc_stmt_handle global_Stmt;
//static FILE* Ofp;
//static FILE* Ifp;
static SCHAR Password[128];
static SCHAR Charset[128];
static bool Merge_stderr;

static ColList global_Cols;
static int global_Col_default = 0; // Need to write code for it in the future.
static Firebird::GlobalPtr<InputDevices> Filelist;
//static TEXT Tmpfile[MAXPATHLEN];
static bool Abort_flag = false;
static bool Interrupt_flag = false;
static bool Echo = false;
static bool Time_display = false;
static bool Sqlda_display = false;
static int Exit_value;
static bool Interactive = true;
static bool Input_file = false;
static int Pagelength = 20;
static bool Stats = false;
static bool Autocommit = true; // Commit ddl
static bool Nodbtriggers = false; // No database triggers
static bool Warnings = true; // Print warnings
#ifdef SCROLLABLE_CURSORS
static bool Autofetch = true; // automatically fetch all records
static USHORT fetch_direction = isc_fetch_next;
static SLONG fetch_offset = 1;
#endif
static int Doblob = 1;		// Default to printing only text types
static bool List = false;
static bool Docount = false;
static size_t rowCount = 0;
static bool Plan = false;
static bool Planonly = false;
static bool Heading = true;
static bool BailOnError = false;
static int Termlen = 0;
static SCHAR ISQL_charset[MAXCHARSET_SIZE] = { 0 };
static bool Merge_diagnostic = false;
static FILE* Diag;
static FILE* Help;
static const TEXT* const sql_prompt = "SQL> ";
#ifdef SCROLLABLE_CURSORS
static const TEXT* const fetch_prompt = "FETCH> ";
#endif

static bool global_psw = false;
static bool global_usr = false;
static bool global_role = false;
static bool has_global_numbufs = false;
static bool have_trans = false; // translation of word "Yes"
static TEXT yesword[BUFFER_LENGTH128];

// Didn't replace it by FB_SHORT_MONTHS because these are uppercased.
static const SCHAR* alpha_months[] =
{
	"JAN",
	"FEB",
	"MAR",
	"APR",
	"MAY",
	"JUN",
	"JUL",
	"AUG",
	"SEP",
	"OCT",
	"NOV",
	"DEC"
};

#ifdef NOT_USED_OR_REPLACED
// There's a local version of this, more complete, in ISQL_get_version()
static const UCHAR db_version_info[] =
{
	isc_info_ods_version,
	isc_info_ods_minor_version,
	isc_info_db_sql_dialect
};
#endif
#ifdef NOT_USED_OR_REPLACED
static const UCHAR text_bpb[] =
{
	isc_bpb_version1,
	isc_bpb_source_type, 1, isc_blob_text,
	isc_bpb_target_type, 1, isc_blob_text
};
#endif
static UCHAR predefined_blob_subtype_bpb[] =
{
	isc_bpb_version1,
	isc_bpb_source_type, 1, 0,
	isc_bpb_target_type, 1, isc_blob_text
};

// No check on input argument for now.
inline void set_bpb_for_translation(const unsigned int blob_sub_type)
{
	predefined_blob_subtype_bpb[3] = (UCHAR) blob_sub_type;
}

// Note that these transaction options aren't understood in Version 3.3
static const UCHAR default_tpb[] =
{
	isc_tpb_version1, isc_tpb_write,
	isc_tpb_read_committed, isc_tpb_wait,
	isc_tpb_no_rec_version
};

#ifdef NOT_USED_OR_REPLACED
// CVC: Just in case we need it for R/O operations in the future.
static const UCHAR	batch_tpb[] =
{
	isc_tpb_version3, isc_tpb_read,
	isc_tpb_read_committed, isc_tpb_nowait,
	isc_tpb_rec_version
};
#endif

// If the action is restrict, do not print anything at all
static const ri_actions ri_actions_all[] =
{
	{RI_ACTION_CASCADE, RI_ACTION_CASCADE, "Cascade"},
	{RI_ACTION_NULL, RI_ACTION_NULL, "Set Null"},
	{RI_ACTION_DEFAULT, RI_ACTION_DEFAULT, "Set Default"},
	{RI_ACTION_NONE, RI_ACTION_NONE, "No Action"},
	{RI_RESTRICT, "", ""},
	{"", "", ""},
	{0, 0, 0}
};


static void atexit_fb_shutdown()
{
	fb_shutdown(0, fb_shutrsn_app_stopped);
}

int CLIB_ROUTINE main(int argc, char* argv[])
{
/**************************************
 *
 *	m a i n
 *
 **************************************
 *
 * Functional description
 *	This calls ISQL_main, and exists to
 *	isolate main which does not exist under
 *	MS Windows.
 *
 **************************************/

	// Initialize globals
	isqlGlob.major_ods = 0;
	isqlGlob.minor_ods = 0;

	int rc = ISQL_main(argc, argv);
	return rc;
}


int ISQL_main(int argc, char* argv[])
{
/**************************************
 *
 *	I S Q L _ m a i n
 *
 **************************************
 *
 * Functional description
 *	Choose between reading and executing or generating SQL
 *	ISQL_main isolates this from main for PC Clients.
 *
 **************************************/
#ifdef MU_ISQL
	// This is code for QA Test bed Multiuser environment.
	// Setup multi-user test environment.
	if (qa_mu_environment())
	{
		// Initialize multi-user test manager only if in that
		// environment.
		if (!qa_mu_init(argc, argv, 1))
		{
			// Some problem in initializing multi-user test
			// environment.
			qa_mu_cleanup();
			exit(1);
		}
		else
		{
			// When invoked by MU, additional command-line argument
			// was added at tail. ISQL is not interested in it so
			// remove it.
			argv[argc - 1] = NULL;
			argc--;
		}
	}
#endif	// MU_ISQL

	setlocale(LC_CTYPE, "");
	atexit(&atexit_fb_shutdown);

	TEXT tabname[WORDLENGTH];
	tabname[0] = '\0';
	isqlGlob.db_SQL_dialect = 0;

	// Output goes to stdout by default
	isqlGlob.Out = stdout;
	isqlGlob.Errfp = stderr;

	const processing_state ret = parse_arg(argc, argv, tabname);

	// Can't do a simple assignment because parse_arg may set Interactive to false.
	if (stdin_redirected())
		Interactive = false;

	// Init the diagnostics and help files
	if (Merge_diagnostic)
	    Diag = isqlGlob.Out;
	else
		Diag = stdout;

	Help = stdout;

	if (Merge_stderr)
		isqlGlob.Errfp = isqlGlob.Out;

	ISQL_make_upper(tabname);
	switch (ret)
	{
	case EXTRACT:
	case EXTRACTALL:
		if (*isqlGlob.global_Db_name)
		{
			Interactive = false; // "extract" option only can be called from command-line

			// Let's use user and password if provided.
			// This should solve bug #112263 FSG 28.Jan.2001
			if (newdb(isqlGlob.global_Db_name, isqlGlob.User, Password, global_numbufs,
				isqlGlob.Role, false) == SKIP)
			{
				LegacyTables flag = ret == EXTRACT ? SQL_objects : ALL_objects;
				Exit_value = EXTRACT_ddl(flag, tabname);
				ISQL_disconnect_database(true);
				// isc_detach_database(isc_status, &DB);
			}
			else
				Exit_value = FINI_ERROR;
		}
		break;

	case ps_ERR:
	    {
			TEXT helpstring[155];
			ISQL_msg_get(USAGE, sizeof(helpstring), helpstring);
			STDERROUT(helpstring);
			for (int i = 0; i < FB_NELEM(switches); i++)
			{
				if (switches[i].usage_id >= 0)
				{
					ISQL_msg_get(switches[i].usage_id, sizeof(helpstring), helpstring);
					STDERROUT(helpstring);
				}
			}
			Exit_value = FINI_ERROR;
			break;
		}

	default:
		do_isql();
		// keep Exit_value to whatever it is set
		// by do_isql()
		// Exit_value = FINI_OK;
		break;
	}
#ifdef MU_ISQL
	// This is code for QA Test bed Multiuser environment.
	// Do multi-user cleanup if running in multi-user domain.
	// qa_mu_cleanup() is NOP in non-multi-user domain.
	qa_mu_cleanup();
#endif	// MU_ISQL

#ifdef DEBUG_GDS_ALLOC
	// As ISQL can run under windows, all memory should be freed before
	// returning.  In debug mode this call will report unfreed blocks.

	//gds_alloc_report(0, __FILE__, __LINE__);
	char fn[] = __FILE__;
	fn[strlen(fn) - 8] = 0; // all isql files in gen dir
	gds_alloc_report(0, fn, 0);
#endif

	return Exit_value;
}


void ISQL_array_dimensions(const TEXT* fieldname)
{
   struct isc_131_struct {
          ISC_LONG isc_132;	/* RDB$UPPER_BOUND */
          ISC_LONG isc_133;	/* RDB$LOWER_BOUND */
          short isc_134;	/* isc_utility */
          short isc_135;	/* RDB$DIMENSION */
   } isc_131;
   struct isc_129_struct {
          char  isc_130 [32];	/* RDB$FIELD_NAME */
   } isc_129;
/**************************************
 *
 *	I S Q L _ a r r a y _ d i m e n s i o n s
 *
 **************************************
 *
 * Functional description
 *	Retrieves the dimensions of arrays and prints them.
 *
 *	Parameters:  fieldname -- the actual name of the array field
 *
 **************************************/

	isqlGlob.printf("[");

	// Transaction for all frontend commands
	if (DB && !gds_trans)
		if (isc_start_transaction(isc_status, &gds_trans, 1, &DB, 0, NULL))
		{
			ISQL_errmsg(isc_status);
			return;
		}

	/*FOR FDIM IN RDB$FIELD_DIMENSIONS WITH
	   FDIM.RDB$FIELD_NAME EQ fieldname
	   SORTED BY FDIM.RDB$DIMENSION*/
	{
        if (!isc_127)
           isc_compile_request2 (isc_status, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &isc_127, (short) sizeof(isc_128), (char*) isc_128);
	isc_vtov ((const char*) fieldname, (char*) isc_129.isc_130, 32);
	if (isc_127)
	   {
           isc_start_and_send (isc_status, (FB_API_HANDLE*) &isc_127, (FB_API_HANDLE*) &gds_trans, (short) 0, (short) 32, &isc_129, (short) 0);
	   }
	if (!isc_status [1]) {
	while (1)
	   {
           isc_receive (isc_status, (FB_API_HANDLE*) &isc_127, (short) 1, (short) 12, &isc_131, (short) 0);
	   if (!isc_131.isc_134 || isc_status [1]) break;

		// Format is [lower:upper, lower:upper,..]
		// When lower == 1 no need to print a range. Done.
		// When upper == 1 no need to print a range either, but it's confusing. Not done.

		if (/*FDIM.RDB$DIMENSION*/
		    isc_131.isc_135 > 0) {
			isqlGlob.printf(", ");
		}
		if (/*FDIM.RDB$LOWER_BOUND*/
		    isc_131.isc_133 == 1)
			isqlGlob.printf("%ld", /*FDIM.RDB$UPPER_BOUND*/
					       isc_131.isc_132);
		else
			isqlGlob.printf("%ld:%ld", /*FDIM.RDB$LOWER_BOUND*/
						   isc_131.isc_133, /*FDIM.RDB$UPPER_BOUND*/
  isc_131.isc_132);

	/*END_FOR*/
	   }
	   };
	/*ON_ERROR*/
	if (isc_status [1])
	   {
		ISQL_errmsg(isc_status);
		return;
	/*END_ERROR;*/
	   }
	}

	isqlGlob.printf("]");
}


// Same than fb_utils::exact_name, but writes the output to the second argument.
TEXT* ISQL_blankterm2(const TEXT* input, TEXT* output)
{
	TEXT* q = output - 1;
	for (TEXT* p = output; (*p = *input) != 0; ++p, ++input)
	{
		if (*p != BLANK)
			q = p;
	}
	*(q + 1) = 0;
	return output;
}


bool ISQL_dbcheck()
{
/**************************************
 *
 *	I S Q L _ d b c h e c k
 *
 **************************************
 *
 * Functional description
 *	Check to see if we are connected to a database.
 *  	Return true if connected, false otherwise
 *
 * Change from miroslavp on 2005.04.25
 *   Value assigned here on Exit_value is not used anywhere in the source
 *   and cause isql.exe to return 1 in Quiet mode even there is no error.
 *   If there is error Exit_value is set by callers - so it is removed from here.
 **************************************/
	TEXT errbuf[MSG_LENGTH];

	if (!isqlGlob.global_Db_name[0])
	{
		if (!Quiet)
		{
			ISQL_msg_get(NO_DB, errbuf);
			STDERROUT(errbuf);
		}
		return false;
	}
	else
		return true;
}

static char userPrompt2[MSG_LENGTH];
static const char* const userPrompt = userPrompt2;
static char* lastInputLine = NULL;
static int getColumn = -1;



void ISQL_prompt(const TEXT* string)
{
/**************************************
 *
 *	I S Q L _ p r o m p t
 *
 **************************************
 *
 * Functional description
 *	Print a prompt string for interactive user
 *	Not for Windows, otherwise flush the string
 **************************************/
	//userPrompt = (const char*) string;
	fb_utils::copy_terminate(userPrompt2, string, MSG_LENGTH);

	//#ifndef HAVE_READLINE_READLINE_H
	//	fprintf(stdout, userPrompt.c_str());
	//	fflush(stdout);
	//#endif

}

static void readNextInputLine(const char* prompt)
{
/**************************************
 *
 *	r e a d N e x t I n p u t L i n e
 *
 **************************************
 *
 * Functional description
 *  Get next input line and put it in lastInputLine
 *  If the first read is EOF set lineInputLine to NULL
 *  Otherwise return the line up to EOF (excluded) or '\n' (included)
 * WARNING: If you bypass getNextInputChar() and call this directly,
 * remember to set getColumn = -1; immediately after calling this function
 * to avoid side effects or reading invalid memory locations.
 **************************************/

	if (lastInputLine != NULL)
	{
		free(lastInputLine);
		lastInputLine = NULL;
	}

	getColumn = 0;

#ifdef HAVE_EDITLINE_H
	if (Filelist->readingStdin())
	{
		// CVC: On 2005-04-02, use an empty prompt when not working in
		// interactive mode to behave like @@@ below at request by Pavel.
		const char* new_prompt = Interactive ? prompt : "";
		lastInputLine = readline(new_prompt);

		if (lastInputLine != NULL && strlen(lastInputLine) != 0) {
			add_history(lastInputLine);
		}
		// Let's count lines if someone wants to enable line number error messages
		// for console reading for whatever reason.
		++Filelist->Ifp().indev_aux;
		return;
	}

#endif
	// @@@ CVC: On 2005-04-02, take the "|| Echo" out at request by Pavel.
	if (Interactive && !Input_file)// || Echo)
	{
		// Write the prompt out.
		fprintf(stdout, "%s", prompt);
		fflush(stdout);
	}

	// Read the line
	char buffer[MAX_USHORT];
	if (fgets(buffer, sizeof(buffer), Filelist->Ifp().indev_fpointer) != NULL)
	{
		size_t lineSize = strlen(buffer);

		// If the last non empty line doesn't end in '\n', indev_aux won't be
		// updated, but then there're no more commands, so it's irrelevant.
		while (lineSize > 0 &&
			   (buffer[lineSize - 1] == '\n' || buffer[lineSize - 1] == '\r'))
		{
			buffer[--lineSize] = '\0';
			++Filelist->Ifp().indev_aux;
		}

		lastInputLine = (char*) malloc(lineSize + 1);
		memcpy(lastInputLine, buffer, lineSize + 1);

	}
}

static void readNextInputLine()
{
	readNextInputLine(userPrompt);
	if (Echo && (lastInputLine != NULL)) {
		isqlGlob.printf("%s%s", lastInputLine, NEWLINE);
	}
}

int getNextInputChar()
{
/**************************************
 *
 *	g e t N e x t I n p u t C h a r
 *
 **************************************
 *
 * Functional description
 *	Read next char from input
 *
 *
 **************************************/
	static int inputLen = 0;
	// At end of line try and read next line
	if (getColumn == -1)
	{
		readNextInputLine();
		if (lastInputLine)
			inputLen = (int) strlen(lastInputLine);
	}

	// readline found EOF
	if (lastInputLine == NULL) {
		return EOF;
	}

	// If at end of line return \n
	if (getColumn == inputLen) //(int) strlen(lastInputLine))
	{
		getColumn = -1;
		return '\n';
	}
    // cast to unsigned char to prevent sign expansion
    // this way we can distinguish russian ya (0xFF) and EOF (usually (-1))
	return (unsigned char)lastInputLine[getColumn++];
}

void ISQL_copy_SQL_id(const TEXT* in_str, TEXT* output_str, TEXT escape_char)
{
/**************************************
 *
 *	I S Q L _ c o p y _ S Q L _ i d
 *
 **************************************
 *
 * Functional description
 *
 *	Copy/rebuild the SQL identifier by adding escape double quote if
 *	double quote is part of the SQL identifier and wraps around the
 *	SQL identifier with delimited double quotes
 *
 **************************************/

	/* CVC: Try to detect if we can get rid of double quotes as
	   requested by Ann. Notice empty names need double quotes.
	   Assume the caller invoked previously ISQL_blankterm() that's
	   just another function like DYN_terminate, MET_exact_name, etc.
	   ISQL_blankterm has been replaced by fb_utils::exact_name. */

	if (escape_char == DBL_QUOTE)
	{
		// Cannot rely on ANSI functions that may be localized.
		bool need_quotes = *in_str < 'A' || *in_str > 'Z';
		TEXT* q1 = output_str;
		for (const TEXT* p1 = in_str; *p1 && !need_quotes; ++p1, ++q1)
		{
			if ((*p1 < 'A' || *p1 > 'Z') && (*p1 < '0' || *p1 > '9') && *p1 != '_' && *p1 != '$')
			{
				need_quotes = true;
				break;
			}
			*q1 = *p1;
		}
		if (!need_quotes && !KEYWORD_stringIsAToken(in_str))
		{
			*q1 = '\0';
			return;
		}
	}

	TEXT* q1 = output_str;
	*q1++ = escape_char;

	for (const TEXT* p1 = in_str; *p1; p1++)
	{
		*q1++ = *p1;
		if (*p1 == escape_char) {
			*q1++ = escape_char;
		}
	}
	*q1++ = escape_char;
	*q1 = '\0';
}


void ISQL_errmsg(const ISC_STATUS* status)
{
/**************************************
 *
 *	I S Q L _ e r r m s g
 *
 **************************************
 *
 * Functional description
 *	Report error conditions
 *	Simulate isc_print_status exactly, to control stderr
 **************************************/
	TEXT errbuf[MSG_LENGTH];

	if (Quiet)
		Exit_value = FINI_ERROR;
	//else
	{
		const ISC_STATUS* vec = status;
		if (vec[0] != isc_arg_gds ||
			(vec[0] == isc_arg_gds && vec[1] == 0 && vec[2] != isc_arg_warning) ||
			(vec[0] == isc_arg_gds && vec[1] == 0 && vec[2] == isc_arg_warning && !Warnings))
		{
			return;
		}
		FB_SQLSTATE_STRING sqlstate;
		fb_sqlstate(sqlstate, status);
		ISQL_msg_get(GEN_ERR, errbuf, SafeArg() << sqlstate);
		STDERROUT(errbuf);
		if (fb_interpret(errbuf, sizeof(errbuf), &vec))
		{
			STDERROUT(errbuf);

			// Continuation of error
			errbuf[0] = '-';
			while (fb_interpret(errbuf + 1, sizeof(errbuf) - 1, &vec)) {
				STDERROUT(errbuf);
			}
		}
		if (Input_file)
		{
			//const char* s = 0;
			int linenum = -1;
			if (status[0] == isc_arg_gds && status[1] == isc_dsql_error &&
				status[2] == isc_arg_gds && status[3] == isc_sqlerr && vec > &status[9])
			{
				switch (status[7])
				{
				case isc_dsql_field_err:
				case isc_dsql_procedure_err:
				case isc_dsql_relation_err:
				case isc_dsql_procedure_use_err:
				case isc_dsql_no_dup_name:
					vec = &status[8];
					while (*vec++ != isc_arg_end)
						if (vec[0] == isc_dsql_line_col_error && vec[1] == isc_arg_number)
						{
							linenum  = vec[2];
							//STDERROUT(s);
							break;
						}
					break;
				case isc_dsql_token_unk_err:
					if (status[8] == isc_arg_number)
					{
						linenum = status[9];
						//s = errbuf;
						//STDERROUT(s);
					}
					break;
				}
			}
			/* CVC: Obsolete on 2005.10.06 because now line & column are numeric arguments.
			if (s)
			{
				while (*s && !fb_isdigit(*s))
					++s;
				if (isdigit(*s))
				{
					linenum = 0;
					for (; *s && isdigit(*s); ++s)
						linenum = linenum * 10 + *s - '0';
				}
			}
			*/
			const InputDevices::indev& Ifp = Filelist->Ifp();
			if (linenum != -1)
			{
				linenum += Ifp.indev_line;
				ISQL_msg_get(EXACTLINE, errbuf, SafeArg() << linenum << Ifp.fileName());
			}
			else
                ISQL_msg_get(AFTERLINE, errbuf, SafeArg() << Ifp.indev_line << Ifp.fileName());

			STDERROUT(errbuf);
		}
	}
}


void ISQL_warning(ISC_STATUS* status)
{
/**************************************
 *
 *	I S Q L _ w a r n i n g
 *
 **************************************
 *
 * Functional desription
 *	Report warning
 *	Simulate isc_print_status exactly, to control stderr
 **************************************/

	const ISC_STATUS* vec = status;
	fb_assert(vec[1] == 0);		// shouldn't be any errors

	//if (!Quiet)
	{
		if (vec[0] != isc_arg_gds ||
			vec[2] != isc_arg_warning ||
			(vec[2] == isc_arg_warning && !Warnings))
		{
			return;
		}
		TEXT buf[MSG_LENGTH];
		if (fb_interpret(buf, sizeof(buf), &vec))
		{
			STDERROUT(buf);

			// Continuation of warning
			buf[0] = '-';
			while (fb_interpret(buf + 1, sizeof(buf) - 1, &vec)) {
				STDERROUT(buf);
			}
		}
	}

	status[2] = isc_arg_end;
}


SSHORT ISQL_get_default_char_set_id()
{
   struct isc_119_struct {
          short isc_120;	/* isc_utility */
          short isc_121;	/* RDB$CHARACTER_SET_ID */
   } isc_119;
   struct isc_117_struct {
          char  isc_118 [32];	/* RDB$CHARACTER_SET_NAME */
   } isc_117;
   struct isc_124_struct {
          char  isc_125 [32];	/* RDB$CHARACTER_SET_NAME */
          short isc_126;	/* isc_utility */
   } isc_124;
/*************************************
*
*	I S Q L _ g e t _ d e f a u l t _ c h a r _ s e t _ i d
*
**************************************
*
* Functional description
*	Return the database default character set
*	id.
*
*	-1 if the value can not be determined.
*
**************************************/

/* What is the default character set for this database?
   There are three states:
   1.	There is no entry available in RDB$DATABASE
	Then - NONE
   2.   The entry in RDB$DATABASE does not exist in
	RDB$CHARACTER_SETS
	Then - -1 to cause all character set defs to show
   3.	An entry in RDB$CHARACTER_SETS
	Then - RDB$CHARACTER_SET_ID
*/
	SSHORT default_char_set_id = 0;
	/*FOR FIRST 1 EXT IN RDB$DATABASE
		WITH EXT.RDB$CHARACTER_SET_NAME NOT MISSING*/
	{
        if (!isc_122)
           isc_compile_request2 (NULL, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &isc_122, (short) sizeof(isc_123), (char*) isc_123);
        isc_start_request (NULL, (FB_API_HANDLE*) &isc_122, (FB_API_HANDLE*) &gds_trans, (short) 0);
	while (1)
	   {
           isc_receive (NULL, (FB_API_HANDLE*) &isc_122, (short) 0, (short) 34, &isc_124, (short) 0);
	   if (!isc_124.isc_126) break;;

		default_char_set_id = -1;

		/*FOR FIRST 1 CHI IN RDB$CHARACTER_SETS
		WITH CHI.RDB$CHARACTER_SET_NAME = EXT.RDB$CHARACTER_SET_NAME*/
		{
                if (!isc_115)
                   isc_compile_request2 (NULL, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &isc_115, (short) sizeof(isc_116), (char*) isc_116);
		isc_vtov ((const char*) isc_124.isc_125, (char*) isc_117.isc_118, 32);
                isc_start_and_send (NULL, (FB_API_HANDLE*) &isc_115, (FB_API_HANDLE*) &gds_trans, (short) 0, (short) 32, &isc_117, (short) 0);
		while (1)
		   {
                   isc_receive (NULL, (FB_API_HANDLE*) &isc_115, (short) 1, (short) 4, &isc_119, (short) 0);
		   if (!isc_119.isc_120) break;

			default_char_set_id = /*CHI.RDB$CHARACTER_SET_ID*/
					      isc_119.isc_121;

		/*END_FOR;*/
		   }
		}
	/*END_FOR;*/
	   }
	}

	return (default_char_set_id);
}


SSHORT ISQL_get_field_length(const TEXT* field_name)
{
   struct isc_110_struct {
          short isc_111;	/* isc_utility */
          short isc_112;	/* RDB$FIELD_LENGTH */
          short isc_113;	/* gds__null_flag */
          short isc_114;	/* RDB$CHARACTER_LENGTH */
   } isc_110;
   struct isc_108_struct {
          char  isc_109 [32];	/* RDB$FIELD_NAME */
   } isc_108;
/**************************************
 *
 *	I S Q L _ g e t _ f i e l d _ l e n g t h
 *
 **************************************
 *
 *	Retrieve character or field length of character types.
 *
 **************************************/

	// Transaction for all frontend commands
	if (DB && !gds_trans)
	{
		if (isc_start_transaction(isc_status, &gds_trans, 1, &DB, 0, NULL))
		{
			ISQL_errmsg(isc_status);
			return 0;
		}
	}

	SSHORT l = 0;
	/*FOR FLD IN RDB$FIELDS WITH
		FLD.RDB$FIELD_NAME EQ field_name*/
	{
        if (!isc_106)
           isc_compile_request2 (isc_status, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &isc_106, (short) sizeof(isc_107), (char*) isc_107);
	isc_vtov ((const char*) field_name, (char*) isc_108.isc_109, 32);
	if (isc_106)
	   {
           isc_start_and_send (isc_status, (FB_API_HANDLE*) &isc_106, (FB_API_HANDLE*) &gds_trans, (short) 0, (short) 32, &isc_108, (short) 0);
	   }
	if (!isc_status [1]) {
	while (1)
	   {
           isc_receive (isc_status, (FB_API_HANDLE*) &isc_106, (short) 1, (short) 8, &isc_110, (short) 0);
	   if (!isc_110.isc_111 || isc_status [1]) break;

		if (/*FLD.RDB$CHARACTER_LENGTH.NULL*/
		    isc_110.isc_113)
			l = /*FLD.RDB$FIELD_LENGTH*/
			    isc_110.isc_112;
		else
			l = /*FLD.RDB$CHARACTER_LENGTH*/
			    isc_110.isc_114;
	/*END_FOR*/
	   }
	   };
	/*ON_ERROR*/
	if (isc_status [1])
	   {
		ISQL_errmsg(isc_status);
		return 0;
	/*END_ERROR;*/
	   }
	}

	return l;
}


void ISQL_get_character_sets(SSHORT char_set_id, SSHORT collation, bool collate_only,
							 bool not_null, TEXT* string)
{
   struct isc_93_struct {
          char  isc_94 [32];	/* RDB$CHARACTER_SET_NAME */
          short isc_95;	/* isc_utility */
   } isc_93;
   struct isc_91_struct {
          short isc_92;	/* RDB$CHARACTER_SET_ID */
   } isc_91;
   struct isc_101_struct {
          char  isc_102 [32];	/* RDB$DEFAULT_COLLATE_NAME */
          char  isc_103 [32];	/* RDB$COLLATION_NAME */
          char  isc_104 [32];	/* RDB$CHARACTER_SET_NAME */
          short isc_105;	/* isc_utility */
   } isc_101;
   struct isc_98_struct {
          short isc_99;	/* RDB$CHARACTER_SET_ID */
          short isc_100;	/* RDB$COLLATION_ID */
   } isc_98;
/**************************************
 *
 *	I S Q L _ g e t _ c h a r a c t e r _ s e t s
 *
 **************************************
 *
 *	Retrieve character set and collation order and format it
 *
 **************************************/
#ifdef	DEV_BUILD
	bool found = false;
#endif

	const char* notNullStr = not_null ? " NOT NULL" : "";
	string[0] = 0;

	// Transaction for all frontend commands
	if (DB && !gds_trans)
	{
		if (isc_start_transaction(isc_status, &gds_trans, 1, &DB, 0, NULL))
		{
			ISQL_errmsg(isc_status);
			return;
		}
	}

	//if (collation) {
	if (collation || collate_only)
	{
		/*FOR FIRST 1 COL IN RDB$COLLATIONS CROSS
			CST IN RDB$CHARACTER_SETS WITH
			COL.RDB$CHARACTER_SET_ID EQ CST.RDB$CHARACTER_SET_ID AND
			COL.RDB$COLLATION_ID EQ collation AND
			CST.RDB$CHARACTER_SET_ID EQ char_set_id
			SORTED BY COL.RDB$COLLATION_NAME, CST.RDB$CHARACTER_SET_NAME*/
		{
                if (!isc_96)
                   isc_compile_request2 (isc_status, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &isc_96, (short) sizeof(isc_97), (char*) isc_97);
		isc_98.isc_99 = char_set_id;
		isc_98.isc_100 = collation;
		if (isc_96)
		   {
                   isc_start_and_send (isc_status, (FB_API_HANDLE*) &isc_96, (FB_API_HANDLE*) &gds_trans, (short) 0, (short) 4, &isc_98, (short) 0);
		   }
		if (!isc_status [1]) {
		while (1)
		   {
                   isc_receive (isc_status, (FB_API_HANDLE*) &isc_96, (short) 1, (short) 98, &isc_101, (short) 0);
		   if (!isc_101.isc_105 || isc_status [1]) break;

#ifdef DEV_BUILD
			found = true;
#endif
			fb_utils::exact_name(/*CST.RDB$CHARACTER_SET_NAME*/
					     isc_101.isc_104);
			fb_utils::exact_name(/*COL.RDB$COLLATION_NAME*/
					     isc_101.isc_103);
			fb_utils::exact_name(/*CST.RDB$DEFAULT_COLLATE_NAME*/
					     isc_101.isc_102);

			// Is specified collation the default collation for character set?
			if (strcmp (/*CST.RDB$DEFAULT_COLLATE_NAME*/
				    isc_101.isc_102, /*COL.RDB$COLLATION_NAME*/
  isc_101.isc_103) == 0)
			{
				if (!collate_only)
					sprintf (string, " CHARACTER SET %s%s", /*CST.RDB$CHARACTER_SET_NAME*/
										isc_101.isc_104, notNullStr);
			}
			else if (collate_only)
				sprintf (string, "%s COLLATE %s", notNullStr, /*COL.RDB$COLLATION_NAME*/
									      isc_101.isc_103);
			else
				sprintf (string, " CHARACTER SET %s%s COLLATE %s",
						 /*CST.RDB$CHARACTER_SET_NAME*/
						 isc_101.isc_104, notNullStr, /*COL.RDB$COLLATION_NAME*/
	      isc_101.isc_103);
		/*END_FOR*/
		   }
		   };
		/*ON_ERROR*/
		if (isc_status [1])
		   {
			ISQL_errmsg(isc_status);
			return;
		/*END_ERROR;*/
		   }
		}
#ifdef DEV_BUILD
		if (!found)
		{
			TEXT Print_buffer[PRINT_BUFFER_LENGTH];
			sprintf(Print_buffer,
					"ISQL_get_character_set: charset %d collation %d not found.\n",
					char_set_id, collation);
			STDERROUT(Print_buffer);
		}
#endif
	}
	else
	{
		/*FOR FIRST 1 CST IN RDB$CHARACTER_SETS WITH
			CST.RDB$CHARACTER_SET_ID EQ char_set_id
			SORTED BY CST.RDB$CHARACTER_SET_NAME*/
		{
                if (!isc_89)
                   isc_compile_request2 (isc_status, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &isc_89, (short) sizeof(isc_90), (char*) isc_90);
		isc_91.isc_92 = char_set_id;
		if (isc_89)
		   {
                   isc_start_and_send (isc_status, (FB_API_HANDLE*) &isc_89, (FB_API_HANDLE*) &gds_trans, (short) 0, (short) 2, &isc_91, (short) 0);
		   }
		if (!isc_status [1]) {
		while (1)
		   {
                   isc_receive (isc_status, (FB_API_HANDLE*) &isc_89, (short) 1, (short) 34, &isc_93, (short) 0);
		   if (!isc_93.isc_95 || isc_status [1]) break;

#ifdef DEV_BUILD
			found = true;
#endif
			fb_utils::exact_name(/*CST.RDB$CHARACTER_SET_NAME*/
					     isc_93.isc_94);

			sprintf (string, " CHARACTER SET %s%s", /*CST.RDB$CHARACTER_SET_NAME*/
								isc_93.isc_94, notNullStr);
		/*END_FOR*/
		   }
		   };
		/*ON_ERROR*/
		if (isc_status [1])
		   {
			ISQL_errmsg(isc_status);
			return;
		/*END_ERROR;*/
		   }
		}
#ifdef DEV_BUILD
		if (!found)
		{
			TEXT Print_buffer[PRINT_BUFFER_LENGTH];
			sprintf(Print_buffer, "ISQL_get_character_set: charset %d not found.\n",
					char_set_id);
			STDERROUT(Print_buffer);
		}
#endif
	}
}


void ISQL_get_default_source(const TEXT* rel_name,
							 TEXT* field_name,
							 ISC_QUAD* blob_id)
{
   struct isc_74_struct {
          ISC_QUAD isc_75;	/* RDB$DEFAULT_SOURCE */
          short isc_76;	/* isc_utility */
          short isc_77;	/* gds__null_flag */
   } isc_74;
   struct isc_72_struct {
          char  isc_73 [32];	/* RDB$FIELD_NAME */
   } isc_72;
   struct isc_83_struct {
          ISC_QUAD isc_84;	/* RDB$DEFAULT_SOURCE */
          ISC_QUAD isc_85;	/* RDB$DEFAULT_SOURCE */
          short isc_86;	/* isc_utility */
          short isc_87;	/* gds__null_flag */
          short isc_88;	/* gds__null_flag */
   } isc_83;
   struct isc_80_struct {
          char  isc_81 [32];	/* RDB$FIELD_NAME */
          char  isc_82 [32];	/* RDB$RELATION_NAME */
   } isc_80;
/**************************************
 *
 *	I S Q L _ g e t _ d e f a u l t _ s o u r c e
 *
 **************************************
 *
 *	Retrieve the default source of a field.
 *	Relation_fields takes precedence over fields if both
 *	are present
 *
 *	For a domain, a NULL is passed for rel_name
 **************************************/

	fb_utils::exact_name(field_name);

	*blob_id = isc_blob_null;

	// Transaction for all frontend commands
	if (DB && !gds_trans)
	{
		if (isc_start_transaction(isc_status, &gds_trans, 1, &DB, 0, NULL))
		{
			ISQL_errmsg(isc_status);
			return;
		}
	}

	if (rel_name)
	{
		// This is default for a column of a table
		/*FOR FLD IN RDB$FIELDS CROSS
			RFR IN RDB$RELATION_FIELDS WITH
			RFR.RDB$FIELD_SOURCE EQ FLD.RDB$FIELD_NAME AND
			RFR.RDB$RELATION_NAME EQ rel_name AND
			FLD.RDB$FIELD_NAME EQ field_name*/
		{
                if (!isc_78)
                   isc_compile_request2 (isc_status, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &isc_78, (short) sizeof(isc_79), (char*) isc_79);
		isc_vtov ((const char*) field_name, (char*) isc_80.isc_81, 32);
		isc_vtov ((const char*) rel_name, (char*) isc_80.isc_82, 32);
		if (isc_78)
		   {
                   isc_start_and_send (isc_status, (FB_API_HANDLE*) &isc_78, (FB_API_HANDLE*) &gds_trans, (short) 0, (short) 64, &isc_80, (short) 0);
		   }
		if (!isc_status [1]) {
		while (1)
		   {
                   isc_receive (isc_status, (FB_API_HANDLE*) &isc_78, (short) 1, (short) 22, &isc_83, (short) 0);
		   if (!isc_83.isc_86 || isc_status [1]) break;

			if (!/*RFR.RDB$DEFAULT_SOURCE.NULL*/
			     isc_83.isc_88)
				*blob_id = /*RFR.RDB$DEFAULT_SOURCE*/
					   isc_83.isc_85;
			else if (!/*FLD.RDB$DEFAULT_SOURCE.NULL*/
				  isc_83.isc_87)
				*blob_id = /*FLD.RDB$DEFAULT_SOURCE*/
					   isc_83.isc_84;

		/*END_FOR*/
		   }
		   };
		/*ON_ERROR*/
		if (isc_status [1])
		   {
			ISQL_errmsg(isc_status);
			return;
		/*END_ERROR;*/
		   }
		}
	}
	else
	{
		// Default for a domain
		/*FOR FLD IN RDB$FIELDS WITH
			FLD.RDB$FIELD_NAME EQ field_name*/
		{
                if (!isc_70)
                   isc_compile_request2 (isc_status, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &isc_70, (short) sizeof(isc_71), (char*) isc_71);
		isc_vtov ((const char*) field_name, (char*) isc_72.isc_73, 32);
		if (isc_70)
		   {
                   isc_start_and_send (isc_status, (FB_API_HANDLE*) &isc_70, (FB_API_HANDLE*) &gds_trans, (short) 0, (short) 32, &isc_72, (short) 0);
		   }
		if (!isc_status [1]) {
		while (1)
		   {
                   isc_receive (isc_status, (FB_API_HANDLE*) &isc_70, (short) 1, (short) 12, &isc_74, (short) 0);
		   if (!isc_74.isc_76 || isc_status [1]) break;

			if (!/*FLD.RDB$DEFAULT_SOURCE.NULL*/
			     isc_74.isc_77)
				*blob_id = /*FLD.RDB$DEFAULT_SOURCE*/
					   isc_74.isc_75;

		/*END_FOR*/
		   }
		   };
		/*ON_ERROR*/
		if (isc_status [1])
		   {
			ISQL_errmsg(isc_status);
			return;
		/*END_ERROR;*/
		   }
		}
	}
}


SLONG ISQL_get_index_segments(TEXT* segs,
								const size_t buf_size,
								const TEXT* indexname,
								bool delimited_yes)
{
   struct isc_67_struct {
          char  isc_68 [32];	/* RDB$FIELD_NAME */
          short isc_69;	/* isc_utility */
   } isc_67;
   struct isc_65_struct {
          char  isc_66 [32];	/* RDB$INDEX_NAME */
   } isc_65;
/**************************************
 *
 *	I S Q L _ g e t _ i n d e x _ s e g m e n t s
 *
 **************************************
 *
 * Functional description
 *	returns the list of columns in an index.
 *
 **************************************/
	TEXT SQL_identifier[BUFFER_LENGTH128];

	*segs = '\0';

	// Transaction for all frontend commands
	if (DB && !gds_trans)
	{
		if (isc_start_transaction(isc_status, &gds_trans, 1, &DB, 0, NULL))
		{
			ISQL_errmsg(isc_status);
			return 0;
		}
	}

	TEXT* const segs_end = segs + buf_size - 1;
	// Query to get column names
	SLONG n = 0;
	bool count_only = false;

	/*FOR SEG IN RDB$INDEX_SEGMENTS WITH
		SEG.RDB$INDEX_NAME EQ indexname
		SORTED BY SEG.RDB$FIELD_POSITION*/
	{
        if (!isc_63)
           isc_compile_request2 (isc_status, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &isc_63, (short) sizeof(isc_64), (char*) isc_64);
	isc_vtov ((const char*) indexname, (char*) isc_65.isc_66, 32);
	if (isc_63)
	   {
           isc_start_and_send (isc_status, (FB_API_HANDLE*) &isc_63, (FB_API_HANDLE*) &gds_trans, (short) 0, (short) 32, &isc_65, (short) 0);
	   }
	if (!isc_status [1]) {
	while (1)
	   {
           isc_receive (isc_status, (FB_API_HANDLE*) &isc_63, (short) 1, (short) 34, &isc_67, (short) 0);
	   if (!isc_67.isc_69 || isc_status [1]) break;

		++n;
		if (count_only)
			continue;

		// Place a comma and a blank between each segment column name

		fb_utils::exact_name(/*SEG.RDB$FIELD_NAME*/
				     isc_67.isc_68);
		if (isqlGlob.db_SQL_dialect > SQL_DIALECT_V6_TRANSITION && delimited_yes) {
			ISQL_copy_SQL_id (/*SEG.RDB$FIELD_NAME*/
					  isc_67.isc_68, SQL_identifier, DBL_QUOTE);
		}
		else {
			strcpy (SQL_identifier, /*SEG.RDB$FIELD_NAME*/
						isc_67.isc_68);
		}

		const size_t len = strlen(SQL_identifier);

		if (n == 1)
		{
			// We assume the buffer is at least size(metadata name), so no initial check.
			strcpy(segs, SQL_identifier);
			segs += len;
		}
		else
		{
			if (segs + len + 2 >= segs_end)
			{
				strncpy(segs, ", ...", segs_end - segs);
				*segs_end = '\0';
				count_only = true;
			}
			else
			{
				sprintf (segs, ", %s", SQL_identifier);
				segs += len + 2;
			}
		}

	/*END_FOR*/
	   }
	   };
	/*ON_ERROR*/
	if (isc_status [1])
	   {
		ISQL_errmsg(isc_status);
		/*ROLLBACK;*/
		{
		isc_rollback_transaction (NULL, (FB_API_HANDLE*) &gds_trans);
		}
		return 0;
	/*END_ERROR;*/
	   }
	}

	return n;
}


bool ISQL_get_base_column_null_flag(const TEXT* view_name,
									const SSHORT view_context,
									const TEXT* base_field)
{
   struct isc_45_struct {
          short isc_46;	/* isc_utility */
          short isc_47;	/* gds__null_flag */
          short isc_48;	/* RDB$NULL_FLAG */
   } isc_45;
   struct isc_41_struct {
          char  isc_42 [32];	/* RDB$PARAMETER_NAME */
          char  isc_43 [32];	/* RDB$VIEW_NAME */
          short isc_44;	/* RDB$VIEW_CONTEXT */
   } isc_41;
   struct isc_55_struct {
          char  isc_56 [32];	/* RDB$RELATION_NAME */
          char  isc_57 [32];	/* RDB$BASE_FIELD */
          short isc_58;	/* isc_utility */
          short isc_59;	/* RDB$VIEW_CONTEXT */
          short isc_60;	/* gds__null_flag */
          short isc_61;	/* RDB$NULL_FLAG */
          short isc_62;	/* gds__null_flag */
   } isc_55;
   struct isc_51_struct {
          char  isc_52 [32];	/* RDB$FIELD_NAME */
          char  isc_53 [32];	/* RDB$VIEW_NAME */
          short isc_54;	/* RDB$VIEW_CONTEXT */
   } isc_51;
/**************************************
 *
 *	I S Q L _ g e t _ b a s e _ c o l u m n _ n u l l _ f l a g
 *
 **************************************
 *
 *	Determine if a field on which view column is based
 *	is nullable. We are passed the view_name
 *	view_context and the base_field of the view column.
 *
 **************************************/
	/*BASED_ON RDB$RELATION_FIELDS.RDB$RELATION_NAME save_view_name,
		save_base_field;*/
	char
	   save_view_name[32],
	   save_base_field[32];


	strcpy(save_view_name, view_name);
	strcpy(save_base_field, base_field);
	SSHORT save_view_context = view_context;


	// Transaction for all frontend commands
	if (DB && !gds_trans)
	{
		if (isc_start_transaction(isc_status, &gds_trans, 1, &DB, 0, NULL))
		{
			ISQL_errmsg(isc_status);
			return false;
		}
	}

	/*
	Using view_name and view_context get the relation name from
	RDB$VIEW_RELATIONS which contains the base_field for this view column.
	Get row corresponding to this base field and relation from
	rdb$field_relations. This will contain info on field's nullability unless
	it is a view column itself, in which case repeat this procedure till
	we get to a "real" column.
	*/
	bool null_flag = true;
	bool done = false;
	bool error = false;
	while (!done && !error)
	{
		fb_utils::exact_name(save_view_name);
		fb_utils::exact_name(save_base_field);
		bool found = false;
		/*FOR FIRST 1
			VR IN RDB$VIEW_RELATIONS
			CROSS NEWRFR IN RDB$RELATION_FIELDS WITH
			VR.RDB$VIEW_NAME EQ save_view_name AND
			VR.RDB$VIEW_CONTEXT EQ save_view_context AND
			NEWRFR.RDB$RELATION_NAME = VR.RDB$RELATION_NAME AND
			NEWRFR.RDB$FIELD_NAME = save_base_field*/
		{
                if (!isc_49)
                   isc_compile_request2 (isc_status, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &isc_49, (short) sizeof(isc_50), (char*) isc_50);
		isc_vtov ((const char*) save_base_field, (char*) isc_51.isc_52, 32);
		isc_vtov ((const char*) save_view_name, (char*) isc_51.isc_53, 32);
		isc_51.isc_54 = save_view_context;
		if (isc_49)
		   {
                   isc_start_and_send (isc_status, (FB_API_HANDLE*) &isc_49, (FB_API_HANDLE*) &gds_trans, (short) 0, (short) 66, &isc_51, (short) 0);
		   }
		if (!isc_status [1]) {
		while (1)
		   {
                   isc_receive (isc_status, (FB_API_HANDLE*) &isc_49, (short) 1, (short) 74, &isc_55, (short) 0);
		   if (!isc_55.isc_58 || isc_status [1]) break;

			found = true;
			if (/*NEWRFR.RDB$BASE_FIELD.NULL*/
			    isc_55.isc_62)
			{
				if (!/*NEWRFR.RDB$NULL_FLAG.NULL*/
				     isc_55.isc_60 && /*NEWRFR.RDB$NULL_FLAG*/
    isc_55.isc_61 == 1)
					null_flag = false;

				done = true;
			}
			else
			{
				strcpy (save_view_name, /*NEWRFR.RDB$RELATION_NAME*/
							isc_55.isc_56);
				save_view_context = /*NEWRFR.RDB$VIEW_CONTEXT*/
						    isc_55.isc_59;
				strcpy (save_base_field, /*NEWRFR.RDB$BASE_FIELD*/
							 isc_55.isc_57);
			}
		/*END_FOR*/
		   }
		   };
		/*ON_ERROR*/
		if (isc_status [1])
		   {
			error = true;
		/*END_ERROR;*/
		   }
		}

		if (!found && ENCODE_ODS(isqlGlob.major_ods, isqlGlob.minor_ods) >= ODS_11_2)
		{
			/*FOR FIRST 1
				VR IN RDB$VIEW_RELATIONS
				CROSS NEWPP IN RDB$PROCEDURE_PARAMETERS WITH
				VR.RDB$VIEW_NAME EQ save_view_name AND
				VR.RDB$VIEW_CONTEXT EQ save_view_context AND
				NEWPP.RDB$PROCEDURE_NAME = VR.RDB$RELATION_NAME AND
				NEWPP.RDB$PARAMETER_NAME = save_base_field AND
				NEWPP.RDB$PARAMETER_TYPE = 1*/
			{
                        if (!isc_39)
                           isc_compile_request2 (isc_status, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &isc_39, (short) sizeof(isc_40), (char*) isc_40);
			isc_vtov ((const char*) save_base_field, (char*) isc_41.isc_42, 32);
			isc_vtov ((const char*) save_view_name, (char*) isc_41.isc_43, 32);
			isc_41.isc_44 = save_view_context;
			if (isc_39)
			   {
                           isc_start_and_send (isc_status, (FB_API_HANDLE*) &isc_39, (FB_API_HANDLE*) &gds_trans, (short) 0, (short) 66, &isc_41, (short) 0);
			   }
			if (!isc_status [1]) {
			while (1)
			   {
                           isc_receive (isc_status, (FB_API_HANDLE*) &isc_39, (short) 1, (short) 6, &isc_45, (short) 0);
			   if (!isc_45.isc_46 || isc_status [1]) break; // output param

				found = true;
				if (!/*NEWPP.RDB$NULL_FLAG.NULL*/
				     isc_45.isc_47 && /*NEWPP.RDB$NULL_FLAG*/
    isc_45.isc_48 == 1)
					null_flag = false;

				done = true;
			/*END_FOR*/
			   }
			   };
			/*ON_ERROR*/
			if (isc_status [1])
			   {
				error = true;
			/*END_ERROR;*/
			   }
			}
		}
		if (!found)
			error = true;
	}

	// The error shouldn't be masked here. It should be propagated.
	return null_flag;
}

bool ISQL_get_null_flag(const TEXT* rel_name,
						TEXT* field_name)
{
   struct isc_22_struct {
          short isc_23;	/* isc_utility */
          short isc_24;	/* RDB$NULL_FLAG */
   } isc_22;
   struct isc_20_struct {
          char  isc_21 [32];	/* RDB$FIELD_NAME */
   } isc_20;
   struct isc_30_struct {
          char  isc_31 [32];	/* RDB$BASE_FIELD */
          short isc_32;	/* isc_utility */
          short isc_33;	/* RDB$VIEW_CONTEXT */
          short isc_34;	/* gds__null_flag */
          short isc_35;	/* RDB$NULL_FLAG */
          short isc_36;	/* gds__null_flag */
          short isc_37;	/* gds__null_flag */
          short isc_38;	/* RDB$NULL_FLAG */
   } isc_30;
   struct isc_27_struct {
          char  isc_28 [32];	/* RDB$FIELD_NAME */
          char  isc_29 [32];	/* RDB$RELATION_NAME */
   } isc_27;
/**************************************
 *
 *	I S Q L _ g e t _ n u l l _ f l a g
 *
 **************************************
 *
 *	Determine if a field has the null flag set.
 *	Look for either rdb$relation_fields or rdb$fields to be
 *	Set to 1 (NOT NULL), then this field cannot be null
 *	We are passed the relation name and the relation_field name
 *  	For domains, the relation name is null.
 **************************************/
	fb_utils::exact_name(field_name);

	bool null_flag = true;

	// Transaction for all frontend commands
	if (DB && !gds_trans)
	{
		if (isc_start_transaction(isc_status, &gds_trans, 1, &DB, 0, NULL))
		{
			ISQL_errmsg(isc_status);
			return false;
		}
	}

	if (rel_name)
	{
		/*FOR FLD IN RDB$FIELDS CROSS
			RFR IN RDB$RELATION_FIELDS WITH
			RFR.RDB$FIELD_SOURCE EQ FLD.RDB$FIELD_NAME AND
			RFR.RDB$RELATION_NAME EQ rel_name AND
			RFR.RDB$FIELD_NAME EQ field_name*/
		{
                if (!isc_25)
                   isc_compile_request2 (isc_status, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &isc_25, (short) sizeof(isc_26), (char*) isc_26);
		isc_vtov ((const char*) field_name, (char*) isc_27.isc_28, 32);
		isc_vtov ((const char*) rel_name, (char*) isc_27.isc_29, 32);
		if (isc_25)
		   {
                   isc_start_and_send (isc_status, (FB_API_HANDLE*) &isc_25, (FB_API_HANDLE*) &gds_trans, (short) 0, (short) 64, &isc_27, (short) 0);
		   }
		if (!isc_status [1]) {
		while (1)
		   {
                   isc_receive (isc_status, (FB_API_HANDLE*) &isc_25, (short) 1, (short) 46, &isc_30, (short) 0);
		   if (!isc_30.isc_32 || isc_status [1]) break;

			if (!/*FLD.RDB$NULL_FLAG.NULL*/
			     isc_30.isc_37 && /*FLD.RDB$NULL_FLAG*/
    isc_30.isc_38 == 1)
				null_flag = false;
			else
			{

				// If RDB$BASE_FIELD is not null then it is a view column

				if (/*RFR.RDB$BASE_FIELD.NULL*/
				    isc_30.isc_36)
				{

					// Simple column. Did user define it not null?

					if (!/*RFR.RDB$NULL_FLAG.NULL*/
					     isc_30.isc_34 && /*RFR.RDB$NULL_FLAG*/
    isc_30.isc_35 == 1)
						null_flag = false;
				}
				else
				{
					null_flag = ISQL_get_base_column_null_flag (rel_name,
						/*RFR.RDB$VIEW_CONTEXT*/
						isc_30.isc_33, /*RFR.RDB$BASE_FIELD*/
  isc_30.isc_31);
				}
			}

		/*END_FOR*/
		   }
		   };
		/*ON_ERROR*/
		if (isc_status [1])
		   {
			ISQL_errmsg(isc_status);
			return null_flag;
		/*END_ERROR;*/
		   }
		}
	}
	else
	{
		// Domains have only field entries to worry about
		/*FOR FLD IN RDB$FIELDS WITH
			FLD.RDB$FIELD_NAME EQ field_name*/
		{
                if (!isc_18)
                   isc_compile_request2 (isc_status, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &isc_18, (short) sizeof(isc_19), (char*) isc_19);
		isc_vtov ((const char*) field_name, (char*) isc_20.isc_21, 32);
		if (isc_18)
		   {
                   isc_start_and_send (isc_status, (FB_API_HANDLE*) &isc_18, (FB_API_HANDLE*) &gds_trans, (short) 0, (short) 32, &isc_20, (short) 0);
		   }
		if (!isc_status [1]) {
		while (1)
		   {
                   isc_receive (isc_status, (FB_API_HANDLE*) &isc_18, (short) 1, (short) 4, &isc_22, (short) 0);
		   if (!isc_22.isc_23 || isc_status [1]) break;

			if (/*FLD.RDB$NULL_FLAG*/
			    isc_22.isc_24 == 1)
				null_flag = false;

		/*END_FOR*/
		   }
		   };
		/*ON_ERROR*/
		if (isc_status [1])
		   {
			ISQL_errmsg(isc_status);
			return null_flag;
		/*END_ERROR;*/
		   }
		}
	}

	return null_flag;
}


void ISQL_disconnect_database(bool nQuietMode)
 {
/**************************************
 *
 *	I S Q L _ d i s c o n n e c t _ d a t a b a s e
 *
 **************************************
 *
 * Functional description
 *	Disconnect from the current database.  First commit work and then
 *	call isc_detach_database to detach from the database and then zero
 *	out the DB handle.
 *
 * Change from miroslavp on 2005.05.06
 * Quiet global variable is preserved in local variable and restored at the end.
 **************************************/

	const bool OriginalQuiet = Quiet;
	// Ignore error msgs during disconnect
	Quiet = nQuietMode;

	// If we were in a database, commit before proceeding
	if (DB && (M__trans || D__trans))
		end_trans();

	// Commit transaction that was started on behalf of the request
	// Don't worry about the error if one occurs it will be network
	// related and caught later.
	if (DB && gds_trans)
		isc_rollback_transaction(isc_status, &gds_trans);

	// If there is  current user statement, free it
	// I think option 2 is the right one (DSQL_drop), but who knows
	if (global_Stmt)
		isc_dsql_free_statement(isc_status, &global_Stmt, DSQL_drop);

	// Detach from old database
	if (DB) {
		isc_detach_database(isc_status, &DB);
	}

	// Restore original value of the flag
	Quiet = OriginalQuiet;

	// Zero database handle and transaction handles
	global_Stmt = 0;
	DB = 0;
	isqlGlob.global_Db_name[0] = '\0';
	D__trans = 0;
	M__trans = 0;
	gds_trans = 0;

	// CVC: If we aren't connected to a db anymore, then the db's dialect is reset.
	// This should fix SF Bug #910430.
	isqlGlob.db_SQL_dialect = 0;
	// BRS this is also needed to fix #910430.
	global_dialect_spoken = 0;
	return;
}


#ifdef NOT_USED_OR_REPLACED
bool ISQL_is_domain(const TEXT* field_name)
{
   struct isc_14_struct {
          char  isc_15 [32];	/* RDB$FIELD_NAME */
          short isc_16;	/* isc_utility */
          short isc_17;	/* RDB$SYSTEM_FLAG */
   } isc_14;
   struct isc_12_struct {
          char  isc_13 [32];	/* RDB$FIELD_NAME */
   } isc_12;
/**************************************
 *
 *	I S Q L _ i s _ d o m a i n
 *
 **************************************
 *
 *	Determine if a field in rdb$fields is a domain,
 *
 **************************************/
	bool is_domain = false;

	// Transaction for all frontend commands
	if (DB && !gds_trans)
	{
		if (isc_start_transaction(isc_status, &gds_trans, 1, &DB, 0, NULL))
		{
			ISQL_errmsg(isc_status);
			return false;
		}
	}

	/*FOR FLD IN RDB$FIELDS WITH
		FLD.RDB$FIELD_NAME EQ field_name*/
	{
        if (!isc_10)
           isc_compile_request2 (isc_status, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &isc_10, (short) sizeof(isc_11), (char*) isc_11);
	isc_vtov ((const char*) field_name, (char*) isc_12.isc_13, 32);
	if (isc_10)
	   {
           isc_start_and_send (isc_status, (FB_API_HANDLE*) &isc_10, (FB_API_HANDLE*) &gds_trans, (short) 0, (short) 32, &isc_12, (short) 0);
	   }
	if (!isc_status [1]) {
	while (1)
	   {
           isc_receive (isc_status, (FB_API_HANDLE*) &isc_10, (short) 1, (short) 36, &isc_14, (short) 0);
	   if (!isc_14.isc_16 || isc_status [1]) break;

		if (!(implicit_domain(/*FLD.RDB$FIELD_NAME*/
				      isc_14.isc_15) && /*FLD.RDB$SYSTEM_FLAG*/
     isc_14.isc_17 != 1))
		{
			is_domain = true;
		}

	/*END_FOR*/
	   }
	   };
	/*ON_ERROR*/
	if (isc_status [1])
	   {
		ISQL_errmsg(isc_status);
		return is_domain;
	/*END_ERROR;*/
	   }
	}

	return is_domain;
}
#endif


void ISQL_make_upper(TEXT* str)
{
/**************************************
 *
 *	I S Q L _ m a k e _ u p p e r
 *
 **************************************
 *
 *	Force the name of a metadata object to
 *	uppercase.
 *
 **************************************/
	if (!str)
		return;

	for (UCHAR* p = reinterpret_cast<UCHAR*>(str); *p; p++)
		*p = UPPER7(*p);
}


void ISQL_msg_get(USHORT number,
				  TEXT* msg,
				  const SafeArg& args)
{
/**************************************
 *
 *	I S Q L _ m s g _ g e t
 *
 **************************************
 *
 * Functional description
 *	Retrieve a message from the error file
 *
 **************************************/

	fb_msg_format(NULL, ISQL_MSG_FAC, number, MSG_LENGTH, msg, args);
}


void ISQL_msg_get(USHORT number,
				  USHORT size,
				  TEXT* msg,
				  const SafeArg& args)
{
/**************************************
 *
 *	I S Q L _ m s g _ g e t
 *
 **************************************
 *
 * Functional description
 *	Retrieve a message from the error file
 *
 **************************************/

	fb_msg_format(NULL, ISQL_MSG_FAC, number, size, msg, args);
}


void ISQL_print_validation(FILE* fp,
						   ISC_QUAD* blobid,
						   bool isComputedField,
						   FB_API_HANDLE trans)
{
/**************************************
 *
 *	I S Q L _ p r i n t _ v a l i d a t i o n
 *
 **************************************
 *
 * Functional description
 *	This does some minor syntax adjustmet for extracting
 *	validation blobs and computed fields.
 *	if it does not start with the word CHECK
 *	if this is a computed field blob, look for () or insert them.
 *	if flag == false, this is a validation clause,
 *	if flag == true, this is a computed field
 *
 **************************************/
	// Don't bother with null blobs

	if (blobid->gds_quad_high == 0)
		return;

	UserBlob blob(isc_status);
	if (!blob.open(DB, trans, *blobid))
	{
		ISQL_errmsg(isc_status);
		return;
	}

	bool issql = false;
	bool firsttime = true;
	TEXT buffer[BUFFER_LENGTH512];

	size_t length;
	while (blob.getSegment(sizeof(buffer) - 1, buffer, length))
	{
		buffer[length] = 0;
		const TEXT* p = buffer;
		if (isComputedField)
		{
			// computed field SQL syntax correction

			while (fb_isspace(*p))
				p++;
			if (*p == '(')
				issql = true;
		}
		else
		{
			// Validation SQL syntax correction

			while (fb_isspace(*p))
				p++;
			if (!fb_utils::strnicmp(p, "CHECK", 5))
				issql = true;
		}
		if (firsttime)
		{
			firsttime = false;
			if (!issql) {
				ISQL_printf2(fp, "%s ", (isComputedField ? "/* " : "("));
			}
		}

		ISQL_printf(fp, buffer);
	}
	if (!issql) {
		ISQL_printf2(fp, "%s", (isComputedField ? " */" : ")"));
	}

	if (isc_status[1] && isc_status[1] != isc_segstr_eof)
		ISQL_errmsg(isc_status);
}


void ISQL_printf(FILE* fp, const char* buffer)
{
/**************************************
 *
 *	I S Q L _ p r i n t f
 *
 **************************************
 *
 *	Centralized printing facility
 *
 **************************************/
	fprintf(fp, "%s", buffer);
	fflush(fp); // John's fix.
}


void ISQL_printf2(FILE* fp, const char* buffer, ...)
{
/**************************************
 *
 *	I S Q L _ p r i n t f 2
 *
 **************************************
 *
 *	Centralized printing facility, more flexible
 *
 **************************************/
	va_list args;
	va_start(args, buffer);
	vfprintf(fp, buffer, args);
	va_end(args);
	fflush(fp); // John's fix.
}


static processing_state add_row(TEXT* tabname)
{
/**************************************
 *
 *	a d d _ r o w
 *
 **************************************
 *
 * Functional description
 *	Allows interactive insert of row, prompting for every column
 *
 *	The technique is to describe a select query of select * from the table.
 *	Then generate an insert with ? in every position, using the same sqlda.
 *	It will allow edits of blobs, skip arrays and computed fields
 *
 *	Parameters:
 *	tabname -- Name of table to insert into
 *
 **************************************/
	if (!*tabname)
		return (ps_ERR);

	if (!Interactive)
		return (ps_ERR);

	if (Input_file)
		return ps_ERR;


	// There may have been a commit transaction before we got here

	if (!M__trans)
	{
		if (isc_start_transaction(isc_status, &M__trans, 1, &DB, 0, NULL))
		{
			ISQL_errmsg(isc_status);
			return FAIL;
		}
	}

	// Start default transaction for prepare
	if (!D__trans)
	{
		if (isc_start_transaction(isc_status, &D__trans, 1, &DB, 0, NULL))
		{
			ISQL_errmsg(isc_status);
			return FAIL;
		}
	}

	chop_at(tabname, QUOTEDLENGTH);
	if (tabname[0] != DBL_QUOTE)
		ISQL_make_upper(tabname);
	/*
	chop_at(tablename, WORDLENGTH);
	const bool delimited_yes = tablename[0] == DBL_QUOTE;
	TEXT tabname[QUOTEDLENGTH];
	if (isqlGlob.db_SQL_dialect > SQL_DIALECT_V6_TRANSITION && delimited_yes) {
		ISQL_copy_SQL_id(tablename, tabname, DBL_QUOTE);
	}
	else
	{
		strcpy(tabname, tablename);
		ISQL_make_upper(tabname);
	}
	*/

	// Query to obtain relation information
	SCHAR string[BUFFER_LENGTH120];
	sprintf(string, "SELECT * FROM %s", tabname);

	XSQLDA* sqlda = (XSQLDA*) ISQL_ALLOC((SLONG) (XSQLDA_LENGTH(20)));
	memset(sqlda, 0, XSQLDA_LENGTH(20));
	sqlda->version = SQLDA_VERSION1;
	sqlda->sqln = 20;
	if (isc_dsql_prepare(isc_status, &D__trans, &global_Stmt, 0, string,
						 isqlGlob.SQL_dialect, sqlda))
	{
		ISQL_errmsg(isc_status);
		if (sqlda)
			ISQL_FREE(sqlda);
		return (SKIP);
	}

	const SSHORT n_cols = sqlda->sqld;

	// Reallocate to the proper size if necessary

	if (sqlda->sqld > sqlda->sqln)
	{
		ISQL_FREE(sqlda);
		sqlda = (XSQLDA*) ISQL_ALLOC((SLONG) (XSQLDA_LENGTH(n_cols)));
		memset(sqlda, 0, XSQLDA_LENGTH(n_cols));
		sqlda->version = SQLDA_VERSION1;
		sqlda->sqld = sqlda->sqln = n_cols;

		//  We must re-describe the sqlda, no need to re-prepare

		if (isc_dsql_describe(isc_status, &global_Stmt, isqlGlob.SQL_dialect, sqlda))
		{
			ISQL_errmsg(isc_status);
			isc_dsql_free_statement(isc_status, &global_Stmt, DSQL_close);
			if (sqlda)
				ISQL_FREE(sqlda);
			return (FAIL);
		}
	}
	isc_dsql_free_statement(isc_status, &global_Stmt, DSQL_close);

	// Array for storing select/insert column mapping, colcheck sets it up
	SSHORT* colnumber = (SSHORT*) ISQL_ALLOC((SLONG) (n_cols * sizeof(SSHORT)));
	col_check(tabname, colnumber);

	// Create the new INSERT statement from the sqlda info

	SCHAR* insertstring = (SCHAR*) ISQL_ALLOC((SLONG) (40 + (QUOTEDLENGTH + 4) * n_cols));

	// There is a question mark for each column that's known to be updatable.

	sprintf(insertstring, "INSERT INTO %s (", tabname);

	SSHORT i_cols = 0;
	{ // scope
		char fldname[WORDLENGTH];
		char fldfixed[QUOTEDLENGTH];
		int i = 0;
		for (const XSQLVAR* var = sqlda->sqlvar; i < n_cols; var++, i++)
		{
			// Skip columns that are computed
			if (colnumber[i] != -1)
			{
				fb_utils::copy_terminate(fldname, var->sqlname, var->sqlname_length + 1);
				const bool delimited_yes = fldname[0] == DBL_QUOTE;
				if (isqlGlob.db_SQL_dialect > SQL_DIALECT_V6_TRANSITION && delimited_yes)
				{
					ISQL_copy_SQL_id(fldname, fldfixed, DBL_QUOTE);
				}
				else
				{
					strcpy(fldfixed, fldname);
					ISQL_make_upper(fldfixed);
				}

				sprintf(insertstring + strlen(insertstring), "%s,", fldfixed);
				i_cols++;
			}
		}
	} // scope

	// Set i_cols to the number of insert columns
	insertstring[strlen(insertstring) - 1] = ')';
	strcat(insertstring, " VALUES (");

	for (int i = 0; i < n_cols; i++)
	{
		if (colnumber[i] != -1)
			strcat(insertstring, " ?,");
	}
	// Patch the last character in the string
	insertstring[strlen(insertstring) - 1] = ')';


	// Allocate INSERT sqlda and null indicators

	XSQLDA* isqlda = (XSQLDA*) ISQL_ALLOC((SLONG) (XSQLDA_LENGTH(i_cols)));

	// ISQL_ALLOC doesn't initialize the structure, so init everything
	// to avoid lots of problems.

	memset(isqlda, 0, XSQLDA_LENGTH(i_cols));
	isqlda->version = SQLDA_VERSION1;
	isqlda->sqln = isqlda->sqld = i_cols;

	SSHORT* nullind = (SSHORT*) ISQL_ALLOC((SLONG) (i_cols * sizeof(SSHORT)));

	// For each column, get the type, and length then prompt for a value
	// and scanf the resulting string into a buffer of the right type.

	SCHAR infobuf[BUFFER_LENGTH180];
	ISQL_msg_get(ADD_PROMPT, sizeof(infobuf), infobuf);
	STDERROUT(infobuf);

	TEXT msg[MSG_LENGTH];
	SCHAR name[WORDLENGTH];
	bool done = false;
	while (!done)
	{
		SSHORT* nullp = nullind;
		int i = 0;
		for (const XSQLVAR* var = sqlda->sqlvar; i < n_cols && !done; var++, i++)
		{
			if (colnumber[i] == -1)
				continue;

			// SQLDA for INSERT statement might have fewer columns
			const SSHORT j = colnumber[i];
			XSQLVAR* ivar = &isqlda->sqlvar[j];
			*ivar = *var;
			*nullp = 0;
			ivar->sqlind = nullp++;
			ivar->sqldata = NULL;
			{ // scope
				const SSHORT length = var->sqlname_length;
				fb_utils::copy_terminate(name, var->sqlname, length + 1);
			} // scope
			const SSHORT type = var->sqltype & ~1;
			const SSHORT nullable = var->sqltype & 1;

			// Prompt with the name and read the line

			switch (type)
			{
			case SQL_BLOB:
				ISQL_msg_get(BLOB_PROMPT, msg, SafeArg() << name);
				// Blob: %s, type 'edit' or filename to load>
				break;
			case SQL_TYPE_DATE:
				ISQL_msg_get(DATE_PROMPT, msg, SafeArg() << name);
				// Enter %s as Y/M/D>
				break;
			case SQL_TYPE_TIME:
				ISQL_msg_get(TIME_PROMPT, msg, SafeArg() << name);
				// Enter %s as H:M:S>
				break;
			case SQL_TIMESTAMP:
				ISQL_msg_get(TIMESTAMP_PROMPT, msg, SafeArg() << name);
				// Enter %s as Y/MON/D H:MIN:S[.MSEC]>
				break;
			default:
				ISQL_msg_get(NAME_PROMPT, msg, SafeArg() << name);
				// Enter %s>
				break;
			}

			// On blank line or EOF, break the two loops without doing an insert.

			readNextInputLine(msg);
			getColumn = -1; // We are bypassing getNextInputChar().
			if (lastInputLine == NULL || strlen(lastInputLine) == 0)
			{
				done = true;
				break;
			}

			const size_t length = strlen(lastInputLine);
			if (length > MAX_USHORT - 2)
			{
				ISQL_msg_get(BUFFER_OVERFLOW, msg);
				STDERROUT(msg);
				done = true;
				break;
			}
			STDERROUT("");

			// Convert first 4 chars to upper case for comparison
			SCHAR cmd[5];
			strncpy(cmd, lastInputLine, 4);
			cmd[4] = '\0';
			ISQL_make_upper(cmd);

			// If the user writes NULL, put a null in the column

			if (!strcmp(cmd, "NULL"))
				*ivar->sqlind = -1;
			else
			{
				*ivar->sqlind = 0;
				SLONG res;
				// Data types
				SSHORT* smallint;
				SLONG* integer;
				SINT64* pi64;
				SINT64 n;
				float* fvalue;
				double* dvalue;
				tm times;
				// Initialize the time structure
				memset(&times, 0, sizeof(times));
				char msec_str[5] = "";
				int msec = 0;
				ISC_QUAD* blobid;

				switch (type)
				{
				case SQL_BLOB:

					blobid = (ISC_QUAD*) ISQL_ALLOC((SLONG) sizeof(ISC_QUAD));

					if (!strcmp(cmd, "EDIT")) // If edit, we edit a temp file.
					{
						const Firebird::PathName ftmp = TempFile::create(SCRATCH);
						if (ftmp.empty())
							res = 0;
						else
						{
							gds__edit(ftmp.c_str(), 0);
							res = BLOB_text_load(blobid, DB, M__trans, ftmp.c_str());
							unlink(ftmp.c_str());
						}
					}
					else	// Otherwise just load the named file
					{
						// We can't be sure if it's a TEXT or BINARY file
						// being loaded.  As we aren't sure, we'll
						// do binary operation - this is NEW data in
						// the database, not updating existing info
						res = BLOB_load(blobid, DB, M__trans, lastInputLine);
					}

					if (!res)
					{
						STDERROUT("Unable to load file");
						done = true;
					}

					ivar->sqldata = (SCHAR*) blobid;
					break;

				case SQL_FLOAT:
					fvalue = (float*) ISQL_ALLOC(sizeof(float));
					if (sscanf(lastInputLine, "%g", fvalue) != 1)
					{
						STDERROUT("Input parsing problem");
						done = true;
					}
					ivar->sqldata = (SCHAR*) fvalue;
					break;

				case SQL_DOUBLE:
					dvalue = (double*) ISQL_ALLOC(sizeof(double));
					if (sscanf(lastInputLine, "%lg", dvalue) != 1)
					{
						STDERROUT("Input parsing problem");
						done = true;
					}
					ivar->sqldata = (SCHAR*) dvalue;
					break;

				case SQL_TYPE_DATE:
					if (3 != sscanf(lastInputLine, "%d/%d/%d", &times.tm_year,
									&times.tm_mon, &times.tm_mday) ||
						!check_date(times))
					{
						ISQL_msg_get(DATE_ERR, msg, SafeArg() << lastInputLine);
						STDERROUT(msg);	// Bad date %s\n
						done = true;
					}
					else
					{
						--times.tm_mon;
						times.tm_year -= 1900; // tm_year is 1900-based.
						ISC_DATE* date = (ISC_DATE*) ISQL_ALLOC((SLONG) ivar->sqllen);
						isc_encode_sql_date(&times, date);
						ivar->sqldata = (char*) date;
					}
					break;

				case SQL_TYPE_TIME:
					if (3 != sscanf(lastInputLine, "%d:%d:%d", &times.tm_hour,
									&times.tm_min, &times.tm_sec) ||
						!check_time(times))
					{
						ISQL_msg_get(TIME_ERR, msg, SafeArg() << lastInputLine);
						STDERROUT(msg);	// Bad time %s\n
						done = true;
					}
					else
					{
						ISC_TIME* vtime = (ISC_TIME*) ISQL_ALLOC((SLONG) ivar->sqllen);
						isc_encode_sql_time(&times, vtime);
						ivar->sqldata = (char*) vtime;
					}
					break;

				case SQL_TIMESTAMP:
					if (6 <= sscanf(lastInputLine, "%d/%d/%d %d:%d:%d.%4s",
									&times.tm_year, &times.tm_mon, &times.tm_mday,
									&times.tm_hour, &times.tm_min, &times.tm_sec,
									msec_str))
					{
						unsigned int count = 0;
						for (; count < 4; ++count)
						{
							if (fb_isdigit(msec_str[count]))
								msec = msec * 10 + msec_str[count] - '0';
							else
								break;
						}
						if (count != strlen(msec_str))
							done = true;
						else
						{
							while (count++ < 4)
								msec *= 10;
							done = !check_timestamp(times, msec);
						}
					}
					else
						done = true;
					if (done)
					{
						ISQL_msg_get(TIMESTAMP_ERR, msg, SafeArg() << lastInputLine);
						STDERROUT(msg);	// Bad timestamp %s\n
					}
					else
					{
						--times.tm_mon;
						times.tm_year -= 1900;
						ISC_TIMESTAMP* datetime =
							(ISC_TIMESTAMP*) ISQL_ALLOC((SLONG) ivar->sqllen);
						isc_encode_timestamp(&times, datetime);
						datetime->timestamp_time += msec;
						ivar->sqldata = (char*) datetime;
					}
					break;

				case SQL_TEXT:
					// Force all text to varying
					ivar->sqltype = SQL_VARYING + nullable;
					// Fall into:

				case SQL_VARYING:
					{
						vary* avary =
							(vary*) ISQL_ALLOC((SLONG) (length + sizeof(USHORT)));
						avary->vary_length = length;
						strncpy(avary->vary_string, lastInputLine, length);
						ivar->sqldata = (SCHAR*) avary;
						ivar->sqllen = length;
						break;
					}

				case SQL_SHORT:
				case SQL_LONG:
					if (type == SQL_SHORT)
					{
						smallint = (SSHORT*) ISQL_ALLOC(sizeof(SSHORT));
						ivar->sqldata = (SCHAR*) smallint;
					}
					else if (type == SQL_LONG)
					{
						integer = (SLONG*) ISQL_ALLOC(sizeof(SLONG));
						ivar->sqldata = (SCHAR*) integer;
					}
					// Fall into:

					// Fall through from SQL_SHORT and SQL_LONG ...
				case SQL_INT64:
					if (type == SQL_INT64)
					{
						pi64 = (SINT64*) ISQL_ALLOC(sizeof(SINT64));
						ivar->sqldata = (SCHAR*) pi64;
					}
					if (ivar->sqlscale < 0)
					{
						SSHORT lscale = 0;
						if (!get_numeric((UCHAR*) lastInputLine, length, &lscale, &n))
						{
							STDERROUT("Input parsing problem");
							done = true;
						}
						else
						{
							int dscale = ivar->sqlscale - lscale;
							if (dscale > 0)
							{
								TEXT err_buf[256];
								sprintf(err_buf,
										"input error: input scale %d exceeds the field's scale %d",
										-lscale, -ivar->sqlscale);
								STDERROUT(err_buf);
								done = true;
							}
							while (dscale++ < 0)
								n *= 10;
						}
					}
					// sscanf assumes a 64-bit integer target
					else if (sscanf(lastInputLine, "%" SQUADFORMAT, &n) != 1)
					{
						STDERROUT("Input parsing problem");
						done = true;
					}
					if (done)
						break; // Nothing else, we found an error.

					switch (type)
					{
					case SQL_INT64:
						*pi64 = n;
						break;
					case SQL_SHORT:
						*smallint = n;
						if (SINT64(*smallint) != n)
						{
							STDERROUT("Value too big");
							done = true;
						}
						break;
					case SQL_LONG:
						*integer = n;
						if (SINT64(*integer) != n)
						{
							STDERROUT("Value too big");
							done = true;
						}
					}
					break;

				default:
					done = true;
					STDERROUT("Unexpected SQLTYPE in add_row()");
					break;
				}
			}
		}
		if (!done)
		{
			// having completed all columns, try the insert statement with the isqlda

			if (isc_dsql_exec_immed2(isc_status, &DB, &M__trans, 0,
									 insertstring, isqlGlob.SQL_dialect, isqlda, NULL))
			{
				ISQL_errmsg(isc_status);
				break;
			}

			// Release each of the variables for next time
			XSQLVAR* ivar = isqlda->sqlvar;
			for (int i = 0; i < i_cols; i++, ivar++)
			{
				if (ivar->sqldata)
					ISQL_FREE(ivar->sqldata);
			}
		}
	}

	ISQL_FREE(insertstring);
	ISQL_FREE(sqlda);
	if (isqlda)
	{
		XSQLVAR* ivar = isqlda->sqlvar;
		for (int i = 0; i < i_cols; i++, ivar++)
		{
			if (ivar->sqldata)
				ISQL_FREE(ivar->sqldata);
		}

		ISQL_FREE(isqlda);
	}
	if (colnumber)
		ISQL_FREE(colnumber);
	if (nullind)
		ISQL_FREE(nullind);

	return (SKIP);
}


static processing_state blobedit(const TEXT* action, const TEXT* const* cmd)
{
/**************************************
 *
 *	b l o b e d i t
 *
 **************************************
 *
 * Functional description
 *	Edit the text blob indicated by blobid
 *
 *	Parameters:  cmd -- Array of words interpreted as file name
 *
 **************************************/
	if (!ISQL_dbcheck())
		return FAIL;

	if (*cmd[1] == 0)
		return ps_ERR;

	const TEXT* p = cmd[1];

	// Find the high and low values of the blob id
	ISC_QUAD blobid;
	sscanf(p, "%" xLONGFORMAT":%" xLONGFORMAT, &blobid.gds_quad_high, &blobid.gds_quad_low);

	// If it isn't an explicit blobedit, then do a dump. Since this is a
	// user operation, put it on the M__trans handle.

	processing_state rc = SKIP;
	if (!strcmp(action, "BLOBVIEW"))
		BLOB_edit(&blobid, DB, M__trans, "blob");
	else if ((!strcmp(action, "BLOBDUMP")) && (*cmd[2]))
	{
		// If this is a blobdump, make sure there is a file name
		// We can't be sure if the BLOB is TEXT or BINARY data,
		// as we're not sure, we'll dump it in BINARY mode.
		TEXT path[MAXPATHLEN];
		strip_quotes(cmd[2], path);
		if (!BLOB_dump(&blobid, DB, M__trans, path))
		{
			TEXT errbuf[MSG_LENGTH];
			ISQL_msg_get(FILE_OPEN_ERR, errbuf, SafeArg() << path);
			STDERROUT(errbuf);
			rc = ps_ERR;
		}
	}
	else
		rc = ps_ERR;

	return rc;
}


// *******************************
// b u l k _ i n s e r t _ h a c k
// *******************************
// Primitive processing for prepared insertions. Invocation is
// SET BULK_INSERT <insert_statement>
// (val1, ..., valN)
// (val1, ..., valN)
// Finish with an empty line or anything different than an opening parenthesis.
// For example, STOP may be explicit, but any word without '(' will do the trick.
// Tuples must go in a single line. Only quoted strings can span more than one line.
// Added a very visible +++ to put other parameters of the same row in another line.
// Use +++ after a comma and continue in the next line. No comments in the middle.
// Only single quote accepted. Strings without special characters can go unquoted.
// Use the default question mark (?) to designate parameters.
// Single line comments are recognized only in the first column on a separate line
// and they can only appear before or after full tuples (not between multi-line tuples).
// The command COMMIT or COMMIT WORK can appear only in the first column in a
// single line and it will be recognized. We do not check if there's more text
// in the same line, thus it can be the terminator or random garbage.
// Two commas are invalid as empty/NULL value, use NULL or the appropriate "empty" value
// for the data type instead (zero for numbers, two single quotes for string, etc.)
// The code needs review, cleanup and moving some messages to the msg db.
// Since the code is forgiving, it will check for double ')' in the row but you can
// write (val1, ..., valN); with the terminator at the end and garbage following it.
// Indeed, the terminator can be replaced by unfamiliar characters to the parser
// like #, $, %, etc., and anything can follow them, including random text.
// It's invalid to put +++ (signaling row continuation) after a tuple is complete.
// If you came here looking for robust parsing code, you're at the wrong place.
static processing_state bulk_insert_hack(const char* command, XSQLDA** sqldap)
{
	// Skip "SET BULK_INSERT" part.
	for (int j = 0; j < 2; ++j)
	{
		while (*command && *command != 0x20)
			++command;
		while (*command && *command == 0x20)
			++command;
	}
	if (!*command)
		return ps_ERR;

	processing_state ret = SKIP;

	// We received the address of the sqlda in case we realloc it
	XSQLDA* sqlda = *sqldap;

	// If somebody did a commit or rollback, we are out of a transaction

	if (!M__trans)
	{
		if (isc_start_transaction(isc_status, &M__trans, 1, &DB, 0, NULL))
			ISQL_errmsg(isc_status);
	}

	// No need to start a default transaction unless there is no current one

	if (Autocommit && !D__trans)
	{
		if (isc_start_transaction(isc_status, &D__trans, 1, &DB, 5, default_tpb))
		{
			ISQL_errmsg(isc_status);
		}
	}

	// If statistics are requested, then reserve them here

	perf64 perf_before;
	if (Stats)
		perf64_get_info(&DB, &perf_before);

	// Prepare the dynamic query stored in string.
	// But put this on the DDL transaction to get maximum visibility of
	// metadata.

	FB_API_HANDLE prepare_trans;
	if (Autocommit)
		prepare_trans = D__trans;
	else
		prepare_trans = M__trans;

	if (isc_dsql_prepare(isc_status, &prepare_trans, &global_Stmt, 0, command,
						 isqlGlob.SQL_dialect, sqlda))
	{
		if (isqlGlob.SQL_dialect == SQL_DIALECT_V6_TRANSITION && Input_file)
		{
			isqlGlob.printf("%s%s%s%s%s%s",
							NEWLINE,
							"**** Error preparing statement:",
							NEWLINE,
							NEWLINE,
							command,
							NEWLINE);
		}
		ISQL_errmsg(isc_status);
		return ps_ERR;
	}

	// check for warnings
	if (isc_status[2] == isc_arg_warning)
		ISQL_warning(isc_status);

	// Find out what kind of statement this is
	const int statement_type = process_request_type();
	if (!statement_type)
		return ps_ERR;

	if (statement_type != isc_info_sql_stmt_insert)
	{
		isqlGlob.printf("Only INSERT commands are accepted in bulk mode.%s", NEWLINE);
		return ps_ERR;
	}

//#ifdef DEV_BUILD
	if (Sqlda_display)
	{
		const bool can_have_input_parameters =
			statement_type == isc_info_sql_stmt_select ||
			statement_type == isc_info_sql_stmt_insert ||
			statement_type == isc_info_sql_stmt_update ||
			statement_type == isc_info_sql_stmt_delete ||
			statement_type == isc_info_sql_stmt_exec_procedure ||
			statement_type == isc_info_sql_stmt_select_for_upd;

		if (print_sqlda_input(can_have_input_parameters) == ps_ERR)
			return ps_ERR;
	}
//#endif // DEV_BUILD

	if (isc_dsql_describe_bind(isc_status, &global_Stmt, isqlGlob.SQL_dialect, sqlda))
	{
		ISQL_errmsg(isc_status);
		return ps_ERR;
	}

	// check for warnings
	if (isc_status[2] == isc_arg_warning)
		ISQL_warning(isc_status);

	if (!sqlda->sqld) // No input parameters, doesn't make sense for bulk insertions.
	{
		isqlGlob.printf("There must be at least one parameter in the statement.%s", NEWLINE);
		return ps_ERR;
	}

	if (sqlda->sqld > sqlda->sqln)
	{
		const USHORT n_columns = sqlda->sqld;
		ISQL_FREE(sqlda);
		*sqldap = sqlda = (XSQLDA*) ISQL_ALLOC((SLONG) (XSQLDA_LENGTH(n_columns)));
		memset(sqlda, 0, XSQLDA_LENGTH(n_columns));
		sqlda->version = SQLDA_VERSION1;
		sqlda->sqld = sqlda->sqln = n_columns;
		if (isc_dsql_describe_bind(isc_status, &global_Stmt, isqlGlob.SQL_dialect,
								   sqlda))
		{
			ISQL_errmsg(isc_status);
			return ps_ERR;
		}
		else if (isc_status[2] == isc_arg_warning)
			ISQL_warning(isc_status);
	}
	else
	{
		// Our dear process_statement doesn't clean the global sqlda after using it,
		// although it deallocates the sqldata in a single continuous memory section.
		const int n_cols = sqlda->sqln;
		XSQLVAR* ivar = sqlda->sqlvar;
		for (int i = 0; i < n_cols; ++i, ++ivar)
		{
			if (ivar->sqldata)
			{
				//ISQL_FREE(ivar->sqldata); Assume everything was deallocated.
				ivar->sqldata = 0;
			}
		}
	}

	// If PLAN is set, print out the plan now.

	if (Plan)
	{
		process_plan();
		if (Planonly)
			return ret;	// Do not execute.
	}

	const int n_cols = sqlda->sqld;
	SSHORT* nullind = new SSHORT[n_cols];
	PtrSentry<SSHORT> nullindSentry(nullind, true);

	int textFieldCounter = 0;
	for (int i = 0; i < n_cols; ++i)
	{
		switch (sqlda->sqlvar[i].sqltype & ~1)
		{
			case SQL_TEXT:
			case SQL_VARYING:
			    ++textFieldCounter;
			    break;
		}
	}

	// Used to optimize allocations to text fields.
	Extender* const stringBuffers = textFieldCounter ? new Extender[textFieldCounter] : 0;
	PtrSentry<Extender> stringbufSentry(stringBuffers, true);

	char bulk_prompt[BUFFER_LENGTH180] = "";
	ISQL_msg_get(BULK_PROMPT, sizeof(bulk_prompt), bulk_prompt);

	// Lookup the continuation prompt once
	TEXT con_prompt[MSG_LENGTH];
	ISQL_msg_get(CON_PROMPT, con_prompt);

	TEXT msg[MSG_LENGTH];
	bool commit_failureM = false;
	bool commit_failureD = false;
	bool done = false; // This is mostly "done with errors".
	while (!done)
	{
		if (bulk_insert_retriever(bulk_prompt)) // We finished normally, found EOF.
			break;

		// We only support single line comments and they should be at the first column,
		// no spaces before, etc. Go to read the next line.
		if (lastInputLine[0] == '-' && lastInputLine[1] == '-')
			continue;

		const char* insert = lastInputLine;
		while (*insert && *insert != '(')
			++insert;

		if (!*insert || *insert != '(')
		{
			// Again, we are strict, we need COMMIT or COMMIT WORK exactly at the beginning
			// and it will be the only command in the line (we don't care about the rest).
			if (fb_utils::strnicmp(lastInputLine, "COMMIT", 6) == 0 ||
				fb_utils::strnicmp(lastInputLine, "COMMIT WORK", 11) == 0)
			{
				if (isc_commit_transaction(isc_status, &M__trans) ||
					isc_start_transaction(isc_status, &M__trans, 1, &DB, 0, NULL))
				{
					ISQL_errmsg(isc_status);
					done = true;
					commit_failureM = M__trans ? true : false;
					break; // We failed to commit, quit the bulk insertion.
				}

				// CVC: Commit may fail with AUTO-DDL off and DDL changes rejected by DFW.
				if (D__trans && commit_trans(&D__trans) &&
					isc_start_transaction(isc_status, &D__trans, 1, &DB, 5, default_tpb))
				{
					ISQL_errmsg(isc_status);
					done = true;
					commit_failureD = D__trans ? true : false;
					break; // We failed to commit, quit the bulk insertion.
				}
				continue; // Go to read another line.
			}
			break; // For example, STOP or blank line not inside quoted string.
		}

		++insert;

		while (*insert == 0x20 || *insert == '\r' || *insert == '\t')
			++insert;

		if (!*insert) // Did we finish gracefully? The last line may be spaces or tab.
			break;

		const char* lastPos = insert;
		Extender extender; // Used only for multi-line tuples.

		SSHORT* nullp = nullind;
		for (int i = 0, textFieldIter = 0; i < n_cols && !done; ++i)
		{
			XSQLVAR* const ivar = &sqlda->sqlvar[i];
			*nullp = 0;
			ivar->sqlind = nullp++;
			//ivar->sqldata = NULL; Not here because we are reusing fixed size elements.

			const SSHORT type = ivar->sqltype & ~1;
			const SSHORT nullable = ivar->sqltype & 1;

			const char* finder = 0;
			int subtract = 0;
			const bool quote = *insert == SINGLE_QUOTE;
			if (quote)
			{
				subtract = 2; // Ignore the quotes at the beginning and end.
				bool finished = false; // Did the string close?
				++insert;
				while (*insert)
				{
					if (*insert == SINGLE_QUOTE)
					{
						++insert;
						if (*insert == SINGLE_QUOTE)
							++subtract;
						else
						{
							finished = true;
							break;
						}
					}
					++insert;
				}
				if (!finished)
				{
					extender.alloc(MAX_USHORT);
					extender.append(lastPos, insert - lastPos);
					while (!finished && !bulk_insert_retriever(con_prompt))
					{
						finder = lastInputLine;
						while (*finder)
						{
							if (*finder == SINGLE_QUOTE)
							{
								++finder;
								if (*finder == SINGLE_QUOTE)
									++subtract;
								else
								{
									finished = true;
									break;
								}
							}
							++finder;
						}
						size_t how_many = finder - lastInputLine;
						if (extender.append(lastInputLine, how_many) <= how_many)
						{
							done = true;
							ret = ps_ERR; // Do not delete, see if() below.
							STDERROUT("Failed to concatenate string");
							break;
						}
					}
					if (ret != ps_ERR && !finished)
					{
						done = true;
						STDERROUT("Failed to find end of quoted string");
					}
					if (done)
						break;

					lastPos = extender.getBuffer();
					insert = lastPos + extender.getUsed();
				}

				if (subtract > 2)
				{
					// Get rid of pairs of single quotes; they are a syntax artifact.
					const char* view = lastPos + 1;
					// We are working over lastInputLine or Extender's buffer, so "unconst" is safe.
					char* mover = const_cast<char*>(lastPos + 1);
					for (int counter = subtract; view < insert; ++view, ++mover)
					{
						*mover = *view;
						if (*view == SINGLE_QUOTE && view[1] == SINGLE_QUOTE && counter > 2)
						{
							++view;
							--counter;
						}
					}
				}
			}
			else
			{
				//if ((type == SQL_TEXT || type == SQL_VARYING || type == SQL_BLOB)
				//	&& fb_utils::strnicmp(insert, "NULL", 4))
				//{
				//	STDERROUT("Looking for unquoted string in:");
				//	STDERROUT(insert);
				//}

				for (bool go = true; go && *insert; ++insert)
				{
					switch (*insert)
					{
					case 0x20:
					//case '\n':
					case '\r':
					case '\t':
					case ',':
					case ')':
						go = false;
						--insert;
						break;
					}
				}
			}

			SCHAR cmd[5] = "";
			const USHORT length = insert - lastPos - subtract;
			if (!length && !quote)
			{
				// Go ahead with an aux var and see if we hit end of string.
				const char* ipeek = insert;
				//char s[3] = {ipeek[-1], ipeek[0] ? ipeek[0] : 'Z', 0};
				//STDERROUT(s);
				while (*ipeek == 0x20 || *ipeek == '\r' || *ipeek == '\t')
					++ipeek;

				//s[0] = ipeek[-1];
				//s[1] = ipeek[0] ? ipeek[0] : 'Z';
				//STDERROUT(s);
				switch (*ipeek)
				{
				case ',':
					STDERROUT("Fields with zero length only allowed in quoted strings");
					break;
				case ')':
					STDERROUT("Found ')' before reading all fields in a row");
					break;
				default:
					STDERROUT("Unterminated row, use +++ to put more parameters in another line");
				}

				done = true;
				continue;
			}

			if (quote)
				++lastPos;
			else
			{
				// Convert first 4 chars to upper case for comparison.
				strncpy(cmd, lastPos, 4);
				cmd[4] = '\0';
				ISQL_make_upper(cmd);
			}

			// If the user writes NULL, put a null in the column.

			if (!strcmp(cmd, "NULL"))
				*ivar->sqlind = -1;
			else
			{
				*ivar->sqlind = 0;
				// Data types
				SSHORT* smallint;
				SLONG* integer;
				SINT64* pi64;
				SINT64 n;
				float* fvalue;
				double* dvalue;
				tm times;
				// Initialize the time structure.
				memset(&times, 0, sizeof(times));
				char msec_str[5] = "";
				int msec = 0;
				ISC_QUAD* blobid;

				switch (type)
				{
				case SQL_BLOB:

					if (ivar->sqldata)
						blobid = (ISC_QUAD*) ivar->sqldata;
					else
						blobid = (ISC_QUAD*) ISQL_ALLOC(sizeof(ISC_QUAD));

					{ // scope
						UserBlob bs(isc_status);
						if (!bs.create(DB, M__trans, *blobid))
						{
							STDERROUT("Unable to create blob");
							ISQL_errmsg(isc_status);
							done = true;
							break; // Quit the switch()
						}

						size_t real_len;
						if (!bs.putData(length, lastPos, real_len) || real_len != length)
						{
							STDERROUT("Unable to write to blob");
							ISQL_errmsg(isc_status);
							done = true;
						}
						if (!bs.close())
						{
							STDERROUT("Unable to close blob");
							ISQL_errmsg(isc_status);
							done = true;
						}
					} // scope

					ivar->sqldata = (SCHAR*) blobid;
					break;

				case SQL_FLOAT:
					if (ivar->sqldata)
						fvalue = (float*) ivar->sqldata;
					else
						fvalue = (float*) ISQL_ALLOC(sizeof(float));

					if (sscanf(lastPos, "%g", fvalue) != 1)
					{
						STDERROUT("Input parsing problem in 'float' value");
						done = true;
					}
					ivar->sqldata = (SCHAR*) fvalue;
					break;

				case SQL_DOUBLE:
					if (ivar->sqldata)
						dvalue = (double*) ivar->sqldata;
					else
						dvalue = (double*) ISQL_ALLOC(sizeof(double));

					if (sscanf(lastPos, "%lg", dvalue) != 1)
					{
						STDERROUT("Input parsing problem in 'double' value");
						done = true;
					}
					ivar->sqldata = (SCHAR*) dvalue;
					break;

				case SQL_TYPE_DATE:
					if (3 != sscanf(lastPos, "%d-%d-%d", &times.tm_year,
									&times.tm_mon, &times.tm_mday) ||
						!check_date(times))
					{
						ISQL_msg_get(DATE_ERR, msg, SafeArg() << lastPos);
						STDERROUT(msg);	// Bad date %s\n
						done = true;
					}
					else
					{
						--times.tm_mon;
						times.tm_year -= 1900; // tm_year is 1900-based.
						ISC_DATE* date = (ISC_DATE*) ivar->sqldata;
						fb_assert(ivar->sqllen == sizeof(ISC_DATE));
						if (!date)
							date = (ISC_DATE*) ISQL_ALLOC((SLONG) ivar->sqllen);

						isc_encode_sql_date(&times, date);
						ivar->sqldata = (char*) date;
					}
					break;

				case SQL_TYPE_TIME:
					if (3 != sscanf(lastPos, "%d:%d:%d", &times.tm_hour,
									&times.tm_min, &times.tm_sec) ||
						!check_time(times))
					{
						ISQL_msg_get(TIME_ERR, msg, SafeArg() << lastPos);
						STDERROUT(msg);	// Bad time %s\n
						done = true;
					}
					else
					{
						ISC_TIME* vtime =  (ISC_TIME*) ivar->sqldata;
						fb_assert(ivar->sqllen == sizeof(ISC_TIME));
						if (!vtime)
							vtime = (ISC_TIME*) ISQL_ALLOC((SLONG) ivar->sqllen);

						isc_encode_sql_time(&times, vtime);
						ivar->sqldata = (char*) vtime;
					}
					break;

				case SQL_TIMESTAMP:
					if (6 <= sscanf(lastPos, "%d-%d-%d %d:%d:%d.%4s",
									&times.tm_year, &times.tm_mon, &times.tm_mday,
									&times.tm_hour, &times.tm_min, &times.tm_sec,
									msec_str))
					{
						unsigned int count = 0;
						for (; count < 4; ++count)
						{
							if (fb_isdigit(msec_str[count]))
								msec = msec * 10 + msec_str[count] - '0';
							else
								break;
						}
						if (count != strlen(msec_str))
							done = true;
						else
						{
							while (count++ < 4)
								msec *= 10;
							done = !check_timestamp(times, msec);
						}
					}
					else
						done = true;

					if (done)
					{
						ISQL_msg_get(TIMESTAMP_ERR, msg, SafeArg() << lastPos);
						STDERROUT(msg);	// Bad timestamp %s\n
					}
					else
					{
						--times.tm_mon;
						times.tm_year -= 1900;
						ISC_TIMESTAMP* datetime = (ISC_TIMESTAMP*) ivar->sqldata;
						fb_assert(ivar->sqllen == sizeof(ISC_TIMESTAMP));
						if (!datetime)
							datetime = (ISC_TIMESTAMP*) ISQL_ALLOC((SLONG) ivar->sqllen);

						isc_encode_timestamp(&times, datetime);
						datetime->timestamp_time += msec;
						ivar->sqldata = (char*) datetime;
					}
					break;

				case SQL_TEXT:
					// Force all text to varying
					ivar->sqltype = SQL_VARYING + nullable;
					// Fall into:

				case SQL_VARYING:
					{
						fb_assert(textFieldIter < textFieldCounter);
						Extender& field = stringBuffers[textFieldIter++];
						field.alloc(length + sizeof(USHORT));
						vary* avary = reinterpret_cast<vary*>(field.getBuffer());
						avary->vary_length = length;
						strncpy(avary->vary_string, lastPos, length);
						ivar->sqldata = (SCHAR*) avary;
						ivar->sqllen = length;
						break;
					}

				case SQL_SHORT:
				case SQL_LONG:
					if (type == SQL_SHORT)
					{
						if (ivar->sqldata)
							smallint = (SSHORT*) ivar->sqldata;
						else
							smallint = (SSHORT*) ISQL_ALLOC(sizeof(SSHORT));

						ivar->sqldata = (SCHAR*) smallint;
					}
					else if (type == SQL_LONG)
					{
						if (ivar->sqldata)
							integer = (SLONG*) ivar->sqldata;
						else
							integer = (SLONG*) ISQL_ALLOC(sizeof(SLONG));

						ivar->sqldata = (SCHAR*) integer;
					}
					// Fall into:

					// Fall through from SQL_SHORT and SQL_LONG ...
				case SQL_INT64:
					if (type == SQL_INT64)
					{
						if (ivar->sqldata)
							pi64 = (SINT64*) ivar->sqldata;
						else
							pi64 = (SINT64*) ISQL_ALLOC(sizeof(SINT64));

						ivar->sqldata = (SCHAR*) pi64;
					}
					if (ivar->sqlscale < 0)
					{
						SSHORT lscale = 0;
						if (!get_numeric((UCHAR*) lastPos, length, &lscale, &n))
						{
							STDERROUT("Input parsing problem in 'numeric' or 'decimal' value");
							done = true;
						}
						else
						{
							int dscale = ivar->sqlscale - lscale;
							if (dscale > 0)
							{
								TEXT err_buf[256];
								sprintf(err_buf,
										"Input error: input scale %d exceeds the field's scale %d",
										-lscale, -ivar->sqlscale);
								STDERROUT(err_buf);
								done = true;
							}
							while (dscale++ < 0)
								n *= 10;
						}
					}
					// sscanf assumes a 64-bit integer target
					else if (sscanf(lastPos, "%" SQUADFORMAT, &n) != 1)
					{
						STDERROUT("Input parsing problem in 'integer' value");
						done = true;
					}
					if (done)
						continue; // Nothing else, we found an error.

					switch (type)
					{
					case SQL_INT64:
						*pi64 = n;
						break;
					case SQL_SHORT:
						*smallint = n;
						if (SINT64(*smallint) != n)
						{
							STDERROUT("Integer value too big");
							done = true;
						}
						break;
					case SQL_LONG:
						*integer = n;
						if (SINT64(*integer) != n)
						{
							STDERROUT("Integer value too big");
							done = true;
						}
					}
					break;

				default:
					done = true;
					STDERROUT("Unexpected SQLTYPE in bulk_insert_hack()");
					break;
				}
			}

			if (done)
				break;

			// Restore "insert" pointer if we needed multi-line hack.
			if (finder)
				insert = finder;

			//while (*insert && *insert != ',' && *insert != ')')
			//	++insert;

			int comma_count = 0;
			int parenthesis_count = 0;
			for (bool go = true; go && *insert; ++insert)
			{
				switch (*insert)
				{
				case 0x20:
				//case '\n':
				case '\r':
				case '\t':
					break;
				case ',':
					if (++comma_count > 1)
					{
						go = false;
						--insert;
					}
					else if (i + 1 == n_cols) // We read all the row, no comma allowed!
					{
						go = false;
						done = true;
						STDERROUT("All fields were read but a comma was found");
					}
					break;
				case ')':
					if (++parenthesis_count > 1)
					{
						go = false;
						done = true;
						STDERROUT("Found more than one ')' in a row");
					}
					else if (i + 1 < n_cols) // We didn't read all fields but found closing parenthesis!
					{
						go = false;
						done = true;
						STDERROUT("Found ')' before reading all fields in a row");
					}
					break;
				default:
					go = false;
					--insert;
					break;
				}
			}

			// Allow line continuation after comma.
			if (!done && strncmp(insert, "+++", 3) == 0)
			{
				if (i + 1 == n_cols) // We read all the row, no continuation allowed!
				{
					done = true;
					STDERROUT("All fields were read but the continuation mark +++ was found");
				}
				else
				{
					if (bulk_insert_retriever(con_prompt))
					{
						done = true;
						STDERROUT("The continuation mark +++ was found but EOF was reached");
					}
					insert = lastInputLine;
				}
			}

			lastPos = insert;

		} // for (int i = 0;

		if (!done)
		{
			// Having completed all columns, try the insert statement with the sqlda.
			// This is a non-select DML statement or trans.

			if (isc_dsql_execute(isc_status, &M__trans, &global_Stmt, isqlGlob.SQL_dialect, sqlda))
			{
				ISQL_errmsg(isc_status);
				done = true;
			}

			// Check for warnings.
			if (isc_status[2] == isc_arg_warning)
				ISQL_warning(isc_status);
		}

		// No need to release each of the variables for next time.
		// Text is handled by an array of Extender and the others are fixed size
		// and therefore allocated only once.

	} // while (!done)

	if (done)
	{
		// Save whatever we were able to pump, except when the failure was the commit itself.
		if (M__trans)
		{
			if (commit_failureM)
				isc_rollback_transaction(isc_status, &M__trans);
			else
				commit_trans(&M__trans);
		}

		if (D__trans)
		{
			if (commit_failureD)
				isc_rollback_transaction(isc_status, &D__trans);
			else
				commit_trans(&D__trans);
		}

		TEXT errbuf[MSG_LENGTH];
		sprintf(errbuf, "Stopped prematurely due to error in line %d with text:",
			Filelist->Ifp().indev_aux);
		STDERROUT(errbuf);
		STDERROUT(lastInputLine);
		STDERROUT("Going to EOF");
		// Avoid thousands of errors. Assume the file is full of bulk insertion data.
		Filelist->gotoEof();

		ret = ps_ERR;
	}

	// Release each of the variables for next usage.
	// The text variables are deallocated separately.
	XSQLVAR* ivar = sqlda->sqlvar;
	for (int i = 0; i < n_cols; ++i, ++ivar)
	{
		if ((ivar->sqltype & ~1) != SQL_VARYING && ivar->sqldata)
		{
			ISQL_FREE(ivar->sqldata);
			ivar->sqldata = 0;
		}
	}

	// Explicit just to test.
	stringbufSentry.clean();
	nullindSentry.clean();

	// Get rid of the statement handle.
	isc_dsql_free_statement(isc_status, &global_Stmt, DSQL_close);

	// Statistics printed here upon request

	if (Stats)
		print_performance(&perf_before);

	return ret;
}


// *****************************************
// b u l k _ i n s e r t _ r e t r i e v e r
// *****************************************
// Helper to the previous bulk_insert_hack to get more lines of input.
// It returns true when it finds end of file or too long string (almost 64K).
static bool bulk_insert_retriever(const char* prompt)
{
	readNextInputLine(prompt);
	getColumn = -1; // We are bypassing getNextInputChar().

	// Stop at end of line only. Empty lines are valid in this mode if inside quoted strings
	// but this function cannot know whether we are in a string because it doesn't parse
	// but only retrieves another line. Therefore, assumes and empty line is valid.
	bool rc = false;
	if (lastInputLine == NULL)
		rc = true;
	else
	{
		size_t length = strlen(lastInputLine);
		/*
		if (length == 0)
			rc = true;
		*/
		if (length > MAX_USHORT - 2)
		{
			TEXT msg[MSG_LENGTH];
			ISQL_msg_get(BUFFER_OVERFLOW, msg);
			STDERROUT(msg);
			rc = true;
		}
	}

	return rc;
}


// *******************
// c h e c k _ d a t e
// *******************
// Check date as entered by the user, before adjustment (year - 1900, month - 1).
static bool check_date(const tm& times)
{
	const int y = times.tm_year;
	const int m = times.tm_mon;
	const int d = times.tm_mday;
	if (y < 1 || y > 4999)
		return false;
	if (m < 1 || m > 12)
		return false;
	const bool leap = y % 4 == 0 && y % 100 != 0 || y % 400 == 0;
	const int days[] = {0, 31, leap ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if (d < 1 || d > days[m])
		return false;
	return true;
}


// *******************
// c h e c k _ t i m e
// *******************
// Check time is in range.
static bool check_time(const tm& times)
{
	if (times.tm_hour < 0 || times.tm_hour > 23)
		return false;
	if (times.tm_min < 0 || times.tm_min > 59)
		return false;
	if (times.tm_sec < 0 || times.tm_sec > 59)
		return false;
	return true;
}


// ******************************
// c h e c k _ t i m e s t a  m p
// ******************************
// Check both date and time according to the previous functions
// and also check milliseconds range.
static bool check_timestamp(const tm& times, const int msec)
{
	return check_date(times) && check_time(times) && msec >= 0 && msec <= 9999;
}


// *************
// c h o p _ a t
// *************
// Simply ensure a given string argument fits in a size, terminator included.
static size_t chop_at(char target[], const size_t size)
{
	size_t len = strlen(target);
	if (len >= size)
	{
		len = size - 1;
		target[len] = 0;
	}
	return len;
}


static void col_check(const TEXT* tabname, SSHORT* colnumber)
{
   struct isc_4_struct {
          ISC_QUAD isc_5;	/* RDB$COMPUTED_BLR */
          short isc_6;	/* isc_utility */
          short isc_7;	/* gds__null_flag */
          short isc_8;	/* gds__null_flag */
          short isc_9;	/* RDB$DIMENSIONS */
   } isc_4;
   struct isc_2_struct {
          char  isc_3 [32];	/* RDB$RELATION_NAME */
   } isc_2;
/**************************************
 *
 *	c o l _ c h e c k
 *
 **************************************
 *
 *	Check for peculiarities not currently revealed by the SQLDA
 *	colnumber array records the mapping of select columns to insert
 *	columns which do not have an equivalent for array or computed cols.
 **************************************/

	// Transaction for all frontend commands
	if (DB && !gds_trans)
	{
		if (isc_start_transaction(isc_status, &gds_trans, 1, &DB, 0, NULL)) {
			ISQL_errmsg(isc_status);
			return;
		}
	}

	// Query to get array info and computed source not available in the sqlda
	int i = 0, j = 0;
	/*FOR F IN RDB$FIELDS CROSS
		R IN RDB$RELATION_FIELDS WITH
		F.RDB$FIELD_NAME = R.RDB$FIELD_SOURCE AND
		R.RDB$RELATION_NAME EQ tabname
		SORTED BY R.RDB$FIELD_POSITION, R.RDB$FIELD_NAME*/
	{
        if (!isc_0)
           isc_compile_request2 (isc_status, (FB_API_HANDLE*) &DB, (FB_API_HANDLE*) &isc_0, (short) sizeof(isc_1), (char*) isc_1);
	isc_vtov ((const char*) tabname, (char*) isc_2.isc_3, 32);
	if (isc_0)
	   {
           isc_start_and_send (isc_status, (FB_API_HANDLE*) &isc_0, (FB_API_HANDLE*) &gds_trans, (short) 0, (short) 32, &isc_2, (short) 0);
	   }
	if (!isc_status [1]) {
	while (1)
	   {
           isc_receive (isc_status, (FB_API_HANDLE*) &isc_0, (short) 1, (short) 16, &isc_4, (short) 0);
	   if (!isc_4.isc_6 || isc_status [1]) break;

		if ((!/*F.RDB$DIMENSIONS.NULL*/
		      isc_4.isc_8 && /*F.RDB$DIMENSIONS*/
    isc_4.isc_9) || (!/*F.RDB$COMPUTED_BLR.NULL*/
       isc_4.isc_7))
			colnumber[i] = -1;
		else
			colnumber[i] = j++;
		++i;
	/*END_FOR*/
	   }
	   };
	/*ON_ERROR*/
	if (isc_status [1])
	   {
		ISQL_errmsg(isc_status);
	/*END_ERROR;*/
	   }
	}
}


static void copy_str(TEXT** output_str,
					 const TEXT** input_str,
					 bool* done,
					 const TEXT* const str_begin,
					 const TEXT* const str_end,
					 literal_string_type str_flag)
{
/**************************************
 *
 *	c o p y _ s t r
 *
 **************************************
 *
 * Functional description
 *
 *	Copy and/or modify the data in the input buffer and stores it
 *	into output buffer.
 *
 *	For SINGLE_QUOTED_STRING, copy everything from the begining of the
 *		the input buffer to the end of the string constant
 *
 *	For DOUBLE_QUOTED_STRING, copy everything from the begining of the
 *		the input buffer to the begining of the string constant
 *
 *		a. replace double quote string delimited character with single
 *			quote
 *		b. if '""' is found with in the string constant, removes one
 *			extra double quote
 *		c. if a single quote is found with in the string constant,
 *			adds one more single quote
 *		d. replace ending double quote string delimited character with
 *			single quote
 *
 *	For NO_MORE_STRING, copy everything from the begining of the
 *		the input buffer to the end of input buffer
 *
 *	Input Arguments:
 *
 *		1. address of the output buffer
 *		2. address of the input buffer
 *		3. address of the end of processing flag
 *
 *	Output Arguments:
 *
 *		1. pointer to begining of the string constant
 *		2. pointer to ending   of the string constant
 *		3. string type
 *
 **************************************/
	int slen = 0;
	const TEXT* b1;

	TEXT* temp_str = *output_str;
	const TEXT* temp_local_stmt_str = *input_str;

	switch (str_flag)
	{
	case SINGLE_QUOTED_STRING:
		slen = str_end - temp_local_stmt_str;
		if (temp_str != temp_local_stmt_str)
		{
			strncpy(temp_str, temp_local_stmt_str, slen);
		}
		*output_str = temp_str + slen;
		*input_str = str_end;
		break;
	case DOUBLE_QUOTED_STRING:
		slen = str_begin - temp_local_stmt_str;
		if (temp_str != temp_local_stmt_str)
		{
			strncpy(temp_str, temp_local_stmt_str, slen);
		}
		temp_str += slen;
		*temp_str = SINGLE_QUOTE;
		temp_str++;
		b1 = str_begin + 1;
		while (b1 < str_end)
		{
			*temp_str = *b1;
			temp_str++;
			switch (*b1)
			{
			case SINGLE_QUOTE:
				*temp_str = SINGLE_QUOTE;
				temp_str++;
				break;
			case DBL_QUOTE:
				b1++;
				if (*b1 != DBL_QUOTE)
				{
					temp_str--;
					*temp_str = SINGLE_QUOTE;
					temp_str++;
					b1--;
				}
				break;
			default:
				break;
			}
			b1++;
		}
		*output_str = temp_str;
		*input_str = b1;
		break;
	case NO_MORE_STRING:
		slen = strlen(temp_local_stmt_str);
		if (temp_str != temp_local_stmt_str)
		{
			strncpy(temp_str, temp_local_stmt_str, slen);
		}
		temp_str += slen;
		*temp_str = '\0';
		*done = true;
		*output_str = temp_str;
		*input_str = temp_local_stmt_str + slen;
		break;
	default:
		break;
	}
}


static processing_state copy_table(TEXT* source,
						 TEXT* destination,
						 TEXT* otherdb)
{
/**************************************
 *
 *	c o p y _ t a b l e
 *
 **************************************
 *
 * Functional description
 *	Create a new table based on an existing one.
 *
 *	Parameters:  source -- name of source table
 *			 destination == name of newly created table
 *
 **************************************/
	if (!source[0] || !destination[0])
	{
		STDERROUT("Either source or destination tables are missing");
		return SKIP;
	}

	TEXT errbuf[MSG_LENGTH];

	// Call list_table with a temporary file, then hand that file to a
	// new version of isql

	FILE* const holdout = isqlGlob.Out;

	// If there is an alternate database, extract the domains
	const bool domain_flag = otherdb[0];

	const Firebird::PathName ftmp = TempFile::create(SCRATCH);
	isqlGlob.Out = fopen(ftmp.c_str(), "w+b");
	if (!isqlGlob.Out)
	{
		// If we can't open a temp file then bail

		ISQL_msg_get(FILE_OPEN_ERR, errbuf, SafeArg() << ftmp.c_str());
		STDERROUT(errbuf);
		Exit_value = FINI_ERROR;
		isqlGlob.Out = holdout;
		return END;
	}

	chop_at(source, QUOTEDLENGTH);
	if (source[0] != DBL_QUOTE)
		ISQL_make_upper(source);
	/*
	chop_at(source_tbl, WORDLENGTH);
	TEXT source[QUOTEDLENGTH];
	bool delimited_yes = source_tbl[0] == DBL_QUOTE;
	if (isqlGlob.db_SQL_dialect > SQL_DIALECT_V6_TRANSITION && delimited_yes) {
		ISQL_copy_SQL_id(source_tbl, source, DBL_QUOTE);
	}
	else
	{
		strcpy(source, source_tbl);
		ISQL_make_upper(source);
	}
	*/

	chop_at(destination, QUOTEDLENGTH);
	if (destination[0] != DBL_QUOTE)
		ISQL_make_upper(destination);
	/*
	chop_at(destination_tbl, WORDLENGTH);
	TEXT destination[QUOTEDLENGTH];
	delimited_yes = destination_tbl[0] == DBL_QUOTE;
	if (isqlGlob.db_SQL_dialect > SQL_DIALECT_V6_TRANSITION && delimited_yes) {
		ISQL_copy_SQL_id(destination_tbl, destination, DBL_QUOTE);
	}
	else
	{
		strcpy(destination, destination_tbl);
		ISQL_make_upper(destination);
	}
	*/

	if (EXTRACT_list_table(source, destination, domain_flag, -1))
	{
		ISQL_msg_get(NOT_FOUND_MSG, errbuf, SafeArg() << source);
		STDERROUT(errbuf);
		fclose(isqlGlob.Out);
	}
	else
	{
		fclose(isqlGlob.Out);

		// easy to make a copy in another database
		const TEXT* altdb = isqlGlob.global_Db_name;
		if (*otherdb)
			altdb = otherdb;
		TEXT cmd[MAXPATHLEN * 2 + 20];
		sprintf(cmd, "isql -q %s -i %s", altdb, ftmp.c_str());
		if (system(cmd))
		{
			ISQL_msg_get(COPY_ERR, errbuf, SafeArg() << destination << altdb);
			STDERROUT(errbuf);
		}
	}

	unlink(ftmp.c_str());
	isqlGlob.Out = holdout;

	return (SKIP);
}


static processing_state create_db(const TEXT* statement,
								TEXT* d_name)
{
/**************************************
 *
 *	c r e a t e _ d b
 *
 **************************************
 *
 * Functional description
 *	Intercept create database commands to
 *	adjust the DB and transaction handles
 *
 *	Parameters:  statement == the entire statement for processing.
 *
 * Note: SQL ROLE settings are ignored - the newly created database
 *	will not have any roles defined in it.
 *
 **************************************/
	processing_state ret = SKIP;

	// Disconnect from the database and cleanup
	ISQL_disconnect_database(false);

	SLONG arglength = strlen(statement) + strlen(isqlGlob.User) + strlen(Password) + 24;
	if (*ISQL_charset && strcmp(ISQL_charset, DEFCHARSET))
		arglength += strlen(ISQL_charset) + strlen(" SET NAMES \'\' ");
	TEXT* local_statement = (TEXT*) ISQL_ALLOC(arglength + 1);
	if (!local_statement)
		return (FAIL);
	TEXT usr[USER_LENGTH];
	TEXT psw[PASSWORD_LENGTH];

	strcpy(local_statement, statement);

	TEXT quote = DBL_QUOTE;
	const TEXT* p = NULL;

	// If there is a user parameter, we will set it into the create stmt.
	if (global_usr || global_psw ||
		(*ISQL_charset && strcmp(ISQL_charset, DEFCHARSET)))
	{
		strip_quotes(isqlGlob.User, usr);
		strip_quotes(Password, psw);
		// Look for the quotes on the database name and find the close quotes.
		// Use '"' first, if not successful try '''.
		// CVC: Again, this is wrong with embedded quotes.
		// Maybe ISQL_remove_and_unescape_quotes coupled with ISQL_copy_SQL_id could work.
		const TEXT* q = strchr(statement, quote);
		if (!q)
		{
			quote = SINGLE_QUOTE;
			q = strchr(statement, quote);
		}

		if (q)
		{
			// Set quote to match open quote
			quote = *q;
			q++;
			p = strchr(q, quote);
			if (p)
			{
				p++;
				const ptrdiff_t slen = p - statement;
				local_statement[slen] = '\0';
				if (isqlGlob.SQL_dialect == 1)
				{
					if (global_usr)
						sprintf(local_statement + strlen(local_statement),
								" USER \'%s\' ", usr);
					if (global_psw)
						sprintf(local_statement + strlen(local_statement),
								" PASSWORD \'%s\' ", psw);
					if (*ISQL_charset && strcmp(ISQL_charset, DEFCHARSET))
						sprintf(local_statement + strlen(local_statement),
								" SET NAMES \'%s\' ", ISQL_charset);
					sprintf(local_statement + strlen(local_statement), "%s",
							p);
				}
			}
		}
	}

	SLONG cnt = 0;
	if ((isqlGlob.SQL_dialect == 0) || (isqlGlob.SQL_dialect > 1))
	{
		const TEXT* q = strchr(statement, SINGLE_QUOTE);
		while (q)
		{
			cnt++;
			const TEXT* l = q + 1;
			q = strchr(l, SINGLE_QUOTE);
		}

		TEXT* new_local_statement = NULL;
		if (cnt > 0)
		{
			arglength = strlen(statement) +
				strlen(isqlGlob.User) + strlen(Password) + 24 + 2 * cnt;

			if (*ISQL_charset && strcmp(ISQL_charset, DEFCHARSET))
				arglength +=
					strlen(ISQL_charset) + strlen(" SET NAMES \'\' ");
			new_local_statement =
				(TEXT*) ISQL_ALLOC(arglength + 1);

			if (!new_local_statement)
			{
				ISQL_FREE(local_statement);
				return (FAIL);
			}
		}

		TEXT errbuf[MSG_LENGTH];

		TEXT* temp_str;
		if (new_local_statement)
			temp_str = new_local_statement;
		else
			temp_str = local_statement;

		const TEXT* temp_local_stmt_str = local_statement;
		bool done = false;
		while (!done)
		{
			const TEXT* str_begin = NULL;
			const TEXT* str_end = NULL;
			literal_string_type str_flag = INIT_STR_FLAG;
			get_str(temp_local_stmt_str, &str_begin, &str_end, &str_flag);
			if (str_flag == INCOMPLETE_STRING)
			{
				ISQL_msg_get(INCOMPLETE_STR, errbuf, SafeArg() << "create database statement");
				STDERROUT(errbuf);
				ISQL_FREE(local_statement);
				if (new_local_statement)
					ISQL_FREE(new_local_statement);
				return (FAIL);
			}
			copy_str(&temp_str, &temp_local_stmt_str, &done,
				 str_begin, str_end, str_flag);
		}

		if (new_local_statement)
			temp_str = new_local_statement;
		else
			temp_str = local_statement;

		if (global_usr)
			sprintf(temp_str + strlen(temp_str), " USER \'%s\' ", usr);

		if (global_psw)
			sprintf(temp_str + strlen(temp_str), " PASSWORD \'%s\' ", psw);

		if (*ISQL_charset && strcmp(ISQL_charset, DEFCHARSET))
			sprintf(temp_str + strlen(temp_str),
					" SET NAMES \'%s\' ", ISQL_charset);

		if (new_local_statement)
			temp_str = new_local_statement + strlen(new_local_statement);
		else
			temp_str = local_statement + strlen(local_statement);

		if (p && strlen(p) > 0)
		{
			temp_local_stmt_str = p;
			bool done2 = false;
			while (!done2)
			{
				const TEXT* str_begin = NULL;
				const TEXT* str_end = NULL;
				literal_string_type str_flag = INIT_STR_FLAG;
				get_str(temp_local_stmt_str, &str_begin, &str_end, &str_flag);
				if (str_flag == INCOMPLETE_STRING)
				{
					ISQL_msg_get(INCOMPLETE_STR, errbuf, SafeArg() << "create database statement");
					STDERROUT(errbuf);
					ISQL_FREE(local_statement);
					if (new_local_statement)
						ISQL_FREE(new_local_statement);
					return (FAIL);
				}
				copy_str(&temp_str, &temp_local_stmt_str, &done2,
					 str_begin, str_end, str_flag);
			}
		}

		if (new_local_statement)
		{
			ISQL_FREE(local_statement);
			local_statement = new_local_statement;
		}
	}

	// execute the create statement
	// If the isqlGlob.SQL_dialect is not set or set to 2, create the database
	// as a dialect 3 database.
	if (isqlGlob.SQL_dialect == 0 || isqlGlob.SQL_dialect == SQL_DIALECT_V6_TRANSITION)
	{
		if (isc_dsql_execute_immediate(isc_status, &DB, &M__trans, 0,
									   local_statement, requested_SQL_dialect,
									   NULL))
		{
			ISQL_errmsg(isc_status);
			if (!DB)
				ret = FAIL;
		}
	}
	else
	{
		if (isc_dsql_execute_immediate(isc_status, &DB, &M__trans, 0,
									   local_statement, isqlGlob.SQL_dialect, NULL))
		{
			ISQL_errmsg(isc_status);
			if (!DB)
				ret = FAIL;
		}
	}

	if (DB)
	{
		// Make it read owner name to display grantor correctly
		SHOW_read_owner();

		// No use in cancel when running non-end-user operators
		fb_cancel_operation(isc_status, &DB, fb_cancel_disable);

		// Load isqlGlob.global_Db_name with some value to show a successful attach

		// CVC: Someone may decide to play strange games with undocumented ability
		// to write crap between CREATE DATABASE and the db name, as described by
		// Helen on CORE-932. Let's see if we can discover the real db name.
		size_t db_name_len = MAXPATHLEN - 1;
		char temp_name[MAXPATHLEN];
		if (!strncmp(d_name, "/*", 2) || !strncmp(d_name, "--", 2))
		{
			const char* target = d_name[0] == '/' ? "/*" : "--";
			const char* comment = strstr(statement, target);

			while (true)
			{

				if (target[0] == '/')
				{
					// We don't accept recursive comment blocks, so looking for the next *./ is safe.
					comment = strstr(comment + 2, "*/");
					// What happens if the comment is not closed or quotes are spoiling the party?
					// Then the statement is ill-formed and will be rejected by the server anyway.
					if (comment)
					{
						comment += 2;
						while (*comment && fb_isspace(*comment))
							++comment;
					}
					else
						break; // no end of block comment, error, do nothing.
				}
				else
				{
					while (!strncmp(comment, "--", 2))
					{
						while (*comment && *comment != '\n')
							++comment;

						if (*comment == '\n')
							++comment; // skip the newline

						while (*comment && fb_isspace(*comment))
							++comment;
					}
				}


				if (!strncmp(comment, "/*", 2))
				{
					target = "/*";
					continue;
				}

				if (!strncmp(comment, "--", 2))
				{
					target = "--";
					continue;
				}

				// Does the advanced user want a charset specification before the name, too?
				// AFAIK, supported by IB and FB but dsql/preparse.cpp doesn't understand it.
				/*
				if (*comment == '_')
				{
					while (*comment && !fb_isspace(*comment) && *comment != '"' && *comment != '\'')
						++comment;

					while (*comment && fb_isspace(*comment))
						++comment;

					// Include the "continue" loops here for this to work in all cases.
					// Check that this happens only once, etc.
				}
				*/

				if (*comment == '"' || *comment == '\'')
				{
					for (const char& newquote = *comment++; *comment; ++comment)
					{
						if (*comment != newquote)
							continue;

						// If we find 'hello ''world''' this is valid.
						if (comment[1] == newquote)
						{
							comment += 2;
							if (!*comment)
								break;
						}

						if (*comment == newquote && comment[1] != newquote)
						{
							db_name_len = comment - &newquote + 1;
							if (db_name_len >= MAXPATHLEN)
								db_name_len = MAXPATHLEN - 1;

							d_name = temp_name;
							fb_utils::copy_terminate(d_name, &newquote, db_name_len + 1);
							break;
						}
					}
				}

				break;
			} // while (true)
		}

		chop_at(d_name, db_name_len + 1);
		strip_quotes(d_name, isqlGlob.global_Db_name);

		ISQL_get_version(true);

		// Start the user transaction

		if (!M__trans)
		{
			if (isc_start_transaction(isc_status, &M__trans, 1, &DB, 0, NULL))
				ISQL_errmsg(isc_status);
			if (D__trans)
				commit_trans(&D__trans);
			if (Autocommit && isc_start_transaction(isc_status, &D__trans, 1, &DB, 5, default_tpb))
				ISQL_errmsg(isc_status);
		}

		// Allocate a new user statement for the database

		if (isc_dsql_allocate_statement(isc_status, &DB, &global_Stmt))
		{
			ISQL_errmsg(isc_status);
			ret = ps_ERR;
		}
	}

	if (local_statement)
		ISQL_FREE(local_statement);
	return (ret);
}


static void do_isql()
{
/**************************************
 *
 *	d o _ i s q l
 *
 **************************************
 *
 * Functional description
 *	Process incoming SQL statements, using the global sqlda
 *
 **************************************/
	TEXT errbuf[MSG_LENGTH];

	// Initialized user transactions

	M__trans = 0;

#if defined(_MSC_VER) && _MSC_VER >= 1400 && _MSC_VER < 1900
	_set_output_format(_TWO_DIGIT_EXPONENT);
#endif

#ifdef WIN_NT
	SetConsoleCtrlHandler(query_abort, TRUE);
#else
	fb_shutdown_callback(0, query_abort, fb_shut_confirmation, 0);
#endif

	// Open database and start tansaction

	//
	// We will not execute this for now on WINDOWS. We are not prompting for
	// a database name, username and password. A connect statement has to be in
	// the file containing the script.

	newdb(isqlGlob.global_Db_name, isqlGlob.User, Password, global_numbufs, isqlGlob.Role, true);

	// If that failed or no Dbname was specified

	ISQL_dbcheck();

	// Set up SQLDA for SELECTs, allow up to 20 fields to be selected.
	//  If we subsequently encounter a query with more columns, we
	//  will realloc the sqlda as needed

	global_sqlda = (XSQLDA*) ISQL_ALLOC((SLONG) (XSQLDA_LENGTH(20)));
	memset(global_sqlda, 0, XSQLDA_LENGTH(20));
	global_sqlda->version = SQLDA_VERSION1;
	global_sqlda->sqln = 20;

	// The sqldap is the handle containing the current sqlda pointer

	global_sqldap = &global_sqlda;

	// Read statements and process them from Ifp until the ret
	// code tells us we are done

	Firebird::Array<TEXT> stmt;
	const size_t stmtLength = MAX_USHORT;
	TEXT* statement = stmt.getBuffer(stmtLength);
	processing_state ret;

	bool done = false;
	while (!done)
	{

		if (Abort_flag)
		{
			if (D__trans)
				isc_rollback_transaction(isc_status, &D__trans);
			if (M__trans)
				isc_rollback_transaction(isc_status, &M__trans);
			if (gds_trans)
				isc_rollback_transaction(isc_status, &gds_trans);

			// If there is current user statement, free it
			if (global_Stmt)
				isc_dsql_free_statement(isc_status, &global_Stmt, DSQL_drop);
			if (DB)
				isc_detach_database(isc_status, &DB);
			break;
		}

		if (Interrupt_flag)
		{
			// SIGINT caught in interactive mode
			Interrupt_flag = false;

			if (Input_file)
			{
				// close input files going back to stdin
				Filelist->clear(stdin);

				// should have two stdin in Filelist
				fb_assert(Filelist->count() == 2);

				Filelist->removeIntoIfp();
				Input_file = false;
			}
		}

		ret = get_statement(statement, stmtLength, sql_prompt);

		// If there is no database yet, remind us of the need to connect

		// But don't execute the statement

		if (!isqlGlob.global_Db_name[0] && (ret == CONT))
		{
			if (!Quiet)
			{
				ISQL_msg_get(NO_DB, errbuf);
				STDERROUT(errbuf);
			}
			if (!Interactive && BailOnError)
				ret = FAIL;
			else
				ret = SKIP;
		}

		switch (ret)
		{
		case CONT:
			if (process_statement(statement, global_sqldap) == ps_ERR)
			{
				Exit_value = FINI_ERROR;
				if (!Interactive && BailOnError)
					Abort_flag = true;
			}
			break;

		case END:
		case EOF:
		case EXIT:
			if (Abort_flag)
			{
				if (D__trans)
					isc_rollback_transaction(isc_status, &D__trans);
				if (M__trans)
					isc_rollback_transaction(isc_status, &M__trans);
				if (gds_trans)
					isc_rollback_transaction(isc_status, &gds_trans);
			}
			else
			{
				if (D__trans)
					commit_trans(&D__trans);
				if (M__trans)
					commit_trans(&M__trans);
				if (gds_trans)
					commit_trans(&gds_trans);
			}

			// If there is  current user statement, free it
			// I think DSQL_drop is the right one, but who knows
			if (global_Stmt)
				isc_dsql_free_statement(isc_status, &global_Stmt, DSQL_drop);
			if (DB) {
				isc_detach_database(isc_status, &DB);
			}
			done = true;
			break;

		case BACKOUT:
			if (D__trans)
				isc_rollback_transaction(isc_status, &D__trans);
			if (M__trans)
				isc_rollback_transaction(isc_status, &M__trans);
			if (gds_trans)
				isc_rollback_transaction(isc_status, &gds_trans);

			// If there is  current user statement, free it
			// I think DSQL_drop is the right one, but who knows
			if (global_Stmt)
				isc_dsql_free_statement(isc_status, &global_Stmt, DSQL_drop);
			if (DB) {
				isc_detach_database(isc_status, &DB);
			}
			done = true;
			break;

		case ERR_BUFFER_OVERFLOW:
			ISQL_msg_get(BUFFER_OVERFLOW, errbuf);
			STDERROUT(errbuf);

		case EXTRACT:
		case EXTRACTALL:
		default:
			// fb_assert (FALSE);  -- removed as finds too many problems
		case ps_ERR:
		case FAIL:
			Exit_value = FINI_ERROR;
			if (!Interactive && BailOnError)
				Abort_flag = true;
			break;

		case SKIP:
			break;
		}
	}

	global_Stmt = 0;
	DB = 0;
	isqlGlob.global_Db_name[0] = '\0';
	D__trans = 0;
	M__trans = 0;
	gds_trans = 0;

	InputDevices::indev& Ofp = Filelist->Ofp();
	// Does it have a valid Temp file pointer?
	if (Ofp.indev_fpointer)
		Ofp.drop();

	if (global_sqlda)
		ISQL_FREE(global_sqlda);
	// CVC: If we were halt by an error and Bail, we have pending cleanup.
	Filelist->clear();
	if (lastInputLine)
		free(lastInputLine);

	global_Cols.clear(); // The destructor would do anyway if we don't reach this point.
}


static processing_state drop_db()
{
/**************************************
 *
 *	d r o p _ d b
 *
 **************************************
 *
 * Functional description
 *	Drop the current database
 *
 **************************************/
	if (isqlGlob.global_Db_name[0])
	{
		if (isc_drop_database(isc_status, &DB))
		{
			ISQL_errmsg(isc_status);
			return (FAIL);
		}

		// If error occurred and drop database got aborted
		if (DB)
			return (FAIL);
	}
	else
		return (FAIL);

	// The database got dropped with or without errors

	M__trans = 0;
	gds_trans = 0;
	global_Stmt = 0;
	D__trans = 0;

	// CVC: If we aren't connected to a db anymore, then the db's dialect is reset.
	// This should fix SF Bug #910430.
	isqlGlob.db_SQL_dialect = 0;
	// BRS this is also needed to fix #910430.
	global_dialect_spoken = 0;

	// Zero database name

	isqlGlob.global_Db_name[0] = '\0';
	DB = 0;

	return (SKIP);
}


static processing_state edit(const TEXT* const* cmd)
{
/**************************************
 *
 *	e d i t
 *
 **************************************
 *
 * Functional description
 *	Edit the current file or named file
 *
 *	Parameters:  cmd -- Array of words interpreted as file name
 *	The result of calling this is to point the global input file
 *	pointer, Ifp, to the named file after editing or the tmp file.
 *
 **************************************/

	// Set up editing command for shell

	const TEXT* file = cmd[1];

	// If there is a file name specified, try to open it

	processing_state rc = SKIP;
	if (*file)
	{
		TEXT path[MAXPATHLEN];
		strip_quotes(file, path);
		FILE* fp = fopen(path, "r");
		if (fp)
		{
			// Push the current ifp on the indev

			Filelist->insertIfp();
			Filelist->Ifp().init(fp, path);
			gds__edit(path, 0);
			Input_file = true;
			getColumn = -1;
		}
		else
		{
			TEXT errbuf[MSG_LENGTH];
			ISQL_msg_get(FILE_OPEN_ERR, errbuf, SafeArg() << path);
			STDERROUT(errbuf);
			rc = ps_ERR;
		}
	}
	else
	{
		// No file given, edit the temp file

		Filelist->insertIfp();
		// Close the file, edit it, then reopen and read from the top
		InputDevices::indev& Ofp = Filelist->Ofp();

		if (!Ofp.indev_fpointer)
		{
			// File used to edit sessions
			const Firebird::PathName filename = TempFile::create(SCRATCH);
			const char* Tmpfile = filename.c_str();
			FILE* f = fopen(Tmpfile, "w+"); // It was w+b
			if (f)
			{
				Ofp.init(f, Tmpfile);
				Filelist->commandsToFile(f);
			}
			else
			{
				// If we can't open a temp file then bail
				TEXT errbuf[MSG_LENGTH];
				ISQL_msg_get(FILE_OPEN_ERR, errbuf, SafeArg() << Tmpfile);
				STDERROUT(errbuf);
				return ps_ERR;
			}
		}

		Ofp.close();
		const char* Tmpfile = Ofp.fileName();
		gds__edit(Tmpfile, 0);
		Ofp.init(fopen(Tmpfile, "r+"), Tmpfile); // We don't check for failure.
		Filelist->Ifp().init(Ofp);
		Input_file = true;
		getColumn = -1;
	}

	return rc;
}


static processing_state end_trans()
{
/**************************************
 *
 *	e n d _ t r a n s
 *
 **************************************
 *
 * Functional description
 *	Prompt the interactive user if there is an extant transaction and
 *	either commit or rollback
 *
 *	Called by newtrans, createdb, newdb;
 *	Returns success or failure.
 *
 **************************************/

	TEXT infobuf[BUFFER_LENGTH60];

	processing_state ret = CONT;
	// Give option of committing or rolling back before proceding unless
	// the last command was a commit or rollback

	if (M__trans)
	{
		if (Interactive)
		{
			ISQL_msg_get(COMMIT_PROMPT, sizeof(infobuf), infobuf);
			readNextInputLine(infobuf);
			getColumn = -1; // We are bypassing getNextInputChar().
			if (lastInputLine && isyesno(lastInputLine))
			{
				// check for Yes answer
				ISQL_msg_get(COMMIT_MSG, sizeof(infobuf), infobuf);
				STDERROUT(infobuf);
				if (DB && M__trans)
				{
					if (isc_commit_transaction(isc_status, &M__trans))
					{
						// Commit failed, so roll back anyway
						ISQL_errmsg(isc_status);
						ret = FAIL;
					}
				}
			}
			else
			{
				ISQL_msg_get(ROLLBACK_MSG, sizeof(infobuf), infobuf);
				STDERROUT(infobuf);
				if (DB && M__trans)
				{
					if (isc_rollback_transaction(isc_status, &M__trans))
					{
						ISQL_errmsg(isc_status);
						ret = FAIL;
					}
				}
			}
		}
		else
		{
			// No answer, just roll back by default

			if (DB && M__trans)
			{
				// For WISQL, we keep track of whether a commit is needed by setting a flag in the ISQLPB
				// structure.  This flag is set whenever a sql command is entered in the SQL input area or
				// if the user uses the create database dialog.  Because of this, this should only show up if
				// the user connects and then disconnects or if the user enters a SET TRANSACTION stat without
				// ever doing anything that would cause changes to the dictionary.
				ISQL_msg_get(ROLLBACK_MSG, sizeof(infobuf), infobuf);
				STDERROUT(infobuf);
				if (isc_rollback_transaction(isc_status, &M__trans))
				{
					ISQL_errmsg(isc_status);
					ret = FAIL;
				}
			}
		}
	}

	// Commit background transaction

	if (DB && D__trans)
	{
		if (isc_commit_transaction(isc_status, &D__trans))
		{
			ISQL_errmsg(isc_status);
			ret = FAIL;
		}
	}
	return ret;
}


static processing_state escape(const TEXT* cmd)
{
/**************************************
 *
 *	e s c a p e
 *
 **************************************
 *
 * Functional description
 *	Permit a shell escape to system call
 *
 *	Parameters:  cmd -- The command string with SHELL
 *
 **************************************/

	// Advance past the shell

	const TEXT* shellcmd = cmd;

	// Search past the 'shell' keyword
	shellcmd += strlen("shell");

	// Eat whitespace at beginning of command
	while (*shellcmd && fb_isspace(*shellcmd))
		shellcmd++;

#ifdef WIN_NT
	// MSDN says: You must explicitly flush (using fflush or _flushall)
	// or close any stream before calling system.
	// CVC: But that function defeats our possible several input streams opened.
	//_flushall();
	// Save Ofp position in case it's being used as input. See EDIT command.
	fpos_t OfpPos = 0;
	if (Filelist->Ofp().indev_fpointer)
		Filelist->Ofp().getPos(&OfpPos);
	fflush(NULL); // Flush only output buffers.
	const char* emptyCmd = "%ComSpec%";
#else
	const char* emptyCmd = "$SHELL";
#endif

	// If no command given just spawn a shell
	if (!*shellcmd)
		shellcmd = emptyCmd;

	int rc  = system(shellcmd);

#ifdef WIN_NT
	// If we are reading from the temp file, restore the read position because
	// it's opened in r+ mode in this case, that's R/W.
	if (Filelist->sameInputAndOutput())
		Filelist->Ofp().setPos(&OfpPos);
#endif

	return rc ? FAIL : SKIP;
}


static processing_state frontend(const TEXT* statement)
{
/**************************************
 *
 *	f r o n t e n d
 *
 **************************************
 *
 * Functional description
 *	Handle any frontend commands that start with
 *	show or set converting the string into an
 *	array of words parms, with MAX_TERMS words only.
 *
 *	Parameters: statement is the string typed by the user
 *
 **************************************/

	class FrontOptions : public OptionsBase
	{
	public:
		enum front_commands
		{
			show, add, copy,
#ifdef MU_ISQL
			pause,
#endif
			blobview, output, shell, set, create, drop, connect,
#ifdef SCROLLABLE_CURSORS
			next, prior, first, last, absolute, relative,
#endif
			edit, input, quit, exit, help,
#ifdef DEV_BUILD
			passthrough,
#endif
			wrong
		};
		FrontOptions(const optionsMap* inmap, size_t insize, int wrongval)
			: OptionsBase(inmap, insize, wrongval)
		{}
	};

	static const FrontOptions::optionsMap options[] =
	{
		{FrontOptions::show, "SHOW", 0},
		{FrontOptions::add, "ADD", 0},
		{FrontOptions::copy, "COPY", 0},
#ifdef MU_ISQL
		{FrontOptions::pause, "PAUSE", 0},
#endif
		{FrontOptions::blobview, "BLOBVIEW", 0},
		{FrontOptions::blobview, "BLOBDUMP", 0},
		//{FrontOptions::output, "OUT", },
		{FrontOptions::output, "OUTPUT", 3},
		{FrontOptions::shell, "SHELL", 0},
		{FrontOptions::set, "SET", 0},
		{FrontOptions::create, "CREATE", 0},
		{FrontOptions::drop, "DROP", 0},
		{FrontOptions::connect, "CONNECT", 0},
#ifdef SCROLLABLE_CURSORS
		{FrontOptions::next, "NEXT", 0},
		{FrontOptions::prior, "PRIOR", 0},
		{FrontOptions::first, "FIRST", 0},
		{FrontOptions::last, "LAST", 0},
		{FrontOptions::absolute, "ABSOLUTE", 0},
		{FrontOptions::relative, "RELATIVE", 0},
#endif
		{FrontOptions::edit, "EDIT", 0},
		//{FrontOptions::input, "IN", },
		{FrontOptions::input, "INPUT", 2},
		{FrontOptions::quit, "QUIT", 0},
		{FrontOptions::exit, "EXIT", 0},
		{FrontOptions::help, "?", 0},
		{FrontOptions::help, "HELP", 0}
#ifdef DEV_BUILD
		,
		{FrontOptions::passthrough, "PASSTHROUGH", 0}
#endif
	};

	TEXT errbuf[MSG_LENGTH];

	// Store the first NUM_TERMS words as they appear in parms, using blanks
	// to delimit.   Each word beyond a real word gets a null char
	// Shift parms to upper case, leaving original case in lparms
	typedef TEXT* isql_params_t[MAX_TERMS];
	isql_params_t parms, lparms;
	for (int iter = 0; iter < FB_NELEM(lparms); ++iter)
	{
		lparms[iter] = NULL;
		parms[iter] = NULL;
	}
	TEXT parm_defaults[MAX_TERMS][1];

	// Any whitespace and comments at the beginning are already swallowed by get_statement()

	// Set beginning of statement past comment
	const TEXT* const cmd = statement;
	if (!*cmd)
	{
		// In case any default transaction was started - commit it here
		if (gds_trans)
			commit_trans(&gds_trans);

		return SKIP;
	}

	frontend_load_parms(statement, parms, lparms, parm_defaults);

    char bad_dialect_buf[512];
	bool bad_dialect = false;

	// Look to see if the words (parms) match any known verbs. If nothing
	// matches then just hand the statement to process_statement
	processing_state ret = SKIP;
	const FrontOptions frontoptions(options, FB_NELEM(options), FrontOptions::wrong);
	switch (frontoptions.getCommand(parms[0]))
	{
	case FrontOptions::show:
		// Transaction for all frontend commands
		if (DB && !gds_trans)
		{
			if (isc_start_transaction(isc_status, &gds_trans, 1, &DB, 0, NULL))
			{
				ISQL_errmsg(isc_status);
				// Free the frontend command
				frontend_free_parms(parms, lparms, parm_defaults);
				return FAIL;
			}
		}

		ret = SHOW_metadata(parms, lparms);
		if (gds_trans)
			commit_trans(&gds_trans);
		break;

	case FrontOptions::add:
		if (DB && !gds_trans)
		{
			if (isc_start_transaction(isc_status, &gds_trans, 1, &DB, 0, NULL))
			{
				ISQL_errmsg(isc_status);
				// Free the frontend command
				frontend_free_parms(parms, lparms, parm_defaults);
				return FAIL;
			}
		}

		ret = add_row(lparms[1]);
		if (gds_trans)
			commit_trans(&gds_trans);
		break;

	case FrontOptions::copy:
		if (DB && !gds_trans)
		{
			if (isc_start_transaction(isc_status, &gds_trans, 1, &DB, 0, NULL))
			{
				ISQL_errmsg(isc_status);
				// Free the frontend command
				frontend_free_parms(parms, lparms, parm_defaults);
				return FAIL;
			}
		}

		ret = copy_table(lparms[1], lparms[2], lparms[3]);
		if (gds_trans)
			commit_trans(&gds_trans);
		break;

#ifdef MU_ISQL
	// This is code for QA Test bed Multiuser environment.
	case FrontOptions::pause:
		if (qa_mu_environment())
		{
			qa_mu_pause();
			ret = SKIP;
		}
		else
			ret = CONT;
		break;
#endif	// MU_ISQL

	case FrontOptions::blobview:
		ret = blobedit(parms[0], lparms);
		break;

	case FrontOptions::output:
		ret = newoutput(lparms[1]);
		break;

	case FrontOptions::shell:
		ret = escape(cmd);
		break;

	case FrontOptions::set:
		ret = frontend_set(cmd, parms, lparms, bad_dialect_buf, bad_dialect);
		break;

	case FrontOptions::create:
		if (!strcmp(parms[1], "DATABASE") || !strcmp(parms[1], "SCHEMA"))
			ret = create_db(cmd, lparms[2]);
		else
			ret = CONT;
		break;

	case FrontOptions::drop:
		if (!strcmp(parms[1], "DATABASE") || !strcmp(parms[1], "SCHEMA"))
			if (*parms[2])
				ret = ps_ERR;
			else
				ret = drop_db();
		else
			ret = CONT;
		break;

	case FrontOptions::connect:
		{
			const TEXT* psw = NULL;
			const TEXT* usr = NULL;
			const TEXT* sql_role_nm = NULL;
			int numbufs = 0;

			// if a parameter is given in the command more than once, the
			// last one will be used. The parameters can appear each any
			// order, but each must provide a value.

			ret = SKIP;
			for (int i = 2; i < (MAX_TERMS - 1);)
			{
				if (!strcmp(parms[i], "CACHE") && *lparms[i + 1])
				{
					char* err;
					long value = strtol(lparms[i + 1], &err, 10);
					if (*err || (value <= 0) || (value >= INT_MAX))
					{
						ret = ps_ERR;
						break;
					}
					numbufs = (int) value;
					i += 2;
				}
				else if (!strcmp(parms[i], "USER") && *lparms[i + 1])
				{
					usr = lparms[i + 1];
					i += 2;
				}
				else if (!strcmp(parms[i], "PASSWORD") && *lparms[i + 1])
				{
					psw = lparms[i + 1];
					i += 2;
				}
				else if (!strcmp(parms[i], "ROLE") && *lparms[i + 1])
				{
					sql_role_nm = lparms[i + 1];
					i += 2;
				}
				else if (*parms[i])
				{
					// Unrecognized option to CONNECT
					ret = ps_ERR;
					break;
				}
				else
					i++;
			}
			if (ret != ps_ERR)
				ret = newdb(lparms[1], usr, psw, numbufs, sql_role_nm, true);
		}
		break;

#ifdef SCROLLABLE_CURSORS
	// Perform scrolling within the currently active cursor

	case FrontOptions::next:
		fetch_offset = 1;
		fetch_direction = isc_fetch_next;
		ret = FETCH;
		break;

	case FrontOptions::prior:
		fetch_offset = 1;
		fetch_direction = isc_fetch_prior;
		ret = FETCH;
		break;

	case FrontOptions::first:
		fetch_offset = 1;
		fetch_direction = isc_fetch_first;
		ret = FETCH;
		break;

	case FrontOptions::last:
		fetch_offset = 1;
		fetch_direction = isc_fetch_last;
		ret = FETCH;
		break;

	case FrontOptions::absolute:
		fetch_direction = isc_fetch_absolute;
		fetch_offset = atoi(parms[1]);
		ret = FETCH;
		break;

	case FrontOptions::relative:
		fetch_direction = isc_fetch_relative;
		fetch_offset = atoi(parms[1]);
		ret = FETCH;
		break;
#endif

	case FrontOptions::edit:
		ret = edit(lparms);
		break;

	case FrontOptions::input:
		// CVC: Set by newinput() below only if successful.
		//Input_file = true;
		ret = newinput(lparms[1]);
		break;

	case FrontOptions::quit:
		ret = BACKOUT;
		break;

	case FrontOptions::exit:
		ret = EXIT;
		break;

	case FrontOptions::help:
		ret = help(parms[1]);
		break;

#ifdef DEV_BUILD
	case FrontOptions::passthrough:
		ret = passthrough(cmd + 11);
		break;
#endif

	default:						// Didn't match, it must be SQL
		ret = CONT;
		break;
	}

	// In case any default transaction was started - commit it here
	if (gds_trans)
		commit_trans(&gds_trans);

	// Free the frontend command
	frontend_free_parms(parms, lparms, parm_defaults);

	if (ret == ps_ERR)
	{
		if (bad_dialect)
			ISQL_msg_get(CMD_ERR, errbuf, SafeArg() << bad_dialect_buf);
		else
			ISQL_msg_get(CMD_ERR, errbuf, SafeArg() << cmd);
		STDERROUT(errbuf);
	}

	return ret;
}


static void frontend_free_parms(TEXT* parms[], TEXT* lparms[], TEXT parm_defaults[][1])
{
	for (int j = 0; j < MAX_TERMS; j++)
	{
		if (parms[j] && parms[j] != parm_defaults[j])
		{
			ISQL_FREE(parms[j]);
			ISQL_FREE(lparms[j]);
		}
	}
}


static void frontend_load_parms(const TEXT* p, TEXT* parms[], TEXT* lparms[],
	TEXT parm_defaults[][1])
{
	TEXT buffer[BUFFER_LENGTH256];

	for (int i = 0; i < MAX_TERMS; ++i)
	{
		if (!*p)
		{
			parms[i] = lparms[i] = parm_defaults[i];
			parm_defaults[i][0] = '\0';
			continue;
		}

	    bool role_found = false;
		TEXT* a = buffer;
		int j = 0;
		const bool quoted = *p == DBL_QUOTE || *p == SINGLE_QUOTE;
		if (quoted)
		{
			if (i > 0 && (!strcmp(parms[i - 1], "ROLE")))
				role_found = true;
			bool delimited_done = false;
			const TEXT end_quote = *p;
			j++;
			*a++ = *p++;
			// Allow a quoted string to have embedded spaces
			//  Prevent overflow
			while (*p && !delimited_done && j < BUFFER_LENGTH256 - 1)
			{
				if (*p == end_quote)
				{
					j++;
					*a++ = *p++;
					if (*p && *p == end_quote && j < BUFFER_LENGTH256 - 1)
					{
						j++;	// do not skip the escape quote here
						*a++ = *p++;
					}
					else
						delimited_done = true;
				}
				else
				{
					j++;
					*a++ = *p++;
				}
			}
			*a = '\0';
		}
		else
		{
			//  Prevent overflow. Do not copy the string (redundant).
			while (*p && !fb_isspace(*p) && j < BUFFER_LENGTH256 - 1)
			{
				j++;
				++p;
			}
		}
		fb_assert(!quoted || strlen(buffer) == size_t(j));
		const size_t length = quoted ? strlen(buffer) : j;
		parms[i] = (TEXT*) ISQL_ALLOC((SLONG) (length + 1));
		lparms[i] = (TEXT*) ISQL_ALLOC((SLONG) (length + 1));
		memcpy(parms[i], quoted ? buffer : p - j, length);
		parms[i][length] = 0;
		while (*p && fb_isspace(*p))
			p++;
		strcpy(lparms[i], parms[i]);
		if (!role_found)
			ISQL_make_upper(parms[i]);
	}
}


// ***********************
// f r o n t e n d _ s e t
// ***********************
// Validates and executes the SET {option {params}} command.
static processing_state frontend_set(const char* cmd, const char* const* parms,
	const char* const* lparms, char* const bad_dialect_buf, bool& bad_dialect)
{

	class SetOptions : public OptionsBase
	{
	public:
		enum set_commands
		{
			stat, count, list, plan, planonly, blobdisplay, echo, autoddl,
#ifdef SCROLLABLE_CURSORS
			autofetch,
#endif
			width, transaction, terminator, names, time,
//#ifdef DEV_BUILD
			sqlda_display,
//#endif
			sql, warning, generator, statistics, heading, bail,
			bulk_insert, rowcount, wrong
		};
		SetOptions(const optionsMap* inmap, size_t insize, int wrongval)
			: OptionsBase(inmap, insize, wrongval)
		{}
	};

	static const SetOptions::optionsMap options[] =
	{
		{SetOptions::stat, "STATS", 4},
		{SetOptions::count, "COUNT", 0},
		{SetOptions::list, "LIST", 0},
		{SetOptions::plan, "PLAN", 0},
		{SetOptions::planonly, "PLANONLY", 0},
		{SetOptions::blobdisplay, "BLOBDISPLAY", 4},
		{SetOptions::echo, "ECHO", 0},
		{SetOptions::autoddl, "AUTODDL", 4},
#ifdef SCROLLABLE_CURSORS
		{SetOptions::autofetch, "AUTOFETCH", 0},
#endif
		{SetOptions::width, "WIDTH", 0},
		{SetOptions::transaction, "TRANSACTION", 5},
		{SetOptions::terminator, "TERMINATOR", 4},
		{SetOptions::names, "NAMES", 0},
		{SetOptions::time, "TIME", 0},
//#ifdef DEV_BUILD
		{SetOptions::sqlda_display, "SQLDA_DISPLAY", 0},
//#endif
		{SetOptions::sql, "SQL", 0},
		{SetOptions::warning, "WARNINGS", 7},
		{SetOptions::warning, "WNG", 0},
		{SetOptions::generator, "GENERATOR", 0},
		{SetOptions::statistics, "STATISTICS", 0},
		{SetOptions::heading, "HEADING", 0},
		{SetOptions::bail, "BAIL", 0},
		{SetOptions::bulk_insert, "BULK_INSERT", 0},
		{SetOptions::rowcount, "ROWCOUNT", 0},
	};

	// Display current set options
	if (!*parms[1])
		return print_sets();

	processing_state ret = SKIP;
	const SetOptions setoptions(options, FB_NELEM(options), SetOptions::wrong);
	switch (setoptions.getCommand(parms[1]))
	{
	case SetOptions::stat:
		ret = do_set_command(parms[2], &Stats);
		break;

	case SetOptions::count:
		ret = do_set_command(parms[2], &Docount);
		break;

	case SetOptions::list:
		ret = do_set_command(parms[2], &List);
		break;

	case SetOptions::plan:
		ret = do_set_command(parms[2], &Plan);
		if (Planonly && !Plan)
			ret = do_set_command("OFF", &Planonly);
		break;

	case SetOptions::planonly:
		ret = do_set_command (parms[2], &Planonly);
		if (Planonly && !Plan)
		{
			// turn on plan
			ret = do_set_command ("ON", &Plan);
		}
		break;

	case SetOptions::blobdisplay:
		// No arg means turn off blob display
		if (!*parms[2] || !strcmp(parms[2], "OFF"))
			Doblob = NO_BLOBS;
		else if (!strcmp(parms[2], "ALL"))
			Doblob = ALL_BLOBS;
		else
			Doblob = atoi(parms[2]);
		break;

	case SetOptions::echo:
		ret = do_set_command(parms[2], &Echo);
		if (!Echo)
			ISQL_prompt("");
		break;

	case SetOptions::autoddl:
		ret = do_set_command(parms[2], &Autocommit);
		break;

#ifdef SCROLLABLE_CURSORS
	case SetOptions::autofetch:
		ret = do_set_command(parms[2], &Autofetch);
		break;
#endif

	case SetOptions::width:
		ret = newsize(parms[2][0] == '"' ? lparms[2] : parms[2], parms[3]);
		break;

	case SetOptions::transaction:
		ret = newtrans(cmd);
		break;

	case SetOptions::terminator:
		{
			const TEXT* a = (*lparms[2]) ? lparms[2] : DEFTERM;
			Termlen = strlen(a);
			if (Termlen < MAXTERM_SIZE)
			{
				strcpy(isqlGlob.global_Term, a);
			}
			else
			{
				Termlen = MAXTERM_SIZE - 1;
				fb_utils::copy_terminate(isqlGlob.global_Term, a, Termlen + 1);
			}
		}
		break;

	case SetOptions::names:
		if (!*parms[2])
		{
			const size_t lgth = strlen(DEFCHARSET);
			if (lgth < MAXCHARSET_SIZE)
				strcpy(ISQL_charset, DEFCHARSET);
			else
				fb_utils::copy_terminate(ISQL_charset, DEFCHARSET, MAXCHARSET_SIZE);
		}
		else
		{
			const size_t lgth = strlen(parms[2]);
			if (lgth < MAXCHARSET_SIZE)
				strcpy(ISQL_charset, parms[2]);
			else
				fb_utils::copy_terminate(ISQL_charset, parms[2], MAXCHARSET_SIZE);
		}
		break;

	case SetOptions::time:
		ret = do_set_command(parms[2], &Time_display);
		break;

//#ifdef DEV_BUILD
	case SetOptions::sqlda_display:
		ret = do_set_command(parms[2], &Sqlda_display);
		break;
//#endif // DEV_BUILD

	case SetOptions::sql:
		if (!strcmp(parms[2], "DIALECT"))
			ret = get_dialect(parms[3], bad_dialect_buf, bad_dialect);
		else
			ret = ps_ERR;
		break;

	case SetOptions::warning:
		ret = do_set_command (parms[2], &Warnings);
		break;

	case SetOptions::generator:
	case SetOptions::statistics:
		ret = CONT;
		break;

	case SetOptions::heading:
		ret = do_set_command(parms[2], &Heading);
		break;

	case SetOptions::bail:
		ret = do_set_command(parms[2], &BailOnError);
		break;

	case SetOptions::bulk_insert:
		if (*parms[2])
			ret = bulk_insert_hack(cmd, global_sqldap);
		else
			ret = ps_ERR;
		break;

	case SetOptions::rowcount:
		ret = newRowCount((*lparms[2]) ? lparms[2] : "0");
		break;

	default:
		{
			TEXT msg_string[MSG_LENGTH];
			ISQL_msg_get(VALID_OPTIONS, msg_string);
			isqlGlob.printf("%s\n", msg_string);
		}
		setoptions.showCommands(isqlGlob.Out);
		ret = ps_ERR;
		break;
	}

	return ret;
}


static processing_state do_set_command(const TEXT* parm, bool* global_flag)
{
/**************************************
 *
 *	d o _ s e t _ c o m m a n d
 *
 **************************************
 *
 * Functional description
 *	set the flag pointed to by global_flag
 *		to true or false.
 *	if parm is missing, toggle it
 *	if parm is "ON", set it to true
 *	if parm is "OFF", set it to false
 *
 **************************************/
	processing_state ret = SKIP;

	if (!*parm)
		*global_flag = !*global_flag;
	else if (!strcmp(parm, "ON"))
		*global_flag = true;
	else if (!strcmp(parm, "OFF"))
		*global_flag = false;
	else
		ret = ps_ERR;
	return (ret);
}


// *********************
// g e t _ d i a l e c t
// *********************
// Validates SET SQL DIALECT command according to the target db.
static processing_state get_dialect(const char* const dialect_str,
	char* const bad_dialect_buf, bool& bad_dialect)
{
	processing_state ret = SKIP;
	bool print_warning = false;
	const USHORT old_SQL_dialect = isqlGlob.SQL_dialect; // save the old SQL dialect
	if (dialect_str && (isqlGlob.SQL_dialect = atoi(dialect_str)))
	{
		if (isqlGlob.SQL_dialect < SQL_DIALECT_V5 ||
			isqlGlob.SQL_dialect > SQL_DIALECT_V6)
		{
			bad_dialect = true;
			sprintf(bad_dialect_buf, "%s%s",
					"invalid SQL dialect ", dialect_str);
			isqlGlob.SQL_dialect = old_SQL_dialect;	// restore SQL dialect
			ret = ps_ERR;
		}
		else
		{
			if (isqlGlob.major_ods)
			{
				if (isqlGlob.major_ods < ODS_VERSION10)
				{
					if (isqlGlob.SQL_dialect > SQL_DIALECT_V5)
					{
						if (global_dialect_spoken)
						{
							sprintf(bad_dialect_buf,
									"%s%d%s%s%s%d%s",
									"ERROR: Database SQL dialect ",
									global_dialect_spoken,
									" database does not accept Client SQL dialect ",
									dialect_str,
									" setting. Client SQL dialect still remains ",
									old_SQL_dialect, NEWLINE);
						}
						else
						{
							sprintf(bad_dialect_buf,
									"%s%s%s%s%s%s",
									"ERROR: Pre IB V6 database only speaks ",
									"Database SQL dialect 1 and ",
									"does not accept Client SQL dialect ",
									dialect_str,
									" setting. Client SQL dialect still remains 1.",
									NEWLINE);
						}
						isqlGlob.SQL_dialect = old_SQL_dialect;	// restore SQL dialect
						isqlGlob.prints(bad_dialect_buf);
					}
				}
				else
				{	// ODS 10 databases
					switch (global_dialect_spoken)
					{
					case SQL_DIALECT_V5:
						if (isqlGlob.SQL_dialect > SQL_DIALECT_V5)
						{
							if (SQL_DIALECT_V6_TRANSITION)
								Merge_stderr = true;
							print_warning = true;
						}
						break;
					case SQL_DIALECT_V6:
						if (isqlGlob.SQL_dialect == SQL_DIALECT_V5 ||
							isqlGlob.SQL_dialect == SQL_DIALECT_V6_TRANSITION)
						{
							if (SQL_DIALECT_V6_TRANSITION)
								Merge_stderr = true;
							print_warning = true;
						}
						break;
					default:
						break;
					}
					if (print_warning && Warnings)
					{
						//print_warning = false;
						sprintf(bad_dialect_buf, "%s%d%s%d%s%s",
								"WARNING: Client SQL dialect has been set to ",
								isqlGlob.SQL_dialect,
								" when connecting to Database SQL dialect ",
								global_dialect_spoken,
								" database. ", NEWLINE);
						isqlGlob.prints(bad_dialect_buf);
					}
				}
			}
		}
	}
	else
	{			// handle non numeric invalid "set sql dialect" case
		isqlGlob.SQL_dialect = old_SQL_dialect;	// restore SQL dialect
		bad_dialect = true;
		sprintf(bad_dialect_buf, "%s%s", "invalid SQL dialect ",
				dialect_str);
		ret = ps_ERR;
	}
	return ret;
}


static processing_state get_statement(TEXT* const statement,
							const size_t bufsize,
							const TEXT* statement_prompt)
{
/**************************************
 *
 *	g e t _ s t a t e m e n t
 *
 **************************************
 *
 * Functional description
 *	Get an SQL statement, or QUIT/EXIT command to process
 *
 *	Arguments:  Pointer to statement, size of statement_buffer and prompt msg.
 *
 **************************************/
	processing_state ret = CONT;

	// Lookup the continuation prompt once
	TEXT con_prompt[MSG_LENGTH];
	ISQL_msg_get(CON_PROMPT, con_prompt);

	if (Interactive && !Input_file || Echo) {
		ISQL_prompt(statement_prompt);
	}

	// Clear out statement buffer

	TEXT* p = statement;
	*p = '\0';

	// Set count of characters to zero

	size_t count = 0;
	size_t valuable_count = 0; // counter of valuable (non-space) chars
	size_t comment_pos = 0; // position of block comment start
	size_t non_comment_pos = 0; // position of char after block comment
	const size_t term_length = Termlen - 1; // additional variable for decreasing calculation

	Filelist->Ifp().indev_line = Filelist->Ifp().indev_aux;
	bool done = false;

	enum
	{
		normal,
		in_single_line_comment,
		in_block_comment,
		in_single_quoted_string,
		in_double_quoted_string
	} state = normal;

	while (!done)
	{
		SSHORT c = getNextInputChar();
		switch (c)
		{
		case EOF:
			// Go back to getc if we get interrupted by a signal.

			if (SYSCALL_INTERRUPTED(errno))
			{
				errno = 0;
				break;
			}

			// If there was something valuable before EOF - error
			if (valuable_count > 0)
			{
				TEXT errbuf[MSG_LENGTH];
				ISQL_msg_get(UNEXPECTED_EOF, errbuf);
				STDERROUT(errbuf);
				Exit_value = FINI_ERROR;
				ret = FAIL;
			}

			// If we hit EOF at the top of the flist, exit time

			if (Filelist->count() == 1)
				return FOUND_EOF;

			// If this is not tmpfile, close it

			if (!Filelist->sameInputAndOutput())
				Filelist->Ifp().close();

			// Reset to previous after other input

			Filelist->removeIntoIfp();

			if (Interactive && !Input_file || Echo)
				ISQL_prompt(statement_prompt);

			// CVC: Let's detect if we went back to the first level.
			if (Filelist->readingStdin())
			{
				Interactive = !stdin_redirected();
				Input_file = false;
			}

			// Try to convince the new routines to go back to previous file(s)
			// This should fix the INPUT bug introduced with editline.
			getColumn = -1;
			break;

		case '\n':
//		case '\0': // In particular with readline the \n is removed
			if (state == in_single_line_comment)
			{
				state = normal;
			}

			// Catch the help ? without a terminator
			if (*statement == '?' && count == 1)
			{
				c = 0;
				done = true;
				break;
			}

			// If in a comment, keep reading
			if (Interactive && !Input_file || Echo)
			{
				if (state == in_block_comment)
				{	//  Block comment prompt.
					ISQL_prompt("--> ");
				}
				else if (valuable_count == 0)
				{	// Ignore a series of nothing at the beginning
					ISQL_prompt(statement_prompt);
				}
				else
				{
					ISQL_prompt(con_prompt);
				}
			}

			break;

		case '-':
			// Could this the be start of a single-line comment.
			if (state == normal && count > 0 && p[-1] == '-')
			{
				state = in_single_line_comment;
				if (valuable_count == 1)
					valuable_count = 0;
			}
			break;

		case '*':
			// Could this the be start of a comment.  We can only look back,
			// not forward.
			// Ignore possibilities of a comment beginning inside
			// quoted strings.
			if (state == normal && count > 0 && p[-1] == '/' && count != non_comment_pos)
			{
				state = in_block_comment;
				comment_pos = count - 1;
				if (valuable_count == 1)
					valuable_count = 0;
			}
			break;

		case '/':
			// Perhaps this is the end of a comment.
			// Ignore possibilities of a comment ending inside
			// quoted strings.
			// Ignore things like /*/ since it isn't a block comment; only the start of it. Or end.
			if (state == in_block_comment && count > 2 && p[-1] == '*' && count > comment_pos + 2)
			{
				state = normal;
				non_comment_pos = count + 1; // mark start of non-comment to track this: /**/*
				valuable_count--; // This char is not valuable
			}
			break;

		case SINGLE_QUOTE:
			switch (state)
			{
				case normal:
					state = in_single_quoted_string;
					break;
				case in_single_quoted_string:
					state = normal;
					break;
			}
			break;

		case DBL_QUOTE:
			switch (state)
			{
				case normal:
					state = in_double_quoted_string;
					break;
				case in_double_quoted_string:
					state = normal;
					break;
			}
			break;


		default:
			if (state == normal && c == isqlGlob.global_Term[term_length] &&
				// one-char terminator or the beginning also match
				(Termlen == 1 ||
				 (valuable_count >= term_length &&
				  strncmp(p - term_length, isqlGlob.global_Term, term_length) == 0)))
			{
				c = 0;
				done = true;
				p -= term_length;
				count -= term_length;
			}
		}

		// Any non-space character is significant if not in comment
		if (state != in_block_comment &&
			state != in_single_line_comment &&
			!fb_isspace(c) && c != EOF)
		{
			valuable_count++;
			if (valuable_count == 1) // this is the first valuable char in stream
			{	// ignore all previous crap
				p = statement;
				count = 0;
				non_comment_pos = 0;
			}
		}

		*p++ = c;
		count++;

		if (count > bufsize && !done)
		{
				ret = ERR_BUFFER_OVERFLOW;

				// move some content to start of buffer just in case if
				// the overflow has splitted terminator
				count = MAX(Termlen, 2);
				memcpy(statement, p - count, count);
				p = statement + count;
				*p = '\0';
		}

	}

	// If this was a null statement, skip it
	if (ret == CONT && !*statement)
		ret = SKIP;

	if (ret == CONT)
		ret = frontend(statement);

	if (ret == CONT)
	{
		// Place each non frontend statement in the temp file if we are reading
		// from stdin.

		Filelist->saveCommand(statement, isqlGlob.global_Term);
	}

	return ret;
}


static void get_str(const TEXT* const input_str,
					const TEXT** str_begin,
					const TEXT** str_end,
					literal_string_type* str_flag)
{
/**************************************
 *
 *	g e t _ s t r
 *
 **************************************
 *
 * Functional description
 *
 *	Scanning a string constant from the input buffer. If it found, then
 *	passes back the starting address and ending address of the string
 *	constant. It also marks the type of string constant and passes back.
 *
 *	string type:
 *
 *		1. SINGLE_QUOTED_STRING --- string constant delimited by
 *						single quotes
 *		2. DOUBLE_QUOTED_STRING --- string constant delimited by
 *						double quotes
 *		3. NO_MORE_STRING	   --- no string constant was found
 *
 *	  4. INCOMPLETE_STRING	--- no matching ending quote
 *
 *	Input Arguments:
 *
 *		1. input buffer
 *
 *	Output Arguments:
 *
 *		1. pointer to the input buffer
 *		2. address to the begining of a string constant
 *		3. address to the ending   of a string constant
 *		4. address to the string flag
 *
 **************************************/
	const TEXT* b1 = strchr(input_str, SINGLE_QUOTE);
	const TEXT* b2 = strchr(input_str, DBL_QUOTE);
	if (!b1 && !b2)
		*str_flag = NO_MORE_STRING;
	else
	{
		if (b1 && !b2)
			*str_flag = SINGLE_QUOTED_STRING;
		else if (!b1 && b2)
		{
			*str_flag = DOUBLE_QUOTED_STRING;
			b1 = b2;
		}
		else if (b1 > b2)
		{
			*str_flag = DOUBLE_QUOTED_STRING;
			b1 = b2;
		}
		else
			*str_flag = SINGLE_QUOTED_STRING;

		*str_begin = b1;
		TEXT delimited_char = *b1;
		b1++;
		bool done = false;
		while (!done)
		{
			if (*b1 == delimited_char)
			{
				b1++;
				if (*b1 == delimited_char)
					b1++;
				else
				{
					done = true;
					*str_end = b1;
				}
			}
			else
				b1 = strchr(b1, delimited_char);

			/* In case there is no matching ending quote (single or double)
			   either because of mismatched quotes  or simply because user
			   didn't complete the quotes, we cannot find a valid string
			   at all. So we return a special value INCOMPLETE_STRING.
			   In this case str_end is not populated and hence copy_str
			   should not be called after get_str, but instead the caller
			   must process this error & return to it's calling routine
			   - Shailesh */
			if (b1 == NULL)
			{
				*str_flag = INCOMPLETE_STRING;
				done = true;
			}
		}
	}
}


void ISQL_get_version(bool call_by_create_db)
{
/**************************************
 *
 *	I S Q L _ g e t _ v e r s i o n
 *
 **************************************
 *
 * Functional description
 *	finds out if the database we just attached to is
 *	V4 or newer as well as other info.
 *
 **************************************/
	const UCHAR db_version_info[] =
	{
		isc_info_ods_version,
		isc_info_ods_minor_version,
		isc_info_db_sql_dialect,
		Version_info ? isc_info_firebird_version: isc_info_end,
		isc_info_end
	};
	/*
	   ** Each info item requested will return
	   **
	   **	 1 byte for the info item tag
	   **	 2 bytes for the length of the information that follows
	   **	 1 to 4 bytes of integer information
	   **
	   ** isc_info_end will not have a 2-byte length - which gives us
	   ** some padding in the buffer.
	 */

	// UCHAR buffer[sizeof(db_version_info) * (1 + 2 + 4)];

	// Now we are also getting the Firebird server version which is a
	// string the above calculation does not apply.	 NM 03-Oct-2001

	UCHAR buffer[PRINT_BUFFER_LENGTH];
	char bad_dialect_buf[BUFFER_LENGTH512];
	bool print_warning = false;

	global_dialect_spoken = 0;

	if (isc_database_info(isc_status, &DB, sizeof(db_version_info),
						  reinterpret_cast<const char*>(db_version_info),
						  sizeof(buffer), (SCHAR*) buffer))
	{
		ISQL_errmsg(isc_status);
		return;
	}

	const UCHAR* p = buffer;
	while (*p != isc_info_end && *p != isc_info_truncated && p < buffer + sizeof(buffer))
	{
		const UCHAR item = (UCHAR) *p++;
		const USHORT length = gds__vax_integer(p, sizeof(USHORT));
		p += sizeof(USHORT);
		switch (item)
		{
		case isc_info_ods_version:
			isqlGlob.major_ods = gds__vax_integer(p, length);
			break;
		case isc_info_ods_minor_version:
			isqlGlob.minor_ods = gds__vax_integer(p, length);
			break;
		case isc_info_db_sql_dialect:
			global_dialect_spoken = gds__vax_integer(p, length);
			if (isqlGlob.major_ods < ODS_VERSION10)
			{
				if (isqlGlob.SQL_dialect > SQL_DIALECT_V5 && Warnings)
				{
					isqlGlob.printf(NEWLINE);
					sprintf(bad_dialect_buf, "%s%s%s%d%s%s",
							"WARNING: Pre IB V6 database only speaks",
							" SQL dialect 1 and ",
							"does not accept Client SQL dialect ",
							isqlGlob.SQL_dialect,
							" . Client SQL dialect is reset to 1.", NEWLINE);
					isqlGlob.prints(bad_dialect_buf);
				}
			}
			else
			{				// ODS 10 databases

				switch (global_dialect_spoken)
				{
				case SQL_DIALECT_V5:
					if (isqlGlob.SQL_dialect > SQL_DIALECT_V5)
						print_warning = true;
					break;

				case SQL_DIALECT_V6:
					if (isqlGlob.SQL_dialect != 0 && isqlGlob.SQL_dialect < SQL_DIALECT_V6)
						print_warning = true;
					break;
				default:
					break;
				}
				if (print_warning && Warnings)
				{
					print_warning = false;
					isqlGlob.printf(NEWLINE);
					sprintf(bad_dialect_buf, "%s%d%s%d%s%s",
							"WARNING: This database speaks SQL dialect ",
							global_dialect_spoken,
							" but Client SQL dialect was set to ",
							isqlGlob.SQL_dialect, " .", NEWLINE);
					isqlGlob.prints(bad_dialect_buf);
				}
			}
			break;
		case isc_info_error:
			// Error indicates an option was not understood by the
			// remote server.
			if (*p == isc_info_firebird_version)
			{
				// must be an old or non Firebird server
				break;
			}
			if (isqlGlob.SQL_dialect && isqlGlob.SQL_dialect != SQL_DIALECT_V5 && Warnings)
			{
				isqlGlob.printf(NEWLINE);
				if (call_by_create_db)
					sprintf(bad_dialect_buf, "%s%s%d%s%s",
							"WARNING: Pre IB V6 server only speaks SQL dialect 1",
							" and does not accept Client SQL dialect ",
							isqlGlob.SQL_dialect,
							" . Client SQL dialect is reset to 1.", NEWLINE);
				else
				{
					//connecting_to_pre_v6_server = true; Not used anywhere.
					sprintf(bad_dialect_buf, "%s%s%d%s%s",
							"ERROR: Pre IB V6 server only speaks SQL dialect 1",
							" and does not accept Client SQL dialect ",
							isqlGlob.SQL_dialect,
							" . Client SQL dialect is reset to 1.", NEWLINE);
				}
				isqlGlob.prints(bad_dialect_buf);
			}
			else
			{
				if (isqlGlob.SQL_dialect == 0)
				{
					//connecting_to_pre_v6_server = true; Not used anywhere.
					sprintf(bad_dialect_buf, "%s%s%d%s%s",
							"ERROR: Pre IB V6 server only speaks SQL dialect 1",
							" and does not accept Client SQL dialect ",
							isqlGlob.SQL_dialect,
							" . Client SQL dialect is reset to 1.", NEWLINE);
					isqlGlob.prints(bad_dialect_buf);
				}
			}
			break;
		case isc_info_firebird_version:
			if (Version_info)
			{
				// This information will be skipped if the server isn't given enough buffer
				// to put it all. It's a FULL or NOTHING answer. It grows with redirection.
				// The command SHOW version that calls isc_version() will return more info.
				isqlGlob.printf("Server version:%s", NEWLINE);
				const UCHAR* q = p; // We don't want to spoil p with a wrong calculation.
				const UCHAR* limit = q + length;
				for (int times = *q++; times && q < limit; --times)
				{
					int l = *q++;
					if (l > limit - q)
						l = limit - q;

					isqlGlob.printf("%.*s%s", l, q, NEWLINE);
					q += l;
				}
			}
			break;

		default:
			isqlGlob.printf("Internal error: Unexpected isc_info_value %d%s",
							item, NEWLINE);
			break;
		}
		p += length;
	}

	if (isqlGlob.major_ods < ODS_VERSION8)
	{
		TEXT errbuf[MSG_LENGTH];
		ISQL_msg_get(SERVER_TOO_OLD, errbuf);
		STDERROUT(errbuf);
		return;
	}

	/* If the remote server did not respond to our request for
   "dialects spoken", then we can assume it can only speak
   the V5 dialect.  We automatically change the connection
   dialect to that spoken by the server.  Otherwise the
   current dialect is set to whatever the user requested. */

	if (global_dialect_spoken == 0)
		isqlGlob.SQL_dialect = SQL_DIALECT_V5;
	else if (isqlGlob.major_ods < ODS_VERSION10)
		isqlGlob.SQL_dialect = global_dialect_spoken;
	else if (isqlGlob.SQL_dialect == 0)	// client SQL dialect has not been set
		isqlGlob.SQL_dialect = global_dialect_spoken;

	if (global_dialect_spoken > 0)
		isqlGlob.db_SQL_dialect = global_dialect_spoken;
	else
		isqlGlob.db_SQL_dialect = SQL_DIALECT_V5;
}


void ISQL_remove_and_unescape_quotes(TEXT* string, const char quote)
{
/**************************************
 *
 *	I S Q L _ r e m o v e _ a n d _ u n e s c a p e _ q u o t e s
 *
 **************************************
 *
 * Functional description
 *	Remove the delimited quotes. Blanks could be part of
 *	delimited SQL identifier. It has to deal with embedded quotes, too.
 *
 **************************************/
	const size_t cmd_len = strlen(string);
	TEXT* q = string;
	const TEXT* p = q;
	const TEXT* const end_of_str = p + cmd_len;

	for (size_t cnt = 1; cnt < cmd_len && p < end_of_str; cnt++)
	{
		p++;
		if (cnt < cmd_len - 1)
		{
			*q = *p;
			if (p + 1 < end_of_str)
			{
				if (*(p + 1) == quote)	// skip the escape double quote
					p++;
			}
			else
			{
				p++;
				*q = '\0';
			}
		}
		else
			*q = '\0';

		q++;
	}
	*q = '\0';
}


void ISQL_truncate_term(TEXT* str, USHORT len)
{
/**************************************
 *
 *	I S Q L _ t r u n c a t e _ t e r m
 *
 **************************************
 *
 * Functional description
 *	Truncates the rightmost contiguous spaces on a string.
 * CVC: Notice isspace may be influenced by locales.
 **************************************/
	int i;
	for (i = len - 1; i >= 0 && ((fb_isspace(str[i])) || (str[i] == 0)); i--);
	str[i + 1] = 0;
}


void ISQL_ri_action_print(const TEXT* ri_action_str,
						  const TEXT* ri_action_prefix_str,
						  bool all_caps)
{
/**************************************
 *
 *	I S Q L _ r i _ a c t i o n _ p r i n t
 *
 **************************************
 *
 * Functional description
 *	prints the description of ref. integrity actions.
 *	  The actions must be one of the cascading actions or RESTRICT.
 *	  RESTRICT is used to indicate that the user did not specify any
 *	  actions, so do not print it out.
 *
 **************************************/
	for (const ri_actions* ref_int = ri_actions_all; ref_int->ri_action_name;
		++ref_int)
	{
		if (!strcmp(ref_int->ri_action_name, ri_action_str))
		{
			if (*ref_int->ri_action_print_caps)
			{
				// we have something to print
				if (all_caps)
					isqlGlob.printf("%s %s", ri_action_prefix_str, ref_int->ri_action_print_caps);
				else if (*ref_int->ri_action_print_mixed)
					isqlGlob.printf("%s %s", ri_action_prefix_str, ref_int->ri_action_print_mixed);
			}
			return;
		}
	}

	fb_assert(FALSE);
}


static bool get_numeric(const UCHAR* string,
						USHORT length,
						SSHORT* scale,
						SINT64* ptr)
{
/**************************************
 *
 *	  g e t _ n u m e r i c
 *
 **************************************
 *
 * Functional description
 *	  Convert a numeric literal (string) to its binary value.
 *
 *	  The binary value (int64) is stored at the
 *	  address given by ptr.
 *
 **************************************/
	SINT64 value = 0;

	SSHORT local_scale = 0, sign = 0;
	bool digit_seen = false, fraction = false;

	const UCHAR* const end = string + length;
	for (const UCHAR* p = string; p < end; p++)
	{
		if (DIGIT(*p))
		{
			digit_seen = true;

			// Before computing the next value, make sure there will be
			// no overflow. Trying to detect overflow after the fact is
			// tricky: the value doesn't always become negative after an
			// overflow!

			if (value > INT64_LIMIT)
				return false;

			if (value == INT64_LIMIT)
			{
				// possibility of an overflow

				if ((*p > '8' && sign == -1) || (*p > '7' && sign != -1))
					return false;
			}

			// Force the subtraction to be performed before the addition,
			// thus preventing a possible signed arithmetic overflow.
			value = value * 10 + (*p - '0');
			if (fraction)
				--local_scale;
		}
		else if (*p == '.')
		{
			if (fraction)
				return false;

			fraction = true;
		}
		else if (*p == '-' && !digit_seen && !sign && !fraction)
			sign = -1;
		else if (*p == '+' && !digit_seen && !sign && !fraction)
			sign = 1;
		else if (*p != BLANK)
			return false;
	}

	if (!digit_seen)
		return false;

	*scale = local_scale;
	*(SINT64*) ptr = ((sign == -1) ? -value : value);
	return true;
}


// Helper to print boolean values in the SET options.
static void print_set(const char* str, bool v)
{
	isqlGlob.printf("%-25s%s%s", str, v ? "ON" : "OFF", NEWLINE);
}


static processing_state print_sets()
{
/**************************************
 *
 *	p r i n t _ s e t s
 *
 **************************************
 *
 * Functional description
 *	Print the current set values
 *
 **************************************/

	print_set("Print statistics:", Stats);
	print_set("Echo commands:", Echo);
	print_set("List format:", List);
	print_set("List Row Count:", Docount);
	//print_set("Row Count:", Docount);   // Changed print to the above to avoid confusion with next one
	isqlGlob.printf("%-25s%lu%s", "Select rowcount limit:", rowCount, NEWLINE);
	print_set("Autocommit DDL:", Autocommit);
#ifdef SCROLLABLE_CURSORS
	print_set("Autofetch records:", Autofetch);
#endif
	print_set("Access Plan:", Plan);
	print_set("Access Plan only:", Planonly);

	isqlGlob.printf("%-25s", "Display BLOB type:");
	switch (Doblob)
	{
	case ALL_BLOBS:
		isqlGlob.printf("ALL");
		break;
	case NO_BLOBS:
		isqlGlob.printf("NONE");
		break;
	default:
		isqlGlob.printf("%d", Doblob);
	}
	isqlGlob.printf(NEWLINE);

	if (*ISQL_charset && strcmp(ISQL_charset, DEFCHARSET)) {
		isqlGlob.printf("%-25s%s%s", "Set names:", ISQL_charset, NEWLINE);
	}

	print_set("Column headings:", Heading);

	if (global_Cols.count())
	{
		isqlGlob.printf("Column print widths:%s", NEWLINE);
		const ColList::item* p = global_Cols.getHead();
		while (p)
		{
			isqlGlob.printf("%s%s width: %d%s", TAB_AS_SPACES, p->col_name, p->col_len, NEWLINE);
			p = p->next;
		}
	}
	isqlGlob.printf("%-25s%s%s", "Terminator:", isqlGlob.global_Term, NEWLINE);

	print_set("Time:", Time_display);
	print_set("Warnings:", Warnings);
	print_set("Bail on error:", BailOnError);
	return SKIP;
}


static processing_state help(const TEXT* what)
{
/**************************************
 *
 *	h e l p
 *
 **************************************
 *
 * Functional description
 *	List the known commands.
 *
 **************************************/

	// Ordered list of help messages to display.  Use -1 to terminate list,
	// and 0 for an empty blank line
	static const SSHORT help_ids[] =
	{
		HLP_FRONTEND,			// Frontend commands:
		HLP_BLOBDMP,			// BLOBDUMP <blobid> <file>	-- dump BLOB to a file
		HLP_BLOBVIEW,			// BLOBVIEW <blobid>		-- view BLOB in text editor
		HLP_EDIT,				// EDIT	 [<filename>]		-- edit SQL script file and execute
		HLP_EDIT2,				// EDIT						-- edit current command buffer and execute
		HLP_HELP,				// HELP						-- display this menu
		HLP_INPUT,				// INput	<filename>		-- take input from the named SQL file
		HLP_OUTPUT,				// OUTput   [<filename>]	-- write output to named file
		HLP_OUTPUT2,			// OUTput					-- return output to stdout
		HLP_SET_ROOT,			// SET	  <option>			-- (use HELP SET for list)
		HLP_SHELL,				// SHELL	<command>		-- execute Operating System command in sub-shell
		HLP_SHOW,				// SHOW	 <object> [<name>]	-- display system information
		HLP_OBJTYPE,			//	<object> = CHECK, DATABASE, DOMAIN, EXCEPTION, FILTER, FUNCTION, GENERATOR,
		HLP_OBJTYPE2,			//			   GRANT, INDEX, PROCEDURE, ROLE, SQL DIALECT, SYSTEM, TABLE,
		HLP_OBJTYPE3,			//			   TRIGGER, VERSION, VIEW
		HLP_EXIT,				// EXIT					   -- exit and commit changes
		HLP_QUIT,				// QUIT					   -- exit and roll back changes
		0,
		HLP_ALL,				// All commands may be abbreviated to letters in CAPitals
		-1						// end of list
	};

	static const SSHORT help_set_ids[] =
	{
		HLP_SETCOM,				//Set commands:
		HLP_SET,				//	SET						-- display current SET options
		HLP_SETAUTO,			//	SET AUTOddl				-- toggle autocommit of DDL statements
#ifdef SCROLLABLE_CURSORS
		HLP_SETFETCH,			//	SET AUTOfetch			-- toggle autofetch of records
#endif
		HLP_SETBAIL,			//	SET BAIL				-- toggle bailing out on errors in non-interactive mode
		HLP_SETBLOB,			//	SET BLOB [ALL|<n>]		-- display BLOBS of subtype <n> or ALL
		HLP_SETBLOB2,			//	SET BLOB				-- turn off BLOB display
		HLP_SETCOUNT,			//	SET COUNT				-- toggle count of selected rows on/off
		HLP_SETROWCOUNT,		//	SET ROWCOUNT [<n>]		-- toggle limit of selected rows to <n>, zero is no limit
		HLP_SETECHO,			//	SET ECHO				-- toggle command echo on/off
		HLP_SETHEADING,			//  SET HEADING 	        -- toggle column titles display on/off
		HLP_SETLIST,			//	SET LIST				-- toggle column or table display format
		HLP_SETNAMES,			//	SET NAMES <csname>		-- set name of runtime character set
		HLP_SETPLAN,			//	SET PLAN				-- toggle display of query access plan
		HLP_SETPLANONLY,		//	SET PLANONLY			-- toggle display of query plan without executing
		HLP_SETSQLDIALECT,		//	SET SQL DIALECT <n>		-- set sql dialect to <n>
		HLP_SETSTAT,			//	SET STATs				-- toggle display of performance statistics
		HLP_SETTIME,			//	SET TIME				-- toggle display of timestamp with DATE values
		HLP_SETTERM,			//	SET TERM <string>		-- change statement terminator string
		HLP_SETWIDTH,			//	SET WIDTH <col> [<n>]	-- set/unset print width to <n> for column <col>
		0,
		HLP_ALL,				// All commands may be abbreviated to letters in CAPitals
		-1						// end of list
	};

	TEXT msg[MSG_LENGTH];
	const SSHORT* msgid;
	if (!strcmp(what, "SET")) {
		msgid = help_set_ids;
	}
	else {
		msgid = help_ids;
	}
	for (; *msgid != -1; msgid++)
	{
		if (*msgid != 0)
		{
			ISQL_msg_get(*msgid, msg);
			ISQL_printf(Help, msg);
		}
		ISQL_printf(Help, NEWLINE);
	}
	return (SKIP);
}


static bool isyesno(const TEXT* buffer)
{
/**********************************************
 *
 *	i s y e s n o
 *
 **********************************************
 *
 * Functional description
 *	check if the first letter of the user's response
 *	corresponds to the first letter of Yes
 *	(in whatever language they are using)
 *
 *	returns true for Yes, otherwise false.
 *
 **********************************************/

	if (!have_trans)
	{
		// get the translation if we don't have it already
		ISQL_msg_get(YES_ANS, sizeof(yesword), yesword);
		have_trans = true;
	}

	// Just check first byte of yes response -- could be multibyte problem

	return UPPER7(buffer[0]) == UPPER7(yesword[0]);
}


static processing_state newdb(TEXT* dbname,
					const TEXT* usr,
					const TEXT* psw,
					int numbufs,
					const TEXT* sql_role_nm,
					bool start_user_trans)
{
/**************************************
 *
 *	n e w d b
 *
 **************************************
 *
 * Functional description
 *	Change the current database from what it was.
 *	This procedure is called when we first enter this program.
 *
 *	Parameters:	dbname	  -- Name of database to open
 *			usr		 -- user name, if given
 *			psw		 -- password, if given
 *			numbufs	 -- # of connection cache buffers, if given, 0 if not
 *			sql_role_nm -- sql role name
 *
 **************************************/
	// No name of a database, just return an error

	if (!dbname || !*dbname)
		return ps_ERR;

	/*
	 * Since the dbname is set already, in the case where a database is specified
	 * on the command line, we need to save it so ISQL_disconnect does not NULL it
	 * out.  We will restore it after the disconnect.  The save_database buffer
	 * will also be used to translate dbname to the proper character set.
	*/
	const size_t len = chop_at(dbname, MAXPATHLEN);
	SCHAR* save_database = (SCHAR*) ISQL_ALLOC(len + 1);
	if (!save_database)
		return ps_ERR;

	strcpy(save_database, dbname);
	ISQL_disconnect_database(false);
	strcpy(dbname, save_database);
	ISQL_FREE(save_database);

	TEXT local_psw[BUFFER_LENGTH128];
	TEXT local_usr[BUFFER_LENGTH128];
	TEXT local_sql_role[BUFFER_LENGTH256];

	// global user and passwords are set only if they are not originally set

	local_psw[0] = 0;
	local_usr[0] = 0;
	local_sql_role[0] = 0;

	// Strip quotes if well-intentioned

	strip_quotes(dbname, isqlGlob.global_Db_name);
	strip_quotes(usr, local_usr);
	strip_quotes(psw, local_psw);

	/* if local user is not specified, see if global options are
	   specified - don't let a global role setting carry forward if a
	   specific local user was specified

	   Special handling for SQL Roles:
	 *	 V5  client -- strip the quotes
	 *	 V6  client -- pass the role name as it is
	 */

	if (sql_role_nm)
	{
		if (isqlGlob.SQL_dialect == SQL_DIALECT_V5)
			strip_quotes(sql_role_nm, local_sql_role);
		else
			strcpy(local_sql_role, sql_role_nm);
	}

	if (!(strlen(local_sql_role)) && global_role)
	{
		if (isqlGlob.SQL_dialect == SQL_DIALECT_V5)
			strip_quotes(isqlGlob.Role, local_sql_role);
		else
			strcpy(local_sql_role, isqlGlob.Role);
	}

	if (!(strlen(local_usr)) && global_usr)
		strcpy(local_usr, isqlGlob.User);

	if (!(strlen(local_psw)) && global_psw)
		strcpy(local_psw, Password);

	int local_numbufs = numbufs;
	if ((local_numbufs == 0) && has_global_numbufs)
		local_numbufs = global_numbufs;

	// Build up a dpb
	Firebird::ClumpletWriter dpb(Firebird::ClumpletReader::Tagged, MAX_DPB_SIZE, isc_dpb_version1);

	if (*ISQL_charset && strcmp(ISQL_charset, DEFCHARSET)) {
		dpb.insertString(isc_dpb_lc_ctype, ISQL_charset, strlen(ISQL_charset));
	}

    SSHORT l;
	if (l = strlen(local_usr)) {
		dpb.insertString(isc_dpb_user_name, local_usr, l);
	}

	if (l = strlen(local_psw)) {
		dpb.insertString(isc_dpb_password, local_psw, l);
	}

	if (l = strlen(local_sql_role))
	{
		dpb.insertInt(isc_dpb_sql_dialect, isqlGlob.SQL_dialect);
		dpb.insertString(isc_dpb_sql_role_name, local_sql_role, l);
	}

	if (local_numbufs > 0) {
		dpb.insertInt(isc_dpb_num_buffers, local_numbufs);
	}

	if (Nodbtriggers)
		dpb.insertInt(isc_dpb_no_db_triggers, 1);

#ifdef TRUSTED_AUTH
	if (Trusted_auth) {
		dpb.insertTag(isc_dpb_trusted_auth);
	}
#endif

	{ // scope
		const TEXT* local_name = isqlGlob.global_Db_name;

		if (isc_attach_database(isc_status, 0, local_name, &DB, dpb.getBufferLength(),
									reinterpret_cast<const char*>(dpb.getBuffer())))
		{
			ISQL_errmsg(isc_status);
			isqlGlob.global_Db_name[0] = '\0';
			return FAIL;
		}

		// Make it read owner name to display grantor correctly
		SHOW_read_owner();

		// No use in cancel when running non-end-user operators
		fb_cancel_operation(isc_status, &DB, fb_cancel_disable);
	} // scope

	ISQL_get_version(false);

	if (*local_sql_role)
	{
		switch (isqlGlob.SQL_dialect)
		{
		case SQL_DIALECT_V5:
			// Uppercase the Sql isqlGlob.Role name
			ISQL_make_upper(local_sql_role);
			break;
		case SQL_DIALECT_V6_TRANSITION:
		case SQL_DIALECT_V6:
			if (*local_sql_role == DBL_QUOTE || *local_sql_role == SINGLE_QUOTE)
			{
				// Remove the delimited quotes and escape quote from ROLE name.
				const TEXT end_quote = *local_sql_role;
				ISQL_remove_and_unescape_quotes(local_sql_role, end_quote);
			}
			else {
				ISQL_make_upper(local_sql_role);
			}
			break;
		default:
			break;
		}
	}

	// CVC: Do not put the user and pw used to extract metadata in a script!
	// Only acknowledge user connection parameters in interactive logins.

	if (Interactive)
	{
		if (local_usr[0] != '\0')
		{
			if (local_sql_role[0] != '\0')
			{
				isqlGlob.printf("Database:  %s, User: %s, Role: %s%s",
						dbname, local_usr, local_sql_role, NEWLINE);
			}
			else {
				isqlGlob.printf("Database:  %s, User: %s%s", dbname, local_usr, NEWLINE);
			}
		}
		else
		{
			if (local_sql_role[0] != '\0') {
				isqlGlob.printf("Database:  %s, Role:  %s%s", dbname, local_sql_role, NEWLINE);
			}
			else {
				isqlGlob.printf("Database:  %s%s", dbname, NEWLINE);
			}
		}
	}

	// CVC: We do not require those pesky transactions for -x or -a options.
	// Metadata extraction works exclusively with default transaction gds__trans.

	if (start_user_trans)
	{

		// Start the user transaction and default transaction

		if (!M__trans)
		{
			if (isc_start_transaction(isc_status, &M__trans, 1, &DB, 0, NULL))
				ISQL_errmsg(isc_status);
			if (D__trans)
				commit_trans(&D__trans);
			if (Autocommit && isc_start_transaction(isc_status, &D__trans, 1, &DB, 5, default_tpb))
				ISQL_errmsg(isc_status);
		}


		// Allocate a new user statement for this database

		global_Stmt = 0;
		if (isc_dsql_allocate_statement(isc_status, &DB, &global_Stmt))
		{
			ISQL_errmsg(isc_status);
			if (M__trans)
				end_trans();
			if (D__trans)
				commit_trans(&D__trans);
			isc_detach_database(isc_status, &DB);
			DB = 0;
			isqlGlob.global_Db_name[0] = '\0';
			M__trans = 0;
			D__trans = 0;
			return FAIL;
		}
	}
	else {
		global_Stmt = 0;
	}

	return SKIP;
}


static processing_state newinput(const TEXT* infile)
{
/**************************************
 *
 *	n e w i n p u t
 *
 **************************************
 *
 * Functional description
 *	Read commands from the named input file
 *
 *	Parameters:  infile -- Second word of the command line
 *		filelist is a stack of file pointers to
 *			return from nested inputs
 *
 *	The result of calling this is to point the
 *	global input file pointer, Ifp, to the named file.
 *
 **************************************/
	TEXT errbuf[MSG_LENGTH];

	// If there is no file name specified, return error
	if (!infile || !*infile) {
		return ps_ERR;
	}

	TEXT path[MAXPATHLEN];
	strip_quotes(infile, path);
	const TEXT* file = path;

	// filelist is a linked list of file pointers.  We must add a node to
	// the linked list before discarding the current Ifp.
	//  Filelist is a global pointing to base of list.

	FILE* fp = fopen(file, "r");
	if (fp)
	{
		Filelist->insertIfp();
		Filelist->Ifp().init(fp, file);
	}
	else
	{
		ISQL_msg_get(FILE_OPEN_ERR, errbuf, SafeArg() << file);
		STDERROUT(errbuf);
		return FAIL;
	}

	Input_file = true;
	return SKIP;
}


static processing_state newoutput(const TEXT* outfile)
{
/**************************************
 *
 *	n e w o u t p u t
 *
 **************************************
 *
 * Functional description
 *	Change the current output file
 *
 *	Parameters:  outfile : Name of file to receive query output
 *
 **************************************/
	processing_state ret = SKIP;
	// If there is a file name, attempt to open it for append

	if (*outfile)
	{
		TEXT path[MAXPATHLEN];
		strip_quotes(outfile, path);
		outfile = path;
		FILE* fp = fopen(outfile, "a");
		if (fp)
		{
			if (isqlGlob.Out && isqlGlob.Out != stdout)
				fclose(isqlGlob.Out);
			isqlGlob.Out = fp;
			if (Merge_stderr)
				isqlGlob.Errfp = isqlGlob.Out;
			if (Merge_diagnostic)
			    Diag = isqlGlob.Out;
		}
		else
		{
			TEXT errbuf[MSG_LENGTH];
			ISQL_msg_get(FILE_OPEN_ERR, errbuf, SafeArg() << outfile);
			STDERROUT(errbuf);
			ret = FAIL;
		}
	}
	else
	{
		// Revert to stdout
		if (isqlGlob.Out != stdout)
		{
			fclose(isqlGlob.Out);
			isqlGlob.Out = stdout;
			if (Merge_stderr)
				isqlGlob.Errfp = isqlGlob.Out;
			if (Merge_diagnostic)
			    Diag = isqlGlob.Out;
		}
	}
	return (ret);
}


static processing_state newsize(const TEXT* colname, const TEXT* sizestr)
{
/**************************************
 *
 *	n e w s i z e
 *
 **************************************
 *
 * Functional description
 *	Add a column name and print width to collist
 *
 **************************************/
	if (!*colname || (strlen(colname) >= QUOTEDLENGTH))
		return ps_ERR;

	char buf[QUOTEDLENGTH];
	if (colname[0] == DBL_QUOTE)
	{
		strcpy(buf, colname);
		ISQL_remove_and_unescape_quotes(buf, DBL_QUOTE);
		colname = buf;
	}

	if (strlen(colname) > MAX_SQL_IDENTIFIER_LEN)
		return ps_ERR;

	// If no size is given, remove the entry
	if (!*sizestr)
	{
		// We don't signal error if the item to be removed doesn't exist.
		global_Cols.remove(colname);
		return SKIP;
	}

	const int size = atoi(sizestr);
	if (size <= 0)
		return ps_ERR;

	// We don't care if it was insertion or replacement.
	global_Cols.put(colname, size);
	return SKIP;
}

static processing_state newRowCount(const TEXT* newRowCountStr)
{
/**************************************
 *
 *	newRowCount
 *
 **************************************
 *
 * Functional description
 *
 **************************************/

	char* p;

	errno = 0;
	const long newRowCount = strtol(newRowCountStr, &p, 0);
	// I was going to use this one, but "-1" parses as max ulong without error and it would be politer to give an error.
	//const ULONG newRowCount = strtoul(newRowCountStr, &p, 0);

	if (newRowCount < 0)
	{
		isqlGlob.printf("The value (%s) for rowcount must be zero or greater", newRowCountStr);
		isqlGlob.printf(NEWLINE);
		return ps_ERR;
	}

	if (errno)
	{
		switch (errno)
		{
		case ERANGE:
			isqlGlob.printf("Unable to convert %s to a number for rowcount ", newRowCountStr);
			isqlGlob.printf(NEWLINE);
			break;
		case EINVAL:
			isqlGlob.printf("Value %s for rowcount is out of range. Max value is %ld", newRowCountStr, SLONG_MAX);
			isqlGlob.printf(NEWLINE);
			break;
		default:
			isqlGlob.printf("Unable to process %s as new limit for rowcount", newRowCountStr);
			isqlGlob.printf(NEWLINE);
			break;
		}

		return ps_ERR;
	}

	rowCount = newRowCount;
	return SKIP;
}


static processing_state newtrans(const TEXT* statement)
{
/**************************************
 *
 *	n e w t r a n s
 *
 **************************************
 *
 * Functional description
 *	Intercept and handle a set transaction statement by zeroing M__trans
 *
 *	Leave the default transaction, gds_trans, alone
 *	Parameters:  The statement in its entirety
 *
 **************************************/

	if (!ISQL_dbcheck())
		return FAIL;

	if (end_trans() == FAIL)
		return FAIL;

	M__trans = 0;

	// M__trans = 0 after the commit or rollback. Ready for a new transaction

	if (isc_dsql_execute_immediate(isc_status, &DB, &M__trans, 0, statement,
								   isqlGlob.SQL_dialect, NULL))
	{
		ISQL_errmsg(isc_status);
		return FAIL;
	}

	return SKIP;
}


static processing_state parse_arg(int argc, SCHAR** argv, SCHAR* tabname)
//	, FILE** sess) Last param was for wisql
{
/**************************************
 *
 *	p a r s e _ a r g
 *
 **************************************
 *
 * Functional description
 *	Parse command line flags
 *	All flags except database are -X.  Look for
 *	the - and make it so.
 *
 **************************************/
	processing_state ret = SKIP;

	TEXT errbuf[MSG_LENGTH];

	// Initialize database name

	isqlGlob.global_Db_name[0] = '\0';
	isqlGlob.global_Target_db[0] = '\0';
	Password[0] = '\0';
	isqlGlob.User[0] = '\0';
	isqlGlob.Role[0] = '\0';
	global_numbufs = 0;
	Quiet = false;
	Exit_value = FINI_OK;

	// default behavior in V6.0 is SQL_DIALECT_V6

	requested_SQL_dialect = SQL_DIALECT_V6;

	Merge_stderr = false;
	Merge_diagnostic =  false;

	{ // scope
		const size_t lgth = strlen(DEFCHARSET);
		if (lgth < MAXCHARSET_SIZE)
			strcpy(ISQL_charset, DEFCHARSET);
		else
			fb_utils::copy_terminate(ISQL_charset, DEFCHARSET, MAXCHARSET_SIZE);
	} // scope

	// redirected stdin means act like -i was set

	Filelist->Ifp().init(stdin, "stdin");

	// Terminators are initialized

	Termlen = strlen(DEFTERM);
	if (Termlen < MAXTERM_SIZE)
		strcpy(isqlGlob.global_Term, DEFTERM);
	else
	{
		Termlen = MAXTERM_SIZE - 1;
		fb_utils::copy_terminate(isqlGlob.global_Term, DEFTERM, Termlen + 1);
	}

	// Initialize list of input file pointers

	//Filelist->clear();

	// Interpret each command line argument

	const SCHAR switchchar = '-';

#ifdef	DEV_BUILD
	bool istable = false;
#endif

	bool switch_used[SWITCH_UNKNOWN];
	for (int su = 0; su < SWITCH_UNKNOWN; ++su) {
		switch_used[su] = false;
	}

	for (int i = 1; i < argc; ++i)
	{
		const char* s = argv[i];
		// Look at flags to find unique match.  If the flag has an arg,
		// advance the pointer (and i).  Only change the return value
		// for extract switch or error.
		if (*s == switchchar)
		{
			switch_id swid = SWITCH_UNKNOWN;
			int swarg_int = 0;
			char* swarg_str = NULL;
			const size_t swlen = strlen(s + 1);

			if (swlen == 1 && s[1] == '?')
			{
				ret = ps_ERR;
				break;
			}

			for (int j = 0; j < FB_NELEM(switches); j++)
			{
				if (swlen >= switches[j].abbrlen && !fb_utils::strnicmp(s + 1, switches[j].text, swlen))
				{
					swid = switches[j].id;
					if (switch_used[swid]) // && !switches[j].allow_dup
					{
						ISQL_msg_get(USAGE_DUPSW, errbuf, SafeArg() << (s + 1));
						STDERROUT(errbuf);
						ret = ps_ERR;
						break;
					}
					switch_used[swid] = true;
					if (switches[j].argtype != SWARG_NONE)
					{
						// make sure swarg_str is really a pointer to argv, not something else !!
						if (++i < argc)
							swarg_str = argv[i];
						if (!swarg_str || !*swarg_str)
						{
							ISQL_msg_get(USAGE_NOARG, errbuf, SafeArg() << (s + 1));
							STDERROUT(errbuf);
							ret = ps_ERR;
						}
						else if (switches[j].argtype == SWARG_INTEGER)
						{
							char* err;
							long value = strtol(swarg_str, &err, 10);
							if (*err)
							{
								// conversion error
								ISQL_msg_get(USAGE_NOTINT, errbuf, SafeArg() << swarg_str << (s + 1));
								STDERROUT(errbuf);
								ret = ps_ERR;
							}
							else if ((value < INT_MIN) || (value > INT_MAX))
							{
								ISQL_msg_get(USAGE_RANGE, errbuf, SafeArg() << swarg_str << (s + 1));
								STDERROUT(errbuf);
								ret = ps_ERR;
							}
							else {
								swarg_int = (int) value;
							}
						}
					}
					break; // switch found, exit the loop
				}
			}

			// Quit the loop of interpreting if we got an error
			if (ret == ps_ERR)
				break;

			switch (swid)
			{
#ifdef DEV_BUILD
				case SWITCH_EXTRACTTBL:
					istable = true;
					ret = EXTRACT;
					break;
#endif
				case SWITCH_EXTRACT:
					ret = EXTRACT;
					break;

				case SWITCH_EXTRACTALL:
					ret = EXTRACTALL;
					break;

				case SWITCH_BAIL:
					BailOnError = true;
					break;

				case SWITCH_ECHO:
					Echo = true;
					break;

				case SWITCH_MERGE:
					Merge_stderr = true;
					break;

				case SWITCH_MERGE2:
					Merge_diagnostic = true;
					break;

				case SWITCH_NOAUTOCOMMIT:
					Autocommit = false;
					break;

				case SWITCH_NODBTRIGGERS:
					Nodbtriggers = true;
					break;

				case SWITCH_NOWARN:
					Warnings = false;
					break;

				case SWITCH_OUTPUT:
					if (newoutput(swarg_str) == FAIL) {
						ret = ps_ERR;
					}
					break;

				case SWITCH_INPUT:
					if (newinput(swarg_str) == SKIP) {
						Interactive = false;
					}
					else {
						ret = ps_ERR;
					}
					// CVC: Set by newinput() above only if successful.
					// Input_file = true;
					break;

				case SWITCH_TERM:
					Termlen = strlen(swarg_str);
					if (Termlen >= MAXTERM_SIZE) {
						Termlen = MAXTERM_SIZE - 1;
					}
					fb_utils::copy_terminate(isqlGlob.global_Term, swarg_str, Termlen + 1);
					break;

				case SWITCH_DATABASE:
					fb_utils::copy_terminate(isqlGlob.global_Target_db, swarg_str, sizeof(isqlGlob.global_Target_db));
					break;

				case SWITCH_PAGE:
					if (swarg_int < 0)
					{
						ISQL_msg_get(USAGE_RANGE, errbuf, SafeArg() << swarg_str << (s + 1));
						STDERROUT(errbuf);
						ret = ps_ERR;
					}
					else if (swarg_int == 0) // let's interpret -pag 0 as SET HEADING OFF
						Heading = false;
					Pagelength = swarg_int;
					break;

				case SWITCH_PASSWORD:
					fb_utils::copy_terminate(Password, fb_utils::get_passwd(swarg_str), sizeof(Password));
					// make sure swarg_str is really a pointer to argv, not something else !!
					global_psw = true;
					break;

				case SWITCH_FETCHPASS:
				{
					const char* pass = NULL;
					const fb_utils::FetchPassResult rez = fb_utils::fetchPassword(swarg_str, pass);
					if (rez == fb_utils::FETCH_PASS_OK)
					{
						fb_utils::copy_terminate(Password, pass, sizeof(Password));
						global_psw = true;
					}
					else
					{
						switch (rez)
						{
						case fb_utils::FETCH_PASS_FILE_OPEN_ERROR:
							ISQL_msg_get(PASS_FILE_OPEN, errbuf, SafeArg() << swarg_str << errno);
							// could not open password file @1, errno @2
							break;
						case fb_utils::FETCH_PASS_FILE_READ_ERROR:
							ISQL_msg_get(PASS_FILE_READ, errbuf, SafeArg() << swarg_str << errno);
							// could not read password file @1, errno @2
							break;
						case fb_utils::FETCH_PASS_FILE_EMPTY:
							ISQL_msg_get(EMPTY_PASS, errbuf, SafeArg() << swarg_str);
							// empty password file @1
							break;
						}
						STDERROUT(errbuf);
						ret = ps_ERR;
					}
					break;
				}

				case SWITCH_USER:
					fb_utils::copy_terminate(isqlGlob.User, swarg_str, sizeof(isqlGlob.User));
					global_usr = true;
					break;

				case SWITCH_ROLE:
					fb_utils::copy_terminate(isqlGlob.Role, swarg_str, sizeof(isqlGlob.Role));
					global_role = true;
					break;

				case SWITCH_ROLE2:
					strcpy(isqlGlob.Role, "\"");
					fb_utils::copy_terminate(isqlGlob.Role + 1, swarg_str, sizeof(isqlGlob.Role) - 2);
					strcat(isqlGlob.Role, "\"");
					global_role = true;
					break;

				case SWITCH_CACHE:
					if (swarg_int <= 0)
					{
						ISQL_msg_get(USAGE_RANGE, errbuf, SafeArg() << swarg_str << (s + 1));
						STDERROUT(errbuf);
						ret = ps_ERR;
					}
					global_numbufs = swarg_int;
					has_global_numbufs = true;
					break;

				case SWITCH_CHARSET:
					{
						fb_utils::copy_terminate(Charset, swarg_str, sizeof(Charset));
						const size_t lgth = strlen(Charset);
						if (lgth < MAXCHARSET_SIZE) {
							strcpy(ISQL_charset, Charset);
						}
						else
							fb_utils::copy_terminate(ISQL_charset, Charset, MAXCHARSET_SIZE);
					}
					break;

				case SWITCH_QUIET:
					Quiet = true;
					break;

#ifdef TRUSTED_AUTH
				case SWITCH_TRUSTED:
					Trusted_auth = true;
					break;
#endif

				case SWITCH_VERSION:
					Version_info = true;
					ISQL_msg_get(VERSION, errbuf, SafeArg() << FB_VERSION);
					isqlGlob.printf("%s%s", errbuf, NEWLINE);
					break;

				case SWITCH_SQLDIALECT:
					requested_SQL_dialect = swarg_int;
					if (requested_SQL_dialect < SQL_DIALECT_V5 ||
						requested_SQL_dialect > SQL_DIALECT_CURRENT)
					{
						ret = ps_ERR;
					}
					else
					{
						// requested_SQL_dialect is used to specify the database dialect
						// if SQL dialect was not specified.  Since it is possible to
						// have a client dialect of 2, force the database dialect to 3, but
						// leave the client dialect as 2
						isqlGlob.SQL_dialect = requested_SQL_dialect;
						if (requested_SQL_dialect == SQL_DIALECT_V6_TRANSITION)
						{
							Merge_stderr = true;
							requested_SQL_dialect = SQL_DIALECT_V6;
						}
					}
					break;

				default: // unknown switch
					ISQL_msg_get(SWITCH, errbuf, SafeArg() << (s + 1));
					STDERROUT(errbuf);
					ret = ps_ERR;

			}
		}
		else
		{
			// This is not a switch, it is a db_name
			if (isqlGlob.global_Db_name[0])
			{
				// We already have a database name
				ISQL_msg_get(USAGE_DUPDB, errbuf, SafeArg() << isqlGlob.global_Db_name << s);
				STDERROUT(errbuf);
				ret = ps_ERR;
			}
			else
			{
				fb_utils::copy_terminate(isqlGlob.global_Db_name, s, sizeof(isqlGlob.global_Db_name));
#ifdef DEV_BUILD
				// If there is a table name, it follows
				if (istable && (++i < argc) && (s = argv[i]) && *s)
					fb_utils::copy_terminate(tabname, s, WORDLENGTH);
#endif
			}
		}

		// Quit the loop of interpreting if we got an error
		if (ret == ps_ERR)
			break;
	}

	// If not input, then set up first filelist

	if (Filelist->readingStdin())
	{
		Filelist->insert(stdin, "stdin");
		fb_assert(Filelist->count() == 1);
	}

	return ret;
}


// *********************
// p a s s t h r o u g h
// *********************
// Execute a command directly on the server. No interpretation done in isql.
// Use with care. Only for debug builds.
#ifdef DEV_BUILD
static processing_state passthrough(const char* cmd)
{
	processing_state ret = SKIP;
	if (isc_dsql_execute_immediate(isc_status, &DB, &M__trans, 0, cmd, isqlGlob.SQL_dialect, NULL))
	{
		ISQL_errmsg(isc_status);
		if (!DB)
			ret = FAIL;
		else
			ret = ps_ERR;
	}
	return ret;
}
#endif

static bool checkSpecial(TEXT* const p, const int length, const double value)
{
/**************************************
 *
 *	c h e c k S p e c i a l
 *
 **************************************
 *
 * Functional description
 *	Special case - Nan and Infinity.
 *	Some libraries (SFIO) work wrong with them.
 *
 **************************************/
	const TEXT* t = NULL;
	if (isnan(value))
		t = "NaN";
	else if (isinf(value))
		t = "Infinity";
	else
		return false;

	if (List) {
		isqlGlob.printf("%s%s", t, NEWLINE);
	}
	sprintf(p, "%*.*s ", length, length, t);

	return true;
}


static void align_string(char* out, unsigned clen, unsigned slen, const char* s)
{
	if (slen > clen)
		slen = clen;
	memcpy(out, s, slen);
	if (clen > slen)
		memset(out + slen, ' ', clen - slen);
	out[clen] = ' ';
	out[clen + 1] = '\0';
}


static SSHORT print_item(TEXT** s, XSQLVAR* var, const int length)
{
/**************************************
 *
 *	p r i n t _ i t e m
 *
 **************************************
 *
 * Functional description
 *	Print an SQL data item as described.
 *
 **************************************/
 	TEXT d[32];

	TEXT* p = *s;
	*p = '\0';

	SSHORT dtype = var->sqltype & ~1; // may be modified by print_item_blob()
	const int dscale = var->sqlscale;

	// The printlength should have in it the right length
	//const int length = printlength;

	if (List) {
		isqlGlob.printf("%-31s ", fb_utils::exact_name(var->aliasname));
	}

	if ((var->sqltype & 1) && (*var->sqlind < 0))
	{
		// If field was missing print <null>

		if (List) {
			isqlGlob.printf("<null>%s", NEWLINE);
		}

		if ((dtype == SQL_TEXT) || (dtype == SQL_VARYING))
			sprintf(p, "%-*s ", length, "<null>");
		else
			sprintf(p, "%*s ", length, "<null>");
	}
	else if (!strncmp(var->sqlname, "DB_KEY", 6))
	{
		// Special handling for db_keys printed in hex

		// Keep a temp buf, d for building the binary string

		for (const TEXT* t = var->sqldata; t < var->sqldata + var->sqllen; t++)
		{
			if (List) {
				isqlGlob.printf("%02X", (unsigned int) (UCHAR) *t);
			}
			else
			{
				sprintf(d, "%02X", (unsigned int) (UCHAR) *t);
				strcat(p, d);
			}
		}
		if (List)
			isqlGlob.printf(NEWLINE);
		else
			strcat(p, " ");
	}
	else
	{
		const ISC_QUAD* blobid;
		TEXT blobbuf[30];
		TEXT* string;
		tm times;

		switch (dtype)
		{
		case SQL_ARRAY:

			// Print blob-ids only here

			blobid = (ISC_QUAD*) var->sqldata;
			sprintf(blobbuf, "%" xLONGFORMAT":%" xLONGFORMAT, blobid->gds_quad_high,
					blobid->gds_quad_low);
			sprintf(p, "%17s ", blobbuf);
			break;

		case SQL_BLOB:

			// Print blob-ids only here

			blobid = (ISC_QUAD*) var->sqldata;
			sprintf(blobbuf, "%" xLONGFORMAT":%" xLONGFORMAT, blobid->gds_quad_high,
					blobid->gds_quad_low);
			sprintf(p, "%17s ", blobbuf);
			if (List)
			{
				isqlGlob.printf("%s%s", blobbuf, NEWLINE);
				dtype = ISQL_print_item_blob(isqlGlob.Out, var, M__trans, Doblob);
				isqlGlob.printf(NEWLINE);
			}
			break;

		case SQL_SHORT:
		case SQL_LONG:
		case SQL_INT64:
			{
				SINT64 value;

				switch (dtype)
				{
				case SQL_SHORT:
					value = (SINT64) (*(SSHORT*) var->sqldata);
					break;
				case SQL_LONG:
					value = (SINT64) (*(SLONG*) var->sqldata);
					break;
				case SQL_INT64:
					value = *(SINT64*) var->sqldata;
					break;
				}

				Firebird::string str_buf;
				print_item_numeric(value, length, dscale, str_buf.getBuffer(length));
				sprintf(p, "%s ", str_buf.c_str());
				if (List)
				{
					str_buf.ltrim(); // Added 1999-03-23 to left-justify in LIST ON mode
					isqlGlob.printf("%s%s", str_buf.c_str(), NEWLINE);
				}
			}
			break;

#ifdef NATIVE_QUAD
		case SQL_QUAD:
			if (dscale)
			{
				// Handle floating scale and precision

				double numeric = *(SQUAD*) var->sqldata;
				const double exponent = -dscale;
				numeric = numeric / pow(10.0, exponent);
				sprintf(p, "%*.*f ", length, -dscale, numeric);
				if (List) {
					isqlGlob.printf("%.*f%s", -dscale, numeric, NEWLINE);
				}
			}
			else
			{
				sprintf(p, "%*ld ", length, *(SQUAD*) var->sqldata);
				if (List) {
					isqlGlob.printf("%ld%s", *(SQUAD*) var->sqldata, NEWLINE);
				}
			}
			break;
#endif

		case SQL_FLOAT:
			{
				//
				// BRS 08 Aug 2003
				// MSVC6 has a bug in the g format when used with # and display
				// one digit more than the specified precision when the value is 0
				// The bug appears in TCS DSQL_DOMAIN_12 and 13
				//
				const double value = (double) *(float*) var->sqldata;

				if (checkSpecial(p, length, value))
				{
				    break;
				}

#if defined(MINGW)
				if (value == 0)
				{
					sprintf(p, "% #*.*g ", length, (int) MIN(8, (length - 6)) - 1, value);
					if (List) {
						isqlGlob.printf("%.*g%s", FLOAT_LEN - 6 -1, value, NEWLINE);
					}
				}
				else
				{
					sprintf(p, "% #*.*g ", length, (int) MIN(8, (length - 6)), value);
					if (List) {
						isqlGlob.printf("%.*g%s", FLOAT_LEN - 6, value, NEWLINE);
					}
				}
#else
				sprintf(p, "% #*.*g ", length, (int) MIN(8, (length - 6)), value);
				if (List) {
					isqlGlob.printf("%.*g%s", FLOAT_LEN - 6, value, NEWLINE);
				}
#endif
			}
			break;

		case SQL_DOUBLE:
			{
				const double value = *(double*) var->sqldata;

				if (checkSpecial(p, length, value))
				{
				    break;
				}

				// Don't let numeric/decimal doubles overflow print length
				// Special handling for 0 -- don't test log for length
				int rounded = 0;
				if (dscale && (!value ||
					(rounded = static_cast<int>(ceil(fabs(log10(fabs(value)))))) < length - 10))
				{
					int precision = 0;

					// CVC: Test values are
					// select -2.488355210669293e+01 from rdb$database;
					// select +2.488355210669293e+01 from rdb$database;
					// select +2.488355210669293e-01 from rdb$database;
					// select -2.488355210669293e-01 from rdb$database;
					// and sprintf should return length + 1.
					// See http://tracker.firebirdsql.org/browse/CORE-1363
					// The only way to enter this code is with a literal value (see above).
					// Taking SQL_DOUBLE from table fields or expressions will
					// yield dscale being zero.
					// This means that dscale is not what Borland expected; it's simply
					// the length of the incoming literal.
					if (value > 1)
						precision = length - rounded - 1; // nnn.nnn
					else if (value >= 0)
						precision = length - 2; // 0.nnn
					else if (value >= -1)
						precision = length - 3; // -0.nnn
					else // -nnn.nnn
						precision = length - rounded - 2;

					// Take into account old database containing negative scales for double
					if (dscale < 0 && precision > -dscale)
					{
						precision = -dscale;
					}

					sprintf(p, "%*.*f ", length, precision, value);

					if (List) {
						isqlGlob.printf("%.*f%s", precision, /*dscale,*/ value, NEWLINE);
					}
				}
				else
				{
#if defined(MINGW)
					if (value == 0)
					{
						sprintf(p, "% #*.*g ", length, (int) MIN(16, (length - 7)) - 1, value);
						if (List) {
							isqlGlob.printf("%#.*g%s", DOUBLE_LEN - 7 - 1, value, NEWLINE);
						}
					}
					else
					{
						sprintf(p, "% #*.*g ", length, (int) MIN(16, (length - 7)), value);
						if (List) {
							isqlGlob.printf("%#.*g%s", DOUBLE_LEN - 7, value, NEWLINE);
						}
					}
#else
					sprintf(p, "% #*.*g ", length, (int) MIN(16, (length - 7)), value);
					if (List) {
						isqlGlob.printf("%#.*g%s", DOUBLE_LEN - 7, value, NEWLINE);
					}
#endif
				}
			}
			break;

		case SQL_TEXT:
			string = var->sqldata;
			string[var->sqllen] = '\0';
			// See if it is character set OCTETS
			if (var->sqlsubtype == 1)
			{
				const SLONG hex_len = 2 * var->sqllen;
				TEXT* buff2 = (TEXT*) ISQL_ALLOC(hex_len + 1);
				// Convert the string to hex digits
				for (USHORT i = 0; i < var->sqllen; i++) {
					sprintf(&buff2[i * 2], "%02X", (UCHAR)string[i]);
				}
				buff2[hex_len] = 0;
				if (List) {
					isqlGlob.printf("%s%s", buff2, NEWLINE);
				}
				else
					sprintf(p, "%-*s ", length, buff2);
				ISQL_FREE(buff2);
			}
			else if (List) {
				isqlGlob.printf("%s%s", string, NEWLINE);
			}
			else
			{
				align_string(p, length, var->sqllen, var->sqldata);
			}
			break;

		case SQL_TIMESTAMP:
			isc_decode_timestamp((ISC_TIMESTAMP*) var->sqldata, &times);

			if (isqlGlob.SQL_dialect > SQL_DIALECT_V5)
			{
				sprintf(d, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d.%4.4" ULONGFORMAT,
						times.tm_year + 1900, (times.tm_mon + 1),
						times.tm_mday, times.tm_hour, times.tm_min,
						times.tm_sec,
						((ULONG*) var->sqldata)[1] % ISC_TIME_SECONDS_PRECISION);
			}
			else
			{
				if (Time_display)
					sprintf(d, "%2d-%s-%4d %2.2d:%2.2d:%2.2d.%4.4" ULONGFORMAT,
							times.tm_mday, alpha_months[times.tm_mon],
							times.tm_year + 1900, times.tm_hour, times.tm_min,
							times.tm_sec,
							((ULONG*) var->sqldata)[1] % ISC_TIME_SECONDS_PRECISION);
				else
					sprintf(d, "%2d-%s-%4d", times.tm_mday,
							alpha_months[times.tm_mon], times.tm_year + 1900);
			}

			sprintf(p, "%-*s ", length, d);
			if (List) {
				isqlGlob.printf("%s%s", d, NEWLINE);
			}
			break;

		case SQL_TYPE_TIME:
			isc_decode_sql_time((ISC_TIME*) var->sqldata, &times);
			sprintf(d, "%2.2d:%2.2d:%2.2d.%4.4" ULONGFORMAT,
					times.tm_hour, times.tm_min, times.tm_sec,
					(*(ISC_TIME*) var->sqldata) % ISC_TIME_SECONDS_PRECISION);
			sprintf(p, "%-*s ", length, d);
			if (List) {
				isqlGlob.printf("%s%s", d, NEWLINE);
			}
			break;

		case SQL_TYPE_DATE:
			isc_decode_sql_date((ISC_DATE*) var->sqldata, &times);
			sprintf(d, "%4.4d-%2.2d-%2.2d", times.tm_year + 1900,
					(times.tm_mon + 1), times.tm_mday);

			sprintf(p, "%-*s ", length, d);
			if (List) {
				isqlGlob.printf("%s%s", d, NEWLINE);
			}
			break;

		case SQL_VARYING:
			{
				vary* avary = (vary*) var->sqldata;

				// Note: we allocated the dataspace 1 byte larger already
				avary->vary_string[avary->vary_length] = 0;

				// If CHARACTER SET OCTETS, print two hex digits per octet
				if (var->sqlsubtype == 1)
				{
					const SLONG hex_len = 2 * avary->vary_length;
					char* buff2 = static_cast<char*>(ISQL_ALLOC(hex_len + 1));
					char* bp = buff2;

					for (USHORT i = 0; i < avary->vary_length; i++, bp += 2)
					{
						sprintf(bp, "%02X",
						static_cast<UCHAR>(avary->vary_string[i]));
					}
					buff2[hex_len] = '\0'; // there is an extra byte for terminator

					if (List)
					{
						isqlGlob.printf("%s%s", buff2, NEWLINE);
					}
					else
					{
						// Truncate if necessary
						sprintf(p, "%-*.*s ", length, static_cast<int>(MIN(length, hex_len)), buff2);
					}
					ISQL_FREE(buff2);
				}
				else if (List) {
					isqlGlob.printf("%s%s", avary->vary_string, NEWLINE);
				}
				else
				{
					align_string(p, length, avary->vary_length, avary->vary_string);
				}
				break;
			}

		default:
			sprintf(d, "Unknown type: %d", dtype);
			sprintf(p, "%-*s ", length, d);
			if (List) {
				isqlGlob.printf("%s%s", d, NEWLINE);
			}
			break;
		}
	}

	while (*p)
		++p;

	*s = p;

	return dtype;
}


processing_state ISQL_print_item_blob(FILE* fp,
						   const XSQLVAR* var,
						   FB_API_HANDLE trans,
						   int subtype)
{
/******************************************
 *
 *	I S Q L _ p r i n t _ i t e m _ b l o b
 *
 ******************************************
 *
 * Functional description
 *	Print a User Selected BLOB field (as oppposed to
 *	any BLOB selected in a show or extract command)
 *
 **************************************/
	TEXT msg[MSG_LENGTH];

	ISC_QUAD* blobid = (ISC_QUAD*) var->sqldata;

	// Don't bother with null blobs

	if (UserBlob::blobIsNull(*blobid))
		return CONT;

	if ((var->sqlsubtype != subtype) && (subtype != ALL_BLOBS))
	{
		ISQL_msg_get(BLOB_SUBTYPE, msg, SafeArg() << subtype << var->sqlsubtype);
		// Blob display set to subtype %d. This blob: subtype = %d\n
		ISQL_printf(fp, msg);
		return CONT;
	}

	USHORT bpb_length = 0;
	const UCHAR* bpb = NULL;
	UCHAR bpb_buffer[64];
	ISC_BLOB_DESC from_desc;

	const int blob_subtype = var->sqlsubtype;
	if (blob_subtype == isc_blob_text)
	{
		// ASF: Since ODS11.1, BLOBs are automatically transliterated to the client charset.
		if (isqlGlob.major_ods < ODS_VERSION11 ||
			(isqlGlob.major_ods == ODS_VERSION11 && isqlGlob.minor_ods == 0))
		{
			// Lookup the remaining descriptor information for the BLOB field,
			// most specifically we're interested in the Character Set so
			// we can set up a BPB that requests character set transliteration

			// CVC: Adriano changed trans to D__trans with the following comment:
			// Fix a bug that occur when a BLOB column is added after the start
			// of the DML transaction.
			// ASF: But use M__trans if D__trans is not started;
			if (!var->relname[0] ||
				isc_blob_lookup_desc(isc_status, &DB, (D__trans ? &D__trans : &M__trans),
					(const UCHAR*) var->relname, (const UCHAR*) var->sqlname, &from_desc, NULL) == 0)
			{
				if (!var->relname[0])
				{
					from_desc.blob_desc_subtype = var->sqlsubtype;
					from_desc.blob_desc_charset = var->sqlscale;
				}

				ISC_BLOB_DESC to_desc;
				isc_blob_default_desc(&to_desc, (const UCHAR*) var->relname, (const UCHAR*) var->sqlname);
				if (!isc_blob_gen_bpb(isc_status, &to_desc, &from_desc, sizeof(bpb_buffer),
									  bpb_buffer, &bpb_length))
				{
					bpb = bpb_buffer;
				}
			}
		}
	}
	else if (blob_subtype > isc_blob_text && blob_subtype < isc_blob_max_predefined_subtype)
	{
		bpb = predefined_blob_subtype_bpb;
		bpb_length = sizeof(predefined_blob_subtype_bpb);
		set_bpb_for_translation(blob_subtype);
	}

	UserBlob blob(isc_status);
	if (!blob.open(DB, trans, *blobid, bpb_length, bpb))
	{
		ISQL_errmsg(isc_status);
		return ps_ERR;
	}

	TEXT buffer[BUFFER_LENGTH512];
	size_t length;
	while (blob.getSegment(sizeof(buffer) - 1, buffer, length))
	{
		// Special displays for blr or acl subtypes

		if (blob_subtype > isc_blob_text && blob_subtype < isc_blob_max_predefined_subtype)
		{
			buffer[length--] = 0;
			for (char* b = buffer + length; b >= buffer;)
			{
				if (*b == '\n' || *b == '\t' || *b == BLANK)
					*b-- = 0;
				else
					break;
			}
			ISQL_printf2(fp, "%s	%s%s", TAB_AS_SPACES, buffer, NEWLINE);
		}
		else
		{
			buffer[length] = 0;
			ISQL_printf(fp, buffer);
		}
	}

	if (isc_status[1] && isc_status[1] != isc_segstr_eof)
	{
		ISQL_errmsg(isc_status);
		return ps_ERR;
	}
	return CONT;
}


static void print_item_numeric(SINT64 value,
							   int length,
							   int scale,
							   TEXT* buf)
{
/**************************************
 *
 *	p r i n t _ i t e m _ n u m e r i c
 *
 **************************************
 *
 * Functional description
 *	Print a INT64 value into a buffer by accomodating
 *	decimal point '.' for scale notation
 *
 **************************************/
	// Handle special case of no negative scale, no '.' required!
	if (scale >= 0)
	{
		if (scale > 0)
			value *= (SINT64) pow(10.0, (double) scale);
		sprintf(buf, "%*" SQUADFORMAT, length, value);
		return;
	}

	const bool neg = (value < 0);

	// Use one less space than allowed, to leave room for '.'
	length--;
	sprintf(buf, "%*" SQUADFORMAT, length, value);

	// start from LSByte towards MSByte
	SSHORT from = length - 1;
	SSHORT to = length;

	// make space for decimal '.' point in buffer
	buf[to + 1] = '\0';
	for (; from >= 0 && DIGIT(buf[from]) && scale; from--, to--, ++scale)
		buf[to] = buf[from];

	// Check whether we need a '0' (zero) in front of the '.' point
	const bool all_digits_used = !DIGIT(buf[from]);

	// Insert new '0's to satisfy larger scale than input digits
	// For e.g: 12345 with scale -7 would be .0012345
	if (from > 0 && scale)
		do {
			buf[to--] = '0';
		} while (++scale != 0);

	// Insert '.' decimal point, and if required, '-' and '0'
	buf[to--] = '.';
	if (all_digits_used)
	{
		buf[to--] = '0';
		if (neg)
			buf[to--] = '-';
	}
}


static processing_state print_line(XSQLDA* sqlda, const int pad[], TEXT line[])
{
/**************************************
 *
 *	p r i n t _ l i n e
 *
 **************************************
 *
 * Functional description
 *	Print a line of SQL variables.
 *
 *	Args:  sqlda, an XSQLDA
 *		pad = an array of the print lengths of all the columns
 *		line = pointer to the line buffer.
 *
 **************************************/
	const int maxblob = 20;
	XSQLVAR* varlist[maxblob]; // No more than 20 blobs per line

	int varnum = 0;
	{ // scope
		TEXT* p = line;
		int i = 0;
		XSQLVAR* var = sqlda->sqlvar;
		for (const XSQLVAR* const end = var + sqlda->sqld; var < end;
			 var++, i++)
		{
			if (!Interrupt_flag && !Abort_flag)
			{
				// Save all the blob vars and print them at the end
				// CVC: If varnum reaches 20, we must print an error instead of crashing.
				const int rc = print_item(&p, var, pad[i]);
				if (rc == SQL_BLOB && varnum < maxblob)
					varlist[varnum++] = var;
			}
		}
		*p = 0;
	} // scope

	if (List)
	{
		isqlGlob.printf(NEWLINE);
		return (CONT);
	}

	isqlGlob.printf("%s%s", line, NEWLINE);

	// If blobdisplay is not wanted, set varnum back to -1

	if (Doblob == NO_BLOBS)
		varnum = 0;
	else if (varnum >= maxblob)
	{
		TEXT msg[MSG_LENGTH];
		ISQL_msg_get(ONLY_FIRST_BLOBS, msg, SafeArg() << maxblob);
		isqlGlob.printf("%s%s", msg, NEWLINE);
	}

	// If there were Blobs to print, print them passing the blobid

	for (int i = 0; i < varnum; i++)
	{
		const XSQLVAR* var = varlist[i];
		if (!(var->sqltype & 1) || *var->sqlind == 0)
		{
			// Print blob title

			isqlGlob.printf(
					"==============================================================================%s",
					NEWLINE);
			isqlGlob.printf("%s:  %s", var->aliasname, NEWLINE);
			if (ISQL_print_item_blob(isqlGlob.Out, var, M__trans, Doblob) != CONT)
				return (ps_ERR);
			isqlGlob.printf(
					"%s==============================================================================%s",
					NEWLINE, NEWLINE);
		}
	}
	return CONT;
}


// *********************************
// p r i n t _ p e r f o r m a n c e
// *********************************
// As the name implies, show performance as extracted from the API routines.
static void print_performance(const perf64* perf_before)
{
	// Translation of report strings. Do not remove "static" modifier.
	static bool have_report = false;
	static TEXT report_1[MSG_LENGTH + MSG_LENGTH];

	perf64 perf_after;
	perf64_get_info(&DB, &perf_after);
	if (!have_report)
	{
		ISQL_msg_get(REPORT1, report_1);
		// Current memory = !c\nDelta memory = !d\nMax memory = !x\nElapsed time= !e sec\n
		ISQL_msg_get(REPORT2, &report_1[strlen(report_1)]);
		// For GUI_TOOLS:  Buffers = !b\nReads = !r\nWrites = !w\nFetches
		//   = !f\n
		// Else:  Cpu = !u sec\nBuffers = !b\nReads = !r\nWrites = !w\nFetch
		//   es = !f\n
		have_report = true;
	}
	SCHAR statistics[256];
	perf64_format(perf_before, &perf_after, report_1, statistics, NULL);
	ISQL_printf2(Diag, "%s%s", statistics, NEWLINE);
}


// *********************************
// p r i n t _ s q l d a _ i n p u t
// *********************************
// Show the contents of the input sqlda, used typically for parameters.
static processing_state print_sqlda_input(const bool can_have_input_parameters)
{
	// Describe the input
	XSQLDA* input_sqlda = (XSQLDA*) ISQL_ALLOC((SLONG) (XSQLDA_LENGTH(10)));
	memset(input_sqlda, 0, XSQLDA_LENGTH(10));
	input_sqlda->version = SQLDA_VERSION1;
	input_sqlda->sqld = input_sqlda->sqln = 10;

	if (isc_dsql_describe_bind(isc_status, &global_Stmt, isqlGlob.SQL_dialect, input_sqlda))
	{
		ISQL_FREE(input_sqlda);
		ISQL_errmsg(isc_status);
		return ps_ERR;
	}
	else if (isc_status[2] == isc_arg_warning)
		ISQL_warning(isc_status);

	// Reallocate if there's LOTS of input
	if (input_sqlda->sqld > input_sqlda->sqln)
	{
		USHORT n_columns = input_sqlda->sqld;
		ISQL_FREE(input_sqlda);
		input_sqlda = (XSQLDA*) ISQL_ALLOC((SLONG) (XSQLDA_LENGTH(n_columns)));
		memset(input_sqlda, 0, XSQLDA_LENGTH(n_columns));
		input_sqlda->version = SQLDA_VERSION1;
		input_sqlda->sqld = input_sqlda->sqln = n_columns;
		if (isc_dsql_describe_bind(isc_status, &global_Stmt, isqlGlob.SQL_dialect, input_sqlda))
		{
			ISQL_FREE(input_sqlda);
			ISQL_errmsg(isc_status);
			return ps_ERR;
		}
		else if (isc_status[2] == isc_arg_warning)
			ISQL_warning(isc_status);
	}

	if (can_have_input_parameters)
	{
		isqlGlob.printf(
				//"\nINPUT  SQLDA version: %d sqldaid: %s sqldabc: %ld sqln: %d sqld: %d\n",
				"\nINPUT  SQLDA version: %d sqln: %d sqld: %d\n",
				//input_sqlda->version, input_sqlda->sqldaid,
				//input_sqlda->sqldabc, input_sqlda->sqln,
				input_sqlda->version, input_sqlda->sqln,
				input_sqlda->sqld);
		int i = 0;
		const XSQLVAR* var = input_sqlda->sqlvar;
		for (const XSQLVAR* const end = var + input_sqlda->sqld; var < end; var++, i++)
		{
			isqlGlob.printf("%02d: sqltype: %d %s %s sqlscale: %d sqlsubtype: %d sqllen: %d\n",
					i + 1, var->sqltype, sqltype_to_string(var->sqltype),
					(var->sqltype & 1) ? "Nullable" : "		",
					var->sqlscale, var->sqlsubtype, var->sqllen);
			isqlGlob.printf("  :  name: (%d)%*s  alias: (%d)%*s\n",
					var->sqlname_length, var->sqlname_length,
					var->sqlname, var->aliasname_length,
					var->aliasname_length, var->aliasname);
			isqlGlob.printf("  : table: (%d)%*s  owner: (%d)%*s\n",
					var->relname_length, var->relname_length,
					var->relname, var->ownname_length,
					var->ownname_length, var->ownname);
		}
	}

	ISQL_FREE(input_sqlda);
	return CONT;
}


// ***********************************
// p r i n t _ s q l d a _ o u t p u t
// ***********************************
// Show the contents of the output sqlda, that's the result of SELECT from
// both tables and procedures and execution of procedures that return output.
static void print_sqlda_output(const XSQLDA& sqlda)
{
	isqlGlob.printf(
			//"\nOUTPUT SQLDA version: %d sqldaid: %s sqldabc: %ld sqln: %d sqld: %d\n",
			"\nOUTPUT SQLDA version: %d sqln: %d sqld: %d\n",
			//sqlda.version, sqlda.sqldaid, sqlda.sqldabc, sqlda.sqln,
			sqlda.version, sqlda.sqln,
			sqlda.sqld);
	int i = 0;
	const XSQLVAR* var = sqlda.sqlvar;
	for (const XSQLVAR* const end = var + sqlda.sqld; var < end; var++, i++)
	{
		isqlGlob.printf("%02d: sqltype: %d %s %s sqlscale: %d sqlsubtype: %d sqllen: %d\n",
				i + 1, var->sqltype, sqltype_to_string(var->sqltype),
				(var->sqltype & 1) ? "Nullable" : "		",
				var->sqlscale, var->sqlsubtype, var->sqllen);
		isqlGlob.printf("  :  name: (%d)%*s  alias: (%d)%*s\n",
				var->sqlname_length, var->sqlname_length, var->sqlname,
				var->aliasname_length, var->aliasname_length,
				var->aliasname);
		isqlGlob.printf("  : table: (%d)%*s  owner: (%d)%*s\n",
				var->relname_length, var->relname_length, var->relname,
				var->ownname_length, var->ownname_length, var->ownname);
	}
}


// ***************************
// p r o c e s s _ h e a d e r
// ***************************
// Write into buffers the information that will be printed as page header for
// statements that return data.
static void process_header(const XSQLDA* sqlda, const int pad[], TEXT header[], TEXT header2[])
{
	// Create the column header :  Left justify strings

	TEXT* p = header;
	TEXT* p2 = header2;
	int i = 0;
	const XSQLVAR* var = sqlda->sqlvar;
	for (const XSQLVAR* const end = var + sqlda->sqld; var < end; var++, i++)
	{
		const SSHORT type = var->sqltype & ~1;
		if (type == SQL_TEXT || type == SQL_VARYING)
			sprintf(p, "%-*.*s ", pad[i], pad[i], var->aliasname);
		else
			sprintf(p, "%*s ", pad[i], var->aliasname);
		// Separators need not go on forever no more than a line
		size_t limit = strlen(p);
		for (size_t j = 1; j < limit && j < 80; j++)
			*p2++ = '=';
		*p2++ = BLANK;
		p += limit;
	}
	*p2 = '\0';
}


// ***********************
// p r o c e s s _ p l a n
// ***********************
// Retrieve and show the server's execution plan.
// We don't consider critical a failure to get the plan, so we don't return
// any result to the caller.
static void process_plan()
{
	// Bug 7565: A plan larger than plan_buffer will not be displayed
	// Bug 7565: Note also that the plan must fit into Print_Buffer

	const SCHAR plan_info[] = { isc_info_sql_get_plan };
	TEXT plan_buffer[PRINT_BUFFER_LENGTH];
	memset(plan_buffer, 0, sizeof(plan_buffer));
	char* planPtr = plan_buffer;
	size_t planSize = sizeof(plan_buffer);
	PtrSentry<char> planSentry;

	for (int i = 0; i < 3; ++i)
	{
		if (isc_dsql_sql_info(isc_status, &global_Stmt, sizeof(plan_info), plan_info,
							  planSize, planPtr))
		{
			ISQL_errmsg(isc_status);
			return;
		}

		switch (planPtr[0])
		{
		case isc_info_sql_get_plan:
			{
				ULONG l = gds__vax_integer(reinterpret_cast<const UCHAR*>(planPtr) + 1, 2);
				if (l > planSize - 3)
					l = planSize - 3;
				ISQL_printf2(Diag, "%.*s%s", l, planPtr + 3, NEWLINE);
			}
			return;
		case isc_info_truncated:
			switch (i)
			{
			case 0:
				planSentry.exchange((planPtr = new char[planSize = MAX_SSHORT]), true);
				break;
			case 1:
				// Probably FB 2.1 and before will crash with MAX_USHORT.
				if ((isqlGlob.major_ods == 11 && isqlGlob.minor_ods >= 2) || // FB2.5
					isqlGlob.major_ods > 11)
				{
					planSentry.clean();
					planSentry.exchange((planPtr = new char[planSize = MAX_USHORT]), true);
					break;
				}
				// else fall into
			case 2:
				ISQL_printf2(Diag, "Plan truncated%s", NEWLINE);
				return;
			default:
				ISQL_printf2(Diag, "Logical error in isql%s", NEWLINE);
				fb_assert(false);
				return;
			}
			break;
		case isc_info_end:
			return; // Assume no plan.
		default:
			ISQL_printf2(Diag, "Unknown error while retrieving plan%s", NEWLINE);
			return;
		}
	}
}


// ***************************************
// p r o c e s s _ r e c o r d _ c o u n t
// ***************************************
// Get number of records affected for updates, deletions and insertions.
// Return -1 if the statement is not one of those or if there's an error
// retrieving the information from the server.
static SINT64 process_record_count(const int statement_type)
{
	UCHAR count_type = 0;

	// Skip selects, better to count records incoming later
	switch (statement_type)
	{
	case isc_info_sql_stmt_update:
		count_type = isc_info_req_update_count;
		break;
	case isc_info_sql_stmt_delete:
		count_type = isc_info_req_delete_count;
		break;
	case isc_info_sql_stmt_insert:
		count_type = isc_info_req_insert_count;
		break;
	}

	if (count_type)
	{
		const SCHAR count_info[] = { isc_info_sql_records };
		SCHAR count_buffer[33];
		if (isc_dsql_sql_info(isc_status, &global_Stmt, sizeof(count_info), count_info,
							  sizeof(count_buffer), count_buffer))
		{
			ISQL_errmsg(isc_status);
			return -1;
		}

		if (count_buffer[0] == isc_info_sql_records)
		{
			SINT64 total = 0;
			const UCHAR* p = reinterpret_cast<const UCHAR*>(count_buffer + 3);
			while (*p != isc_info_end)
			{
				const UCHAR count_is = *p++;
				const SSHORT l = gds__vax_integer(p, 2);
				p += 2;
				const ULONG count = gds__vax_integer(p, l);
				p += l;
				if (count_is != isc_info_req_select_count)
					total += count;
			}
			return total;
		}
	}
	return -1;
}


// ***************************************
// p r o c e s s _ r e q u e s t _ t y p e
// ***************************************
// Retrieve the statement type according to the DSQL layer.
// A failure is indicated by returning zero.
static SSHORT process_request_type()
{
	const SCHAR sqlda_info[] = { isc_info_sql_stmt_type };
	SCHAR info_buffer[16];
	if (isc_dsql_sql_info (isc_status, &global_Stmt, sizeof (sqlda_info), sqlda_info,
						   sizeof (info_buffer), info_buffer))
	{
		ISQL_errmsg (isc_status);
	}
	else
	{
		if (info_buffer[0] == isc_info_sql_stmt_type)
		{
		    const UCHAR* b = reinterpret_cast<const UCHAR*>(info_buffer);
			const SSHORT l = gds__vax_integer(b + 1, 2);
			return (SSHORT) gds__vax_integer(b + 3, l);
		}

		//ISQL_msg_get(UNKNOWN_STATEMENT_TYPE, errmsg);
		STDERROUT("Cannot determine statement type");
	}
	return 0;
}


// ***************************************
// p r o c e s s _ s q l d a _ b u f f e r
// ***************************************
// Generate the buffer for sqlda output and assign the null indicators already created.
// The caller should deallocate the returned buffer after using it.
static SLONG* process_sqlda_buffer(XSQLDA* const sqlda, SSHORT nullind[])
{
	SLONG bufsize = 0;
	SSHORT* nullp = nullind;
	XSQLVAR* var = sqlda->sqlvar;
	for (const XSQLVAR* const end = var + sqlda->sqld; var < end; var++)
	{
		// Allocate enough space for all the sqldata and null pointers

		const SSHORT type = var->sqltype & ~1;
		USHORT alignment, data_length;
		alignment = data_length = var->sqllen;

		// special case db_key alignment which arrives as an SQL_TEXT
		// Bug 8562:  since DB_KEY arrives as SQL_TEXT, we need to account
		// for the null character at the end of the string
		if (!strncmp(var->sqlname, "DB_KEY", 6))
		{
			alignment = 8;
			// Room for null string terminator
			data_length++;
		}
		else
		{
			// Special alignment cases
			if (type == SQL_TEXT)
			{
				alignment = 1;
				// Room for null string terminator
				data_length++;
			}
			else if (type == SQL_VARYING)
			{
				alignment = sizeof(SSHORT);
				// Room for a null string terminator for display
				data_length += sizeof(SSHORT) + 1;
			}
		}

		bufsize = FB_ALIGN(bufsize, alignment) + data_length;
		var->sqlind = nullp++;
	}

	// Buffer for reading data from the fetch
	if (bufsize)
	{
		SLONG* buffer = (SLONG*) ISQL_ALLOC ((SLONG) bufsize);
		memset(buffer, 0, (size_t) bufsize);
		return buffer;
	}
	return 0;
}


// *****************************************
// p r o c e s s _ s q l d a _ d i s p l a y
// *****************************************
// Calculate individual column widths and return total line width.
// Assign previously allocated buffer in pieces to every XSQLVAR.
static SLONG process_sqlda_display(XSQLDA* const sqlda, SLONG buffer[], int pad[])
{
	int linelength = 0;
	SLONG offset = 0;
	int i = 0;
	XSQLVAR* var = sqlda->sqlvar;
	for (const XSQLVAR* const end = var + sqlda->sqld; var < end; ++var, ++i)
	{
		// Record the length of name and var, then convert to print
		// length, later to be stored in pad array

		USHORT data_length, disp_length, alignment;
		data_length = disp_length = alignment = var->sqllen;
		SSHORT namelength = var->aliasname_length;

		// Minimum display length should not be less than that needed
		// for displaying null

		if (namelength < NULL_DISP_LEN)
			namelength = NULL_DISP_LEN;

		const SSHORT type = var->sqltype & ~1;

		switch (type)
		{
		case SQL_BLOB:
		case SQL_ARRAY:
			// enough room for the blob id to print

			disp_length = 17;
			break;
		case SQL_TIMESTAMP:
			if (Time_display || isqlGlob.SQL_dialect > SQL_DIALECT_V5)
				disp_length = DATETIME_LEN;
			else
				disp_length = DATE_ONLY_LEN;
			break;
		case SQL_TYPE_TIME:
			disp_length = TIME_ONLY_LEN;
			break;
		case SQL_TYPE_DATE:
			disp_length = DATE_ONLY_LEN;
			break;
		case SQL_FLOAT:
			disp_length = FLOAT_LEN;
			break;
		case SQL_DOUBLE:
			disp_length = DOUBLE_LEN;
			break;
		case SQL_TEXT:
			alignment = 1;
			data_length++;
			// OCTETS data is displayed in hex
			if (var->sqlsubtype == 1)
				disp_length = 2 * var->sqllen;
			break;
		case SQL_VARYING:
			data_length += sizeof(USHORT) + 1;
			alignment = sizeof(USHORT);
			// OCTETS data is displayed in hex
			if (var->sqlsubtype == 1)
				disp_length = 2 * var->sqllen;
			break;
		case SQL_SHORT:
			disp_length = SHORT_LEN;
			break;
		case SQL_LONG:
			disp_length = LONG_LEN;
			break;
		case SQL_INT64:
			disp_length = INT64_LEN;
			break;
#ifdef NATIVE_QUAD
		case SQL_QUAD:
			disp_length = QUAD_LEN;
			break;
#endif
		default:
			disp_length = UNKNOWN_LEN;
			break;
		}

		// special case db_key alignment which arrives as an SQL_TEXT

		if (!strncmp(var->sqlname, "DB_KEY", 6))
		{
			alignment = 8;
			disp_length = 2 * var->sqllen;
		}

		// This is the print width of each column

		pad[i] = (disp_length > namelength ? disp_length : namelength);

		// Is there a collist entry, then use that width, but only for text
		if (type == SQL_TEXT || type == SQL_VARYING)
		{
			if (!global_Cols.find(var->aliasname, &pad[i]) && global_Col_default)
				pad[i] = global_Col_default;
		}

		// The total line length
		linelength += pad[i] + 1;

		// Allocate space in buffer for data

		offset = FB_ALIGN(offset, alignment);
		var->sqldata = (SCHAR*) buffer + offset;
		offset += data_length;
	}
	return linelength;
}


static int process_statement(const TEXT* string, XSQLDA** sqldap)
{
/**************************************
 *
 *	p r o c e s s _ s t a t e m e n t
 *
 **************************************
 *
 * Functional description
 *	Prepare and execute a dynamic SQL statement.
 *	This function uses the isc_dsql user functions rather
 *	than embedded dynamic statements.  The user request
 *	is placed on transaction M__trans, while all
 *	background work is on the default gds_trans.
 *	This function now returns CONT (success) or ps_ERR.
 **************************************/

	// Here we actively use the fact that fb_cancel_enable/disable commands may be send many times.
	// They are ignored if cancel already has requested state. This let's us disable cancel
	// in the middle of processing, but do not care about special scope for CancelHolder variable.
	class CancelHolder
	{
	public:
		CancelHolder()
		{
			fb_cancel_operation(isc_status, &DB, fb_cancel_enable);
		}
		~CancelHolder()
		{
			fb_cancel_operation(isc_status, &DB, fb_cancel_disable);
		}
	};

	processing_state ret = CONT;

	// enable CANCEL during statement processing
	CancelHolder cHolder;

	// We received the address of the sqlda in case we realloc it
	XSQLDA* sqlda = *sqldap;

	// If somebody did a commit or rollback, we are out of a transaction

	if (!M__trans)
	{
		if (isc_start_transaction(isc_status, &M__trans, 1, &DB, 0, NULL))
			ISQL_errmsg(isc_status);
	}

	// No need to start a default transaction unless there is no current one

	if (Autocommit && !D__trans)
	{
		if (isc_start_transaction(isc_status, &D__trans, 1, &DB, 5, default_tpb))
		{
			ISQL_errmsg(isc_status);
		}
	}

	// If statistics are requested, then reserve them here

	perf64 perf_before;
	if (Stats)
		perf64_get_info(&DB, &perf_before);

	// Prepare the dynamic query stored in string.
	// But put this on the DDL transaction to get maximum visibility of
	// metadata.

	FB_API_HANDLE prepare_trans;
	if (Autocommit)
		prepare_trans = D__trans;
	else
		prepare_trans = M__trans;

	if (isc_dsql_prepare(isc_status, &prepare_trans, &global_Stmt, 0, string,
						 isqlGlob.SQL_dialect, sqlda))
	{
		if (isqlGlob.SQL_dialect == SQL_DIALECT_V6_TRANSITION && Input_file)
		{
			isqlGlob.printf("%s%s%s%s%s%s",
							NEWLINE,
							"**** Error preparing statement:",
							NEWLINE,
							NEWLINE,
							string,
							NEWLINE);
		}
		ISQL_errmsg(isc_status);
		return ps_ERR;
	}

	// check for warnings
	if (isc_status[2] == isc_arg_warning)
		ISQL_warning(isc_status);

	// Find out what kind of statement this is
	const int statement_type = process_request_type();
	if (!statement_type)
		return ps_ERR;

//#ifdef DEV_BUILD
	if (Sqlda_display)
	{
		const bool can_have_input_parameters =
			statement_type == isc_info_sql_stmt_select ||
			statement_type == isc_info_sql_stmt_insert ||
			statement_type == isc_info_sql_stmt_update ||
			statement_type == isc_info_sql_stmt_delete ||
			statement_type == isc_info_sql_stmt_exec_procedure ||
			statement_type == isc_info_sql_stmt_select_for_upd;

		if (print_sqlda_input(can_have_input_parameters) == ps_ERR)
			return ps_ERR;
	}
//#endif // DEV_BUILD


	// check for warnings
	if (isc_status[2] == isc_arg_warning)
		ISQL_warning(isc_status);

	const bool is_selectable =
		statement_type == isc_info_sql_stmt_select ||
		statement_type == isc_info_sql_stmt_select_for_upd ||
		statement_type == isc_info_sql_stmt_exec_procedure ||
		statement_type == isc_info_sql_stmt_get_segment;

	// if PLAN is set and this is not DDL, print out the plan now

	if (Plan && statement_type != isc_info_sql_stmt_ddl)
	{
		process_plan();
		if (Planonly && !is_selectable)
		{
			return ret;	// do not execute
		}
	}

	// If the statement isn't a select, execute it and be done

	if (!is_selectable && !Planonly)
	{

		// If this is an autocommit, put the DDL stmt on a special trans

		if (Autocommit && (statement_type == isc_info_sql_stmt_ddl))
		{
			if (isc_dsql_execute_immediate(isc_status, &DB, &D__trans, 0,
										   string, isqlGlob.SQL_dialect, NULL))
			{
				ISQL_errmsg(isc_status);
				ret = ps_ERR;
			}
			else if (isc_status[2] == isc_arg_warning)
			{
				// check for warnings
				ISQL_warning(isc_status);
			}

			// CVC: DDL statements that aren't syntax errors are caught by DFW
			// only at commit time, so we need to check here.
			// AP: isc_status will be cleaned in commit_trans()
			if (!commit_trans(&D__trans))
				ret = ps_ERR;

			// check for warnings from COMMIT
			if (isc_status[2] == isc_arg_warning)
				ISQL_warning(isc_status);

			if (Stats)
				print_performance(&perf_before);
			return ret;
		}

		// Check to see if this is a SET TRANSACTION statement

		if (statement_type == isc_info_sql_stmt_start_trans)
		{
			// CVC: Starting a txn can fail, too. Let's check it, although I
			// suspect isql will catch it in frontend_set() through get_statement(),
			// so this place has little chance to be reached.
			if (newtrans(string) == FAIL)
				return ps_ERR;

			if (Stats)
				print_performance(&perf_before);
			return ret;
		}

		//  This is a non-select DML statement or trans

		if (isc_dsql_execute(isc_status, &M__trans, &global_Stmt, isqlGlob.SQL_dialect, NULL))
		{
			ISQL_errmsg(isc_status);
			// CVC: Make this conditional if it causes problems. For example
			// if (BailOnError)
			ret = ps_ERR;
		}

		// check for warnings
		if (isc_status[2] == isc_arg_warning)
			ISQL_warning(isc_status);

		// We are executing a commit or rollback, commit default trans

		if ((statement_type == isc_info_sql_stmt_commit) ||
			(statement_type == isc_info_sql_stmt_rollback))
		{
			// CVC: Commit may fail with AUTO-DDL off and DDL changes rejected by DFW.
			if (D__trans && !commit_trans(&D__trans))
				ret = ps_ERR;
		}

		// Print statistics report

		if (Docount)
		{
			const SINT64 count = process_record_count(statement_type);
			if (count >= 0)
			{
				TEXT rec_count_msg[MSG_LENGTH];
				ISQL_msg_get(REC_COUNT, rec_count_msg, SafeArg() << count);
				// Records affected: %ld
				isqlGlob.printf("%s%s", rec_count_msg, NEWLINE);
			}
		}

		if (Stats)
			print_performance(&perf_before);
		return ret;
	}

	const SSHORT n_cols = sqlda->sqld;

	// The query is bigger than the curent sqlda, so make more room

	if (n_cols > sqlda->sqln)
	{
		ISQL_FREE(sqlda);
		sqlda = (XSQLDA*) ISQL_ALLOC((SLONG) (XSQLDA_LENGTH(n_cols)));
		memset(sqlda, 0, XSQLDA_LENGTH(n_cols));
		*sqldap = sqlda;
		sqlda->version = SQLDA_VERSION1;
		sqlda->sqld = sqlda->sqln = n_cols;

		//  We must re-describe the sqlda, no need to re-prepare

		if (isc_dsql_describe(isc_status, &global_Stmt, isqlGlob.SQL_dialect, sqlda))
		{
			ISQL_errmsg(isc_status);
			return ps_ERR;
		}
	}

//#ifdef DEV_BUILD

	// To facilitate debugging, the option
	// SET SQLDA_DISPLAY ON
	// will activate code to display the SQLDA after each statement.

	if (Sqlda_display)
		print_sqlda_output(*sqlda);
//#endif // DEV_BUILD
	if (Planonly)
		return ret;

	if (statement_type != isc_info_sql_stmt_exec_procedure)
	{
		// Otherwise, open the cursor to start things up

#ifdef SCROLLABLE_CURSORS
		// Declare the C cursor for what it is worth
		// CVC: isql doesn't do positioned updates, so I assumed declaring a cursor
		// may be needed if scrollable cursors are enabled in the engine.

		if (isc_dsql_set_cursor_name(isc_status, &global_Stmt, "C ", 0))
		{
			ISQL_errmsg(isc_status);
			return (ps_ERR);
		}

		// check for warnings
		if (isc_status[2] == isc_arg_warning)
			ISQL_warning(isc_status);
#endif

		if (isc_dsql_execute(isc_status, &M__trans, &global_Stmt, isqlGlob.SQL_dialect, NULL))
		{
			ISQL_errmsg(isc_status);
			return (ps_ERR);
		}

		// check for warnings
		if (isc_status[2] == isc_arg_warning)
			ISQL_warning(isc_status);

	}

	SSHORT* nullind = NULL;
	if (n_cols) {
		nullind = (SSHORT*) ISQL_ALLOC((SLONG) (n_cols * sizeof(SSHORT)));
	}

	SLONG* buffer = process_sqlda_buffer(sqlda, nullind);

	// Pad is an array of lengths to be passed to the print_item
	int* pad = NULL;
	if (sqlda->sqld) {
		pad = (int*) ISQL_ALLOC ((SLONG) (sqlda->sqld * sizeof(int)));
	}

	// Calculate display width and add a few for line termination, et al
    const SLONG linelength = process_sqlda_display(sqlda, buffer, pad) + 10;

	// Allocate the print line, the header line and the separator

	TEXT* line = (TEXT*) ISQL_ALLOC(linelength);
	TEXT* header = 0;
	TEXT* header2 = 0;

	if (Heading)
	{
		header = (TEXT*) ISQL_ALLOC(linelength);
		header2 = (TEXT*) ISQL_ALLOC(linelength);
		*header = '\0';
		*header2 = '\0';

		process_header(sqlda, pad, header, header2);
	}

	// If this is an exec procedure, execute into the sqlda with one fetch only

	if (statement_type == isc_info_sql_stmt_exec_procedure)
	{
		if (isc_dsql_execute2(isc_status, &M__trans, &global_Stmt,
							  isqlGlob.SQL_dialect, NULL, sqlda))
		{
			ISQL_errmsg(isc_status);
			ret = ps_ERR;
		}
		else
		{
			if (sqlda->sqld)
			{ // do not output unnecessary white text
				isqlGlob.printf(NEWLINE);
				if (!List && Heading)
				{
					isqlGlob.printf("%s%s%s%s",
									header,
									NEWLINE,
									header2,
									NEWLINE);
				}
				print_line(sqlda, pad, line);
				isqlGlob.printf(NEWLINE);
			}
		}
	}
	else
	{
		// Otherwise fetch and print records until EOF

#ifdef HAVE_TERMIOS_H
		int out_fd = fileno(isqlGlob.Out);
		if (isatty(out_fd))
		{
			struct termios tflags;
			if (tcgetattr(out_fd, &tflags) == 0)
			{
				tflags.c_lflag |= NOFLSH;
				tcsetattr(out_fd, TCSANOW, &tflags);
			}
		}
#endif

		const bool printHead = !List && Heading;
		unsigned int lines;
		for (lines = 0; !Interrupt_flag && !Abort_flag; ++lines)
		{

			// Check if exceeded rowCount value
			if (rowCount != 0 && lines >= rowCount)
				break;

			// Fetch the current cursor

#ifdef SCROLLABLE_CURSORS
			if (Autofetch)
			{
#endif
				if (isc_dsql_fetch(isc_status, &global_Stmt, isqlGlob.SQL_dialect, sqlda) == 100)
					break;
#ifdef SCROLLABLE_CURSORS
			}
			else
			{
				TEXT fetch_statement[BUFFER_LENGTH80];

				// if not autofetching, sit and wait for user to input
				// NEXT, PRIOR, FIRST, LAST, ABSOLUTE <n>, or RELATIVE <n>

				const SSHORT fetch_ret =
					get_statement(fetch_statement, sizeof(fetch_statement), fetch_prompt);
				if (fetch_ret == FETCH)
				{
					if (isc_dsql_fetch2(isc_status, &global_Stmt, isqlGlob.SQL_dialect, sqlda,
										fetch_direction, fetch_offset) == 100)
					{
						break;
					}
				}
				else if (fetch_ret == BACKOUT || fetch_ret == EXIT)
				{
					lines = 0;
					break;
				}
				else
				{
					TEXT errbuf[MSG_LENGTH];
					ISQL_msg_get(CMD_ERR, errbuf, SafeArg() <<
								"Expected NEXT, PRIOR, FIRST, LAST, ABSOLUTE <n>, RELATIVE <n>, or QUIT");
					STDERROUT(errbuf);
				}

			}
#endif

			// Print the header every Pagelength number of lines for
			// command-line ISQL only.  For WISQL, print the column
			// headings only once.

			if (printHead && 
				(Pagelength && (lines % Pagelength == 0) ||
				 !Pagelength && !lines) )
			{
				isqlGlob.printf("%s%s%s%s%s",
								NEWLINE,
								header,
								NEWLINE,
								header2,
								NEWLINE);
			}

			if (isc_status[1])
			{
				// CVC: Add a \n so the error message is not at the right of
				// header but in its own line, if no row was output yet.
				if (!lines && !printHead)
					isqlGlob.printf(NEWLINE);

				ISQL_errmsg(isc_status);

				// Avoid cancel during cleanup
				fb_cancel_operation(isc_status, &DB, fb_cancel_disable);

				// CVC: It appears we can have an implicit cursor opened anyways.
				isc_dsql_free_statement(isc_status, &global_Stmt, DSQL_close);
				if (nullind)
					ISQL_FREE(nullind);
				if (pad)
					ISQL_FREE(pad);
				if (buffer)
					ISQL_FREE(buffer);
				if (header)
					ISQL_FREE(header);
				if (header2)
					ISQL_FREE(header2);
				if (line)
					ISQL_FREE(line);

				return (ps_ERR);
			}

			if (!lines && !printHead)
				isqlGlob.printf(NEWLINE);

			ret = print_line(sqlda, pad, line);
		}

		if (lines)
			isqlGlob.printf(NEWLINE);

		// Record count printed here upon request

		if (Docount)
		{
			TEXT rec_count_msg[MSG_LENGTH];
			ISQL_msg_get(REC_COUNT, rec_count_msg, SafeArg() << lines);
			// Records affected: %ld
			isqlGlob.printf("%s%s", rec_count_msg, NEWLINE);
		}
	}

	// Avoid cancel during cleanup
	fb_cancel_operation(isc_status, &DB, fb_cancel_disable);

	// CVC: It appears we can have an implicit cursor opened anyways.
	if (statement_type != isc_info_sql_stmt_exec_procedure) {
		isc_dsql_free_statement(isc_status, &global_Stmt, DSQL_close);
	}

	// Statistics printed here upon request

	if (Stats)
		print_performance(&perf_before);

	if (pad)
		ISQL_FREE(pad);
	if (buffer)
		ISQL_FREE(buffer);
	if (line)
		ISQL_FREE(line);
	if (header)
		ISQL_FREE(header);
	if (header2)
		ISQL_FREE(header2);
	if (nullind)
		ISQL_FREE(nullind);

	return (ret);
}


#ifdef WIN_NT
static BOOL CALLBACK query_abort(DWORD dwCtrlType)
#else
static int query_abort(const int reason, const int, void*)
#endif
{
/**************************************
 *
 *	q u e r y _ a b o r t
 *
 **************************************
 *
 * Functional description
 *	Signal handler for interrupting output of a query.
 *  Note: this function is currently used in completelly different ways in Windows x POSIX.
 *
 **************************************/

	bool flag = true;

#ifdef WIN_NT
	if (dwCtrlType != CTRL_C_EVENT)
		return FALSE;
#else
	if (reason != fb_shutrsn_signal)
	{
		return FB_SUCCESS;
	}
#endif

	if (DB)
	{
		ISC_STATUS_ARRAY status;
		flag = (fb_cancel_operation(status, &DB, fb_cancel_raise) == FB_SUCCESS);
	}

	if (flag)
	{
		if (Interactive)
			Interrupt_flag = true;
		else
			Abort_flag = true;
	}

#ifdef WIN_NT
	return TRUE;
#else
	// we do not want to proceed with shutdown except when exit() was called
	return FB_FAILURE;
#endif
}


// Detect if stdin is redirected, IE we aren't reading from the console.
static bool stdin_redirected()
{
#ifdef WIN_NT
	HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
	const DWORD file_type = GetFileType(in);
	if (file_type == FILE_TYPE_CHAR || file_type == FILE_TYPE_PIPE)
		return false;
#else
	if (isatty(fileno(stdin)))
		return false;
#endif
	return true;
}


// CVC: There's something either wrong or on purpose in this routine:
// it doesn't unescape the embedded quotes that may exist.
// But it's useful for paths as it's now.
static void strip_quotes(const TEXT* in, TEXT* out)
{
/**************************************
 *
 *	s t r i p _ q u o t e s
 *
 **************************************
 *
 * Functional description
 *	Get rid of quotes around strings
 *
 **************************************/
	if (!in || !*in)
	{
		*out = 0;
		return;
	}

	TEXT quote = 0;
	// Skip any initial quote
	if ((*in == DBL_QUOTE) || (*in == SINGLE_QUOTE))
		quote = *in++;
	const TEXT* p = in;

	// Now copy characters until we see the same quote or EOS
	while (*p && (*p != quote)) {
		*out++ = *p++;
	}
	*out = 0;
}


//#ifdef DEV_BUILD
static const char* sqltype_to_string(USHORT sqltype)
{
/**************************************
 *
 *	s q l t y p e _ t o _ s t r i n g
 *
 **************************************
 *
 * Functional description
 *	Return a more readable version of SQLDA.sqltype
 *
 **************************************/
	switch (sqltype & ~1)
	{
	case SQL_TEXT:
		return "TEXT	 ";
	case SQL_VARYING:
		return "VARYING  ";
	case SQL_SHORT:
		return "SHORT	";
	case SQL_LONG:
		return "LONG	 ";
	case SQL_INT64:
		return "INT64	";
	case SQL_FLOAT:
		return "FLOAT	";
	case SQL_DOUBLE:
		return "DOUBLE   ";
	case SQL_D_FLOAT:
		return "D_FLOAT  ";
	case SQL_TIMESTAMP:
		return "TIMESTAMP";
	case SQL_TYPE_DATE:
		return "SQL DATE ";
	case SQL_TYPE_TIME:
		return "TIME	 ";
	case SQL_BLOB:
		return "BLOB	 ";
	case SQL_ARRAY:
		return "ARRAY	";
	case SQL_QUAD:
		return "QUAD	 ";
	case SQL_NULL:
		return "NULL	 ";
	default:
		return "unknown  ";
	}
}
//#endif


