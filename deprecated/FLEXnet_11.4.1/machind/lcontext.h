/******************************************************************************

	    Copyright (c) 1997-2007 Macrovision Europe Ltd. and/or Macrovision Corporation. All Rights Reserved.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of
	Macrovision Corporation and is protected by law.  It may
	not be copied or distributed in any form or medium, disclosed
	to third parties, reverse engineered or used in any manner not
	provided for in said License Agreement except with the prior
	written authorization from Macrovision Corporation.

 *****************************************************************************/
/*
 *
 *	Function:	l_context array -- English context strings
 *
 *
 *	Try to make these < 15 characters long
 */
/* NOERROR */		0,
/* NOCONFFILE */	"Filename",
/* BADFILE */		"License text",
/* NOSERVER */		"Hostname",
/* MAXUSERS */		0,
/* NOFEATURE */		0,
/* NOSERVICE */		0,
/* NOSOCKET */		0,
/* BADCODE */		0,
/* NOTTHISHOST */	"Hostid",
/* LONGGONE */		"Expire date",
/* BADDATE */		"Date text",
/* BADCOMM */		"Hostname",
/* NO_SERVER_IN_FILE */	0,
/* BADHOST */		"Hostname",
/* CANTCONNECT */	"Server name",
/* CANTREAD */		"Hostname",
/* CANTWRITE */		"Hostname",
/* NOSERVSUPP */	"Server name",
/* SELECTERR */		0,
/* SERVBUSY */		0,
/* OLDVER */		"Application version > License version",
/* CHECKINBAD */	0,
/* BUSYNEWSERV */	0,
/* USERSQUEUED */	0,
/* SERVLONGGONE */	0,
/* TOOMANY */		0,
/* CANTREADKMEM */	0,
/* CANTREADVMUNIX */	0,
/* CANTFINDETHER */	0,
/* NOREADLIC */		"Hostname",
/* TOOEARLY */		"Start date",
/* NOSUCHATTR */	0,
/* BADHANDSHAKE */	0,
/* CLOCKBAD */		0,
/* FEATQUEUE */		0,
/* FEATCORRUPT */	0,
/* BADFEATPARAM */	0,
/* FEATEXCLUDE */	0,
/* FEATNOTINCLUDE */	0,
/* CANTMALLOC */	0,
/* NEVERCHECKOUT */	0,
/* BADPARAM */		0,
/* NOKEYDATA */		0,
/* BADKEYDATA */	0,
/* FUNCNOTAVAIL */	0,
/* DEMOKIT */		0,
/* NOCLOCKCHECK */	0,
/* BADPLATFORM */	0,
/* DATE_TOOBIG */	0,
/* EXPIREDKEYS */	0,
/* NOFLEXLMINIT */	0,
/* NOSERVRESP */	0,
/* CHECKOUTFILTERED */	0,
/* NOFEATSET */		0,
/* BADFEATSET */	0,
/* CANTCOMPUTEFEATSET */ 0,
/* SOCKETFAIL */ 	0,
/* SETSOCKFAIL */	0,
/* BADCHECKSUM */	0,
/* SERVBADCHECKSUM */	0,
/* SERVNOREADLIC */	"Server name",
/* NONETWORK */		0,
/* NOTLICADMIN */	0,
/* REMOVETOOSOON */	0,
/* BADVENDORDATA */	0,
/* LIBRARYMISMATCH */	0,
/* NONETOBORROW */	0,
/* NOBORROWSUPP */	0,
/* LM_NOTONSERVER */ 	"Hostid",
/* LM_BORROWLOCKED */	"locked counter != requested",
/* BAD_TZ */		0,
/* OLDVENDORDATA */	0,
/* LOCALFILTER */	0,
/* LM_ENDPATH */	0,
/* LM_VMS_SETIMR_FAILED */ 0,
/* LM_INTERNAL_ERROR */	0,
/* LM_BAD_VERSION */	"Version text",
/* LM_NOADMINAPI */	0,
/* LM_NOFILEOPS */	0,
/* LM_NODATAFILE */	0,
/* LM_NOFILEVSEND */	0,
/* LM_BADPKG */	 	"PACKAGE text",
/* LM_SERVOLDVER */	"Server name",
/* LM_USERBASED	*/ 	0,
/* LM_NOSERVCAP	*/ 	0,
/* LM_OBJECTUSED */	0,
/* LM_MAXLIMIT */	0,
/* LM_BADSYSDATE */	0,
/* LM_PLATNOTLIC */	"Platforms",
/* LM_FUTURE_FILE */ 	"License text",
/* LM_DEFAULT_SEEDS */ 	0,
/* LM_BADATTRS */   0,
/* LM_POOL */ 		0,
/* LM_LGEN_VER */     0,
/* LM_NOT_THIS_HOST */  0,
/* LM_HOSTDOWN */ 	"Hostname",
/* LM_VENDOR_DOWN */ 	"Vendor:Host",
/* LM_CANT_DECIMAL */ 	"License text",
/* LM_BADDECFILE */ 	"License text",
/* LM_REMOVE_LINGER */ 	0,
/* LM_RESVFOROTHERS */ 	0,
/* LM_BORROW_ERROR */ 	"FLEXid",
/* LM_TSOK_ERR */ 	0,
/* LM_BORROWED_TOOLONG */ "Requested hours > allowed hours",
/* LM_UNBORROWED_ALREADY */ 0,
/* LM_SERVER_MAXED_OUT */ 0,
/* LM_BORROWNOCOMP */ 0,
/* LM_BORROW_METEREMPTY */ 0,
/* LM_NOBORROW_METER */ 0,
/* LM_NODONGLE */ 0,
/* LM_NORESLINK */ 0,
/* LM_NODONGLEDRIVER */ "Dongle Type",
/* LM_FLEXLOCK2CKOUT */ 0,
/* LM_SIGN_REQ */ "Missing",
/* LM_PUBKEY_ERR */ 0,
/* LM_NOTRLSUPPORT */ 0,
/* LM_BORROW_LINGER_ERR */ 0,
/* LM_BORROW_EXPIRED */ 0,
/* LM_MUST_BE_LOCAL */ "This host is not server host",
/* LM_BORROW_DOWN */ 0,
/* LM_BORROW_DOWN */ "Hostids",
/* LM_BORROW_DELETE_ERR */ 0,
/* LM_BORROW_RETURN_EARLY_ERR */	0,
/* LM_BORROW_RETURN_SERVER_ERR */	0,
/*	LM_CANT_CHECKOUT_JUST_PACKAGE */	0,
/*	LM_COMPOSITEID_INIT_ERR */	0,
/*	LM_COMPOSITEID_ITEM_ERR */	0,
/*	LM_BORROW_MATCH_ERR */	0,
/*	LM_NULLPOINTER */	0,
/*	LM_BADHANDLE */		0,
/*	LM_EMPTYSTRING */	0,
/*	LM_BADMEMORYACCESS */	0,
/*	LM_NOTSUPPORTED */	0,
/*	LM_NULLJOBHANDLE */	0,
/*	LM_EVENTLOG_INIT_ERR */	0,
/*	LM_EVENTLOG_DISABLED */	0,
/*	LM_EVENTLOG_WRITE_ERR */	0,
/*	LM_BADINDEX */		0,
/* LM_TIMEOUT  */ 0,
/* LM_BADCOMMAND */			0,
/* LM_SOCKET_BROKEN_PIPE */ 0,
/* LM_INVALID_SIGNATURE	*/ 0,
/* LM_UNCOUNTED_NOT_SUPPORTED */ 0,
/* LM_REDUNDANT_SIGNATURES */ 0,
/* LM_BADCODE_V71_LK */ 0,
/* LM_BADCODE_V71_SIGN */ 0,
/* LM_BADCODE_V80_LK */ 0,
/* LM_BADCODE_V80_SIGN */ 0,
/* LM_BADCODE_V81_LK */ 0,
/* LM_BADCODE_V81_SIGN */ 0,
/* LM_BADCODE_V81_SIGN2 */ 0,
/* LM_BADCODE_V84_LK */ 0,
/* LM_BADCODE_V84_SIGN */ 0,
/* LM_BADCODE_V84_SIGN2 */ 0,
/* LM_LK_REQ */ 0,
/* LM_BADAUTH */ 0,
/* LM_REPAIR_NEEDED */ 0,
/* LM_TS_OPEN */ 0,
/* LM_BAD_FULFILLMENT */ 0,
/* LM_BAD_ACTREQ */ 0,
/* LM_TS_NO_FULFILL_MATCH */ 0,
/* LM_BAD_ACT_RESP */	0,
/* LM_CANTRETURN */	0,
/* LM_RETURNEXCEEDSMAX */ 0,
/* LM_NO_REPAIRS_LEFT */ 0,
/* LM_NOT_ALLOWED */ 0,
/* ENTLEXCLUDE */	0,
/* ENTLNOTINCLUDE */	0,
/* LM_ACTIVATION */ 0,
/* LM_TS_BADDATE */ 0,
/* LM_ENCRYPTION_FAILED */ 0,
/* LM_DECRYPTION_FAILED */0,
/* LM_BADCONTEXT */ 0,
/* LM_ONE_TSOK_ERR */ 0
