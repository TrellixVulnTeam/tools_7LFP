/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-2004, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/
/********************************************************************************
*
* File CRESTST.C
*
* Modification History:
*        Name              Date               Description
*   Madhu Katragadda    05/09/2000   Ported Tests for New ResourceBundle API
*   Madhu Katragadda    05/24/2000   Added new tests to test RES_BINARY for collationElements
*********************************************************************************
*/


#include <time.h>
#include "unicode/utypes.h"
#include "cintltst.h"
#include "unicode/utypes.h"
#include "unicode/ustring.h"
#include "unicode/ucnv.h"
#include "string.h"
#include "cstring.h"
#include "unicode/uchar.h"
#include "ucol_imp.h"  /* for U_ICUDATA_COLL */
#define RESTEST_HEAP_CHECK 0

#include "unicode/uloc.h"
#include "uresimp.h"
#include "creststn.h"
#include "unicode/ctest.h"
#include "ucbuf.h"
static int32_t pass;
static int32_t fail;

/*****************************************************************************/
/**
 * Return a random unsigned long l where 0N <= l <= ULONG_MAX.
 */

static uint32_t
randul()
{
    uint32_t l=0;
    int32_t i;
    static UBool initialized = FALSE;
    if (!initialized)
    {
        srand((unsigned)time(NULL));
        initialized = TRUE;
    }
    /* Assume rand has at least 12 bits of precision */
    
    for (i=0; i<sizeof(l); ++i)
        ((char*)&l)[i] = (char)((rand() & 0x0FF0) >> 4);
    return l;
}

/**
 * Return a random double x where 0.0 <= x < 1.0.
 */
static double
randd()
{
    return ((double)randul()) / UINT32_MAX;
}

/**
 * Return a random integer i where 0 <= i < n.
 */
static int32_t randi(int32_t n)
{
    return (int32_t)(randd() * n);
}
/***************************************************************************************/
/**
 * Convert an integer, positive or negative, to a character string radix 10.
 */
static char*
itoa1(int32_t i, char* buf)
{
  char *p = 0;
  char* result = buf;
  /* Handle negative */
  if(i < 0) {
    *buf++ = '-';
    i = -i;
  }

  /* Output digits in reverse order */
  p = buf;
  do {
    *p++ = (char)('0' + (i % 10));
    i /= 10;
  }
  while(i);
  *p-- = 0;

  /* Reverse the string */
  while(buf < p) {
    char c = *buf;
    *buf++ = *p;
    *p-- = c;
  }

  return result;
}
static const int32_t kERROR_COUNT = -1234567;
static const UChar kERROR[] = { 0x0045 /*E*/, 0x0052 /*'R'*/, 0x0052 /*'R'*/,
             0x004F /*'O'*/, 0x0052/*'R'*/, 0x0000 /*'\0'*/};

/*****************************************************************************/

enum E_Where
{
  e_Root,
  e_te,
  e_te_IN,
  e_Where_count
};
typedef enum E_Where E_Where;
/*****************************************************************************/

#define CONFIRM_EQ(actual,expected) if (u_strcmp(expected,actual)==0){ record_pass(); } else { record_fail(); log_err("%s  returned  %s  instead of %s\n", action, austrdup(actual), austrdup(expected)); }
#define CONFIRM_INT_EQ(actual,expected) if ((expected)==(actual)) { record_pass(); } else { record_fail(); log_err("%s returned %d instead of %d\n",  action, actual, expected); }
#define CONFIRM_INT_GE(actual,expected) if ((actual)>=(expected)) { record_pass(); } else { record_fail(); log_err("%s returned %d instead of x >= %d\n",  action, actual, expected); }
#define CONFIRM_INT_NE(actual,expected) if ((expected)!=(actual)) { record_pass(); } else { record_fail(); log_err("%s returned %d instead of x != %d\n",  action, actual, expected); }
#define CONFIRM_ErrorCode(actual,expected) if ((expected)==(actual)) { record_pass(); } else { record_fail();  log_err("%s returned  %s  instead of %s\n", action, myErrorName(actual), myErrorName(expected)); }


/* Array of our test objects */

static struct
{
  const char* name;
  UErrorCode expected_constructor_status;
  E_Where where;
  UBool like[e_Where_count];
  UBool inherits[e_Where_count];
}
param[] =
{
  /* "te" means test */
  /* "IN" means inherits */
  /* "NE" or "ne" means "does not exist" */

  { "root",         U_ZERO_ERROR,             e_Root,    { TRUE, FALSE, FALSE }, { TRUE, FALSE, FALSE } },
  { "te",           U_ZERO_ERROR,             e_te,      { FALSE, TRUE, FALSE }, { TRUE, TRUE, FALSE  } },
  { "te_IN",        U_ZERO_ERROR,             e_te_IN,   { FALSE, FALSE, TRUE }, { TRUE, TRUE, TRUE   } },
  { "te_NE",        U_USING_FALLBACK_WARNING, e_te,      { FALSE, TRUE, FALSE }, { TRUE, TRUE, FALSE  } },
  { "te_IN_NE",     U_USING_FALLBACK_WARNING, e_te_IN,   { FALSE, FALSE, TRUE }, { TRUE, TRUE, TRUE   } },
  { "ne",           U_USING_DEFAULT_WARNING,  e_Root,    { TRUE, FALSE, FALSE }, { TRUE, FALSE, FALSE } }
};

static int32_t bundles_count = sizeof(param) / sizeof(param[0]);


static void printUChars(UChar*);
static void TestDecodedBundle(void);

/***************************************************************************************/

/* Array of our test objects */

void addNEWResourceBundleTest(TestNode** root)
{
    addTest(root, &TestErrorCodes,            "tsutil/creststn/TestErrorCodes");
    addTest(root, &TestEmptyBundle,           "tsutil/creststn/TestEmptyBundle");
    addTest(root, &TestConstruction1,         "tsutil/creststn/TestConstruction1");
    addTest(root, &TestResourceBundles,       "tsutil/creststn/TestResourceBundle");
    addTest(root, &TestFallback,              "tsutil/creststn/TestFallback");
    addTest(root, &TestGetVersion,            "tsutil/creststn/TestGetVersion");
    addTest(root, &TestGetVersionColl,        "tsutil/creststn/TestGetVersionColl");
    addTest(root, &TestAliasConflict,         "tsutil/creststn/TestAliasConflict");
    addTest(root, &TestNewTypes,              "tsutil/creststn/TestNewTypes");
    addTest(root, &TestEmptyTypes,            "tsutil/creststn/TestEmptyTypes");
    addTest(root, &TestBinaryCollationData,   "tsutil/creststn/TestBinaryCollationData");
    addTest(root, &TestAPI,                   "tsutil/creststn/TestAPI");
    addTest(root, &TestErrorConditions,       "tsutil/creststn/TestErrorConditions");
    addTest(root, &TestDecodedBundle,         "tsutil/creststn/TestDecodedBundle");
    addTest(root, &TestResourceLevelAliasing, "tsutil/creststn/TestResourceLevelAliasing");
    addTest(root, &TestDirectAccess,          "tsutil/creststn/TestDirectAccess"); 

}


/***************************************************************************************/
static const char* norwayNames[] = {
    "no_NO_NY",
    "no_NO",
    "no",
    "nn_NO",
    "nn",
    "nb_NO",
    "nb"
};

static const char* norwayLocales[] = {
    "nn_NO",
    "nb_NO",
    "nb",
    "nn_NO",
    "nn",
    "nb_NO",
    "nb"
};

static void checkStatus(int32_t line, UErrorCode expected, UErrorCode status) {
  if(U_FAILURE(status)) {
    log_data_err("Resource not present, cannot test (%s:%d)\n", __FILE__, line);
  }
  if(status != expected) {
    log_err("%s:%d: Expected error code %s, got error code %s\n", __FILE__, line, u_errorName(expected), u_errorName(status));
  }
}

static void TestErrorCodes(void) {
  UErrorCode status = U_USING_DEFAULT_WARNING;

  UResourceBundle *r = NULL, *r2 = NULL;

  /* First check with ICUDATA */
  /* first bundle should return fallback warning */
  r = ures_open(NULL, "ti_ER_ASSAB", &status);
  checkStatus(__LINE__, U_USING_FALLBACK_WARNING, status);
  ures_close(r);

  /* this bundle should return zero error, so it shouldn't change the status*/
  status = U_USING_DEFAULT_WARNING;
  r = ures_open(NULL, "ti_ER", &status);
  checkStatus(__LINE__, U_USING_DEFAULT_WARNING, status);

  /* we look up the resource which is aliased, but it lives in fallback */
  if(U_SUCCESS(status) && r != NULL) {
    status = U_USING_DEFAULT_WARNING; 
    r2 = ures_getByKey(r, "Languages", NULL, &status);  /* languages of 'ti' aliases to 'am' */
    checkStatus(__LINE__, U_USING_FALLBACK_WARNING, status);
  } 
  ures_close(r);

  /* this bundle should return zero error, so it shouldn't change the status*/
  status = U_USING_DEFAULT_WARNING;
  r = ures_open(NULL, "ti", &status);
  checkStatus(__LINE__, U_USING_DEFAULT_WARNING, status);

  /* we look up the resource which is aliased and at our level */
  if(U_SUCCESS(status) && r != NULL) {
    status = U_USING_DEFAULT_WARNING; 
    r2 = ures_getByKey(r, "Languages", r2, &status);
    checkStatus(__LINE__, U_USING_DEFAULT_WARNING, status);
  }
  ures_close(r);

  status = U_USING_FALLBACK_WARNING;
  r = ures_open(NULL, "nolocale", &status);
  checkStatus(__LINE__, U_USING_DEFAULT_WARNING, status);
  ures_close(r);
  ures_close(r2);

  /** Now, with the collation bundle **/
  
  /* first bundle should return fallback warning */
  r = ures_open(U_ICUDATA_COLL, "sr_YU_VOJVODINA", &status);
  checkStatus(__LINE__, U_USING_FALLBACK_WARNING, status);
  ures_close(r);

  /* this bundle should return zero error, so it shouldn't change the status*/
  status = U_USING_FALLBACK_WARNING;
  r = ures_open(U_ICUDATA_COLL, "sr", &status);
  checkStatus(__LINE__, U_USING_FALLBACK_WARNING, status);

  /* we look up the resource which is aliased  */
  if(U_SUCCESS(status) && r != NULL) {
    status = U_USING_DEFAULT_WARNING; 
    r2 = ures_getByKey(r, "collations", NULL, &status);
    checkStatus(__LINE__, U_USING_DEFAULT_WARNING, status);
  } 
  ures_close(r);

  /* this bundle should return zero error, so it shouldn't change the status*/
  status = U_USING_DEFAULT_WARNING;
  r = ures_open(U_ICUDATA_COLL, "sr", &status);
  checkStatus(__LINE__, U_USING_DEFAULT_WARNING, status);

  /* we look up the resource which is aliased and at our level */
  if(U_SUCCESS(status) && r != NULL) {
    status = U_USING_DEFAULT_WARNING; 
    r2 = ures_getByKey(r, "collations", r2, &status);
    checkStatus(__LINE__, U_USING_DEFAULT_WARNING, status);
  }
  ures_close(r);

  status = U_USING_FALLBACK_WARNING;
  r = ures_open(U_ICUDATA_COLL, "nolocale", &status);
  checkStatus(__LINE__, U_USING_DEFAULT_WARNING, status);
  ures_close(r);
  ures_close(r2);
}

static void TestAliasConflict(void) {
    UErrorCode status = U_ZERO_ERROR;
    UResourceBundle *he = NULL;
    UResourceBundle *iw = NULL;
    UResourceBundle *norway = NULL;
    const UChar *result = NULL;
    int32_t resultLen;
    uint32_t size = 0;
    uint32_t i = 0;
    const char *realName = NULL;

    he = ures_open(NULL, "he", &status);
    iw = ures_open(NULL, "iw", &status);
    if(U_FAILURE(status)) { 
        log_err("Failed to get resource with %s\n", myErrorName(status));
    }
    ures_close(iw);
    result = ures_getStringByKey(he, "localPatternChars", &resultLen, &status);
    if(U_FAILURE(status) || result == NULL) { 
        log_err("Failed to get resource localPatternChars with %s\n", myErrorName(status));
    }
    ures_close(he);

    size = sizeof(norwayNames)/sizeof(norwayNames[0]);
    for(i = 0; i < size; i++) {
        status = U_ZERO_ERROR;
        norway = ures_open(NULL, norwayNames[i], &status);
        if(U_FAILURE(status)) { 
            log_err("Failed to get resource with %s for %s\n", myErrorName(status), norwayNames[i]);
            continue;
        }
        realName = ures_getLocale(norway, &status);
        log_verbose("ures_getLocale(\"%s\")=%s\n", norwayNames[i], realName);
        if(realName == NULL || strcmp(norwayLocales[i], realName) != 0) {
            log_data_err("Wrong locale name for %s, expected %s, got %s\n", norwayNames[i], norwayLocales[i], realName);
        }
        ures_close(norway);
    }
}

