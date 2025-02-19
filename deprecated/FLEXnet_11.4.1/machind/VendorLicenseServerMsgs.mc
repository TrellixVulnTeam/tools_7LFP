;//****************************************************************************
;//	COPYRIGHT (c) 2003 by Macrovision Corporation.	
;//	This software has been provided pursuant to a License Agreement	
;//	containing restrictions on its use.  This software contains
;//	valuable trade secrets and proprietary information of
;//	Macrovision Corporation and is protected by law.
;//	It may 	not be copied or distributed in any form or medium, disclosed
;//	to third parties, reverse engineered or used in any manner not
;//	provided for in said License Agreement except with the prior
;//	written authorization from Macrovision Corporation.
;//*****************************************************************************/
;//*	$Id: VendorLicenseServerMsg.mc,v 1.2 2003/06/06 19:32:50 jwong Exp $   */
;//*****************************************************************************/

;// Mvsn Event Log Message File

;// Message ID Type

MessageIdTypedef=DWORD

;// Severity Names

SeverityNames=(Success       = 0x0:STATUS_SEVERITY_SUCCESS
               Informational = 0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning       = 0x2:STATUS_SEVERITY_WARNING
               Error         = 0x3:STATUS_SEVERITY_ERROR)

;// Facility Names

FacilityNames=(System  = 0x0:FACILITY_SYSTEM
               Runtime = 0x2:FACILITY_RUNTIME
               Stubs   = 0x3:FACILITY_STUBS
               Io      = 0x4:FACILITY_IO_ERROR_CODE)

;// Language Support

LanguageNames=(English = 0x409:VendorLicenseServerMsgs409)

MessageId=0x1
Severity=Success
SymbolicName=CAT_INTERNAL_ERROR
Language=English
Internal Software Logic
.

MessageId=0x2
Severity=Success
SymbolicName=CAT_SYSTEM
Language=English
OS System Configuration
.

MessageId=0x3
Severity=Success
SymbolicName=CAT_ENVIRONMENT
Language=English
Environmental Configuration
.

MessageId=0x4
Severity=Success
SymbolicName=CAT_FLEXLM_LMGRD
Language=English
FLEXnet License Server Manager
.

MessageId=0x5
Severity=Success
SymbolicName=CAT_FLEXLM_LICENSE_SERVER
Language=English
FLEXnet License Server System
.

MessageId=0x6
Severity=Success
SymbolicName=CAT_FLEXLM_LICENSE_FILE
Language=English
FLEXnet License File
.

MessageId=0x7
Severity=Success
SymbolicName=CAT_FLEXLM_OPTIONS_FILE
Language=English
FLEXnet Vendor Options File
.

MessageId=0x8
Severity=Success
SymbolicName=CAT_FLEXLM_LMGRD_PERFORMANCE
Language=English
FLEXnet License Server Manager Performance
.

MessageId=0x9
Severity=Success
SymbolicName=CAT_FLEXLM_LICENSE_SERVER_PERFORMANCE
Language=English
FLEXnet License Server System Performance
.

MessageId=0xA
Severity=Success
SymbolicName=CAT_FLEXLM_LMGRD_HEALTH
Language=English
FLEXnet License Server Manager Health
.

MessageId=0xB
Severity=Success
SymbolicName=CAT_FLEXLM_SERVER_HEALTH
Language=English
FLEXnet License Server System Health
.

MessageId=0xC
Severity=Success
SymbolicName=CAT_FLEXLM_NETWORK_COMM
Language=English
FLEXnet Network Communications
.

MessageId=0xD
Severity=Success
SymbolicName=CAT_FLEXLM_DEBUGLOG
Language=English
FLEXnet Debug Log
.

MessageId=0xE
Severity=Success
SymbolicName=CAT_FLEXLM_SERVER_REPORTLOG
Language=English
FLEXnet License Server System Report Log
.

MessageId=0xF
Severity=Success
SymbolicName=CAT_FLEXLM_LMGRD_EVENT
Language=English
FLEXnet License Server Manager Event
.

MessageId=0x10
Severity=Success
SymbolicName=CAT_FLEXLM_SERVER_EVENT
Language=English
FLEXnet License Server System Event
.

MessageId=0x11
Severity=Success
SymbolicName=CAT_FLEXLM_EVENT_BROKER
Language=English
FLEXnet Event Broker
.

MessageId=0x12
Severity=Success
SymbolicName=CAT_FLEXLM_EVENTLOG
Language=English
FLEXnet Event Log Engine
.

MessageId=0x13
Severity=Success
SymbolicName=CAT_FLEXLM_AGENT
Language=English
FLEXnet Remote Agent
.