static void TestDecodedBundle(){
   
    UErrorCode error = U_ZERO_ERROR;
   
    UResourceBundle* resB; 

    const UChar* srcFromRes;
    int32_t len;
    static const UChar uSrc[] = {
        0x0009,0x092F,0x0941,0x0928,0x0947,0x0938,0x094D,0x0915,0x094B,0x0020,0x002E,0x0915,0x0947,0x0020,0x002E,0x090F,
        0x0915,0x0020,0x002E,0x0905,0x0927,0x094D,0x092F,0x092F,0x0928,0x0020,0x002E,0x0915,0x0947,0x0020,0x0905,0x0928,
        0x0941,0x0938,0x093E,0x0930,0x0020,0x0031,0x0039,0x0039,0x0030,0x0020,0x0924,0x0915,0x0020,0x0915,0x0902,0x092A,
        0x094D,0x092F,0x0942,0x091F,0x0930,0x002D,0x092A,0x094D,0x0930,0x092C,0x0902,0x0927,0x093F,0x0924,0x0020,0x0938,
        0x0942,0x091A,0x0928,0x093E,0x092A,0x094D,0x0930,0x0923,0x093E,0x0932,0x0940,0x0020,0x002E,0x0915,0x0947,0x0020,
        0x002E,0x092F,0x094B,0x0917,0x0926,0x093E,0x0928,0x0020,0x002E,0x0915,0x0947,0x0020,0x002E,0x092B,0x0932,0x0938,
        0x094D,0x0935,0x0930,0x0942,0x092A,0x0020,0x002E,0x0935,0x093F,0x0936,0x094D,0x0935,0x0020,0x002E,0x092E,0x0947,
        0x0902,0x0020,0x002E,0x0938,0x093E,0x0932,0x093E,0x0928,0x093E,0x0020,0x002E,0x0032,0x0032,0x0030,0x0030,0x0020,
        0x0905,0x0930,0x092C,0x0020,0x0930,0x0941,0x092A,0x092F,0x0947,0x0020,0x092E,0x0942,0x0932,0x094D,0x092F,0x0915,
        0x0940,0x0020,0x002E,0x0034,0x0935,0x0938,0x094D,0x0924,0x0941,0x0913,0x0902,0x0020,0x002E,0x0034,0x0915,0x093E,
        0x0020,0x002E,0x0034,0x0909,0x0924,0x094D,0x092A,0x093E,0x0926,0x0928,0x0020,0x002E,0x0034,0x0939,0x094B,0x0917,
        0x093E,0x002C,0x0020,0x002E,0x0033,0x091C,0x092C,0x0915,0x093F,0x0020,0x002E,0x0033,0x0915,0x0902,0x092A,0x094D,
        0x092F,0x0942,0x091F,0x0930,0x0020,0x002E,0x0033,0x0915,0x093E,0x0020,0x002E,0x0033,0x0915,0x0941,0x0932,0x0020,
        0x002E,0x0033,0x092F,0x094B,0x0917,0x0926,0x093E,0x0928,0x0020,0x002E,0x0033,0x0907,0x0938,0x0938,0x0947,0x0915,
        0x0939,0x093F,0x0020,0x002E,0x002F,0x091C,0x094D,0x092F,0x093E,0x0926,0x093E,0x0020,0x002E,0x002F,0x0939,0x094B,
        0x0917,0x093E,0x0964,0x0020,0x002E,0x002F,0x0905,0x0928,0x0941,0x0938,0x0902,0x0927,0x093E,0x0928,0x0020,0x002E,
        0x002F,0x0915,0x0940,0x0020,0x002E,0x002F,0x091A,0x0930,0x092E,0x0020,0x0938,0x0940,0x092E,0x093E,0x0913,0x0902,
        0x0020,0x092A,0x0930,0x0020,0x092A,0x0939,0x0941,0x0902,0x091A,0x0928,0x0947,0x0020,0x0915,0x0947,0x0020,0x0932,
        0x093F,0x090F,0x0020,0x0915,0x0902,0x092A,0x094D,0x092F,0x0942,0x091F,0x0930,0x090F,0x0915,0x0020,0x002E,0x002F,
        0x0906,0x092E,0x0020,0x002E,0x002F,0x091C,0x0930,0x0942,0x0930,0x0924,0x0020,0x002E,0x002F,0x091C,0x0948,0x0938,
        0x093E,0x0020,0x092C,0x0928,0x0020,0x0917,0x092F,0x093E,0x0020,0x0939,0x0948,0x0964,0x0020,0x092D,0x093E,0x0930,
        0x0924,0x0020,0x092E,0x0947,0x0902,0x0020,0x092D,0x0940,0x002C,0x0020,0x0916,0x093E,0x0938,0x0915,0x0930,0x0020,
        0x092E,0x094C,0x091C,0x0942,0x0926,0x093E,0x0020,0x0938,0x0930,0x0915,0x093E,0x0930,0x0928,0x0947,0x002C,0x0020,
        0x0915,0x0902,0x092A,0x094D,0x092F,0x0942,0x091F,0x0930,0x0020,0x0915,0x0947,0x0020,0x092A,0x094D,0x0930,0x092F,
        0x094B,0x0917,0x0020,0x092A,0x0930,0x0020,0x091C,0x092C,0x0930,0x0926,0x0938,0x094D,0x0924,0x0020,0x090F,0x095C,
        0x0020,0x0932,0x0917,0x093E,0x092F,0x0940,0x0020,0x0939,0x0948,0x002C,0x0020,0x0915,0x093F,0x0902,0x0924,0x0941,
        0x0020,0x0907,0x0938,0x0915,0x0947,0x0020,0x0938,0x0930,0x092A,0x091F,0x0020,0x0926,0x094C,0x095C,0x0932,0x0917,
        0x093E,0x0928,0x0947,0x0020,0x002E,0x0032,0x0915,0x0947,0x0020,0x002E,0x0032,0x0932,0x093F,0x090F,0x0020,0x002E,
        0x0032,0x0915,0x094D,0x092F,0x093E,0x0020,0x002E,0x0032,0x0938,0x092A,0x093E,0x091F,0x0020,0x002E,0x0032,0x0930,
        0x093E,0x0938,0x094D,0x0924,0x093E,0x0020,0x002E,0x0032,0x0909,0x092A,0x0932,0x092C,0x094D,0x0927,0x0020,0x002E,
        0x0939,0x0948,0x002C,0x0020,0x002E,0x0905,0x0925,0x0935,0x093E,0x0020,0x002E,0x0935,0x093F,0x0936,0x094D,0x0935,
        0x0020,0x002E,0x092E,0x0947,0x0902,0x0020,0x002E,0x0915,0x0902,0x092A,0x094D,0x092F,0x0942,0x091F,0x0930,0x0020,
        0x002E,0x0915,0x0940,0x0938,0x092B,0x0932,0x0924,0x093E,0x0020,0x002E,0x0033,0x0935,0x0020,0x002E,0x0033,0x0935,
        0x093F,0x092B,0x0932,0x0924,0x093E,0x0020,0x002E,0x0033,0x0938,0x0947,0x0020,0x002E,0x0033,0x0938,0x092C,0x0915,
        0x0020,0x002E,0x0033,0x0932,0x0947,0x0020,0x002E,0x0033,0x0915,0x0930,0x0020,0x002E,0x0033,0x0915,0x094D,0x092F,
        0x093E,0x0020,0x002E,0x0033,0x0939,0x092E,0x0020,0x002E,0x0033,0x0907,0x0938,0x0915,0x093E,0x0020,0x002E,0x0033,
        0x092F,0x0941,0x0915,0x094D,0x0924,0x093F,0x092A,0x0942,0x0930,0x094D,0x0923,0x0020,0x002E,0x0032,0x0935,0x093F,
        0x0938,0x094D,0x0924,0x093E,0x0930,0x0020,0x0905,0x092A,0x0947,0x0915,0x094D,0x0937,0x093F,0x0924,0x0020,0x0915,
        0x0930,0x0020,0x0938,0x0915,0x0947,0x0902,0x0917,0x0947,0x0020,0x003F,0x0020,
        0
    };

    /* pre-flight */
    int32_t num =0;
    const char *testdatapath = loadTestData(&error);
    resB = ures_open(testdatapath, "iscii", &error);
    srcFromRes=ures_getStringByKey(resB,"str",&len,&error);
    if(U_FAILURE(error)){
#if UCONFIG_NO_LEGACY_CONVERSION
        log_info("Couldn't load iscii.bin from test data bundle, (because UCONFIG_NO_LEGACY_CONVERSION  is turned on)\n");
#else
        log_err("Could not find iscii.bin from test data bundle. Error: %s\n", u_errorName(error));
#endif
        ures_close(resB);
        return;
    }
    if(u_strncmp(srcFromRes,uSrc,len)!=0){
        log_err("Genrb produced res files after decoding failed\n");
    }
    while(num<len){
        if(uSrc[num]!=srcFromRes[num]){
            log_verbose(" Expected:  0x%04X Got: 0x%04X \n", uSrc[num],srcFromRes[num]);
        }
        num++;
    }
    if (len != u_strlen(uSrc)) {
        log_err("Genrb produced a string larger than expected\n");
    }
    ures_close(resB);
}

static void TestNewTypes() {
    UResourceBundle* theBundle = NULL;
    char action[256];
    const char* testdatapath;
    UErrorCode status = U_ZERO_ERROR;
    UResourceBundle* res = NULL;
    uint8_t *binResult = NULL;
    int32_t len = 0;
    int32_t i = 0;
    int32_t intResult = 0;
    uint32_t uintResult = 0;
    const UChar *empty = NULL;
    const UChar *zeroString;
    UChar expected[] = { 'a','b','c','\0','d','e','f' };
    const char* expect ="tab:\t cr:\r ff:\f newline:\n backslash:\\\\ quote=\\\' doubleQuote=\\\" singlequoutes=''";
    UChar uExpect[200];

    testdatapath=loadTestData(&status);

    if(U_FAILURE(status))
    {
        log_err("Could not load testdata.dat %s \n",myErrorName(status));
        return;
    }

    theBundle = ures_open(testdatapath, "testtypes", &status);

    empty = ures_getStringByKey(theBundle, "emptystring", &len, &status);
    if(empty && (*empty != 0 || len != 0)) {
      log_err("Empty string returned invalid value\n");
    }

    CONFIRM_ErrorCode(status, U_ZERO_ERROR);

    CONFIRM_INT_NE(theBundle, NULL);

    /* This test reads the string "abc\u0000def" from the bundle   */
    /* if everything is working correctly, the size of this string */
    /* should be 7. Everything else is a wrong answer, esp. 3 and 6*/

    strcpy(action, "getting and testing of string with embeded zero");
    res = ures_getByKey(theBundle, "zerotest", res, &status);
    CONFIRM_ErrorCode(status, U_ZERO_ERROR);
    CONFIRM_INT_EQ(ures_getType(res), URES_STRING);
    zeroString=ures_getString(res, &len, &status);
    if(U_SUCCESS(status)){
        CONFIRM_ErrorCode(status, U_ZERO_ERROR);
        CONFIRM_INT_EQ(len, 7);
        CONFIRM_INT_NE(len, 3);
    }
    for(i=0;i<len;i++){
        if(zeroString[i]!= expected[i]){
            log_verbose("Output did not match Expected: \\u%4X Got: \\u%4X", expected[i], zeroString[i]);
        }
    }

    strcpy(action, "getting and testing of binary type");
    res = ures_getByKey(theBundle, "binarytest", res, &status);
    CONFIRM_ErrorCode(status, U_ZERO_ERROR);
    CONFIRM_INT_EQ(ures_getType(res), URES_BINARY);
    binResult=(uint8_t*)ures_getBinary(res,  &len, &status);
    if(U_SUCCESS(status)){
        CONFIRM_ErrorCode(status, U_ZERO_ERROR);
        CONFIRM_INT_EQ(len, 15);
        for(i = 0; i<15; i++) {
            CONFIRM_INT_EQ(binResult[i], i);
        }
    }

    strcpy(action, "getting and testing of imported binary type");
    res = ures_getByKey(theBundle, "importtest", res, &status);
    CONFIRM_ErrorCode(status, U_ZERO_ERROR);
    CONFIRM_INT_EQ(ures_getType(res), URES_BINARY);
    binResult=(uint8_t*)ures_getBinary(res,  &len, &status);
    if(U_SUCCESS(status)){
        CONFIRM_ErrorCode(status, U_ZERO_ERROR);
        CONFIRM_INT_EQ(len, 15);
        for(i = 0; i<15; i++) {
            CONFIRM_INT_EQ(binResult[i], i);
        }
    }

    strcpy(action, "getting and testing of integer types");
    res = ures_getByKey(theBundle, "one", res, &status);
    CONFIRM_ErrorCode(status, U_ZERO_ERROR);
    CONFIRM_INT_EQ(ures_getType(res), URES_INT);
    intResult=ures_getInt(res, &status);
    uintResult = ures_getUInt(res, &status);
    if(U_SUCCESS(status)){
        CONFIRM_ErrorCode(status, U_ZERO_ERROR);
        CONFIRM_INT_EQ(uintResult, (uint32_t)intResult);
        CONFIRM_INT_EQ(intResult, 1);
    }

    strcpy(action, "getting minusone");
    res = ures_getByKey(theBundle, "minusone", res, &status);
    CONFIRM_ErrorCode(status, U_ZERO_ERROR);
    CONFIRM_INT_EQ(ures_getType(res), URES_INT);
    intResult=ures_getInt(res, &status);
    uintResult = ures_getUInt(res, &status);
    if(U_SUCCESS(status)){
        CONFIRM_ErrorCode(status, U_ZERO_ERROR);
        CONFIRM_INT_EQ(uintResult, 0x0FFFFFFF); /* a 28 bit integer */
        CONFIRM_INT_EQ(intResult, -1);
        CONFIRM_INT_NE(uintResult, (uint32_t)intResult);
    }

    strcpy(action, "getting plusone");
    res = ures_getByKey(theBundle, "plusone", res, &status);
    CONFIRM_ErrorCode(status, U_ZERO_ERROR);
    CONFIRM_INT_EQ(ures_getType(res), URES_INT);
    intResult=ures_getInt(res, &status);
    uintResult = ures_getUInt(res, &status);
    if(U_SUCCESS(status)){
        CONFIRM_ErrorCode(status, U_ZERO_ERROR);
        CONFIRM_INT_EQ(uintResult, (uint32_t)intResult);
        CONFIRM_INT_EQ(intResult, 1);
    }

    res = ures_getByKey(theBundle, "onehundredtwentythree", res, &status);
    CONFIRM_ErrorCode(status, U_ZERO_ERROR);
    CONFIRM_INT_EQ(ures_getType(res), URES_INT);
    intResult=ures_getInt(res, &status);
    if(U_SUCCESS(status)){
        CONFIRM_ErrorCode(status, U_ZERO_ERROR);
        CONFIRM_INT_EQ(intResult, 123);
    }

    /* this tests if escapes are preserved or not */
    {
        const UChar* str = ures_getStringByKey(theBundle,"testescape",&len,&status);
        CONFIRM_ErrorCode(status, U_ZERO_ERROR);
        if(U_SUCCESS(status)){
            u_charsToUChars(expect,uExpect,uprv_strlen(expect)+1);
            if(u_strcmp(uExpect,str)){
                log_err("Did not get the expected string for testescape\n");
            }
        }
    }
    /* this tests if unescaping works are expected */
    len=0;
    {
        char pattern[2048] = "";
        int32_t patternLen;
        UChar* expectedEscaped;
        const UChar* got;
        int32_t expectedLen;

        /* This strcpy fixes compiler warnings about long strings */
        strcpy(pattern, "[ \\\\u0020 \\\\u00A0 \\\\u1680 \\\\u2000 \\\\u2001 \\\\u2002 \\\\u2003 \\\\u2004 \\\\u2005 \\\\u2006 \\\\u2007 "
            "\\\\u2008 \\\\u2009 \\\\u200A \\u200B \\\\u202F \\u205F \\\\u3000 \\u0000-\\u001F \\u007F \\u0080-\\u009F "
            "\\\\u06DD \\\\u070F \\\\u180E \\\\u200C \\\\u200D \\\\u2028 \\\\u2029 \\\\u2060 \\\\u2061 \\\\u2062 \\\\u2063 "
            "\\\\u206A-\\\\u206F \\\\uFEFF \\\\uFFF9-\\uFFFC \\U0001D173-\\U0001D17A \\U000F0000-\\U000FFFFD "
            "\\U00100000-\\U0010FFFD \\uFDD0-\\uFDEF \\uFFFE-\\uFFFF \\U0001FFFE-\\U0001FFFF \\U0002FFFE-\\U0002FFFF "
            );
        strcat(pattern, 
            "\\U0003FFFE-\\U0003FFFF \\U0004FFFE-\\U0004FFFF \\U0005FFFE-\\U0005FFFF \\U0006FFFE-\\U0006FFFF "
            "\\U0007FFFE-\\U0007FFFF \\U0008FFFE-\\U0008FFFF \\U0009FFFE-\\U0009FFFF \\U000AFFFE-\\U000AFFFF "
            "\\U000BFFFE-\\U000BFFFF \\U000CFFFE-\\U000CFFFF \\U000DFFFE-\\U000DFFFF \\U000EFFFE-\\U000EFFFF "
            "\\U000FFFFE-\\U000FFFFF \\U0010FFFE-\\U0010FFFF \\uD800-\\uDFFF \\\\uFFF9 \\\\uFFFA \\\\uFFFB "
            "\\uFFFC \\uFFFD \\u2FF0-\\u2FFB \\u0340 \\u0341 \\\\u200E \\\\u200F \\\\u202A \\\\u202B \\\\u202C "
            );
        strcat(pattern, 
            "\\\\u202D \\\\u202E \\\\u206A \\\\u206B \\\\u206C \\\\u206D \\\\u206E \\\\u206F \\U000E0001 \\U000E0020-\\U000E007F "
            "]"
            );

        patternLen = uprv_strlen(pattern);
        expectedEscaped = (UChar*)malloc(U_SIZEOF_UCHAR * patternLen);
        got = ures_getStringByKey(theBundle,"test_unescaping",&len,&status);
        expectedLen = u_unescape(pattern,expectedEscaped,patternLen);
        if(got==NULL || u_strncmp(expectedEscaped,got,expectedLen)!=0 || expectedLen != len){
            log_err("genrb failed to unescape string\n");
        }
        if(got != NULL){
            for(i=0;i<expectedLen;i++){
                if(expectedEscaped[i] != got[i]){
                    log_verbose("Expected: 0x%04X Got: 0x%04X \n",expectedEscaped[i], got[i]);
                }
            }
        }
        free(expectedEscaped);
        status = U_ZERO_ERROR;
    }
    /* test for jitterbug#1435 */
    {
        const UChar* str = ures_getStringByKey(theBundle,"test_underscores",&len,&status);
        expect ="test message ....";
        u_charsToUChars(expect,uExpect,uprv_strlen(expect)+1);
        CONFIRM_ErrorCode(status, U_ZERO_ERROR);
        if(u_strcmp(uExpect,str)){
            log_err("Did not get the expected string for test_underscores.\n");
        }
    }
    /* test for jitterbug#2626 */
    {
        UResourceBundle* resB = NULL;
        const UChar* str  = NULL;
        int32_t strLength = 0;
        const UChar my[] = {0x0026,0x0027,0x0075,0x0027,0x0020,0x003d,0x0020,0x0027,0xff55,0x0027,0x0000}; /* &'\u0075' = '\uFF55' */
        status = U_ZERO_ERROR;
        resB = ures_getByKey(theBundle, "collations", resB, &status);
        resB = ures_getByKey(resB, "standard", resB, &status);
        str  = ures_getStringByKey(resB,"Sequence",&strLength,&status);
        if(!str || U_FAILURE(status)) {
            log_data_err("Could not load collations from theBundle: %s\n", u_errorName(status));
        } else if(u_strcmp(my,str) != 0){
            log_err("Did not get the expected string for escaped \\u0075\n");
        }
        ures_close(resB);
    }
    {
        const char *sourcePath = ctest_dataSrcDir();
        int32_t srcPathLen = strlen(sourcePath);
        const char *deltaPath = ".." U_FILE_SEP_STRING"test" U_FILE_SEP_STRING"testdata" U_FILE_SEP_STRING;
        int32_t deltaPathLen = strlen(deltaPath);
        char *testDataFileName = (char *) malloc( srcPathLen+ deltaPathLen + 50 );
        char *path = testDataFileName;

        strcpy(path, sourcePath);
        path += srcPathLen;
        strcpy(path, deltaPath);
        path += deltaPathLen;
        status = U_ZERO_ERROR;
        {
            int32_t strLen =0;
            const UChar* str = ures_getStringByKey(theBundle, "testincludeUTF",&strLen,&status);
            strcpy(path, "riwords.txt");
            path[strlen("riwords.txt")]=0;
            if(U_FAILURE(status)){
                log_err("Could not get testincludeUTF resource from testtypes bundle. Error: %s\n",u_errorName(status));
            }else{
                /* open the file */
                const char* cp = NULL; 
                UCHARBUF* ucbuf = ucbuf_open(testDataFileName,&cp,FALSE,FALSE,&status);
                len = 0;
                if(U_SUCCESS(status)){
                    const UChar* buffer = ucbuf_getBuffer(ucbuf,&len,&status);
                    if(U_SUCCESS(status)){
                        /* verify the contents */
                        if(strLen != len ){
                            log_err("Did not get the expected len for riwords. Expected: %i , Got: %i\n", len ,strLen);
                        }
                        /* test string termination */
                        if(u_strlen(str) != strLen || str[strLen]!= 0 ){
                            log_err("testinclude not null terminated!\n");
                        }
                        if(u_strncmp(str, buffer,strLen)!=0){
                            log_err("Did not get the expected string from riwords. Include functionality failed for genrb.\n");
                        }
                    }else{
                        log_err("ucbuf failed to open %s. Error: %s\n", testDataFileName, u_errorName(status));
                    }

                    ucbuf_close(ucbuf);
                }else{
                    log_err("Could not get riwords.txt (path : %s). Error: %s\n",testDataFileName,u_errorName(status));
                }
            }
        }
        status = U_ZERO_ERROR;
        {
            int32_t strLen =0;
            const UChar* str = ures_getStringByKey(theBundle, "testinclude",&strLen,&status);
            strcpy(path, "translit_rules.txt");
            path[strlen("translit_rules.txt")]=0;

            if(U_FAILURE(status)){
                log_err("Could not get testinclude resource from testtypes bundle. Error: %s\n",u_errorName(status));
            }else{
                /* open the file */
                const char* cp=NULL;
                UCHARBUF* ucbuf = ucbuf_open(testDataFileName,&cp,FALSE,FALSE,&status);
                len = 0;
                if(U_SUCCESS(status)){
                    const UChar* buffer = ucbuf_getBuffer(ucbuf,&len,&status);
                    if(U_SUCCESS(status)){
                        /* verify the contents */
                        if(strLen != len ){
                            log_err("Did not get the expected len for translit_rules. Expected: %i , Got: %i\n", len ,strLen);
                        }
                        if(u_strncmp(str, buffer,strLen)!=0){
                            log_err("Did not get the expected string from translit_rules. Include functionality failed for genrb.\n");
                        }
                    }else{
                        log_err("ucbuf failed to open %s. Error: %s\n", testDataFileName, u_errorName(status));
                    }
                    ucbuf_close(ucbuf);
                }else{
                    log_err("Could not get translit_rules.txt (path : %s). Error: %s\n",testDataFileName,u_errorName(status));
                }
            }
        }
        free(testDataFileName);
    }
    ures_close(res);
    ures_close(theBundle);

}