;// Message definitions
;// -------------------
MessageId=0x2CC
Severity=Error
Facility=System
SymbolicName=MSG_FLEXLM_LICENSE_EXPIRED
Language=English
(%1) This license (%2) has expired.
.

MessageId=0x2CE
Severity=Warning
Facility=System
SymbolicName=MSG_FLEXLM_NEAR_EXPIRED
Language=English
(%1) Feature: %2, expires on %3.
This is a two week notice.
.

MessageId=0x2D4
Severity=Error
Facility=System
SymbolicName=MSG_FLEXLM_BAD_AUTHCODE
Language=English
(%1) Invalid license key: %2.  Inconsistent authentication code.
.

MessageId=0x2D5
Severity=Informational
Facility=System
SymbolicName=MSG_FLEXLM_NOT_READY
Language=English
(%1) Feature: %2 has not been enabled yet.
The current date is earlier than the start date for this feature.
.

MessageId=0x2D6
Severity=Informational
Facility=System
SymbolicName=MSG_FLEXLM_NOT_READY_YET
Language=English
(%1) Feature: %2 has not been enabled yet; it starts on %3.
The current date is earlier than the start date for this feature.
.

MessageId=0x2D7
Severity=Informational
Facility=System
SymbolicName=MSG_FLEXLM_LICENSE_EXPIRED_ONREREAD
Language=English
(%1) Feature: %2 (Date: %3), has expired on rereading.
.

MessageId=0x2D8
Severity=Warning
Facility=System
SymbolicName=MSG_FLEXLM_LICENSE_NOINCREMENTLINE_ONUPGRADE
Language=English
(%1) Warning: no prior INCREMENT line for UPGRADE.
Feature: %2, Version: %3->%4
.

MessageId=0x2D9
Severity=Error
Facility=System
SymbolicName=MSG_FLEXLM_DAEMON_INIT_ERROR
Language=English
(%1) Daemon initialization error: %2.
.

MessageId=0x2E1
Severity=Error
Facility=System
SymbolicName=MSG_FLEXLM_VENDOR_FEATUREDB_ERROR
Language=English
(%1) Feature database corrupted.
.

MessageId=0x2ED
Severity=Warning
Facility=System
SymbolicName=MSG_FLEXLM_LMGRD_VERSION_ERROR
Language=English
(%1) Client/server comm version mismatch (client: %2.%3, server: %4.%5).
The client and server programs are running potentially incompatible versions of FLEXnet comm software.  
.

MessageId=0x2EE
Severity=Warning
Facility=System
SymbolicName=MSG_FLEXLM_CLIENT_COMM_VERSION_ERROR
Language=English
(%1) Invalid client communications revision: %2 %3 message.
COMM revision: %3 (%5)
Version: %6 (%7)
.

MessageId=0x2F6
Severity=Informational
Facility=System
SymbolicName=MSG_FLEXLM_VENDOR_OPTIONS_FILE
Language=English
(%1) Using options file: %2
.

MessageId=0x2F7
Severity=Warning
Facility=System
SymbolicName=MSG_FLEXLM_VENDOR_OPTIONS_FILE_OPEN_ERROR
Language=English
(%1) Cannot open options file: %2
.

MessageId=0x2F8
Severity=Informational
Facility=System
SymbolicName=MSG_FLEXLM_VENDOR_REPORT_LOG_STARTED
Language=English
(%1) Report logging started, using report log: %2
.

MessageId=0x2F9
Severity=Error
Facility=System
SymbolicName=MSG_FLEXLM_VENDOR_REPORT_LOG_FAILED
Language=English
(%1) Report logging did not start: %2
.

MessageId=0x2FA
Severity=Informational
Facility=System
SymbolicName=MSG_FLEXLM_VENDOR_DEBUG_LOG_STARTED
Language=English
(%1) Debug logging started, using debug log: %2
.

MessageId=0x2FB
Severity=Error
Facility=System
SymbolicName=MSG_FLEXLM_VENDOR_REPORT_LOG_OPEN_ERROR
Language=English
(%1) Cannot open report log: %2; %3
.

MessageId=0x2FC
Severity=Error
Facility=System
SymbolicName=MSG_FLEXLM_VENDOR_WRONG_HOSTID
Language=English
(%1) Wrong hostid on SERVER line in license file: %2.  HOSTID specified, %3, does not match actual: %4.
.

MessageId=0x2FE
Severity=Error
Facility=System
SymbolicName=MSG_FLEXLM_VENDOR_CLOCK_ALTERED
Language=English
(%1) System clock has been altered.
.

MessageId=0x302
Severity=Error
Facility=System
SymbolicName=MSG_FLEXLM_DEBUG_LOG_OPEN_ERROR
Language=English
(%1) Cannot open debug log: %2; %3
.