static void TestEmptyTypes() {
    UResourceBundle* theBundle = NULL;
    char action[256];
    const char* testdatapath;
    UErrorCode status = U_ZERO_ERROR;
    UResourceBundle* res = NULL;
    UResourceBundle* resArray = NULL;
    const uint8_t *binResult = NULL;
    int32_t len = 0;
    int32_t intResult = 0;
    const UChar *zeroString;
    const int32_t *zeroIntVect;

    strcpy(action, "Construction of testtypes bundle");
    testdatapath=loadTestData(&status);
    if(U_FAILURE(status))
    {
        log_err("Could not load testdata.dat %s \n",myErrorName(status));
        return;
    }
    
    theBundle = ures_open(testdatapath, "testtypes", &status);

    CONFIRM_ErrorCode(status, U_ZERO_ERROR);

    CONFIRM_INT_NE(theBundle, NULL);

    /* This test reads the string "abc\u0000def" from the bundle   */
    /* if everything is working correctly, the size of this string */
    /* should be 7. Everything else is a wrong answer, esp. 3 and 6*/

    status = U_ZERO_ERROR;
    strcpy(action, "getting and testing of explicit string of zero length string");
    res = ures_getByKey(theBundle, "emptyexplicitstring", res, &status);
    CONFIRM_ErrorCode(status, U_ZERO_ERROR);
    CONFIRM_INT_EQ(ures_getType(res), URES_STRING);
    zeroString=ures_getString(res, &len, &status);
    if(U_SUCCESS(status)){
        CONFIRM_ErrorCode(status, U_ZERO_ERROR);
        CONFIRM_INT_EQ(len, 0);
        CONFIRM_INT_EQ(u_strlen(zeroString), 0);
    }
    else {
        log_err("Couldn't get emptyexplicitstring\n");
    }

    status = U_ZERO_ERROR;
    strcpy(action, "getting and testing of normal string of zero length string");
    res = ures_getByKey(theBundle, "emptystring", res, &status);
    CONFIRM_ErrorCode(status, U_ZERO_ERROR);
    CONFIRM_INT_EQ(ures_getType(res), URES_STRING);
    zeroString=ures_getString(res, &len, &status);
    if(U_SUCCESS(status)){
        CONFIRM_ErrorCode(status, U_ZERO_ERROR);
        CONFIRM_INT_EQ(len, 0);
        CONFIRM_INT_EQ(u_strlen(zeroString), 0);
    }
    else {
        log_err("Couldn't get emptystring\n");
    }

    status = U_ZERO_ERROR;
    strcpy(action, "getting and testing of empty int");
    res = ures_getByKey(theBundle, "emptyint", res, &status);
    CONFIRM_ErrorCode(status, U_ZERO_ERROR);
    CONFIRM_INT_EQ(ures_getType(res), URES_INT);
    intResult=ures_getInt(res, &status);
    if(U_SUCCESS(status)){
        CONFIRM_ErrorCode(status, U_ZERO_ERROR);
        CONFIRM_INT_EQ(intResult, 0);
    }
    else {
        log_err("Couldn't get emptystring\n");
    }

    status = U_ZERO_ERROR;
    strcpy(action, "getting and testing of zero length intvector");
    res = ures_getByKey(theBundle, "emptyintv", res, &status);
    CONFIRM_ErrorCode(status, U_ZERO_ERROR);
    CONFIRM_INT_EQ(ures_getType(res), URES_INT_VECTOR);

    if(U_FAILURE(status)){
        log_err("Couldn't get emptyintv key %s\n", u_errorName(status));
    }
    else {
        zeroIntVect=ures_getIntVector(res, &len, &status);
        if(!U_SUCCESS(status) || resArray != NULL || len != 0) {
            log_err("Shouldn't get emptyintv\n");
        }
    }

    status = U_ZERO_ERROR;
    strcpy(action, "getting and testing of zero length emptybin");
    res = ures_getByKey(theBundle, "emptybin", res, &status);
    CONFIRM_ErrorCode(status, U_ZERO_ERROR);
    CONFIRM_INT_EQ(ures_getType(res), URES_BINARY);

    if(U_FAILURE(status)){
        log_err("Couldn't get emptybin key %s\n", u_errorName(status));
    }
    else {
        binResult=ures_getBinary(res, &len, &status);
        if(!U_SUCCESS(status) || binResult != NULL || len != 0) {
            log_err("Shouldn't get emptybin\n");
        }
    }

    status = U_ZERO_ERROR;
    strcpy(action, "getting and testing of zero length emptyarray");
    res = ures_getByKey(theBundle, "emptyarray", res, &status);
    CONFIRM_ErrorCode(status, U_ZERO_ERROR);
    CONFIRM_INT_EQ(ures_getType(res), URES_ARRAY);

    if(U_FAILURE(status)){
        log_err("Couldn't get emptyarray key %s\n", u_errorName(status));
    }
    else {
        resArray=ures_getByIndex(res, 0, resArray, &status);
        if(U_SUCCESS(status) || resArray != NULL){
            log_err("Shouldn't get emptyarray\n");
        }
    }

    status = U_ZERO_ERROR;
    strcpy(action, "getting and testing of zero length emptytable");
    res = ures_getByKey(theBundle, "emptytable", res, &status);
    CONFIRM_ErrorCode(status, U_ZERO_ERROR);
    CONFIRM_INT_EQ(ures_getType(res), URES_TABLE);

    if(U_FAILURE(status)){
        log_err("Couldn't get emptytable key %s\n", u_errorName(status));
    }
    else {
        resArray=ures_getByIndex(res, 0, resArray, &status);
        if(U_SUCCESS(status) || resArray != NULL){
            log_err("Shouldn't get emptytable\n");
        }
    }

    ures_close(res);
    ures_close(theBundle);
}

static void TestEmptyBundle(){
    UErrorCode status = U_ZERO_ERROR;
    const char* testdatapath=NULL;
    UResourceBundle *resb=0, *dResB=0;
    
    testdatapath=loadTestData(&status);
    if(U_FAILURE(status))
    {
        log_err("Could not load testdata.dat %s \n",myErrorName(status));
        return;
    }
    resb = ures_open(testdatapath, "testempty", &status);

    if(U_SUCCESS(status)){
        dResB =  ures_getByKey(resb,"test",dResB,&status);
        if(status!= U_MISSING_RESOURCE_ERROR){
            log_err("Did not get the expected error from an empty resource bundle. Expected : %s Got: %s\n", 
                u_errorName(U_MISSING_RESOURCE_ERROR),u_errorName(status)); 
        }
    }
    ures_close(dResB);
    ures_close(resb);
}

static void TestBinaryCollationData(){
    UErrorCode status=U_ZERO_ERROR;
    const char*      locale="te";
#if !UCONFIG_NO_COLLATION 
    const char* testdatapath;
#endif
    UResourceBundle *teRes = NULL;
    UResourceBundle *coll=NULL;
    UResourceBundle *binColl = NULL;
    uint8_t *binResult = NULL;
    int32_t len=0;
    const char* action="testing the binary collaton data";

#if !UCONFIG_NO_COLLATION 
    log_verbose("Testing binary collation data resource......\n");

    testdatapath=loadTestData(&status);
    if(U_FAILURE(status))
    {
        log_err("Could not load testdata.dat %s \n",myErrorName(status));
        return;
    }


    teRes=ures_open(testdatapath, locale, &status);
    if(U_FAILURE(status)){
        log_err("ERROR: Failed to get resource for \"te\" with %s", myErrorName(status));
        return;
    }
    status=U_ZERO_ERROR;
    coll = ures_getByKey(teRes, "collations", coll, &status);
    coll = ures_getByKey(coll, "standard", coll, &status);
    if(U_SUCCESS(status)){
        CONFIRM_ErrorCode(status, U_ZERO_ERROR);
        CONFIRM_INT_EQ(ures_getType(coll), URES_TABLE);
        binColl=ures_getByKey(coll, "%%CollationBin", binColl, &status);  
        if(U_SUCCESS(status)){
            CONFIRM_ErrorCode(status, U_ZERO_ERROR);
            CONFIRM_INT_EQ(ures_getType(binColl), URES_BINARY);
            binResult=(uint8_t*)ures_getBinary(binColl,  &len, &status);
            if(U_SUCCESS(status)){
                CONFIRM_ErrorCode(status, U_ZERO_ERROR);
                CONFIRM_INT_GE(len, 1);
            }

        }else{
            log_err("ERROR: ures_getByKey(locale(te), %%CollationBin) failed\n");
        }
    }
    else{
        log_err("ERROR: ures_getByKey(locale(te), collations) failed\n");
        return;
    }
    ures_close(binColl);
    ures_close(coll);
    ures_close(teRes);
#endif
}

static void TestAPI() {
    UErrorCode status=U_ZERO_ERROR;
    int32_t len=0;
    const char* key=NULL;
    const UChar* value=NULL;
    const char* testdatapath;
    UChar* utestdatapath=NULL;
    char convOutput[256];
    UResourceBundle *teRes = NULL;
    UResourceBundle *teFillin=NULL;
    UResourceBundle *teFillin2=NULL;
    
    log_verbose("Testing ures_openU()......\n");
    
    testdatapath=loadTestData(&status);
    if(U_FAILURE(status))
    {
        log_err("Could not load testdata.dat %s \n",myErrorName(status));
        return;
    }
    len =strlen(testdatapath);
    utestdatapath = (UChar*) malloc((len+10)*sizeof(UChar));

    u_charsToUChars(testdatapath, utestdatapath, strlen(testdatapath)+1);
    /*u_uastrcpy(utestdatapath, testdatapath);*/

    /*Test ures_openU */

    teRes=ures_openU(utestdatapath, "te", &status);
    if(U_FAILURE(status)){
        log_err("ERROR: ures_openU() failed path =%s with %s", austrdup(utestdatapath), myErrorName(status));
        return;
    }
    /*Test ures_getLocale() */
    log_verbose("Testing ures_getLocale() .....\n");
    if(strcmp(ures_getLocale(teRes, &status), "te") != 0){
        log_err("ERROR: ures_getLocale() failed. Expected = te_TE Got = %s\n", ures_getLocale(teRes, &status));
    }
    /*Test ures_getNextString() */
    teFillin=ures_getByKey(teRes, "tagged_array_in_te_te_IN", teFillin, &status);
    key=ures_getKey(teFillin);
    value=(UChar*)ures_getNextString(teFillin, &len, &key, &status);
    ures_resetIterator(NULL);
    value=(UChar*)ures_getNextString(teFillin, &len, &key, &status);
    if(status !=U_INDEX_OUTOFBOUNDS_ERROR){
        log_err("ERROR: calling getNextString where index out of bounds should return U_INDEX_OUTOFBOUNDS_ERROR, Got : %s\n",
                       myErrorName(status));
    }
    ures_resetIterator(teRes);    
    /*Test ures_getNextResource() where resource is table*/
    status=U_ZERO_ERROR;
#if (U_CHARSET_FAMILY == U_ASCII_FAMILY)
    /* The next key varies depending on the charset. */
    teFillin=ures_getNextResource(teRes, teFillin, &status);
    if(U_FAILURE(status)){
        log_err("ERROR: ures_getNextResource() failed \n");
    }
    key=ures_getKey(teFillin);
    /*if(strcmp(key, "%%CollationBin") != 0){*/
    if(strcmp(key, "array_2d_in_Root_te") != 0){
        log_err("ERROR: ures_getNextResource() failed\n");
    }
#endif

    /*Test ures_getByIndex on string Resource*/
    teFillin=ures_getByKey(teRes, "string_only_in_te", teFillin, &status);
    teFillin2=ures_getByIndex(teFillin, 0, teFillin2, &status);
    if(U_FAILURE(status)){
        log_err("ERROR: ures_getByIndex on string resource failed\n");
    }
    if(strcmp(u_austrcpy(convOutput, ures_getString(teFillin2, &len, &status)), "TE") != 0){
        status=U_ZERO_ERROR;
        log_err("ERROR: ures_getByIndex on string resource fetched the key=%s, expected \"TE\" \n", austrdup(ures_getString(teFillin2, &len, &status)));
    }

    /*ures_close(teRes);*/

    /*Test ures_openFillIn*/
    log_verbose("Testing ures_openFillIn......\n");
    status=U_ZERO_ERROR;
    ures_openFillIn(teRes, testdatapath, "te", &status);
    if(U_FAILURE(status)){
        log_err("ERROR: ures_openFillIn failed\n");
        return;
    }
    if(strcmp(ures_getLocale(teRes, &status), "te") != 0){
        log_err("ERROR: ures_openFillIn did not open the ResourceBundle correctly\n");
    }
    ures_getByKey(teRes, "string_only_in_te", teFillin, &status);
    teFillin2=ures_getNextResource(teFillin, teFillin2, &status);
    if(ures_getType(teFillin2) != URES_STRING){
        log_err("ERROR: getType for getNextResource after ures_openFillIn failed\n");
    }
    teFillin2=ures_getNextResource(teFillin, teFillin2, &status);
    if(status !=U_INDEX_OUTOFBOUNDS_ERROR){
        log_err("ERROR: calling getNextResource where index out of bounds should return U_INDEX_OUTOFBOUNDS_ERROR, Got : %s\n",
                       myErrorName(status));
    }

    ures_close(teFillin);
    ures_close(teFillin2);
    ures_close(teRes);

    /* Test that ures_getLocale() returns the "real" locale ID */
    status=U_ZERO_ERROR;
    teRes=ures_open(NULL, "dE_At_NOWHERE_TO_BE_FOUND", &status);
    if(U_FAILURE(status)) {
        log_data_err("unable to open a locale resource bundle from \"dE_At_NOWHERE_TO_BE_FOUND\"(%s)\n", u_errorName(status));
    } else {
        if(0!=strcmp("de_AT", ures_getLocale(teRes, &status))) {
            log_data_err("ures_getLocale(\"dE_At_NOWHERE_TO_BE_FOUND\")=%s but must be de_AT\n", ures_getLocale(teRes, &status));
        }
        ures_close(teRes);
    }

    /* same test, but with an aliased locale resource bundle */
    status=U_ZERO_ERROR;
    teRes=ures_open(NULL, "iW_Il_depRecaTed_HebreW", &status);
    if(U_FAILURE(status)) {
        log_data_err("unable to open a locale resource bundle from \"iW_Il_depRecaTed_HebreW\"(%s)\n", u_errorName(status));
    } else {
        if(0!=strcmp("he_IL", ures_getLocale(teRes, &status))) {
            log_data_err("ures_getLocale(\"iW_Il_depRecaTed_HebreW\")=%s but must be he_IL\n", ures_getLocale(teRes, &status));
        }
        ures_close(teRes);
    }
    free(utestdatapath);
}

static void TestErrorConditions(){
    UErrorCode status=U_ZERO_ERROR;
    const char *key=NULL;
    const UChar *value=NULL;
    const char* testdatapath;
    UChar* utestdatapath;
    int32_t len=0;
    UResourceBundle *teRes = NULL;
    UResourceBundle *coll=NULL;
    UResourceBundle *binColl = NULL;
    UResourceBundle *teFillin=NULL;
    UResourceBundle *teFillin2=NULL;
    uint8_t *binResult = NULL;
    int32_t resultLen;
    
    
    testdatapath = loadTestData(&status);
    if(U_FAILURE(status))
    {
        log_err("Could not load testdata.dat %s \n",myErrorName(status));
        return;
    }
    len = strlen(testdatapath);
    utestdatapath = (UChar*) malloc(sizeof(UChar) *(len+10));
    u_uastrcpy(utestdatapath, testdatapath);
  
    /*Test ures_openU with status != U_ZERO_ERROR*/
    log_verbose("Testing ures_openU() with status != U_ZERO_ERROR.....\n");
    status=U_ILLEGAL_ARGUMENT_ERROR;
    teRes=ures_openU(utestdatapath, "te", &status);
    if(U_FAILURE(status)){
        log_verbose("ures_openU() failed as expected path =%s with status != U_ZERO_ERROR\n", testdatapath);
    }else{
        log_err("ERROR: ures_openU() is supposed to fail path =%s with status != U_ZERO_ERROR\n", austrdup(utestdatapath));
        ures_close(teRes);
    }
    /*Test ures_openFillIn with UResourceBundle = NULL*/
    log_verbose("Testing ures_openFillIn with UResourceBundle = NULL.....\n");
    status=U_ZERO_ERROR;
    ures_openFillIn(NULL, testdatapath, "te", &status);
    if(status != U_INTERNAL_PROGRAM_ERROR){
        log_err("ERROR: ures_openFillIn with UResourceBundle= NULL should fail.  Expected U_INTERNAL_PROGRAM_ERROR, Got: %s\n",
                        myErrorName(status));
    }
    /*Test ures_getLocale() with status != U_ZERO_ERROR*/
    status=U_ZERO_ERROR;
    teRes=ures_openU(utestdatapath, "te", &status);
    if(U_FAILURE(status)){
        log_err("ERROR: ures_openU() failed path =%s with %s", austrdup(utestdatapath), myErrorName(status));
        return;
    }
    status=U_ILLEGAL_ARGUMENT_ERROR;
    if(ures_getLocale(teRes, &status) != NULL){
        log_err("ERROR: ures_getLocale is supposed to fail with errorCode != U_ZERO_ERROR\n");
    }
    /*Test ures_getLocale() with UResourceBundle = NULL*/
    status=U_ZERO_ERROR;
    if(ures_getLocale(NULL, &status) != NULL && status != U_ILLEGAL_ARGUMENT_ERROR){
        log_err("ERROR: ures_getLocale is supposed to fail when UResourceBundle = NULL. Expected: errorCode = U_ILLEGAL_ARGUMENT_ERROR, Got: errorCode=%s\n",
                                           myErrorName(status));
    }
    /*Test ures_getSize() with UResourceBundle = NULL */
    status=U_ZERO_ERROR;
    if(ures_getSize(NULL) != 0){
        log_err("ERROR: ures_getSize() should return 0 when UResourceBundle=NULL.  Got =%d\n", ures_getSize(NULL));
    }
    /*Test ures_getType() with UResourceBundle = NULL should return URES_NONE==-1*/
    status=U_ZERO_ERROR;
    if(ures_getType(NULL) != URES_NONE){  
        log_err("ERROR: ures_getType() should return URES_NONE when UResourceBundle=NULL.  Got =%d\n", ures_getType(NULL));
    }
    /*Test ures_getKey() with UResourceBundle = NULL*/
    status=U_ZERO_ERROR;
    if(ures_getKey(NULL) != NULL){  
        log_err("ERROR: ures_getKey() should return NULL when UResourceBundle=NULL.  Got =%d\n", ures_getKey(NULL));
    }
    /*Test ures_hasNext() with UResourceBundle = NULL*/
    status=U_ZERO_ERROR;
    if(ures_hasNext(NULL) != FALSE){  
        log_err("ERROR: ures_hasNext() should return FALSE when UResourceBundle=NULL.  Got =%d\n", ures_hasNext(NULL));
    }
    /*Test ures_get() with UResourceBundle = NULL*/
    status=U_ZERO_ERROR;
    if(ures_getStringByKey(NULL, "string_only_in_te", &resultLen, &status) != NULL && status != U_ILLEGAL_ARGUMENT_ERROR){
        log_err("ERROR: ures_get is supposed to fail when UResourceBundle = NULL. Expected: errorCode = U_ILLEGAL_ARGUMENT_ERROR, Got: errorCode=%s\n",
                                           myErrorName(status));
    } 
    /*Test ures_getByKey() with UResourceBundle = NULL*/
    status=U_ZERO_ERROR;
    teFillin=ures_getByKey(NULL, "string_only_in_te", teFillin, &status);
    if( teFillin != NULL && status != U_ILLEGAL_ARGUMENT_ERROR){
        log_err("ERROR: ures_getByKey is supposed to fail when UResourceBundle = NULL. Expected: errorCode = U_ILLEGAL_ARGUMENT_ERROR, Got: errorCode=%s\n",
                                           myErrorName(status));
    } 
    /*Test ures_getByKey() with status != U_ZERO_ERROR*/
    teFillin=ures_getByKey(NULL, "string_only_in_te", teFillin, &status);
    if(teFillin != NULL ){
        log_err("ERROR: ures_getByKey is supposed to fail when errorCode != U_ZERO_ERROR\n");
    } 
    /*Test ures_getStringByKey() with UResourceBundle = NULL*/
    status=U_ZERO_ERROR;
    if(ures_getStringByKey(NULL, "string_only_in_te", &len, &status) != NULL && status != U_ILLEGAL_ARGUMENT_ERROR){
        log_err("ERROR: ures_getStringByKey is supposed to fail when UResourceBundle = NULL. Expected: errorCode = U_ILLEGAL_ARGUMENT_ERROR, Got: errorCode=%s\n",
                                           myErrorName(status));
    } 
    /*Test ures_getStringByKey() with status != U_ZERO_ERROR*/
    if(ures_getStringByKey(teRes, "string_only_in_te", &len, &status) != NULL){
        log_err("ERROR: ures_getStringByKey is supposed to fail when status != U_ZERO_ERROR. Expected: errorCode = U_ILLEGAL_ARGUMENT_ERROR, Got: errorCode=%s\n",
                                           myErrorName(status));
    } 
    /*Test ures_getString() with UResourceBundle = NULL*/
    status=U_ZERO_ERROR;
    if(ures_getString(NULL, &len, &status) != NULL && status != U_ILLEGAL_ARGUMENT_ERROR){
        log_err("ERROR: ures_getString is supposed to fail when UResourceBundle = NULL. Expected: errorCode = U_ILLEGAL_ARGUMENT_ERROR, Got: errorCode=%s\n",
                                           myErrorName(status));
    } 
    /*Test ures_getString() with status != U_ZERO_ERROR*/
    if(ures_getString(teRes, &len, &status) != NULL){
        log_err("ERROR: ures_getString is supposed to fail when status != U_ZERO_ERROR. Expected: errorCode = U_ILLEGAL_ARGUMENT_ERROR, Got: errorCode=%s\n",
                                           myErrorName(status));
    } 
    /*Test ures_getBinary() with UResourceBundle = NULL*/
    status=U_ZERO_ERROR;
    if(ures_getBinary(NULL, &len, &status) != NULL && status != U_ILLEGAL_ARGUMENT_ERROR){
        log_err("ERROR: ures_getBinary is supposed to fail when UResourceBundle = NULL. Expected: errorCode = U_ILLEGAL_ARGUMENT_ERROR, Got: errorCode=%s\n",
                                           myErrorName(status));
    } 
    /*Test ures_getBinary(0 status != U_ILLEGAL_ARGUMENT_ERROR*/
    status=U_ZERO_ERROR;
    coll = ures_getByKey(teRes, "collations", coll, &status);
    coll = ures_getByKey(teRes, "standard", coll, &status);
    binColl=ures_getByKey(coll, "%%CollationBin", binColl, &status);

    status=U_ILLEGAL_ARGUMENT_ERROR;
    binResult=(uint8_t*)ures_getBinary(binColl,  &len, &status);
    if(binResult != NULL){
        log_err("ERROR: ures_getBinary() with status != U_ZERO_ERROR is supposed to fail\n");
    }
        
    /*Test ures_getNextResource() with status != U_ZERO_ERROR*/
    teFillin=ures_getNextResource(teRes, teFillin, &status);
    if(teFillin != NULL){
        log_err("ERROR: ures_getNextResource() with errorCode != U_ZERO_ERROR is supposed to fail\n");
    }
    /*Test ures_getNextResource() with UResourceBundle = NULL*/
    status=U_ZERO_ERROR;
    teFillin=ures_getNextResource(NULL, teFillin, &status);
    if(teFillin != NULL || status != U_ILLEGAL_ARGUMENT_ERROR){
        log_err("ERROR: ures_getNextResource() with UResourceBundle = NULL is supposed to fail.  Expected : U_IILEGAL_ARGUMENT_ERROR, Got : %s\n", 
                                          myErrorName(status));
    }
    /*Test ures_getNextString with errorCode != U_ZERO_ERROR*/
    teFillin=ures_getByKey(teRes, "tagged_array_in_te_te_IN", teFillin, &status);
    key=ures_getKey(teFillin);
    status = U_ILLEGAL_ARGUMENT_ERROR;
    value=(UChar*)ures_getNextString(teFillin, &len, &key, &status);
    if(value != NULL){
        log_err("ERROR: ures_getNextString() with errorCode != U_ZERO_ERROR is supposed to fail\n");
    }
    /*Test ures_getNextString with UResourceBundle = NULL*/
    status=U_ZERO_ERROR;
    value=(UChar*)ures_getNextString(NULL, &len, &key, &status);
    if(value != NULL || status != U_ILLEGAL_ARGUMENT_ERROR){
        log_err("ERROR: ures_getNextString() with UResourceBundle=NULL is supposed to fail\n Expected: U_ILLEGAL_ARGUMENT_ERROR, Got: %s\n",
                                    myErrorName(status));
    }
    /*Test ures_getByIndex with errorCode != U_ZERO_ERROR*/
    status=U_ZERO_ERROR;
    teFillin=ures_getByKey(teRes, "array_only_in_te", teFillin, &status);
    if(ures_countArrayItems(teRes, "array_only_in_te", &status) != 4) {
      log_err("ERROR: Wrong number of items in an array!\n");
    }
    status=U_ILLEGAL_ARGUMENT_ERROR;
    teFillin2=ures_getByIndex(teFillin, 0, teFillin2, &status);
    if(teFillin2 != NULL){
        log_err("ERROR: ures_getByIndex() with errorCode != U_ZERO_ERROR is supposed to fail\n");
    }
    /*Test ures_getByIndex with UResourceBundle = NULL */
    status=U_ZERO_ERROR;
    teFillin2=ures_getByIndex(NULL, 0, teFillin2, &status);
    if(status != U_ILLEGAL_ARGUMENT_ERROR){
        log_err("ERROR: ures_getByIndex() with UResourceBundle=NULL is supposed to fail\n Expected: U_ILLEGAL_ARGUMENT_ERROR, Got: %s\n",
                                    myErrorName(status));
    } 
    /*Test ures_getStringByIndex with errorCode != U_ZERO_ERROR*/
    status=U_ZERO_ERROR;
    teFillin=ures_getByKey(teRes, "array_only_in_te", teFillin, &status);
    status=U_ILLEGAL_ARGUMENT_ERROR;
    value=(UChar*)ures_getStringByIndex(teFillin, 0, &len, &status);
    if( value != NULL){
        log_err("ERROR: ures_getSringByIndex() with errorCode != U_ZERO_ERROR is supposed to fail\n");
    }
    /*Test ures_getStringByIndex with UResourceBundle = NULL */
    status=U_ZERO_ERROR;
    value=(UChar*)ures_getStringByIndex(NULL, 0, &len, &status);
    if(value != NULL || status != U_ILLEGAL_ARGUMENT_ERROR){
        log_err("ERROR: ures_getStringByIndex() with UResourceBundle=NULL is supposed to fail\n Expected: U_ILLEGAL_ARGUMENT_ERROR, Got: %s\n",
                                    myErrorName(status));
    } 
    /*Test ures_getStringByIndex with UResourceBundle = NULL */
    status=U_ZERO_ERROR;
    value=(UChar*)ures_getStringByIndex(teFillin, 9999, &len, &status);
    if(value != NULL || status != U_MISSING_RESOURCE_ERROR){
        log_err("ERROR: ures_getStringByIndex() with index that is too big is supposed to fail\n Expected: U_MISSING_RESOURCE_ERROR, Got: %s\n",
                                    myErrorName(status));
    } 
    /*Test ures_getInt() where UResourceBundle = NULL */
    status=U_ZERO_ERROR;
    if(ures_getInt(NULL, &status) != -1 && status != U_ILLEGAL_ARGUMENT_ERROR){
        log_err("ERROR: ures_getInt() with UResourceBundle = NULL should fail. Expected: U_IILEGAL_ARGUMENT_ERROR, Got: %s\n",
                           myErrorName(status));
    }
    /*Test ures_getInt() where status != U_ZERO_ERROR */  
    if(ures_getInt(teRes, &status) != -1){
        log_err("ERROR: ures_getInt() with errorCode != U_ZERO_ERROR should fail\n");
    }

    ures_close(teFillin);
    ures_close(teFillin2);
    ures_close(coll);
    ures_close(binColl);
    ures_close(teRes);
    free(utestdatapath);
    

}

static void TestGetVersion(){
    UVersionInfo minVersionArray = {0x01, 0x00, 0x00, 0x00};
    UVersionInfo maxVersionArray = {0x50, 0x80, 0xcf, 0xcf};
    UVersionInfo versionArray;
    UErrorCode status= U_ZERO_ERROR;
    UResourceBundle* resB = NULL; 
    int i=0, j = 0;
    int locCount = uloc_countAvailable();
    const char *locName = "root";
    
    log_verbose("The ures_getVersion tests begin : \n");

    for(j = -1; j < locCount; j++) {
        if(j >= 0) {
            locName = uloc_getAvailable(j);
        }
        log_verbose("Testing version number for locale %s\n", locName);
        resB = ures_open(NULL,locName, &status);
        if (U_FAILURE(status)) {
            log_err("Resource bundle creation for locale %s failed.: %s\n", locName, myErrorName(status));
            ures_close(resB);
            return;
        }
        ures_getVersion(resB, versionArray);
        for (i=0; i<4; ++i) {
            if (versionArray[i] < minVersionArray[i] ||
                versionArray[i] > maxVersionArray[i])
            {
                log_err("Testing ures_getVersion(%-5s) - unexpected result: %d.%d.%d.%d\n", 
                    locName, versionArray[0], versionArray[1], versionArray[2], versionArray[3]);
                break;
            }
        }
        ures_close(resB);
    }
}


static void TestGetVersionColl(){
    UVersionInfo minVersionArray = {0x01, 0x00, 0x00, 0x00};
    UVersionInfo maxVersionArray = {0x50, 0x80, 0xcf, 0xcf};
    UVersionInfo versionArray;
    UErrorCode status= U_ZERO_ERROR;
    UResourceBundle* resB = NULL;   
    UEnumeration *locs= NULL;
    int i=0;
    const char *locName = "root";
    int32_t locLen;
    const UChar* rules =NULL;
    int32_t len = 0;
    
    log_verbose("The ures_getVersion(%s) tests begin : \n", U_ICUDATA_COLL);
    locs = ures_openAvailableLocales(U_ICUDATA_COLL, &status);
    if (U_FAILURE(status)) {
       log_err("enumeration of %s failed.: %s\n", U_ICUDATA_COLL, myErrorName(status));
       return;
    }

    do{
        log_verbose("Testing version number for locale %s\n", locName);
        resB = ures_open(U_ICUDATA_COLL,locName, &status);
        if (U_FAILURE(status)) {
            log_err("Resource bundle creation for locale %s:%s failed.: %s\n", U_ICUDATA_COLL, locName, myErrorName(status));
            ures_close(resB);
            return;
        }
        /* test NUL termination of UCARules */
        rules = ures_getStringByKey(resB,"%%UCARULES",&len, &status);
        if(!rules || U_FAILURE(status)) {
          log_data_err("Could not load %%UCARULES for locale %s\n", locName);
          continue;
        }
        if(u_strlen(rules) != len){
            log_err("UCARules string not nul terminated! \n");
        }
        ures_getVersion(resB, versionArray);
        for (i=0; i<4; ++i) {
            if (versionArray[i] < minVersionArray[i] ||
                versionArray[i] > maxVersionArray[i])
            {
                log_err("Testing ures_getVersion(%-5s) - unexpected result: %d.%d.%d.%d\n", 
                    locName, versionArray[0], versionArray[1], versionArray[2], versionArray[3]);
                break;
            }
        }
        ures_close(resB);
    } while((locName = uenum_next(locs,&locLen,&status))&&U_SUCCESS(status));
    
    if(U_FAILURE(status)) {
        log_err("Err %s testing Collation locales.\n", u_errorName(status));
    }
    uenum_close(locs);
}

static void TestResourceBundles()
{

    testTag("only_in_Root", TRUE, FALSE, FALSE);
    testTag("in_Root_te", TRUE, TRUE, FALSE);
    testTag("in_Root_te_te_IN", TRUE, TRUE, TRUE);
    testTag("in_Root_te_IN", TRUE, FALSE, TRUE);
    testTag("only_in_te", FALSE, TRUE, FALSE);
    testTag("only_in_te_IN", FALSE, FALSE, TRUE);
    testTag("in_te_te_IN", FALSE, TRUE, TRUE);
    testTag("nonexistent", FALSE, FALSE, FALSE);

    log_verbose("Passed:=  %d   Failed=   %d \n", pass, fail);

}


static void TestConstruction1()
{
    UResourceBundle *test1 = 0, *test2 = 0,*empty = 0;
    const UChar *result1, *result2;
    UErrorCode status= U_ZERO_ERROR;
    UErrorCode   err = U_ZERO_ERROR;
    const char*      locale="te_IN";
    const char* testdatapath;

    int32_t len1=0;
    int32_t len2=0;
    UVersionInfo versionInfo;
    char versionString[256];
    char verboseOutput[256];

    U_STRING_DECL(rootVal, "ROOT", 4);
    U_STRING_DECL(te_inVal, "TE_IN", 5);

    U_STRING_INIT(rootVal, "ROOT", 4);
    U_STRING_INIT(te_inVal, "TE_IN", 5);

    testdatapath=loadTestData(&status);
    if(U_FAILURE(status))
    {
        log_err("Could not load testdata.dat %s \n",myErrorName(status));
        return;
    }
    
    log_verbose("Testing ures_open()......\n");

    empty = ures_open(testdatapath, "testempty", &status);
    if(empty == NULL || U_FAILURE(status)) {
        log_err("opening empty failed!\n");
    }
    ures_close(empty);

    test1=ures_open(testdatapath, NULL, &err);

    if(U_FAILURE(err))
    {
        log_err("construction of NULL did not succeed :  %s \n", myErrorName(status));
        return;
    }
    test2=ures_open(testdatapath, locale, &err);
    if(U_FAILURE(err))
    {
        log_err("construction of %s did not succeed :  %s \n", locale, myErrorName(status));
        return;
    }
    result1= ures_getStringByKey(test1, "string_in_Root_te_te_IN", &len1, &err);
    result2= ures_getStringByKey(test2, "string_in_Root_te_te_IN", &len2, &err);
    if (U_FAILURE(err) || len1==0 || len2==0) {
        log_err("Something threw an error in TestConstruction(): %s\n", myErrorName(status));
        return;
    }
    log_verbose("for string_in_Root_te_te_IN, default.txt had  %s\n", u_austrcpy(verboseOutput, result1));
    log_verbose("for string_in_Root_te_te_IN, te_IN.txt had %s\n", u_austrcpy(verboseOutput, result2));
    if(u_strcmp(result1, rootVal) !=0  || u_strcmp(result2, te_inVal) !=0 ){
        log_err("construction test failed. Run Verbose for more information");
    }


    /* Test getVersionNumber*/
    log_verbose("Testing version number\n");
    log_verbose("for getVersionNumber :  %s\n", ures_getVersionNumber(test1));

    log_verbose("Testing version \n");
    ures_getVersion(test1, versionInfo);
    u_versionToString(versionInfo, versionString);

    log_verbose("for getVersion :  %s\n", versionString);

    if(strcmp(versionString, ures_getVersionNumber(test1)) != 0) {
        log_err("Versions differ: %s vs %s\n", versionString, ures_getVersionNumber(test1));
    }

    ures_close(test1);
    ures_close(test2);

}

/*****************************************************************************/
/*****************************************************************************/

static UBool testTag(const char* frag,
           UBool in_Root,
           UBool in_te,
           UBool in_te_IN)
{
    int32_t failNum = fail;

    /* Make array from input params */

    UBool is_in[3];
    const char *NAME[] = { "ROOT", "TE", "TE_IN" };

    /* Now try to load the desired items */
    UResourceBundle* theBundle = NULL;
    char tag[99];
    char action[256];
    UErrorCode expected_status,status = U_ZERO_ERROR,expected_resource_status = U_ZERO_ERROR;
    UChar* base = NULL;
    UChar* expected_string = NULL;
    const UChar* string = NULL;
    char buf[5];
    char item_tag[10];
    int32_t i,j,row,col, len;
    int32_t actual_bundle;
    int32_t count = 0;
    int32_t row_count=0;
    int32_t column_count=0;
    int32_t index = 0;
    int32_t tag_count= 0;
    const char* testdatapath;
    char verboseOutput[256];
    UResourceBundle* array=NULL;
    UResourceBundle* array2d=NULL;
    UResourceBundle* tags=NULL;
    UResourceBundle* arrayItem1=NULL;

    testdatapath = loadTestData(&status);
    if(U_FAILURE(status))
    {
        log_err("Could not load testdata.dat %s \n",myErrorName(status));
        return FALSE;
    }

    is_in[0] = in_Root;
    is_in[1] = in_te;
    is_in[2] = in_te_IN;

    strcpy(item_tag, "tag");

    for (i=0; i<bundles_count; ++i)
    {
        strcpy(action,"construction for ");
        strcat(action, param[i].name);


        status = U_ZERO_ERROR;

        theBundle = ures_open(testdatapath, param[i].name, &status);
        CONFIRM_ErrorCode(status,param[i].expected_constructor_status);

        if(i == 5)
            actual_bundle = 0; /* ne -> default */
        else if(i == 3)
            actual_bundle = 1; /* te_NE -> te */
        else if(i == 4)
            actual_bundle = 2; /* te_IN_NE -> te_IN */
        else
            actual_bundle = i;

        expected_resource_status = U_MISSING_RESOURCE_ERROR;
        for (j=e_te_IN; j>=e_Root; --j)
        {
            if (is_in[j] && param[i].inherits[j])
            {

                if(j == actual_bundle) /* it's in the same bundle OR it's a nonexistent=default bundle (5) */
                    expected_resource_status = U_ZERO_ERROR;
                else if(j == 0)
                    expected_resource_status = U_USING_DEFAULT_WARNING;
                else
                    expected_resource_status = U_USING_FALLBACK_WARNING;

                log_verbose("%s[%d]::%s: in<%d:%s> inherits<%d:%s>.  actual_bundle=%s\n",
                            param[i].name, 
                            i,
                            frag,
                            j,
                            is_in[j]?"Yes":"No",
                            j,
                            param[i].inherits[j]?"Yes":"No",
                            param[actual_bundle].name);

                break;
            }
        }

        for (j=param[i].where; j>=0; --j)
        {
            if (is_in[j])
            {
                if(base != NULL) {
                    free(base);
                    base = NULL;
                }
                base=(UChar*)malloc(sizeof(UChar)*(strlen(NAME[j]) + 1));
                u_uastrcpy(base,NAME[j]);

                break;
            }
            else {
                if(base != NULL) {
                    free(base);
                    base = NULL;
                }
                base = (UChar*) malloc(sizeof(UChar) * 1);
                *base = 0x0000;
            }
        }

        /*----string---------------------------------------------------------------- */

        strcpy(tag,"string_");
        strcat(tag,frag);

        strcpy(action,param[i].name);
        strcat(action, ".ures_getStringByKey(" );
        strcat(action,tag);
        strcat(action, ")");


        status = U_ZERO_ERROR;
        len=0;

        string=ures_getStringByKey(theBundle, tag, &len, &status);
        if(U_SUCCESS(status)) {
            expected_string=(UChar*)malloc(sizeof(UChar)*(u_strlen(base) + 4));
            u_strcpy(expected_string,base);
            CONFIRM_INT_EQ(len, u_strlen(expected_string));
        }else{
            expected_string = (UChar*)malloc(sizeof(UChar)*(u_strlen(kERROR) + 1));
            u_strcpy(expected_string,kERROR);
            string=kERROR;
        }
        log_verbose("%s got %d, expected %d\n", action, status, expected_resource_status);

        CONFIRM_ErrorCode(status, expected_resource_status);
        CONFIRM_EQ(string, expected_string);



        /*--------------array------------------------------------------------- */

        strcpy(tag,"array_");
        strcat(tag,frag);

        strcpy(action,param[i].name);
        strcat(action, ".ures_getByKey(" );
        strcat(action,tag);
        strcat(action, ")");

        len=0;

        count = kERROR_COUNT;
        status = U_ZERO_ERROR;
        array=ures_getByKey(theBundle, tag, array, &status);
        CONFIRM_ErrorCode(status,expected_resource_status);
        if (U_SUCCESS(status)) {
            /*confirm the resource type is an array*/
            CONFIRM_INT_EQ(ures_getType(array), URES_ARRAY);
            /*confirm the size*/
            count=ures_getSize(array);
            CONFIRM_INT_GE(count,1);
            for (j=0; j<count; ++j) {
                UChar element[3];
                u_strcpy(expected_string, base);
                u_uastrcpy(element, itoa1(j,buf));
                u_strcat(expected_string, element);
                arrayItem1=ures_getNextResource(array, arrayItem1, &status);
                if(U_SUCCESS(status)){
                    CONFIRM_EQ(ures_getString(arrayItem1, &len, &status),expected_string);
                }
            }

        }
        else {
            CONFIRM_INT_EQ(count,kERROR_COUNT);
            CONFIRM_ErrorCode(status, U_MISSING_RESOURCE_ERROR);
            /*CONFIRM_INT_EQ((int32_t)(unsigned long)array,(int32_t)0);*/
            count = 0;
        }

        /*--------------arrayItem------------------------------------------------- */

        strcpy(tag,"array_");
        strcat(tag,frag);

        strcpy(action,param[i].name);
        strcat(action, ".ures_getStringByIndex(");
        strcat(action, tag);
        strcat(action, ")");


        for (j=0; j<10; ++j){
            index = count ? (randi(count * 3) - count) : (randi(200) - 100);
            status = U_ZERO_ERROR;
            string=kERROR;
            array=ures_getByKey(theBundle, tag, array, &status);
            if(!U_FAILURE(status)){
                UChar *t=NULL;
                t=(UChar*)ures_getStringByIndex(array, index, &len, &status);
                if(!U_FAILURE(status)){
                    UChar element[3];
                    string=t;
                    u_strcpy(expected_string, base);
                    u_uastrcpy(element, itoa1(index,buf));
                    u_strcat(expected_string, element);
                } else {
                    u_strcpy(expected_string, kERROR);
                }

            }
            expected_status = (index >= 0 && index < count) ? expected_resource_status : U_MISSING_RESOURCE_ERROR;
            CONFIRM_ErrorCode(status,expected_status);
            CONFIRM_EQ(string,expected_string);

        }


        /*--------------2dArray------------------------------------------------- */  

        strcpy(tag,"array_2d_");
        strcat(tag,frag);

        strcpy(action,param[i].name);
        strcat(action, ".ures_getByKey(" );
        strcat(action,tag);
        strcat(action, ")");



        row_count = kERROR_COUNT, column_count = kERROR_COUNT;
        status = U_ZERO_ERROR;
        array2d=ures_getByKey(theBundle, tag, array2d, &status);

        CONFIRM_ErrorCode(status,expected_resource_status);
        if (U_SUCCESS(status))
        {
            /*confirm the resource type is an 2darray*/
            CONFIRM_INT_EQ(ures_getType(array2d), URES_ARRAY);
            row_count=ures_getSize(array2d);
            CONFIRM_INT_GE(row_count,1);

            for(row=0; row<row_count; ++row){
                UResourceBundle *tableRow=NULL;
                tableRow=ures_getByIndex(array2d, row, tableRow, &status);
                CONFIRM_ErrorCode(status, expected_resource_status);
                if(U_SUCCESS(status)){
                    /*confirm the resourcetype of each table row is an array*/
                    CONFIRM_INT_EQ(ures_getType(tableRow), URES_ARRAY);
                    column_count=ures_getSize(tableRow);
                    CONFIRM_INT_GE(column_count,1);

                    for (col=0; j<column_count; ++j) {
                        UChar element[3];
                        u_strcpy(expected_string, base);
                        u_uastrcpy(element, itoa1(row, buf));
                        u_strcat(expected_string, element);
                        u_uastrcpy(element, itoa1(col, buf));
                        u_strcat(expected_string, element);
                        arrayItem1=ures_getNextResource(tableRow, arrayItem1, &status);
                        if(U_SUCCESS(status)){
                            const UChar *stringValue=ures_getString(arrayItem1, &len, &status);
                            CONFIRM_EQ(stringValue, expected_string);
                        }
                    }
                }
                ures_close(tableRow);
            }
        }else{
            CONFIRM_INT_EQ(row_count,kERROR_COUNT);
            CONFIRM_INT_EQ(column_count,kERROR_COUNT);
            row_count=column_count=0;
        }


        /*------2dArrayItem-------------------------------------------------------------- */
        /* 2dArrayItem*/
        for (j=0; j<10; ++j)
        {
            row = row_count ? (randi(row_count * 3) - row_count) : (randi(200) - 100);
            col = column_count ? (randi(column_count * 3) - column_count) : (randi(200) - 100);
            status = U_ZERO_ERROR;
            string = kERROR;
            len=0;
            array2d=ures_getByKey(theBundle, tag, array2d, &status);
            if(U_SUCCESS(status)){
                UResourceBundle *tableRow=NULL;
                tableRow=ures_getByIndex(array2d, row, tableRow, &status);
                if(U_SUCCESS(status)) {
                    UChar *t=NULL;
                    t=(UChar*)ures_getStringByIndex(tableRow, col, &len, &status);
                    if(U_SUCCESS(status)){
                        string=t;
                    }
                }
                ures_close(tableRow);
            }
            expected_status = (row >= 0 && row < row_count && col >= 0 && col < column_count) ?
                                   expected_resource_status: U_MISSING_RESOURCE_ERROR;
            CONFIRM_ErrorCode(status,expected_status);

            if (U_SUCCESS(status)){
                UChar element[3];
                u_strcpy(expected_string, base);
                u_uastrcpy(element, itoa1(row, buf));
                u_strcat(expected_string, element);
                u_uastrcpy(element, itoa1(col, buf));
                u_strcat(expected_string, element);
            } else {
                u_strcpy(expected_string,kERROR);
            }
            CONFIRM_EQ(string,expected_string);

        }


        /*--------------taggedArray----------------------------------------------- */
        strcpy(tag,"tagged_array_");
        strcat(tag,frag);

        strcpy(action,param[i].name);
        strcat(action,".ures_getByKey(");
        strcat(action, tag);
        strcat(action,")");


        status = U_ZERO_ERROR;
        tag_count=0;
        tags=ures_getByKey(theBundle, tag, tags, &status);
        CONFIRM_ErrorCode(status, expected_resource_status);
        if (U_SUCCESS(status)) {
            UResType bundleType=ures_getType(tags);
            CONFIRM_INT_EQ(bundleType, URES_TABLE);

            tag_count=ures_getSize(tags);
            CONFIRM_INT_GE((int32_t)tag_count, (int32_t)0); 

            for(index=0; index <tag_count; index++){
                UResourceBundle *tagelement=NULL;
                const char *key=NULL;
                UChar* value=NULL;
                tagelement=ures_getByIndex(tags, index, tagelement, &status);
                key=ures_getKey(tagelement);
                value=(UChar*)ures_getNextString(tagelement, &len, &key, &status);
                log_verbose("tag = %s, value = %s\n", key, u_austrcpy(verboseOutput, value));
                if(strncmp(key, "tag", 3) == 0 && u_strncmp(value, base, u_strlen(base)) == 0){
                    record_pass();
                }else{
                    record_fail();
                }
                ures_close(tagelement);
            }
        }else{
            tag_count=0;
        }

        /*---------taggedArrayItem----------------------------------------------*/
        count = 0;
        for (index=-20; index<20; ++index)
        {

            status = U_ZERO_ERROR;
            string = kERROR;
            strcpy(item_tag, "tag");
            strcat(item_tag, itoa1(index,buf));
            tags=ures_getByKey(theBundle, tag, tags, &status);
            if(U_SUCCESS(status)){
                UResourceBundle *tagelement=NULL;
                UChar *t=NULL;
                tagelement=ures_getByKey(tags, item_tag, tagelement, &status);
                if(!U_FAILURE(status)){
                    UResType elementType=ures_getType(tagelement);
                    CONFIRM_INT_EQ(elementType, URES_STRING);
                    if(strcmp(ures_getKey(tagelement), item_tag) == 0){
                        record_pass();
                    }else{
                        record_fail();
                    }
                    t=(UChar*)ures_getString(tagelement, &len, &status);
                    if(!U_FAILURE(status)){
                        string=t;
                    }
                }
                if (index < 0) {
                    CONFIRM_ErrorCode(status,U_MISSING_RESOURCE_ERROR);
                }
                else{
                    if (status != U_MISSING_RESOURCE_ERROR) {
                        UChar element[3];
                        u_strcpy(expected_string, base);
                        u_uastrcpy(element, itoa1(index,buf));
                        u_strcat(expected_string, element);
                        CONFIRM_EQ(string,expected_string);
                        count++;
                    }
                }
                ures_close(tagelement);
            }
        }
        CONFIRM_INT_EQ(count, tag_count);

        free(expected_string);
        ures_close(theBundle);
    }
    ures_close(array);
    ures_close(array2d);
    ures_close(tags);
    ures_close(arrayItem1);
    free(base);
    return (UBool)(failNum == fail);
}

static void record_pass()
{
    ++pass;
}

static void record_fail()
{
    ++fail;
}

/**
 * Test to make sure that the U_USING_FALLBACK_ERROR and U_USING_DEFAULT_ERROR
 * are set correctly
 */

static void TestFallback()
{
    UErrorCode status = U_ZERO_ERROR;
    UResourceBundle *fr_FR = NULL;
    UResourceBundle *subResource = NULL;
    const UChar *junk; /* ignored */
    int32_t resultLen;

    log_verbose("Opening fr_FR..");
    fr_FR = ures_open(NULL, "fr_FR", &status);
    if(U_FAILURE(status))
    {
        log_err("Couldn't open fr_FR - %d\n", status);
        return;
    }

    status = U_ZERO_ERROR;


    /* clear it out..  just do some calls to get the gears turning */
    junk = ures_getStringByKey(fr_FR, "LocaleID", &resultLen, &status);
    status = U_ZERO_ERROR;
    junk = ures_getStringByKey(fr_FR, "LocaleString", &resultLen, &status);
    status = U_ZERO_ERROR;
    junk = ures_getStringByKey(fr_FR, "LocaleID", &resultLen, &status);
    status = U_ZERO_ERROR;

    /* OK first one. This should be a Default value. */
    subResource = ures_getByKey(fr_FR, "CurrencyMap", NULL, &status);
    if(status != U_USING_DEFAULT_WARNING)
    {
        log_data_err("Expected U_USING_DEFAULT_ERROR when trying to get LocaleScript from fr_FR, got %s\n", 
            u_errorName(status));
    }

    status = U_ZERO_ERROR;
    ures_close(subResource);

    /* and this is a Fallback, to fr */
    junk = ures_getStringByKey(fr_FR, "Countries", &resultLen, &status);
    if(status != U_USING_FALLBACK_WARNING)
    {
        log_data_err("Expected U_USING_FALLBACK_ERROR when trying to get Countries from fr_FR, got %d\n", 
            status);
    }
    
    status = U_ZERO_ERROR;

    ures_close(fr_FR);
    /* Temporary hack err actually should be U_USING_FALLBACK_ERROR */
    /* Test Jitterbug 552 fallback mechanism of aliased data */
    {
        UErrorCode err =U_ZERO_ERROR;
        UResourceBundle* myResB = ures_open(NULL,"no_NO_NY",&err);
        UResourceBundle* resLocID = ures_getByKey(myResB, "Version", NULL, &err);
        UResourceBundle* tResB;
        static const UChar versionStr[] = { 0x0033, 0x002E, 0x0030, 0};

        if(err != U_ZERO_ERROR){
            log_data_err("Expected U_ZERO_ERROR when trying to test no_NO_NY aliased to nn_NO for Version err=%s\n",u_errorName(err));
            return;
        }
        if(u_strcmp(ures_getString(resLocID, &resultLen, &err), versionStr) != 0){
            log_data_err("ures_getString(resLocID, &resultLen, &err) returned an unexpected version value\n");
        }
        tResB = ures_getByKey(myResB, "calendar", NULL, &err);
        if(err != U_USING_FALLBACK_WARNING){
            log_err("Expected U_USING_FALLBACK_ERROR when trying to test no_NO_NY aliased with nn_NO_NY for calendar err=%s\n",u_errorName(err));
        }
        ures_close(resLocID);
        ures_close(myResB);
        ures_close(tResB);

    }

}

static void printUChars(UChar* uchars){
    int16_t i=0;
    for(i=0; i<u_strlen(uchars); i++){
        log_err("%04X ", *(uchars+i));
    }
}

static void TestResourceLevelAliasing(void) {
    UErrorCode status = U_ZERO_ERROR;
    UResourceBundle *aliasB = NULL, *tb = NULL;
    UResourceBundle *en = NULL, *uk = NULL, *testtypes = NULL;  
    const char* testdatapath = NULL;
    const UChar *string = NULL, *sequence = NULL;
    const uint8_t *binary = NULL, *binSequence = NULL;
    int32_t strLen = 0, seqLen = 0, binLen = 0, binSeqLen = 0;
    char buffer[100];
    char *s;

    testdatapath=loadTestData(&status);
    if(U_FAILURE(status))
    {
        log_err("Could not load testdata.dat %s \n",myErrorName(status));
        return;
    }
    
    aliasB = ures_open(testdatapath, "testaliases", &status);
    
    /* this should fail - circular alias */
    tb = ures_getByKey(aliasB, "aaa", tb, &status);
    if(status != U_TOO_MANY_ALIASES_ERROR) {
        log_err("Failed to detect circular alias\n");
    } else {
        status = U_ZERO_ERROR;
    }
    tb = ures_getByKey(aliasB, "aab", tb, &status);
    if(status != U_TOO_MANY_ALIASES_ERROR) {
        log_err("Failed to detect circular alias\n");
    } else {
        status = U_ZERO_ERROR;
    }
    if(U_FAILURE(status) ) {
        log_data_err("err loading tb resource\n");
    }  else {
      /* testing aliasing to a non existing resource */
      tb = ures_getByKey(aliasB, "nonexisting", tb, &status);
      if(status != U_MISSING_RESOURCE_ERROR) {
        log_err("Managed to find an alias to non-existing resource\n");
      } else {
        status = U_ZERO_ERROR;
      }
      
      /* testing referencing/composed alias */
      uk = ures_findResource("ja/LocaleScript/2", uk, &status);
      if((uk == NULL) || U_FAILURE(status)) {
        log_err("Couldn't findResource('ja/LocaleScript/2') err %s\n", u_errorName(status));
        goto cleanup;
      } 
      
      sequence = ures_getString(uk, &seqLen, &status);
      
      tb = ures_getByKey(aliasB, "referencingalias", tb, &status);
      string = ures_getString(tb, &strLen, &status);
      
      if(seqLen != strLen || u_strncmp(sequence, string, seqLen) != 0) {
        log_err("Referencing alias didn't get the right string\n");
      }
      
      string = ures_getStringByKey(aliasB, "referencingalias", &strLen, &status);
      if(seqLen != strLen || u_strncmp(sequence, string, seqLen) != 0) {
        log_err("Referencing alias didn't get the right string\n");
      }
      
      checkStatus(__LINE__, U_ZERO_ERROR, status);
      tb = ures_getByKey(aliasB, "LocaleScript", tb, &status);
      checkStatus(__LINE__, U_ZERO_ERROR, status);
      tb = ures_getByIndex(tb, 2, tb, &status);
      checkStatus(__LINE__, U_ZERO_ERROR, status);
      string = ures_getString(tb, &strLen, &status);
      checkStatus(__LINE__, U_ZERO_ERROR, status);
      
      if(U_FAILURE(status)) {
        log_err("%s trying to get string via separate getters\n", u_errorName(status));
      } else if(seqLen != strLen || u_strncmp(sequence, string, seqLen) != 0) {
        log_err("Referencing alias didn't get the right string\n");
      }
      
      /* check whether the binary collation data is properly referenced by an alias */
      uk = ures_findResource("th/BreakDictionaryData", uk, &status);
      binSequence = ures_getBinary(uk, &binSeqLen, &status);
      
      tb = ures_getByKey(aliasB, "BreakDictionaryData", tb, &status);
      binary = ures_getBinary(tb, &binLen, &status);
      
      if(U_FAILURE(status)) {
        log_err("%s trying to read binary BreakDictionaryData\n");
      } else if(binSeqLen != binLen || uprv_memcmp(binSequence, binary, binSeqLen) != 0) {
        log_err("Referencing alias didn't get the right data\n");
      }
      
      /* simple alias */
      testtypes = ures_open(testdatapath, "testtypes", &status);
      strcpy(buffer, "menu/file/open");
      s = buffer;
      uk = ures_findSubResource(testtypes, s, uk, &status);
      sequence = ures_getString(uk, &seqLen, &status);
      
      tb = ures_getByKey(aliasB, "simplealias", tb, &status);
      string = ures_getString(tb, &strLen, &status);
      
      if(U_FAILURE(status) || seqLen != strLen || u_strncmp(sequence, string, seqLen) != 0) {
        log_err("Referencing alias didn't get the right string\n");
      }
      
      /* test indexed aliasing */
      
      tb = ures_getByKey(aliasB, "zoneTests", tb, &status);
      tb = ures_getByKey(tb, "zoneAlias2", tb, &status);
      string = ures_getString(tb, &strLen, &status);
      
      en = ures_findResource("en/zoneStrings/3/0", en, &status);
      sequence = ures_getString(en, &seqLen, &status);
      
      if(U_FAILURE(status) || seqLen != strLen || u_strncmp(sequence, string, seqLen) != 0) {
        log_err("Referencing alias didn't get the right string\n");
      }
    }
    /* test getting aliased string by index */
    {
        const char* keys[] = {
            "KeyAlias0PST",
                "KeyAlias1PacificStandardTime",
                "KeyAlias2PDT",
                "KeyAlias3LosAngeles"
        };
        
        const char* strings[] = {
            "PST",
                "Pacific Standard Time",
                "PDT",
                "Los Angeles",
        };
        UChar uBuffer[256];
        const UChar* result;
        int32_t uBufferLen = 0, resultLen = 0;
        int32_t i = 0;
        const char *key = NULL;
        tb = ures_getByKey(aliasB, "testGetStringByKeyAliasing", tb, &status);
        if(U_FAILURE(status)) {
            log_err("Couldn't get testGetStringByKeyAliasing resource\n");
        }
        for(i = 0; i < sizeof(strings)/sizeof(strings[0]); i++) {
            result = ures_getStringByKey(tb, keys[i], &resultLen, &status);
            uBufferLen = u_unescape(strings[i], uBuffer, 256);
            if(resultLen != uBufferLen || u_strncmp(result, uBuffer, resultLen) != 0) {
                log_err("Didn't get correct string while accesing alias table by key\n");
            }
        }
        for(i = 0; i < sizeof(strings)/sizeof(strings[0]); i++) {
            result = ures_getStringByIndex(tb, i, &resultLen, &status);
            uBufferLen = u_unescape(strings[i], uBuffer, 256);
            if(resultLen != uBufferLen || u_strncmp(result, uBuffer, resultLen) != 0) {
                log_err("Didn't get correct string while accesing alias table by index\n");
            }
        }
        for(i = 0; i < sizeof(strings)/sizeof(strings[0]); i++) {
            result = ures_getNextString(tb, &resultLen, &key, &status);
            uBufferLen = u_unescape(strings[i], uBuffer, 256);
            if(resultLen != uBufferLen || u_strncmp(result, uBuffer, resultLen) != 0) {
                log_err("Didn't get correct string while iterating over alias table\n");
            }
        }
        tb = ures_getByKey(aliasB, "testGetStringByIndexAliasing", tb, &status);
        if(U_FAILURE(status)) {
            log_err("Couldn't get testGetStringByIndexAliasing resource\n");
        }
        for(i = 0; i < sizeof(strings)/sizeof(strings[0]); i++) {
            result = ures_getStringByIndex(tb, i, &resultLen, &status);
            uBufferLen = u_unescape(strings[i], uBuffer, 256);
            if(resultLen != uBufferLen || u_strncmp(result, uBuffer, resultLen) != 0) {
                log_err("Didn't get correct string while accesing alias by index in an array\n");
            }
        }
        for(i = 0; i < sizeof(strings)/sizeof(strings[0]); i++) {
            result = ures_getNextString(tb, &resultLen, &key, &status);
            uBufferLen = u_unescape(strings[i], uBuffer, 256);
            if(resultLen != uBufferLen || u_strncmp(result, uBuffer, resultLen) != 0) {
                log_err("Didn't get correct string while iterating over aliases in an array\n");
            }
        }
    }
cleanup:
    ures_close(aliasB);
    ures_close(tb);
    ures_close(en);
    ures_close(uk);
    ures_close(testtypes);
}

static void TestDirectAccess(void) {
    UErrorCode status = U_ZERO_ERROR;
    UResourceBundle *t = NULL, *t2 = NULL;
    const char* key = NULL;
    
    char buffer[100];
    char *s;
    
    t = ures_findResource("en/zoneStrings/3/2", t, &status);
    if(U_FAILURE(status)) {
        log_err("Couldn't access indexed resource, error %s\n", u_errorName(status));
        status = U_ZERO_ERROR;
    } else {
        key = ures_getKey(t);
        if(key != NULL) {
            log_err("Got a strange key, expected NULL, got %s\n", key);
        }
    }
    t = ures_findResource("en/zoneStrings/3", t, &status);
    if(U_FAILURE(status)) {
        log_err("Couldn't access indexed resource, error %s\n", u_errorName(status));
        status = U_ZERO_ERROR;
    } else {
        key = ures_getKey(t);
        if(key != NULL) {
            log_err("Got a strange key, expected NULL, got %s\n", key);
        }
    }
    
    t = ures_findResource("ja/LocaleScript", t, &status);
    if(U_FAILURE(status)) {
        log_err("Couldn't access keyed resource, error %s\n", u_errorName(status));
        status = U_ZERO_ERROR;
    } else {
        key = ures_getKey(t);
        if(strcmp(key, "LocaleScript")!=0) {
            log_err("Got a strange key, expected 'LocaleScript', got %s\n", key);
        }
    }
    
    t2 = ures_open(NULL, "sr", &status);
    if(U_FAILURE(status)) {
        log_err("Couldn't open 'sr' resource bundle, error %s\n", u_errorName(status));
        log_data_err("No 'sr', no test - you have bigger problems than testing direct access. "
                     "You probably have no data! Aborting this test\n");
    }
    
    if(U_SUCCESS(status)) {
        strcpy(buffer, "Languages/hr");
        s = buffer;
        t = ures_findSubResource(t2, s, t, &status);
        if(U_FAILURE(status)) {
            log_err("Couldn't access keyed resource, error %s\n", u_errorName(status));
            status = U_ZERO_ERROR;
        } else {
            key = ures_getKey(t);
            if(strcmp(key, "hr")!=0) {
                log_err("Got a strange key, expected 'hr', got %s\n", key);
            }
        }
    }
    
    ures_close(t);
    ures_close(t2);
}

