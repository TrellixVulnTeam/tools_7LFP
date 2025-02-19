/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-2004, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/
/********************************************************************************
*
* File CU_CAPITST.C
*
* Modification History:
*        Name                      Description            
*     Madhu Katragadda              Ported for C API
*********************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "unicode/uloc.h"
#include "unicode/ucnv.h"
#include "unicode/ucnv_err.h"
#include "unicode/utypes.h"
#include "unicode/ustring.h"
#include "ucnv_bld.h" /* for sizeof(UConverter) */
#include "cintltst.h"
#include "ccapitst.h"

/* for not including "cstring.h" -begin*/    
#ifdef WIN32
#   define ctest_stricmp(str1, str2) U_STANDARD_CPP_NAMESPACE _stricmp(str1, str2)
#elif defined(POSIX) 
#   define ctest_stricmp(str1, str2) U_STANDARD_CPP_NAMESPACE strcasecmp(str1, str2) 
#else
#   define ctest_stricmp(str1, str2) T_CString_stricmp(str1, str2)
#endif

static int U_EXPORT2
T_CString_stricmp(const char *str1, const char *str2) {
    if(str1==NULL) {
        if(str2==NULL) {
            return 0;
        } else {
            return -1;
        }
    } else if(str2==NULL) {
        return 1;
    } else {
        /* compare non-NULL strings lexically with lowercase */
        int rc;
        unsigned char c1, c2;
        for(;;) {
            c1=(unsigned char)*str1;
            c2=(unsigned char)*str2;
            if(c1==0) {
                if(c2==0) {
                    return 0;
                } else {
                    return -1;
                }
            } else if(c2==0) {
                return 1;
            } else {
                /* compare non-zero characters with lowercase */
                rc=(int)(unsigned char)tolower(c1)-(int)(unsigned char)tolower(c2);
                if(rc!=0) {
                    return rc;
                }
            }
            ++str1;
            ++str2;
        }
    }
}
/* for not including "cstring.h"  -end*/    

#define LENGTHOF(array) (int32_t)(sizeof(array)/sizeof((array)[0]))

#define NUM_CODEPAGE 1
#define MAX_FILE_LEN 1024*20
#define UCS_FILE_NAME_SIZE 512

/*returns an action other than the one provided*/
static UConverterFromUCallback otherUnicodeAction(UConverterFromUCallback MIA);
static UConverterToUCallback otherCharAction(UConverterToUCallback MIA);

static UConverter *
cnv_open(const char *name, UErrorCode *pErrorCode) {
    if(name!=NULL && name[0]=='*') {
        return ucnv_openPackage(loadTestData(pErrorCode), name+1, pErrorCode);
    } else {
        return ucnv_open(name, pErrorCode);
    }
}


static void ListNames(void);
static void TestFlushCache(void);
static void TestDuplicateAlias(void);
static void TestCCSID(void);
static void TestJ932(void);
static void TestJ1968(void);
static void TestLMBCSMaxChar(void);
static void TestConvertSafeCloneCallback(void);
static void TestEBCDICSwapLFNL(void);
static void TestConvertEx(void);
static void TestConvertAlgorithmic(void);
       void TestDefaultConverterError(void);    /* defined in cctest.c */

void addTestConvert(TestNode** root);

void addTestConvert(TestNode** root)
{
    addTest(root, &ListNames,                   "tsconv/ccapitst/ListNames");
    addTest(root, &TestConvert,                 "tsconv/ccapitst/TestConvert");
    addTest(root, &TestFlushCache,              "tsconv/ccapitst/TestFlushCache"); 
    addTest(root, &TestAlias,                   "tsconv/ccapitst/TestAlias"); 
    addTest(root, &TestDuplicateAlias,          "tsconv/ccapitst/TestDuplicateAlias"); 
    addTest(root, &TestConvertSafeClone,        "tsconv/ccapitst/TestConvertSafeClone"); 
    addTest(root, &TestConvertSafeCloneCallback,"tsconv/ccapitst/TestConvertSafeCloneCallback"); 
    addTest(root, &TestCCSID,                   "tsconv/ccapitst/TestCCSID"); 
    addTest(root, &TestJ932,                    "tsconv/ccapitst/TestJ932");
    addTest(root, &TestJ1968,                   "tsconv/ccapitst/TestJ1968");
    addTest(root, &TestLMBCSMaxChar,            "tsconv/ccapitst/TestLMBCSMaxChar");
    addTest(root, &TestEBCDICSwapLFNL,          "tsconv/ccapitst/TestEBCDICSwapLFNL");
    addTest(root, &TestConvertEx,               "tsconv/ccapitst/TestConvertEx");
    addTest(root, &TestConvertAlgorithmic,      "tsconv/ccapitst/TestConvertAlgorithmic");
    addTest(root, &TestDefaultConverterError,   "tsconv/ccapitst/TestDefaultConverterError");
}

static void ListNames(void) {
    UErrorCode          err                 =   U_ZERO_ERROR;
    int32_t             testLong1           =   0;
    const char*            available_conv;
    UEnumeration *allNamesEnum = NULL;
    int32_t allNamesCount = 0;
    uint16_t            count;

    log_verbose("Testing ucnv_openAllNames()...");
    allNamesEnum = ucnv_openAllNames(&err);
    if(U_FAILURE(err)) {
        log_err("FAILURE! ucnv_openAllNames() -> %s\n", myErrorName(err));
    }
    else {
        const char *string = NULL;
        int32_t len = 0;
        int32_t count1 = 0;
        int32_t count2 = 0;
        allNamesCount = uenum_count(allNamesEnum, &err);
        while ((string = uenum_next(allNamesEnum, &len, &err))) {
            count1++;
            log_verbose("read \"%s\", length %i\n", string, len);
        }
        err = U_ZERO_ERROR;
        uenum_reset(allNamesEnum, &err);
        while ((string = uenum_next(allNamesEnum, &len, &err))) {
            count2++;
            ucnv_close(ucnv_open(string, &err));
            log_verbose("read \"%s\", length %i (%s)\n", string, len, U_SUCCESS(err) ? "available" : "unavailable");
            err = U_ZERO_ERROR;
        }
        if (count1 != count2) {
            log_err("FAILURE! uenum_reset(allNamesEnum, &err); doesn't work\n");
        }
    }
    uenum_close(allNamesEnum);
    err = U_ZERO_ERROR;

    /*Tests ucnv_getAvailableName(), getAvialableCount()*/

    log_verbose("Testing ucnv_countAvailable()...");

    testLong1=ucnv_countAvailable();
    log_info("Number of available codepages: %d/%d\n", testLong1, allNamesCount);

    log_verbose("\n---Testing ucnv_getAvailableName..");  /*need to check this out */

    available_conv = ucnv_getAvailableName(testLong1);
       /*test ucnv_getAvailableName with err condition*/
    log_verbose("\n---Testing ucnv_getAvailableName..with index < 0 ");
    available_conv = ucnv_getAvailableName(-1);
    if(available_conv != NULL){
        log_err("ucnv_getAvailableName() with index < 0) should return NULL\n");
    }

    /* Test ucnv_countAliases() etc. */
    count = ucnv_countAliases("utf-8", &err);
    if(U_FAILURE(err)) {
        log_err("FAILURE! ucnv_countAliases(\"utf-8\") -> %s\n", myErrorName(err));
    } else if(count <= 0) {
        log_err("FAILURE! ucnv_countAliases(\"utf-8\") -> %d aliases\n", count);
    } else {
        /* try to get the aliases individually */
        const char *alias;
        alias = ucnv_getAlias("utf-8", 0, &err);
        if(U_FAILURE(err)) {
            log_err("FAILURE! ucnv_getAlias(\"utf-8\", 0) -> %s\n", myErrorName(err));
        } else if(strcmp("UTF-8", alias) != 0) {
            log_err("FAILURE! ucnv_getAlias(\"utf-8\", 0) -> %s instead of UTF-8\n", alias);
        } else {
            uint16_t aliasNum;
            for(aliasNum = 0; aliasNum < count; ++aliasNum) {
                alias = ucnv_getAlias("utf-8", aliasNum, &err);
                if(U_FAILURE(err)) {
                    log_err("FAILURE! ucnv_getAlias(\"utf-8\", %d) -> %s\n", aliasNum, myErrorName(err));
                } else if(strlen(alias) > 20) {
                    /* sanity check */
                    log_err("FAILURE! ucnv_getAlias(\"utf-8\", %d) -> alias %s insanely long, corrupt?!\n", aliasNum, alias);
                } else {
                    log_verbose("alias %d for utf-8: %s\n", aliasNum, alias);
                }
            }
            if(U_SUCCESS(err)) {
                /* try to fill an array with all aliases */
                const char **aliases;
                aliases=(const char **)malloc(count * sizeof(const char *));
                if(aliases != 0) {
                    ucnv_getAliases("utf-8", aliases, &err);
                    if(U_FAILURE(err)) {
                        log_err("FAILURE! ucnv_getAliases(\"utf-8\") -> %s\n", myErrorName(err));
                    } else {
                        for(aliasNum = 0; aliasNum < count; ++aliasNum) {
                            /* compare the pointers with the ones returned individually */
                            alias = ucnv_getAlias("utf-8", aliasNum, &err);
                            if(U_FAILURE(err)) {
                                log_err("FAILURE! ucnv_getAlias(\"utf-8\", %d) -> %s\n", aliasNum, myErrorName(err));
                            } else if(aliases[aliasNum] != alias) {
                                log_err("FAILURE! ucnv_getAliases(\"utf-8\")[%d] != ucnv_getAlias(\"utf-8\", %d)\n", aliasNum, aliasNum);
                            }
                        }
                    }
                    free((char **)aliases);
                }
            }
        }
    }
}


static void TestConvert() 
{
    char                myptr[4];
    char                save[4];
    int32_t             testLong1           =   0;
    uint16_t            rest                =   0;
    int32_t             len                 =   0;
    int32_t             x                   =   0;
    FILE*               ucs_file_in         =   NULL;
    UChar                BOM                 =   0x0000;
    UChar                myUChar           =   0x0000;
    char*               mytarget; /*    [MAX_FILE_LEN] */
    char*               mytarget_1;
    char*               mytarget_use;
    UChar*                consumedUni         =   NULL;
    char*               consumed            =   NULL;
    char*                 output_cp_buffer; /*    [MAX_FILE_LEN] */
    UChar*                ucs_file_buffer; /*    [MAX_FILE_LEN] */
    UChar*                ucs_file_buffer_use;
    UChar*                my_ucs_file_buffer; /*    [MAX_FILE_LEN] */
    UChar*                my_ucs_file_buffer_1;
    int8_t                ii                  =   0;
    int32_t             j                   =   0;
    uint16_t            codepage_index      =   0;
    int32_t             cp                  =   0;
    UErrorCode          err                 =   U_ZERO_ERROR;
    char                ucs_file_name[UCS_FILE_NAME_SIZE];
    UConverterFromUCallback          MIA1, MIA1_2;
    UConverterToUCallback              MIA2, MIA2_2;
    const void         *MIA1Context, *MIA1Context2, *MIA2Context, *MIA2Context2;
    UConverter*            someConverters[5];
    UConverter*         myConverter = 0;
    UChar*                displayname = 0;
   
    const char* locale;

    UChar* uchar1 = 0;
    UChar* uchar2 = 0;
    UChar* uchar3 = 0;
    int32_t targetcapacity2;
    int32_t targetcapacity;
    int32_t targetsize;
    int32_t disnamelen;

    const UChar* tmp_ucs_buf;
    const UChar* tmp_consumedUni=NULL;
    const char* tmp_mytarget_use;
    const char* tmp_consumed; 

    /******************************************************************
                                Checking Unicode -> ksc
     ******************************************************************/

    const char*      CodePagesToTest[NUM_CODEPAGE]       =
    {
       "ibm-949_P110-1999"

        
    }; 
    const uint16_t CodePageNumberToTest[NUM_CODEPAGE]             =
    {
        949
    };
    

    const int8_t     CodePagesMinChars[NUM_CODEPAGE] =
    { 
        1
    
    };

    const int8_t     CodePagesMaxChars[NUM_CODEPAGE] =
    { 
        2
    
    };

    const uint16_t        CodePagesSubstitutionChars[NUM_CODEPAGE]    =
    { 
        0xAFFE
    };

    const char* CodePagesTestFiles[NUM_CODEPAGE]    =
    { 
      "uni-text.bin"
    };

    
    const UConverterPlatform        CodePagesPlatform[NUM_CODEPAGE]    =
    { 
        UCNV_IBM
    
    };

    const char* CodePagesLocale[NUM_CODEPAGE] =
    {
        "ko_KR"
    };

    UConverterFromUCallback oldFromUAction = NULL;
    UConverterToUCallback oldToUAction = NULL;
    const void* oldFromUContext = NULL;
    const void* oldToUContext = NULL;

    /* Allocate memory */
    mytarget = (char*) malloc(MAX_FILE_LEN * sizeof(mytarget[0]));
    output_cp_buffer = (char*) malloc(MAX_FILE_LEN * sizeof(output_cp_buffer[0]));
    ucs_file_buffer = (UChar*) malloc(MAX_FILE_LEN * sizeof(ucs_file_buffer[0]));
    my_ucs_file_buffer = (UChar*) malloc(MAX_FILE_LEN * sizeof(my_ucs_file_buffer[0]));

    ucs_file_buffer_use = ucs_file_buffer;
    mytarget_1=mytarget;
    mytarget_use        = mytarget;
    my_ucs_file_buffer_1=my_ucs_file_buffer;

    /* flush the converter cache to get a consistent state before the flushing is tested */
    ucnv_flushCache();

    /*Testing ucnv_openU()*/
    {
        UChar converterName[]={ 0x0069, 0x0062, 0x006d, 0x002d, 0x0039, 0x0034, 0x0033, 0x0000}; /*ibm-943*/
        UChar firstSortedName[]={ 0x0021, 0x0000}; /* ! */
        UChar lastSortedName[]={ 0x007E, 0x0000}; /* ~ */
        const char *illegalNameChars={ "ibm-943 ibm-943 ibm-943 ibm-943 ibm-943 ibm-943 ibm-943 ibm-943 ibm-943 ibm-943"};
        UChar illegalName[100];
        UConverter *converter=NULL;
        err=U_ZERO_ERROR;
        converter=ucnv_openU(converterName, &err);
        if(U_FAILURE(err)){
            log_data_err("FAILURE! ucnv_openU(ibm-943, err) failed. %s\n", myErrorName(err));
        }
        ucnv_close(converter);
        err=U_ZERO_ERROR;
        converter=ucnv_openU(NULL, &err);
        if(U_FAILURE(err)){
            log_err("FAILURE! ucnv_openU(NULL, err)  failed. %s\n", myErrorName(err));
        }
        ucnv_close(converter);
        /*testing with error value*/
        err=U_ILLEGAL_ARGUMENT_ERROR;
        converter=ucnv_openU(converterName, &err);
        if(!(converter == NULL)){
            log_data_err("FAILURE! ucnv_openU(ibm-943, U_ILLEGAL_ARGUMENT_ERROR) is expected to fail\n");
        }
        ucnv_close(converter);
        err=U_ZERO_ERROR;
        u_uastrcpy(illegalName, "");
        u_uastrcpy(illegalName, illegalNameChars);
        ucnv_openU(illegalName, &err);
        if(!(err==U_ILLEGAL_ARGUMENT_ERROR)){
            log_err("FAILURE! ucnv_openU(illegalName, err) is expected to fail\n");
        }

        err=U_ZERO_ERROR;
        ucnv_openU(firstSortedName, &err);
        if(err!=U_FILE_ACCESS_ERROR){
            log_err("FAILURE! ucnv_openU(firstSortedName, err) is expected to fail\n");
        }

        err=U_ZERO_ERROR;
        ucnv_openU(lastSortedName, &err);
        if(err!=U_FILE_ACCESS_ERROR){
            log_err("FAILURE! ucnv_openU(lastSortedName, err) is expected to fail\n");
        }

        err=U_ZERO_ERROR;
    }
    log_verbose("Testing ucnv_open() with converter name greater than 7 characters\n");
    {
         UConverter *cnv=NULL;
         err=U_ZERO_ERROR;
         cnv=ucnv_open("ibm-949,Madhu", &err);
         if(U_FAILURE(err)){
            log_data_err("FAILURE! ucnv_open(\"ibm-949,Madhu\", err)  failed. %s\n", myErrorName(err));
         }
         ucnv_close(cnv);

    }
      /*Testing ucnv_convert()*/
    {
        int32_t targetLimit=0, sourceLimit=0, i=0, targetCapacity=0;
        const uint8_t source[]={ 0x00, 0x04, 0x05, 0x06, 0xa2, 0xb4, 0x00};
        const uint8_t expectedTarget[]={ 0x00, 0x37, 0x2d, 0x2e, 0x0e, 0x49, 0x62, 0x0f, 0x00};
        char *target=0;
        sourceLimit=sizeof(source)/sizeof(source[0]);
        err=U_ZERO_ERROR;
        targetLimit=0;
            
        targetCapacity=ucnv_convert("ibm-1364", "ibm-1363", NULL, targetLimit , (const char*)source, sourceLimit, &err);
        if(err == U_BUFFER_OVERFLOW_ERROR){
            err=U_ZERO_ERROR;
            targetLimit=targetCapacity+1;
            target=(char*)malloc(sizeof(char) * targetLimit);
            targetCapacity=ucnv_convert("ibm-1364", "ibm-1363", target, targetLimit , (const char*)source, sourceLimit, &err);
        }
        if(U_FAILURE(err)){
            log_data_err("FAILURE! ucnv_convert(ibm-1363->ibm-1364) failed. %s\n", myErrorName(err));
        }
        else {
            for(i=0; i<targetCapacity; i++){
                if(target[i] != expectedTarget[i]){
                    log_err("FAIL: ucnv_convert(ibm-1363->ibm-1364) failed.at index \n i=%d,  Expected: %lx Got: %lx\n", i, (UChar)expectedTarget[i], (uint8_t)target[i]);
                }
            }

            i=ucnv_convert("ibm-1364", "ibm-1363", target, targetLimit , (const char*)source+1, -1, &err);
            if(U_FAILURE(err) || i!=7){
                log_err("FAILURE! ucnv_convert() with sourceLimit=-1 failed: %s, returned %d instead of 7\n",
                    u_errorName(err), i);
            }

            /*Test error conditions*/
            err=U_ZERO_ERROR;
            i=ucnv_convert("ibm-1364", "ibm-1363", target, targetLimit , (const char*)source, 0, &err);
            if(i !=0){
                log_err("FAILURE! ucnv_convert() with sourceLimit=0 is expected to return 0\n");
            }

            err=U_ILLEGAL_ARGUMENT_ERROR;
            sourceLimit=sizeof(source)/sizeof(source[0]);
            i=ucnv_convert("ibm-1364", "ibm-1363", target, targetLimit , (const char*)source, sourceLimit, &err);
            if(i !=0 ){
                log_err("FAILURE! ucnv_convert() with err=U_ILLEGAL_ARGUMENT_ERROR is expected to return 0\n");
            }

            err=U_ZERO_ERROR;
            sourceLimit=sizeof(source)/sizeof(source[0]);
            targetLimit=0;
            i=ucnv_convert("ibm-1364", "ibm-1363", target, targetLimit , (const char*)source, sourceLimit, &err);
            if(!(U_FAILURE(err) && err==U_BUFFER_OVERFLOW_ERROR)){
                log_err("FAILURE! ucnv_convert() with targetLimit=0 is expected to throw U_BUFFER_OVERFLOW_ERROR\n");
            }
            err=U_ZERO_ERROR;
            free(target);
        }
    }

    /*Testing ucnv_openCCSID and ucnv_open with error conditions*/
    log_verbose("\n---Testing ucnv_open with err ! = U_ZERO_ERROR...\n");
    err=U_ILLEGAL_ARGUMENT_ERROR;
    if(ucnv_open(NULL, &err) != NULL){
        log_err("ucnv_open with err != U_ZERO_ERROR is supposed to fail\n");
    }
    if(ucnv_openCCSID(1051, UCNV_IBM, &err) != NULL){
        log_err("ucnv_open with err != U_ZERO_ERROR is supposed to fail\n");
    }
    err=U_ZERO_ERROR;
    
    /* Testing ucnv_openCCSID(), ucnv_open(), ucnv_getName() */
    log_verbose("\n---Testing ucnv_open default...\n");
    someConverters[0] = ucnv_open(NULL,&err);
    someConverters[1] = ucnv_open(NULL,&err);
    someConverters[2] = ucnv_open("utf8", &err);
    someConverters[3] = ucnv_openCCSID(949,UCNV_IBM,&err);
    ucnv_close(ucnv_openCCSID(1051, UCNV_IBM, &err)); /* test for j350; ucnv_close(NULL) is safe */
    if (U_FAILURE(err)){ log_data_err("FAILURE! %s\n", myErrorName(err));}

    /* Testing ucnv_getName()*/
    /*default code page */
    ucnv_getName(someConverters[0], &err);
    if(U_FAILURE(err)) {
        log_data_err("getName[0] failed\n");
    } else {
        log_verbose("getName(someConverters[0]) returned %s\n", ucnv_getName(someConverters[0], &err));
    }
    ucnv_getName(someConverters[1], &err);
    if(U_FAILURE(err)) {
        log_data_err("getName[1] failed\n");
    } else {
        log_verbose("getName(someConverters[1]) returned %s\n", ucnv_getName(someConverters[1], &err));
    }

    /*Testing ucnv_getDefaultName() and ucnv_setDefaultNAme()*/
    {
        static char defaultName[UCNV_MAX_CONVERTER_NAME_LENGTH + 1];
        strcpy(defaultName, ucnv_getDefaultName());

        log_verbose("getDefaultName returned %s\n", defaultName);

        /*change the default name by setting it */
        ucnv_setDefaultName("changed");
        if(strcmp(ucnv_getDefaultName(), "changed")==0)
            log_verbose("setDefaultName o.k");
        else
            log_err("setDefaultName failed");  
        /*set the default name back*/
        ucnv_setDefaultName(defaultName);
    }

    ucnv_close(someConverters[0]);
    ucnv_close(someConverters[1]);
    ucnv_close(someConverters[2]);
    ucnv_close(someConverters[3]);
    
       
    for (codepage_index=0; codepage_index <  NUM_CODEPAGE; ++codepage_index)
    {
        int32_t i = 0;  
        char* index = NULL;

        err = U_ZERO_ERROR;
        strcpy(ucs_file_name, loadTestData(&err));
        
        if(U_FAILURE(err)){
            log_err("\nCouldn't get the test data directory... Exiting...Error:%s\n", u_errorName(err));
            return;
        }

        index=strrchr(ucs_file_name,(char)U_FILE_SEP_CHAR);

        if((unsigned int)(index-ucs_file_name) != (strlen(ucs_file_name)-1)){
                *(index+1)=0;
        }
        
        strcat(ucs_file_name,".." U_FILE_SEP_STRING);
        strcat(ucs_file_name, CodePagesTestFiles[codepage_index]);

        ucs_file_in = fopen(ucs_file_name,"rb");
        if (!ucs_file_in) 
        {
            log_err("Couldn't open the Unicode file [%s]... Exiting...\n", ucs_file_name);
            return;
        }

        /*Creates a converter and testing ucnv_openCCSID(u_int code_page, platform, errstatus*/

        /*  myConverter =ucnv_openCCSID(CodePageNumberToTest[codepage_index],UCNV_IBM, &err); */
        /*  ucnv_flushCache(); */
        myConverter =ucnv_open( "ibm-949", &err);
        if (!myConverter || U_FAILURE(err))   
        {
            log_data_err("Error creating the ibm-949 converter - %s \n", u_errorName(err));

            return;
        }

        /*testing for ucnv_getName()  */
        log_verbose("Testing ucnv_getName()...\n");
        ucnv_getName(myConverter, &err);
        if(U_FAILURE(err))
            log_err("Error in getName\n");
        else
        {
            log_verbose("getName o.k. %s\n", ucnv_getName(myConverter, &err));
        }
        if (ctest_stricmp(ucnv_getName(myConverter, &err), CodePagesToTest[codepage_index]))
            log_err("getName failed\n");
        else 
            log_verbose("getName ok\n");
        /*Test getName with error condition*/
        { 
            const char* name=0;
            err=U_ILLEGAL_ARGUMENT_ERROR;
            log_verbose("Testing ucnv_getName with err != U_ZERO_ERROR");
            name=ucnv_getName(myConverter, &err);
            if(name != NULL){
                log_err("ucnv_getName() with err != U_ZERO_ERROR is expected to fail");
            }
            err=U_ZERO_ERROR;
        }


        /*Tests ucnv_getMaxCharSize() and ucnv_getMinCharSize()*/

        log_verbose("Testing ucnv_getMaxCharSize()...\n");
        if (ucnv_getMaxCharSize(myConverter)==CodePagesMaxChars[codepage_index])  
            log_verbose("Max byte per character OK\n");
        else 
            log_err("Max byte per character failed\n");
    
        log_verbose("\n---Testing ucnv_getMinCharSize()...\n");
        if (ucnv_getMinCharSize(myConverter)==CodePagesMinChars[codepage_index])  
            log_verbose("Min byte per character OK\n");
        else 
            log_err("Min byte per character failed\n");


        /*Testing for ucnv_getSubstChars() and ucnv_setSubstChars()*/
        log_verbose("\n---Testing ucnv_getSubstChars...\n");
        ii=4;
        ucnv_getSubstChars(myConverter, myptr, &ii, &err);

        for(x=0;x<ii;x++) 
            rest = (uint16_t)(((unsigned char)rest << 8) + (unsigned char)myptr[x]);
        if (rest==CodePagesSubstitutionChars[codepage_index])  
            log_verbose("Substitution character ok\n");
        else 
            log_err("Substitution character failed.\n");

        log_verbose("\n---Testing ucnv_setSubstChars RoundTrip Test ...\n");
        ucnv_setSubstChars(myConverter, myptr, ii, &err);
        if (U_FAILURE(err)) 
        {
            log_err("FAILURE! %s\n", myErrorName(err));
        }
        ucnv_getSubstChars(myConverter,save, &ii, &err);
        if (U_FAILURE(err)) 
        {
            log_err("FAILURE! %s\n", myErrorName(err));
        }

        if (strncmp(save, myptr, ii)) 
            log_err("Saved substitution character failed\n");
        else 
            log_verbose("Saved substitution character ok\n");

        /*Testing for ucnv_getSubstChars() and ucnv_setSubstChars() with error conditions*/ 
        log_verbose("\n---Testing ucnv_getSubstChars.. with len < minBytesPerChar\n");
        ii=1;
        ucnv_getSubstChars(myConverter, myptr, &ii, &err);
        if(err != U_INDEX_OUTOFBOUNDS_ERROR){
            log_err("ucnv_getSubstChars() with len < minBytesPerChar should throw U_INDEX_OUTOFBOUNDS_ERROR Got %s\n", myErrorName(err));
        }
        err=U_ZERO_ERROR;
        ii=4;
        ucnv_getSubstChars(myConverter, myptr, &ii, &err);
        log_verbose("\n---Testing ucnv_setSubstChars.. with len < minBytesPerChar\n");
        ucnv_setSubstChars(myConverter, myptr, 0, &err);
        if(err != U_ILLEGAL_ARGUMENT_ERROR){
            log_err("ucnv_setSubstChars() with len < minBytesPerChar should throw U_ILLEGAL_ARGUMENT_ERROR Got %s\n", myErrorName(err));
        }
        log_verbose("\n---Testing ucnv_setSubstChars.. with err != U_ZERO_ERROR \n");
        strcpy(myptr, "abc");
        ucnv_setSubstChars(myConverter, myptr, ii, &err);
        err=U_ZERO_ERROR;
        ucnv_getSubstChars(myConverter, save, &ii, &err);
        if(strncmp(save, myptr, ii) == 0){
            log_err("uncv_setSubstChars() with err != U_ZERO_ERROR shouldn't set the SubstChars and just return\n");
        }
        log_verbose("\n---Testing ucnv_getSubstChars.. with err != U_ZERO_ERROR \n");
        err=U_ZERO_ERROR;
        strcpy(myptr, "abc");
        ucnv_setSubstChars(myConverter, myptr, ii, &err);
        err=U_ILLEGAL_ARGUMENT_ERROR;
        ucnv_getSubstChars(myConverter, save, &ii, &err);
        if(strncmp(save, myptr, ii) == 0){
            log_err("uncv_setSubstChars() with err != U_ZERO_ERROR shouldn't fill the SubstChars in the buffer, it just returns\n");
        }
        err=U_ZERO_ERROR;
        /*------*/

#ifdef U_ENABLE_GENERIC_ISO_2022
        /*resetState  ucnv_reset()*/
        log_verbose("\n---Testing ucnv_reset()..\n");
        ucnv_reset(myConverter);
        {
             UChar32 c;
             const uint8_t in[]={  0x1b, 0x25, 0x42, 0x31, 0x32, 0x61, 0xc0, 0x80, 0xe0, 0x80, 0x80, 0xf0, 0x80, 0x80, 0x80};
             const char *source=(const char *)in, *limit=(const char *)in+sizeof(in);
             UConverter *cnv=ucnv_open("ISO_2022", &err);
             if(U_FAILURE(err)) {
                log_err("Unable to open a iso-2022 converter: %s\n", u_errorName(err));
             }
             c=ucnv_getNextUChar(cnv, &source, limit, &err);
             if((U_FAILURE(err) || c != (UChar32)0x0031)) {
                log_err("ucnv_getNextUChar() failed: %s\n", u_errorName(err));
             }
             ucnv_reset(cnv);
             ucnv_close(cnv);
         
        }
#endif

        /*getDisplayName*/
        log_verbose("\n---Testing ucnv_getDisplayName()...\n");
        locale=CodePagesLocale[codepage_index];
        displayname=(UChar*)malloc(1 * sizeof(UChar));
        len=0;
        disnamelen = ucnv_getDisplayName(myConverter,locale,displayname, len, &err); 
        if(err==U_BUFFER_OVERFLOW_ERROR)
        {    
            err=U_ZERO_ERROR;
            displayname=(UChar*)realloc(displayname, (disnamelen+1) * sizeof(UChar));
            ucnv_getDisplayName(myConverter,locale,displayname,disnamelen+1, &err);
            if(U_FAILURE(err))
            {
                log_err("getDisplayName failed the error is  %s\n", myErrorName(err));
            }
            else
                log_verbose(" getDisplayName o.k.\n");
        }
        /*test ucnv_getDiaplayName with error condition*/
        log_verbose("\n---Testing ucnv_getDisplayName()...\n");
        err= U_ILLEGAL_ARGUMENT_ERROR;
        len=ucnv_getDisplayName(myConverter,locale,displayname,disnamelen+1, &err);  
        if( len !=0 ){
            log_err("ucnv_getDisplayName() with err != U_ZERO_ERROR is supposed to return 0\n");
        }
        err=U_ZERO_ERROR;

        /* testing ucnv_setFromUCallBack() and ucnv_getFromUCallBack()*/
        ucnv_getFromUCallBack(myConverter, &MIA1, &MIA1Context);
            
        log_verbose("\n---Testing ucnv_setFromUCallBack...\n");
        ucnv_setFromUCallBack(myConverter, otherUnicodeAction(MIA1), &BOM, &oldFromUAction, &oldFromUContext, &err);
        if (U_FAILURE(err) || oldFromUAction != MIA1 || oldFromUContext != MIA1Context) 
        {
            log_err("FAILURE! %s\n", myErrorName(err));
        }

        ucnv_getFromUCallBack(myConverter, &MIA1_2, &MIA1Context2);
        if (MIA1_2 != otherUnicodeAction(MIA1) || MIA1Context2 != &BOM) 
            log_err("get From UCallBack failed\n");
        else 
            log_verbose("get From UCallBack ok\n");

        log_verbose("\n---Testing getFromUCallBack Roundtrip...\n");
        ucnv_setFromUCallBack(myConverter,MIA1, MIA1Context, &oldFromUAction, &oldFromUContext, &err);
        if (U_FAILURE(err) || oldFromUAction != otherUnicodeAction(MIA1) || oldFromUContext != &BOM) 
        {
            log_err("FAILURE! %s\n", myErrorName(err));
        }

        ucnv_getFromUCallBack(myConverter, &MIA1_2, &MIA1Context2);
        if (MIA1_2 != MIA1 || MIA1Context2 != MIA1Context) 
            log_err("get From UCallBack action failed\n");
        else 
            log_verbose("get From UCallBack action ok\n");

        /*testing ucnv_setToUCallBack with error conditions*/
        err=U_ILLEGAL_ARGUMENT_ERROR;
        log_verbose("\n---Testing setFromUCallBack. with err != U_ZERO_ERROR..\n");
        ucnv_setFromUCallBack(myConverter, otherUnicodeAction(MIA1), &BOM, &oldFromUAction, &oldFromUContext, &err);
        ucnv_getFromUCallBack(myConverter, &MIA1_2, &MIA1Context2);
        if(MIA1_2 == otherUnicodeAction(MIA1) || MIA1Context2 == &BOM){
            log_err("To setFromUCallBack with err != U_ZERO_ERROR is supposed to fail\n");
        }
        err=U_ZERO_ERROR;


        /*testing ucnv_setToUCallBack() and ucnv_getToUCallBack()*/
        ucnv_getToUCallBack(myConverter, &MIA2, &MIA2Context);

        log_verbose("\n---Testing setTo UCallBack...\n");
        ucnv_setToUCallBack(myConverter,otherCharAction(MIA2), &BOM, &oldToUAction, &oldToUContext, &err);
        if (U_FAILURE(err) || oldToUAction != MIA2 || oldToUContext != MIA2Context) 
        {
            log_err("FAILURE! %s\n", myErrorName(err));
        }

        ucnv_getToUCallBack(myConverter, &MIA2_2, &MIA2Context2);
        if (MIA2_2 != otherCharAction(MIA2) || MIA2Context2 != &BOM) 
            log_err("To UCallBack failed\n");
        else 
            log_verbose("To UCallBack ok\n");

        log_verbose("\n---Testing setTo UCallBack Roundtrip...\n");
        ucnv_setToUCallBack(myConverter,MIA2, MIA2Context, &oldToUAction, &oldToUContext, &err);
        if (U_FAILURE(err) || oldToUAction != otherCharAction(MIA2) || oldToUContext != &BOM) 
        { log_err("FAILURE! %s\n", myErrorName(err));  }

        ucnv_getToUCallBack(myConverter, &MIA2_2, &MIA2Context2);
        if (MIA2_2 != MIA2 || MIA2Context2 != MIA2Context)
            log_err("To UCallBack failed\n");
        else 
            log_verbose("To UCallBack ok\n");

        /*testing ucnv_setToUCallBack with error conditions*/
        err=U_ILLEGAL_ARGUMENT_ERROR;
        log_verbose("\n---Testing setToUCallBack. with err != U_ZERO_ERROR..\n");
        ucnv_setToUCallBack(myConverter,otherCharAction(MIA2), NULL, &oldToUAction, &oldToUContext, &err);
        ucnv_getToUCallBack(myConverter, &MIA2_2, &MIA2Context2);
        if (MIA2_2 == otherCharAction(MIA2) || MIA2Context2 == &BOM){ 
            log_err("To setToUCallBack with err != U_ZERO_ERROR is supposed to fail\n");
        }
        err=U_ZERO_ERROR;


        /*getcodepageid testing ucnv_getCCSID() */
        log_verbose("\n----Testing getCCSID....\n");
        cp =    ucnv_getCCSID(myConverter,&err);
        if (U_FAILURE(err)) 
        {
            log_err("FAILURE!..... %s\n", myErrorName(err));
        }
        if (cp != CodePageNumberToTest[codepage_index]) 
            log_err("Codepage number test failed\n");
        else 
            log_verbose("Codepage number test OK\n");

        /*testing ucnv_getCCSID() with err != U_ZERO_ERROR*/
        err=U_ILLEGAL_ARGUMENT_ERROR;
        if( ucnv_getCCSID(myConverter,&err) != -1){
            log_err("ucnv_getCCSID() with err != U_ZERO_ERROR is supposed to fail\n");
        }
        err=U_ZERO_ERROR;

        /*getCodepagePlatform testing ucnv_getPlatform()*/
        log_verbose("\n---Testing getCodepagePlatform ..\n");
        if (CodePagesPlatform[codepage_index]!=ucnv_getPlatform(myConverter, &err))
            log_err("Platform codepage test failed\n");
        else 
            log_verbose("Platform codepage test ok\n");

        if (U_FAILURE(err)) 
        { 
            log_err("FAILURE! %s\n", myErrorName(err));
        }
        /*testing ucnv_getPlatform() with err != U_ZERO_ERROR*/
        err= U_ILLEGAL_ARGUMENT_ERROR;
        if(ucnv_getPlatform(myConverter, &err) != UCNV_UNKNOWN){
            log_err("ucnv)getPlatform with err != U_ZERO_ERROR is supposed to fail\n");
        }
        err=U_ZERO_ERROR;


        /*Reads the BOM*/
        fread(&BOM, sizeof(UChar), 1, ucs_file_in);
        if (BOM!=0xFEFF && BOM!=0xFFFE) 
        {
            log_err("File Missing BOM...Bailing!\n");
            return;
        }


        /*Reads in the file*/
        while(!feof(ucs_file_in)&&(i+=fread(ucs_file_buffer+i, sizeof(UChar), 1, ucs_file_in)))
        {
            myUChar = ucs_file_buffer[i-1];
            
            ucs_file_buffer[i-1] = (UChar)((BOM==0xFEFF)?myUChar:((myUChar >> 8) | (myUChar << 8))); /*adjust if BIG_ENDIAN*/
        }

        myUChar = ucs_file_buffer[i-1];
        ucs_file_buffer[i-1] = (UChar)((BOM==0xFEFF)?myUChar:((myUChar >> 8) | (myUChar << 8))); /*adjust if BIG_ENDIAN Corner Case*/


        /*testing ucnv_fromUChars() and ucnv_toUChars() */
        /*uchar1---fromUChar--->output_cp_buffer --toUChar--->uchar2*/

        uchar1=(UChar*)malloc(sizeof(UChar) * (i+1));
        u_uastrcpy(uchar1,"");
        u_strncpy(uchar1,ucs_file_buffer,i);
        uchar1[i] = 0;

        uchar3=(UChar*)malloc(sizeof(UChar)*(i+1));
        u_uastrcpy(uchar3,"");
        u_strncpy(uchar3,ucs_file_buffer,i);
        uchar3[i] = 0;

        /*Calls the Conversion Routine */
        testLong1 = MAX_FILE_LEN;
        log_verbose("\n---Testing ucnv_fromUChars()\n");
        targetcapacity = ucnv_fromUChars(myConverter, output_cp_buffer, testLong1,  uchar1, -1, &err);
        if (U_FAILURE(err))  
        {
            log_err("\nFAILURE...%s\n", myErrorName(err));
        }
        else
            log_verbose(" ucnv_fromUChars() o.k.\n");

        /*test the conversion routine */
        log_verbose("\n---Testing ucnv_toUChars()\n");
        /*call it first time for trapping the targetcapacity and size needed to allocate memory for the buffer uchar2 */
        targetcapacity2=0; 
        targetsize = ucnv_toUChars(myConverter,
                     NULL,
                     targetcapacity2,
                     output_cp_buffer,
                     strlen(output_cp_buffer),
                     &err);
        /*if there is an buffer overflow then trap the values and pass them and make the actual call*/

        if(err==U_BUFFER_OVERFLOW_ERROR)
        {
            err=U_ZERO_ERROR;
            uchar2=(UChar*)malloc((targetsize+1) * sizeof(UChar));
            targetsize = ucnv_toUChars(myConverter, 
                   uchar2,
                   targetsize+1,
                   output_cp_buffer,
                   strlen(output_cp_buffer),
                   &err);

            if(U_FAILURE(err))
                log_err("ucnv_toUChars() FAILED %s\n", myErrorName(err));
            else
                log_verbose(" ucnv_toUChars() o.k.\n");

            if(u_strcmp(uchar1,uchar2)!=0) 
                log_err("equality test failed with conversion routine\n");
        }
        else
        {
            log_err("ERR: calling toUChars: Didn't get U_BUFFER_OVERFLOW .. expected it.\n");
        }
        /*Testing ucnv_fromUChars and ucnv_toUChars with error conditions*/
        err=U_ILLEGAL_ARGUMENT_ERROR;
        log_verbose("\n---Testing ucnv_fromUChars() with err != U_ZERO_ERROR\n");
        targetcapacity = ucnv_fromUChars(myConverter, output_cp_buffer, testLong1,  uchar1, -1, &err);
        if (targetcapacity !=0) {
            log_err("\nFAILURE: ucnv_fromUChars with err != U_ZERO_ERROR is expected to fail and return 0\n");
        }
        err=U_ZERO_ERROR;
        log_verbose("\n---Testing ucnv_fromUChars() with converter=NULL\n");
        targetcapacity = ucnv_fromUChars(NULL, output_cp_buffer, testLong1,  uchar1, -1, &err);
        if (targetcapacity !=0 || err != U_ILLEGAL_ARGUMENT_ERROR) {
            log_err("\nFAILURE: ucnv_fromUChars with converter=NULL is expected to fail\n");
        }
        err=U_ZERO_ERROR;
        log_verbose("\n---Testing ucnv_fromUChars() with sourceLength = 0\n");
        targetcapacity = ucnv_fromUChars(myConverter, output_cp_buffer, testLong1,  uchar1, 0, &err);
        if (targetcapacity !=0) {
            log_err("\nFAILURE: ucnv_fromUChars with sourceLength 0 is expected to return 0\n");
        }
        log_verbose("\n---Testing ucnv_fromUChars() with targetLength = 0\n");
        targetcapacity = ucnv_fromUChars(myConverter, output_cp_buffer, 0,  uchar1, -1, &err);
        if (err != U_BUFFER_OVERFLOW_ERROR) {
            log_err("\nFAILURE: ucnv_fromUChars with targetLength 0 is expected to fail and throw U_BUFFER_OVERFLOW_ERROR\n");
        }
        /*toUChars with error conditions*/
        targetsize = ucnv_toUChars(myConverter, uchar2, targetsize, output_cp_buffer, strlen(output_cp_buffer), &err);
        if(targetsize != 0){
            log_err("\nFAILURE: ucnv_toUChars with err != U_ZERO_ERROR is expected to fail and return 0\n");
        }
        err=U_ZERO_ERROR;
        targetsize = ucnv_toUChars(myConverter, uchar2, -1, output_cp_buffer, strlen(output_cp_buffer), &err);
        if(targetsize != 0 || err != U_ILLEGAL_ARGUMENT_ERROR){
            log_err("\nFAILURE: ucnv_toUChars with targetsize < 0 is expected to throw U_ILLEGAL_ARGUMENT_ERROR and return 0\n");
        }
        err=U_ZERO_ERROR;
        targetsize = ucnv_toUChars(myConverter, uchar2, 0, output_cp_buffer, 0, &err);
        if (targetsize !=0) {
            log_err("\nFAILURE: ucnv_toUChars with sourceLength 0 is expected to return 0\n");
        }
        targetcapacity2=0; 
        targetsize = ucnv_toUChars(myConverter, NULL, targetcapacity2, output_cp_buffer,  strlen(output_cp_buffer), &err);
        if (err != U_STRING_NOT_TERMINATED_WARNING) {
            log_err("\nFAILURE: ucnv_toUChars(targetLength)->%s instead of U_STRING_NOT_TERMINATED_WARNING\n",
                    u_errorName(err));
        }
        err=U_ZERO_ERROR;
        /*-----*/


        /*testing for ucnv_fromUnicode() and ucnv_toUnicode() */
        /*Clean up re-usable vars*/
        j=0;
        log_verbose("Testing ucnv_fromUnicode().....\n");
        tmp_ucs_buf=ucs_file_buffer_use; 
        ucnv_fromUnicode(myConverter, &mytarget_1,
                 mytarget + MAX_FILE_LEN,
                 &tmp_ucs_buf,
                 ucs_file_buffer_use+i,
                 NULL,
                 TRUE,
                 &err);
        consumedUni = (UChar*)tmp_consumedUni;

        if (U_FAILURE(err)) 
        {
            log_err("FAILURE! %s\n", myErrorName(err));
        }
        else
            log_verbose("ucnv_fromUnicode()   o.k.\n");

        /*Uni1 ----ToUnicode----> Cp2 ----FromUnicode---->Uni3 */
        log_verbose("Testing ucnv_toUnicode().....\n");
        tmp_mytarget_use=mytarget_use;
        tmp_consumed = consumed;
        ucnv_toUnicode(myConverter, &my_ucs_file_buffer_1,
                my_ucs_file_buffer + MAX_FILE_LEN,
                &tmp_mytarget_use,
                mytarget_use + (mytarget_1 - mytarget),
                NULL,
                FALSE,
                &err);
        consumed = (char*)tmp_consumed;
        if (U_FAILURE(err)) 
        {
            log_err("FAILURE! %s\n", myErrorName(err));
        }
        else
            log_verbose("ucnv_toUnicode()  o.k.\n");


        log_verbose("\n---Testing   RoundTrip ...\n");


        u_strncpy(uchar3, my_ucs_file_buffer,i);
        uchar3[i] = 0;

        if(u_strcmp(uchar1,uchar3)==0)
            log_verbose("Equality test o.k.\n");
        else 
            log_err("Equality test failed\n");

        /*sanity compare */
        if(uchar2 == NULL)
        {
            log_err("uchar2 was NULL (ccapitst.c line %d), couldn't do sanity check\n", __LINE__);
        }
        else
        {
            if(u_strcmp(uchar2, uchar3)==0)
                log_verbose("Equality test o.k.\n");
            else
                log_err("Equality test failed\n");
        }

        fclose(ucs_file_in);
        ucnv_close(myConverter);
        free(displayname);
        if (uchar1 != 0) free(uchar1);
        if (uchar2 != 0) free(uchar2);
        if (uchar3 != 0) free(uchar3);
    }

    free((void*)mytarget);
    free((void*)output_cp_buffer);
    free((void*)ucs_file_buffer);
    free((void*)my_ucs_file_buffer);
}

static UConverterFromUCallback otherUnicodeAction(UConverterFromUCallback MIA)
{
    return (MIA==(UConverterFromUCallback)UCNV_FROM_U_CALLBACK_STOP)?(UConverterFromUCallback)UCNV_FROM_U_CALLBACK_SUBSTITUTE:(UConverterFromUCallback)UCNV_FROM_U_CALLBACK_STOP;
}


static UConverterToUCallback otherCharAction(UConverterToUCallback MIA)
{
    return (MIA==(UConverterToUCallback)UCNV_TO_U_CALLBACK_STOP)?(UConverterToUCallback)UCNV_TO_U_CALLBACK_SUBSTITUTE:(UConverterToUCallback)UCNV_TO_U_CALLBACK_STOP;
}

static void TestFlushCache(void) {
    UErrorCode          err                 =   U_ZERO_ERROR;
    UConverter*            someConverters[5];
    int flushCount = 0;

    /* flush the converter cache to get a consistent state before the flushing is tested */
    ucnv_flushCache();

    /*Testing ucnv_open()*/
    /* Note: These converters have been chosen because they do NOT
       encode the Latin characters (U+0041, ...), and therefore are
       highly unlikely to be chosen as system default codepages */

    someConverters[0] = ucnv_open("ibm-1047", &err);
    if (U_FAILURE(err)) {
        log_data_err("FAILURE! %s\n", myErrorName(err));
    }

    someConverters[1] = ucnv_open("ibm-1047", &err);
    if (U_FAILURE(err)) {
        log_data_err("FAILURE! %s\n", myErrorName(err));
    }

    someConverters[2] = ucnv_open("ibm-1047", &err);
    if (U_FAILURE(err)) {
        log_data_err("FAILURE! %s\n", myErrorName(err));
    }

    someConverters[3] = ucnv_open("gb18030", &err);
    if (U_FAILURE(err)) {
        log_data_err("FAILURE! %s\n", myErrorName(err));
    }

    someConverters[4] = ucnv_open("ibm-954", &err);
    if (U_FAILURE(err)) {
        log_data_err("FAILURE! %s\n", myErrorName(err));
    }


    /* Testing ucnv_flushCache() */
    log_verbose("\n---Testing ucnv_flushCache...\n");
    if ((flushCount=ucnv_flushCache())==0)
        log_verbose("Flush cache ok\n");
    else 
        log_data_err("Flush Cache failed [line %d], expect 0 got %d \n", __LINE__, flushCount);

    /*testing ucnv_close() and ucnv_flushCache() */
    ucnv_close(someConverters[0]);
    ucnv_close(someConverters[1]);

    if ((flushCount=ucnv_flushCache())==0)
        log_verbose("Flush cache ok\n");
    else 
        log_data_err("Flush Cache failed [line %d], expect 0 got %d \n", __LINE__, flushCount);

    ucnv_close(someConverters[2]);
    ucnv_close(someConverters[3]);

    if ((flushCount=ucnv_flushCache())==2) 
        log_verbose("Flush cache ok\n");  /*because first, second and third are same  */
    else 
        log_data_err("Flush Cache failed  line %d, got %d expected 2 or there is an error in ucnv_close()\n",
            __LINE__,
            flushCount);

    ucnv_close(someConverters[4]);
    if ( (flushCount=ucnv_flushCache())==1) 
        log_verbose("Flush cache ok\n");
    else 
        log_data_err("Flush Cache failed line %d, expected 1 got %d \n", __LINE__, flushCount);

}

/**
 * Test the converter alias API, specifically the fuzzy matching of
 * alias names and the alias table integrity.  Make sure each
 * converter has at least one alias (itself), and that its listed
 * aliases map back to itself.  Check some hard-coded UTF-8 and
 * ISO_2022 aliases to make sure they work.
 */
static void TestAlias() {
    int32_t i, ncnv;
    UErrorCode status = U_ZERO_ERROR;

    /* Predetermined aliases that we expect to map back to ISO_2022
     * and UTF-8.  UPDATE THIS DATA AS NECESSARY. */
    const char* ISO_2022_NAMES[] = 
        {"ISO_2022,locale=ja,version=2", "ISO-2022-JP-2", "csISO2022JP2",
         "Iso-2022jP2", "isO-2022_Jp_2", "iSo--2022,locale=ja,version=2"};
    int32_t ISO_2022_NAMES_LENGTH =
        sizeof(ISO_2022_NAMES) / sizeof(ISO_2022_NAMES[0]);
    const char *UTF8_NAMES[] =
        { "UTF-8", "utf-8", "utf8", "ibm-1208",
          "utf_8", "ibm1208", "cp1208" };
    int32_t UTF8_NAMES_LENGTH =
        sizeof(UTF8_NAMES) / sizeof(UTF8_NAMES[0]);

    struct {
        const char *name;
        const char *alias;
    } CONVERTERS_NAMES[] = {
        { "UTF-32BE", "UTF32_BigEndian" },
        { "UTF-32LE", "UTF32_LittleEndian" },
        { "UTF-32",   "ISO-10646-UCS-4" },
        { "UTF32_PlatformEndian", "UTF32_PlatformEndian" },
        { "UTF-32",   "ucs-4" }
    };
    int32_t CONVERTERS_NAMES_LENGTH = sizeof(CONVERTERS_NAMES) / sizeof(*CONVERTERS_NAMES);

    /* When there are bugs in gencnval or in ucnv_io, converters can
       appear to have no aliases. */
    ncnv = ucnv_countAvailable();
    log_verbose("%d converters\n", ncnv);
    for (i=0; i<ncnv; ++i) {
        const char *name = ucnv_getAvailableName(i);
        const char *alias0;
        uint16_t na = ucnv_countAliases(name, &status);
        uint16_t j;
        UConverter *cnv;

        if (na == 0) {
            log_err("FAIL: Converter \"%s\" (i=%d)"
                    " has no aliases; expect at least one\n",
                    name, i);
            continue;
        }
        cnv = ucnv_open(name, &status);
        if (U_FAILURE(status)) {
            log_data_err("FAIL: Converter \"%s\" (i=%d)"
                    " can't be opened.\n",
                    name, i);
        }
        else {
            if (strcmp(ucnv_getName(cnv, &status), name) != 0 
                && (strstr(name, "PlatformEndian") == 0 && strstr(name, "OppositeEndian") == 0)) {
                log_err("FAIL: Converter \"%s\" returned \"%s\" for getName. "
                        "The should be the same\n",
                        name, ucnv_getName(cnv, &status));
            }
        }
        ucnv_close(cnv);

        status = U_ZERO_ERROR;
        alias0 = ucnv_getAlias(name, 0, &status);
        for (j=1; j<na; ++j) {
            const char *alias;
            /* Make sure each alias maps back to the the same list of
               aliases.  Assume that if alias 0 is the same, the whole
               list is the same (this should always be true). */
            const char *mapBack;

            status = U_ZERO_ERROR;
            alias = ucnv_getAlias(name, j, &status);
            if (status == U_AMBIGUOUS_ALIAS_WARNING) {
                log_err("FAIL: Converter \"%s\"is ambiguous\n", name);
            }

            if (alias == NULL) {
                log_err("FAIL: Converter \"%s\" -> "
                        "alias[%d]=NULL\n",
                        name, j);
                continue;
            }

            mapBack = ucnv_getAlias(alias, 0, &status);

            if (mapBack == NULL) {
                log_err("FAIL: Converter \"%s\" -> "
                        "alias[%d]=\"%s\" -> "
                        "alias[0]=NULL, exp. \"%s\"\n",
                        name, j, alias, alias0);
                continue;
            }

            if (0 != strcmp(alias0, mapBack)) {
                int32_t idx;
                UBool foundAlias = FALSE;
                if (status == U_AMBIGUOUS_ALIAS_WARNING) {
                    /* Make sure that we only get this mismapping when there is
                       an ambiguous alias, and the other converter has this alias too. */
                    for (idx = 0; idx < ucnv_countAliases(mapBack, &status); idx++) {
                        if (strcmp(ucnv_getAlias(mapBack, (uint16_t)idx, &status), alias) == 0) {
                            foundAlias = TRUE;
                            break;
                        }
                    }
                }
                /* else not ambiguous, and this is a real problem. foundAlias = FALSE */

                if (!foundAlias) {
                    log_err("FAIL: Converter \"%s\" -> "
                            "alias[%d]=\"%s\" -> "
                            "alias[0]=\"%s\", exp. \"%s\"\n",
                            name, j, alias, mapBack, alias0);
                }
            }
        }
    }


    /* Check a list of predetermined aliases that we expect to map
     * back to ISO_2022 and UTF-8. */
    for (i=1; i<ISO_2022_NAMES_LENGTH; ++i) {
        const char* mapBack = ucnv_getAlias(ISO_2022_NAMES[i], 0, &status);
        if(!mapBack) {
          log_data_err("Couldn't get alias for %s. You probably have no data\n", ISO_2022_NAMES[i]);
          continue;
        }
        if (0 != strcmp(mapBack, ISO_2022_NAMES[0])) {
            log_err("FAIL: \"%s\" -> \"%s\", expect \"ISO_2022,locale=ja,version=2\"\n",
                    ISO_2022_NAMES[i], mapBack);
        }
    }


    for (i=1; i<UTF8_NAMES_LENGTH; ++i) {
        const char* mapBack = ucnv_getAlias(UTF8_NAMES[i], 0, &status);
        if(!mapBack) {
          log_data_err("Couldn't get alias for %s. You probably have no data\n", UTF8_NAMES[i]);
          continue;
        }
        if (mapBack && 0 != strcmp(mapBack, UTF8_NAMES[0])) {
            log_err("FAIL: \"%s\" -> \"%s\", expect UTF-8\n",
                    UTF8_NAMES[i], mapBack);
        }
    }

    /*
     * Check a list of predetermined aliases that we expect to map
     * back to predermined converter names.
     */

    for (i = 0; i < CONVERTERS_NAMES_LENGTH; ++i) {
        const char* mapBack = ucnv_getAlias(CONVERTERS_NAMES[i].alias, 0, &status);
        if(!mapBack) {
          log_data_err("Couldn't get alias for %s. You probably have no data\n", CONVERTERS_NAMES[i]);
          continue;
        }
        if (0 != strcmp(mapBack, CONVERTERS_NAMES[i].name)) {
            log_err("FAIL: \"%s\" -> \"%s\", expect %s\n",
                    CONVERTERS_NAMES[i].alias, mapBack, CONVERTERS_NAMES[i].name);
        }
    }

}

static void TestDuplicateAlias(void) {
    const char *alias;
    UErrorCode status = U_ZERO_ERROR;

    status = U_ZERO_ERROR;
    alias = ucnv_getStandardName("Shift_JIS", "IBM", &status);
    if (alias == NULL || strcmp(alias, "ibm-943") != 0 || status != U_AMBIGUOUS_ALIAS_WARNING) {
        log_err("FAIL: Didn't get ibm-943 for Shift_JIS {IBM}. Got %s\n", alias);
    }
    status = U_ZERO_ERROR;
    alias = ucnv_getStandardName("ibm-943", "IANA", &status);
    if (alias == NULL || strcmp(alias, "Shift_JIS") != 0 || status != U_AMBIGUOUS_ALIAS_WARNING) {
        log_data_err("FAIL: Didn't get Shift_JIS for ibm-943 {IANA}. Got %s\n", alias);
    }
    status = U_ZERO_ERROR;
    alias = ucnv_getStandardName("ibm-943_P130-2000", "IANA", &status);
    if (alias != NULL || status == U_AMBIGUOUS_ALIAS_WARNING) {
        log_data_err("FAIL: Didn't get NULL for ibm-943 {IANA}. Got %s\n", alias);
    }
}


/* Test safe clone callback */

static uint32_t    TSCC_nextSerial()
{
    static uint32_t n = 1;
    
    return (n++);
}

typedef struct
{
    uint32_t       magic;      /* 0xC0FFEE to identify that the object is OK */
    uint32_t       serial;     /* minted from nextSerial, above */
    UBool          wasClosed;  /* close happened on the object */
} TSCCContext;

static TSCCContext *TSCC_clone(TSCCContext *ctx)
{
    TSCCContext *newCtx = (TSCCContext *)malloc(sizeof(TSCCContext));
    
    newCtx->serial = TSCC_nextSerial();
    newCtx->wasClosed = 0;
    newCtx->magic = 0xC0FFEE;
    
    log_verbose("TSCC_clone: %p:%d -> new context %p:%d\n", ctx, ctx->serial, newCtx, newCtx->serial);
    
    return newCtx;
}

static void TSCC_fromU(const void *context,
                        UConverterFromUnicodeArgs *fromUArgs,
                        const UChar* codeUnits,
                        int32_t length,
                        UChar32 codePoint,
                        UConverterCallbackReason reason,
                        UErrorCode * err)
{
    TSCCContext *ctx = (TSCCContext*)context;
    UConverterFromUCallback junkFrom;
    
    log_verbose("TSCC_fromU: Context %p:%d called, reason %d on cnv %p\n", ctx, ctx->serial, reason, fromUArgs->converter);

    if(ctx->magic != 0xC0FFEE) {
        log_err("TSCC_fromU: Context %p:%d magic is 0x%x should be 0xC0FFEE.\n", ctx,ctx->serial, ctx->magic);
        return;
    }

    if(reason == UCNV_CLONE) {
        UErrorCode subErr = U_ZERO_ERROR;
        TSCCContext *newCtx;
        TSCCContext *junkCtx;

        /* "recreate" it */
        log_verbose("TSCC_fromU: cloning..\n");
        newCtx = TSCC_clone(ctx);

        if(newCtx == NULL) {
            log_err("TSCC_fromU: internal clone failed on %p\n", ctx);
        }

        /* now, SET it */
        ucnv_getFromUCallBack(fromUArgs->converter, &junkFrom, (const void**)&junkCtx);
        ucnv_setFromUCallBack(fromUArgs->converter, junkFrom, newCtx, NULL, NULL, &subErr);
        
        if(U_FAILURE(subErr)) {
            *err = subErr;
        }    
    }

    if(reason == UCNV_CLOSE) {
        log_verbose("TSCC_fromU: Context %p:%d closing\n", ctx, ctx->serial);
        ctx->wasClosed = TRUE;
    }
}


static void TSCC_toU(const void *context,
                        UConverterToUnicodeArgs *toUArgs,
                        const char* codeUnits,
                        int32_t length,
                        UConverterCallbackReason reason,
                        UErrorCode * err)
{
    TSCCContext *ctx = (TSCCContext*)context;
    UConverterToUCallback junkFrom;
    
    log_verbose("TSCC_toU: Context %p:%d called, reason %d on cnv %p\n", ctx, ctx->serial, reason, toUArgs->converter);

    if(ctx->magic != 0xC0FFEE) {
        log_err("TSCC_toU: Context %p:%d magic is 0x%x should be 0xC0FFEE.\n", ctx,ctx->serial, ctx->magic);
        return;
    }

    if(reason == UCNV_CLONE) {
        UErrorCode subErr = U_ZERO_ERROR;
        TSCCContext *newCtx;
        TSCCContext *junkCtx;

        /* "recreate" it */
        log_verbose("TSCC_toU: cloning..\n");
        newCtx = TSCC_clone(ctx);

        if(newCtx == NULL) {
            log_err("TSCC_toU: internal clone failed on %p\n", ctx);
        }

        /* now, SET it */
        ucnv_getToUCallBack(toUArgs->converter, &junkFrom, (const void**)&junkCtx);
        ucnv_setToUCallBack(toUArgs->converter, junkFrom, newCtx, NULL, NULL, &subErr);
        
        if(U_FAILURE(subErr)) {
            *err = subErr;
        }    
    }

    if(reason == UCNV_CLOSE) {
        log_verbose("TSCC_toU: Context %p:%d closing\n", ctx, ctx->serial);
        ctx->wasClosed = TRUE;
    }
}

static void TSCC_init(TSCCContext *q)
{
    q->magic = 0xC0FFEE;
    q->serial = TSCC_nextSerial();
    q->wasClosed = 0;
}

static void TSCC_print_log(TSCCContext *q, const char *name)
{
    if(q==NULL) {
        log_verbose("TSCContext: %s is NULL!!\n", name);
    } else {
        if(q->magic != 0xC0FFEE) {
            log_err("TSCCContext: %p:%d's magic is %x, supposed to be 0xC0FFEE\n",
                    q,q->serial, q->magic);
        }
        log_verbose("TSCCContext %p:%d=%s - magic %x, %s\n",
                    q, q->serial, name, q->magic, q->wasClosed?"CLOSED":"open");
    }
}

static void TestConvertSafeCloneCallback()
{
    UErrorCode err = U_ZERO_ERROR;
    TSCCContext from1, to1;
    TSCCContext *from2, *from3, *to2, *to3;
    char hunk[8192];
    int32_t hunkSize = 8192;
    UConverterFromUCallback junkFrom;
    UConverterToUCallback junkTo;
    UConverter *conv1, *conv2 = NULL;

    conv1 = ucnv_open("iso-8859-3", &err);
    
    if(U_FAILURE(err)) {
        log_data_err("Err opening iso-8859-3, %s", u_errorName(err));
        return;
    }

    log_verbose("Opened conv1=%p\n", conv1);

    TSCC_init(&from1);
    TSCC_init(&to1);

    TSCC_print_log(&from1, "from1");
    TSCC_print_log(&to1, "to1");

    ucnv_setFromUCallBack(conv1, TSCC_fromU, &from1, NULL, NULL, &err);
    log_verbose("Set from1 on conv1\n");
    TSCC_print_log(&from1, "from1");

    ucnv_setToUCallBack(conv1, TSCC_toU, &to1, NULL, NULL, &err);
    log_verbose("Set to1 on conv1\n");
    TSCC_print_log(&to1, "to1");

    conv2 = ucnv_safeClone(conv1, hunk, &hunkSize, &err);
    if(U_FAILURE(err)) {
        log_err("safeClone failed: %s\n", u_errorName(err));
        return;
    }
    log_verbose("Cloned to conv2=%p.\n", conv2);

/**********   from *********************/
    ucnv_getFromUCallBack(conv2, &junkFrom, (const void**)&from2);
    ucnv_getFromUCallBack(conv1, &junkFrom, (const void**)&from3);

    TSCC_print_log(from2, "from2");
    TSCC_print_log(from3, "from3(==from1)");

    if(from2 == NULL) {
        log_err("FAIL! from2 is null \n");
        return;
    }

    if(from3 == NULL) {
        log_err("FAIL! from3 is null \n");
        return;
    }

    if(from3 != (&from1) ) {
        log_err("FAIL! conv1's FROM context changed!\n");
    }

    if(from2 == (&from1) ) {
        log_err("FAIL! conv1's FROM context is the same as conv2's!\n");
    }

    if(from1.wasClosed) {
        log_err("FAIL! from1 is closed \n");
    }

    if(from2->wasClosed) {
        log_err("FAIL! from2 was closed\n");
    }

/**********   to *********************/
    ucnv_getToUCallBack(conv2, &junkTo, (const void**)&to2);
    ucnv_getToUCallBack(conv1, &junkTo, (const void**)&to3);

    TSCC_print_log(to2, "to2");
    TSCC_print_log(to3, "to3(==to1)");

    if(to2 == NULL) {
        log_err("FAIL! to2 is null \n");
        return;
    }

    if(to3 == NULL) {
        log_err("FAIL! to3 is null \n");
        return;
    }

    if(to3 != (&to1) ) {
        log_err("FAIL! conv1's TO context changed!\n");
    }

    if(to2 == (&to1) ) {
        log_err("FAIL! conv1's TO context is the same as conv2's!\n");
    }

    if(to1.wasClosed) {
        log_err("FAIL! to1 is closed \n");
    }

    if(to2->wasClosed) {
        log_err("FAIL! to2 was closed\n");
    }

/*************************************/

    ucnv_close(conv1);
    log_verbose("ucnv_closed (conv1)\n");
    TSCC_print_log(&from1, "from1");
    TSCC_print_log(from2, "from2");
    TSCC_print_log(&to1, "to1");
    TSCC_print_log(to2, "to2");

    if(from1.wasClosed == FALSE) {
        log_err("FAIL! from1 is NOT closed \n");
    }

    if(from2->wasClosed) {
        log_err("FAIL! from2 was closed\n");
    }

    if(to1.wasClosed == FALSE) {
        log_err("FAIL! to1 is NOT closed \n");
    }

    if(to2->wasClosed) {
        log_err("FAIL! to2 was closed\n");
    }

    ucnv_close(conv2);
    log_verbose("ucnv_closed (conv2)\n");

    TSCC_print_log(&from1, "from1");
    TSCC_print_log(from2, "from2");

    if(from1.wasClosed == FALSE) {
        log_err("FAIL! from1 is NOT closed \n");
    }

    if(from2->wasClosed == FALSE) {
        log_err("FAIL! from2 was NOT closed\n");
    }   

    TSCC_print_log(&to1, "to1");
    TSCC_print_log(to2, "to2");

    if(to1.wasClosed == FALSE) {
        log_err("FAIL! to1 is NOT closed \n");
    }

    if(to2->wasClosed == FALSE) {
        log_err("FAIL! to2 was NOT closed\n");
    }   

    if(to2 != (&to1)) {
        free(to2); /* to1 is stack based */
    }
    if(from2 != (&from1)) {
        free(from2); /* from1 is stack based */
    }
}

static UBool
containsAnyOtherByte(uint8_t *p, int32_t length, uint8_t b) {
    while(length>0) {
        if(*p!=b) {
            return TRUE;
        }
        ++p;
        --length;
    }
    return FALSE;
}

static void TestConvertSafeClone()
{
    /* one 'regular' & all the 'private stateful' converters */
    static const char *const names[] = {
        "ibm-1047",
        "ISO_2022,locale=zh,version=1",
        "SCSU",
        "HZ",
        "lmbcs",
        "ISCII,version=0",
        "ISO_2022,locale=kr,version=1",
        "ISO_2022,locale=jp,version=2",
        "BOCU-1",
        "UTF-7",
        "IMAP-mailbox-name",
        "ibm-1047-s390"
    };

    static const int32_t bufferSizes[] = {
        U_CNV_SAFECLONE_BUFFERSIZE,
        (int32_t)(3*sizeof(UConverter))/2,  /* 1.5*sizeof(UConverter) */
        (int32_t)sizeof(UConverter)/2       /* 0.5*sizeof(UConverter) */
    };

    char charBuffer [21];   /* Leave at an odd number for alignment testing */
    uint8_t buffer [3] [U_CNV_SAFECLONE_BUFFERSIZE];
    int32_t bufferSize, maxBufferSize;
    const char *maxName;
    UConverter * cnv, *cnv2;
    UErrorCode err;

    char *pCharBuffer;
    const char *pConstCharBuffer;
    const char *charBufferLimit = charBuffer + sizeof(charBuffer)/sizeof(*charBuffer);
    UChar uniBuffer [] = {0x0058, 0x0059, 0x005A}; /* "XYZ" */
    UChar uniCharBuffer [20];
    char  charSourceBuffer [] = { 0x1b, 0x24, 0x42 };
    const char *pCharSource = charSourceBuffer;
    const char *pCharSourceLimit = charSourceBuffer + sizeof(charSourceBuffer);
    UChar *pUCharTarget = uniCharBuffer;
    UChar *pUCharTargetLimit = uniCharBuffer + sizeof(uniCharBuffer)/sizeof(*uniCharBuffer);
    const UChar * pUniBuffer;
    const UChar *uniBufferLimit = uniBuffer + sizeof(uniBuffer)/sizeof(*uniBuffer);
    int32_t index, j;

    err = U_ZERO_ERROR;
    cnv = ucnv_open(names[0], &err);
    if(U_SUCCESS(err)) {
        /* Check the various error & informational states: */

        /* Null status - just returns NULL */
        bufferSize = U_CNV_SAFECLONE_BUFFERSIZE;
        if (0 != ucnv_safeClone(cnv, buffer[0], &bufferSize, 0))
        {
            log_err("FAIL: Cloned converter failed to deal correctly with null status\n");
        }
        /* error status - should return 0 & keep error the same */
        err = U_MEMORY_ALLOCATION_ERROR;
        if (0 != ucnv_safeClone(cnv, buffer[0], &bufferSize, &err) || err != U_MEMORY_ALLOCATION_ERROR)
        {
            log_err("FAIL: Cloned converter failed to deal correctly with incoming error status\n");
        }
        err = U_ZERO_ERROR;

        /* Null buffer size pointer - just returns NULL & set error to U_ILLEGAL_ARGUMENT_ERROR*/
        if (0 != ucnv_safeClone(cnv, buffer[0], 0, &err) || err != U_ILLEGAL_ARGUMENT_ERROR)
        {
            log_err("FAIL: Cloned converter failed to deal correctly with null bufferSize pointer\n");
        }
        err = U_ZERO_ERROR;

        /* buffer size pointer is 0 - fill in pbufferSize with a size */
        bufferSize = 0;
        if (0 != ucnv_safeClone(cnv, buffer[0], &bufferSize, &err) || U_FAILURE(err) || bufferSize <= 0)
        {
            log_err("FAIL: Cloned converter failed a sizing request ('preflighting')\n");
        }
        /* Verify our define is large enough  */
        if (U_CNV_SAFECLONE_BUFFERSIZE < bufferSize)
        {
            log_err("FAIL: Pre-calculated buffer size is too small\n");
        }
        /* Verify we can use this run-time calculated size */
        if (0 == (cnv2 = ucnv_safeClone(cnv, buffer[0], &bufferSize, &err)) || U_FAILURE(err))
        {
            log_err("FAIL: Converter can't be cloned with run-time size\n");
        }
        if (cnv2) {
            ucnv_close(cnv2);
        }

        /* size one byte too small - should allocate & let us know */
        --bufferSize;
        if (0 == (cnv2 = ucnv_safeClone(cnv, 0, &bufferSize, &err)) || err != U_SAFECLONE_ALLOCATED_WARNING)
        {
            log_err("FAIL: Cloned converter failed to deal correctly with too-small buffer size\n");
        }
        if (cnv2) {
            ucnv_close(cnv2);
        }

        err = U_ZERO_ERROR;
        bufferSize = U_CNV_SAFECLONE_BUFFERSIZE;

        /* Null buffer pointer - return converter & set error to U_SAFECLONE_ALLOCATED_ERROR */
        if (0 == (cnv2 = ucnv_safeClone(cnv, 0, &bufferSize, &err)) || err != U_SAFECLONE_ALLOCATED_WARNING)
        {
            log_err("FAIL: Cloned converter failed to deal correctly with null buffer pointer\n");
        }
        if (cnv2) {
            ucnv_close(cnv2);
        }

        err = U_ZERO_ERROR;
    
        /* Null converter - return NULL & set U_ILLEGAL_ARGUMENT_ERROR */
        if (0 != ucnv_safeClone(0, buffer[0], &bufferSize, &err) || err != U_ILLEGAL_ARGUMENT_ERROR)
        {
            log_err("FAIL: Cloned converter failed to deal correctly with null converter pointer\n");
        }

        ucnv_close(cnv);
    }

    maxBufferSize = 0;
    maxName = "";

    /* Do these cloned converters work at all - shuffle UChars to chars & back again..*/

    for(j = 0; j < LENGTHOF(bufferSizes); ++j) {
        for (index = 0; index < LENGTHOF(names); index++)
        {
            err = U_ZERO_ERROR;
            cnv = ucnv_open(names[index], &err);
            if(U_FAILURE(err)) {
                log_data_err("ucnv_open(\"%s\") failed - %s\n", names[index], u_errorName(err));
                continue;
            }

            if(j == 0) {
                /* preflight to get maxBufferSize */
                bufferSize = 0;
                ucnv_safeClone(cnv, NULL, &bufferSize, &err);
                if(bufferSize > maxBufferSize) {
                    maxBufferSize = bufferSize;
                    maxName = names[index];
                }
            }

            memset(buffer, 0xaa, sizeof(buffer));

            bufferSize = bufferSizes[j];
            cnv2 = ucnv_safeClone(cnv, buffer[1], &bufferSize, &err);

            /* close the original immediately to make sure that the clone works by itself */
            ucnv_close(cnv);

            /* check if the clone function overwrote any bytes that it is not supposed to touch */
            if(bufferSize <= bufferSizes[j]) {
                /* used the stack buffer */
                if( containsAnyOtherByte(buffer[0], (int32_t)sizeof(buffer[0]), 0xaa) ||
                    containsAnyOtherByte(buffer[1]+bufferSize, (int32_t)(sizeof(buffer)-(sizeof(buffer[0])+bufferSize)), 0xaa)
                ) {
                    log_err("cloning %s in a stack buffer overwrote bytes outside the bufferSize %d (requested %d)\n",
                        names[index], bufferSize, bufferSizes[j]);
                }
            } else {
                /* heap-allocated the clone */
                if(containsAnyOtherByte(buffer[0], (int32_t)sizeof(buffer), 0xaa)) {
                    log_err("cloning %s used the heap (bufferSize %d, requested %d) but overwrote stack buffer bytes\n",
                        names[index], bufferSize, bufferSizes[j]);
                }
            }

            pCharBuffer = charBuffer;
            pUniBuffer = uniBuffer;

            ucnv_fromUnicode(cnv2, 
                            &pCharBuffer, 
                            charBufferLimit,
                            &pUniBuffer,
                            uniBufferLimit,
                            NULL,
                            TRUE,
                            &err);
            if(U_FAILURE(err)){
                log_err("FAIL: cloned converter failed to do fromU conversion. Error: %s\n",u_errorName(err));
            }
            ucnv_toUnicode(cnv2,
                           &pUCharTarget,
                           pUCharTargetLimit,
                           &pCharSource,
                           pCharSourceLimit,
                           NULL,
                           TRUE,
                           &err
                           );

            if(U_FAILURE(err)){
                log_err("FAIL: cloned converter failed to do toU conversion. Error: %s\n",u_errorName(err));
            }

            pConstCharBuffer = charBuffer;
            if (uniBuffer [0] != ucnv_getNextUChar(cnv2, &pConstCharBuffer, pCharBuffer, &err))
            {
                log_err("FAIL: Cloned converter failed to do conversion. Error: %s\n",u_errorName(err));
            }
            ucnv_close(cnv2);
        }
    }

    log_verbose("ucnv_safeClone(): sizeof(UConverter)=%lu  max preflighted clone size=%d (%s)  U_CNV_SAFECLONE_BUFFERSIZE=%d\n",
        sizeof(UConverter), maxBufferSize, maxName, (int)U_CNV_SAFECLONE_BUFFERSIZE);
}

static void TestCCSID() {
    UConverter *cnv;
    UErrorCode errorCode;
    int32_t ccsids[]={ 37, 850, 943, 949, 950, 1047, 1252, 33722 };
    int32_t i, ccsid;

    for(i=0; i<(int32_t)(sizeof(ccsids)/sizeof(int32_t)); ++i) {
        ccsid=ccsids[i];

        errorCode=U_ZERO_ERROR;
        cnv=ucnv_openCCSID(ccsid, UCNV_IBM, &errorCode);
        if(U_FAILURE(errorCode)) {
    	log_data_err("error: ucnv_openCCSID(%ld) failed (%s)\n", ccsid, u_errorName(errorCode));
            continue;
        }

        if(ccsid!=ucnv_getCCSID(cnv, &errorCode)) {
            log_err("error: ucnv_getCCSID(ucnv_openCCSID(%ld))=%ld\n", ccsid, ucnv_getCCSID(cnv, &errorCode));
        }

        if(UCNV_IBM!=ucnv_getPlatform(cnv, &errorCode)) {
            log_err("error: ucnv_getPlatform(ucnv_openCCSID(%ld))=%ld!=UCNV_IBM\n", ccsid, ucnv_getPlatform(cnv, &errorCode));
        }

        ucnv_close(cnv);
    }
}

/* jitterbug 932: ucnv_convert() bugs --------------------------------------- */

/* CHUNK_SIZE defined in common\ucnv.c: */
#define CHUNK_SIZE 1024

static void bug1(void);
static void bug2(void);
static void bug3(void);

static void
TestJ932(void)
{
   bug1(); /* Unicode intermediate buffer straddle bug */
   bug2(); /* pre-flighting size incorrect caused by simple overflow */
   bug3(); /* pre-flighting size incorrect caused by expansion overflow */
}

/*
 * jitterbug 932: test chunking boundary conditions in

    int32_t  ucnv_convert(const char *toConverterName,
                          const char *fromConverterName,
                          char *target,
                          int32_t targetSize,
                          const char *source,
                          int32_t sourceSize,
                          UErrorCode * err)

 * See discussions on the icu mailing list in
 * 2001-April with the subject "converter 'flush' question".
 *
 * Bug report and test code provided by Edward J. Batutis.
 */
static void bug1()
{
   static char char_in[CHUNK_SIZE+32];
   static char char_out[CHUNK_SIZE*2];

   /* GB 18030 equivalent of U+10000 is 90308130 */
   static const char test_seq[]={ (char)0x90u, 0x30, (char)0x81u, 0x30 };

   UErrorCode err = U_ZERO_ERROR;
   int32_t i, test_seq_len = sizeof(test_seq);

   /*
    * causes straddle bug in Unicode intermediate buffer by sliding the test sequence forward
    * until the straddle bug appears. I didn't want to hard-code everything so this test could
    * be expanded - however this is the only type of straddle bug I can think of at the moment -
    * a high surrogate in the last position of the Unicode intermediate buffer. Apparently no
    * other Unicode sequences cause a bug since combining sequences are not supported by the
    * converters.
    */

   for (i = test_seq_len; i >= 0; i--) {
      /* put character sequence into input buffer */
      memset(char_in, 0x61, sizeof(char_in)); /* GB 18030 'a' */
      memcpy(char_in + (CHUNK_SIZE - i), test_seq, test_seq_len);

      /* do the conversion */
      ucnv_convert("us-ascii", /* out */
                   "gb18030",  /* in */
                   char_out,
                   sizeof(char_out),
                   char_in,
                   sizeof(char_in),
                   &err);

      /* bug1: */
      if (err == U_TRUNCATED_CHAR_FOUND) {
         /* this happens when surrogate pair straddles the intermediate buffer in
          * T_UConverter_fromCodepageToCodepage */
         log_err("error j932 bug 1: expected success, got U_TRUNCATED_CHAR_FOUND\n");
      }
   }
}

/* bug2: pre-flighting loop bug: simple overflow causes bug */
static void bug2()
{
    /* US-ASCII "1234567890" */
    static const char source[]={ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };
    static const char sourceUTF8[]={ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, (char)0xef, (char)0x80, (char)0x80 };
    static const char sourceUTF32[]={ 0x00, 0x00, 0x00, 0x30,
                                      0x00, 0x00, 0x00, 0x31,
                                      0x00, 0x00, 0x00, 0x32,
                                      0x00, 0x00, 0x00, 0x33,
                                      0x00, 0x00, 0x00, 0x34,
                                      0x00, 0x00, 0x00, 0x35,
                                      0x00, 0x00, 0x00, 0x36,
                                      0x00, 0x00, 0x00, 0x37,
                                      0x00, 0x00, 0x00, 0x38,
                                      0x00, 0x00, (char)0xf0, 0x00};
    static char target[5];

    UErrorCode err = U_ZERO_ERROR;
    int32_t size;

    /* do the conversion */
    size = ucnv_convert("iso-8859-1", /* out */
                        "us-ascii",  /* in */
                        target,
                        sizeof(target),
                        source,
                        sizeof(source),
                        &err);

    if ( size != 10 ) {
        /* bug2: size is 5, should be 10 */
        log_data_err("error j932 bug 2 us-ascii->iso-8859-1: got preflighting size %d instead of 10\n", size);
    }

    err = U_ZERO_ERROR;
    /* do the conversion */
    size = ucnv_convert("UTF-32BE", /* out */
                        "UTF-8",  /* in */
                        target,
                        sizeof(target),
                        sourceUTF8,
                        sizeof(sourceUTF8),
                        &err);

    if ( size != 32 ) {
        /* bug2: size is 5, should be 32 */
        log_err("error j932 bug 2 UTF-8->UTF-32BE: got preflighting size %d instead of 32\n", size);
    }

    err = U_ZERO_ERROR;
    /* do the conversion */
    size = ucnv_convert("UTF-8", /* out */
                        "UTF-32BE",  /* in */
                        target,
                        sizeof(target),
                        sourceUTF32,
                        sizeof(sourceUTF32),
                        &err);

    if ( size != 12 ) {
        /* bug2: size is 5, should be 12 */
        log_err("error j932 bug 2 UTF-32BE->UTF-8: got preflighting size %d instead of 12\n", size);
    }
}

/*
 * bug3: when the characters expand going from source to target codepage
 *       you get bug3 in addition to bug2
 */
static void bug3()
{
    static char char_in[CHUNK_SIZE*4];
    static char target[5];
    UErrorCode err = U_ZERO_ERROR;
    int32_t size;

    /*
     * first get the buggy size from bug2 then
     * compare it to buggy size with an expansion
     */
    memset(char_in, 0x61, sizeof(char_in)); /* US-ASCII 'a' */

    /* do the conversion */
    size = ucnv_convert("lmbcs",     /* out */
                        "us-ascii",  /* in */
                        target,
                        sizeof(target),
                        char_in,
                        sizeof(char_in),
                        &err);

    if ( size != sizeof(char_in) ) {
        /*
         * bug2: size is 0x2805 (CHUNK_SIZE*2+5 - maybe 5 is the size of the overflow buffer
         * in the converter?), should be CHUNK_SIZE*4
         *
         * Markus 2001-05-18: 5 is the size of our target[] here, ucnv_convert() did not reset targetSize...
         */
        log_data_err("error j932 bug 2/3a: expected preflighting size 0x%04x, got 0x%04x\n", sizeof(char_in), size);
    }

    /*
     * now do the conversion with expansion
     * ascii 0x08 expands to 0x0F 0x28 in lmbcs
     */
    memset(char_in, 8, sizeof(char_in));
    err = U_ZERO_ERROR;

    /* do the conversion */
    size = ucnv_convert("lmbcs", /* out */
                        "us-ascii",  /* in */
                        target,
                        sizeof(target),
                        char_in,
                        sizeof(char_in),
                        &err);

    /* expect 2X expansion */
    if ( size != sizeof(char_in) * 2 ) {
        /*
         * bug3:
         * bug2 would lead us to expect 0x2805, but it isn't that either, it is 0x3c05:
         */
        log_data_err("error j932 bug 3b: expected 0x%04x, got 0x%04x\n", sizeof(char_in) * 2, size);
    }
}

static void
convertExStreaming(UConverter *srcCnv, UConverter *targetCnv,
                   const char *src, int32_t srcLength,
                   const char *expectTarget, int32_t expectTargetLength,
                   int32_t chunkSize,
                   const char *testName,
                   UErrorCode expectCode) {
    UChar pivotBuffer[CHUNK_SIZE];
    UChar *pivotSource, *pivotTarget;
    const UChar *pivotLimit;

    char targetBuffer[CHUNK_SIZE];
    char *target;
    const char *srcLimit, *finalSrcLimit, *targetLimit;

    int32_t targetLength;

    UBool flush;

    UErrorCode errorCode;

    /* setup */
    if(chunkSize>CHUNK_SIZE) {
        chunkSize=CHUNK_SIZE;
    }

    pivotSource=pivotTarget=pivotBuffer;
    pivotLimit=pivotBuffer+chunkSize;

    finalSrcLimit=src+srcLength;
    target=targetBuffer;
    targetLimit=targetBuffer+chunkSize;

    ucnv_resetToUnicode(srcCnv);
    ucnv_resetFromUnicode(targetCnv);

    errorCode=U_ZERO_ERROR;
    flush=FALSE;

    /* convert, streaming-style (both converters and pivot keep state) */
    for(;;) {
        /* for testing, give ucnv_convertEx() at most <chunkSize> input/pivot/output units at a time */
        if(src+chunkSize<=finalSrcLimit) {
            srcLimit=src+chunkSize;
        } else {
            srcLimit=finalSrcLimit;
        }
        ucnv_convertEx(targetCnv, srcCnv,
                       &target, targetLimit,
                       &src, srcLimit,
                       pivotBuffer, &pivotSource, &pivotTarget, pivotLimit,
                       FALSE, flush, &errorCode);
        targetLength=(int32_t)(target-targetBuffer);
        if(errorCode==U_BUFFER_OVERFLOW_ERROR) {
            /* continue converting another chunk */
            errorCode=U_ZERO_ERROR;
            if(targetLength+chunkSize<=sizeof(targetBuffer)) {
                targetLimit=target+chunkSize;
            } else {
                targetLimit=targetBuffer+sizeof(targetBuffer);
            }
        } else if(U_FAILURE(errorCode)) {
            /* failure */
            break;
        } else if(flush) {
            /* all done */
            break;
        } else if(src==finalSrcLimit && pivotSource==pivotTarget) {
            /* all consumed, now flush without input (separate from conversion for testing) */
            flush=TRUE;
        }
    }

    if(!(errorCode==expectCode || (expectCode==U_ZERO_ERROR && errorCode==U_STRING_NOT_TERMINATED_WARNING))) {
        log_err("ucnv_convertEx(%s) chunk[%d] results in %s instead of %s\n",
                testName, chunkSize, u_errorName(errorCode), u_errorName(expectCode));
    } else if(targetLength!=expectTargetLength) {
        log_err("ucnv_convertEx(%s) chunk[%d] writes %d bytes instead of %d\n",
                testName, chunkSize, targetLength, expectTargetLength);
    } else if(memcmp(targetBuffer, expectTarget, targetLength)!=0) {
        log_err("ucnv_convertEx(%s) chunk[%d] writes different bytes than expected\n",
                testName, chunkSize);
    }
}

static void
convertExMultiStreaming(UConverter *srcCnv, UConverter *targetCnv,
                        const char *src, int32_t srcLength,
                        const char *expectTarget, int32_t expectTargetLength,
                        const char *testName,
                        UErrorCode expectCode) {
    convertExStreaming(srcCnv, targetCnv,
                       src, srcLength,
                       expectTarget, expectTargetLength,
                       1, testName, expectCode);
    convertExStreaming(srcCnv, targetCnv,
                       src, srcLength,
                       expectTarget, expectTargetLength,
                       3, testName, expectCode);
    convertExStreaming(srcCnv, targetCnv,
                       src, srcLength,
                       expectTarget, expectTargetLength,
                       7, testName, expectCode);
}

static void TestConvertEx() {
    static const uint8_t
    utf8[]={
        /* 4e00           30a1              ff61              0410 */
        0xe4, 0xb8, 0x80, 0xe3, 0x82, 0xa1, 0xef, 0xbd, 0xa1, 0xd0, 0x90
    },
    shiftJIS[]={
        0x88, 0xea, 0x83, 0x40, 0xa1, 0x84, 0x40
    },
    errorTarget[]={
        /*
         * expected output when converting shiftJIS[] from UTF-8 to Shift-JIS:
         * SUB, SUB, 0x40, SUB, SUB, 0x40
         */
        0xfc, 0xfc, 0xfc, 0xfc, 0x40, 0xfc, 0xfc, 0xfc, 0xfc, 0x40
    };

    char srcBuffer[100], targetBuffer[100];

    const char *src;
    char *target;

    UChar pivotBuffer[100];
    UChar *pivotSource, *pivotTarget;

    UConverter *cnv1, *cnv2;
    UErrorCode errorCode;

    errorCode=U_ZERO_ERROR;
    cnv1=ucnv_open("UTF-8", &errorCode);
    if(U_FAILURE(errorCode)) {
        log_err("unable to open a UTF-8 converter - %s\n", u_errorName(errorCode));
        return;
    }

    cnv2=ucnv_open("Shift-JIS", &errorCode);
    if(U_FAILURE(errorCode)) {
        log_data_err("unable to open a Shift-JIS converter - %s\n", u_errorName(errorCode));
        ucnv_close(cnv1);
        return;
    }

    /* test ucnv_convertEx() with streaming conversion style */
    convertExMultiStreaming(cnv1, cnv2,
        (const char *)utf8, sizeof(utf8), (const char *)shiftJIS, sizeof(shiftJIS),
        "UTF-8 -> Shift-JIS", U_ZERO_ERROR);

    convertExMultiStreaming(cnv2, cnv1,
        (const char *)shiftJIS, sizeof(shiftJIS), (const char *)utf8, sizeof(utf8),
        "Shift-JIS -> UTF-8", U_ZERO_ERROR);

    /* U_ZERO_ERROR because by default the SUB callbacks are set */
    convertExMultiStreaming(cnv1, cnv2,
        (const char *)shiftJIS, sizeof(shiftJIS), (const char *)errorTarget, sizeof(errorTarget),
        "shiftJIS[] UTF-8 -> Shift-JIS", U_ZERO_ERROR);

    /* test some simple conversions */

    /* NUL-terminated source and target */
    errorCode=U_STRING_NOT_TERMINATED_WARNING;
    memcpy(srcBuffer, utf8, sizeof(utf8));
    srcBuffer[sizeof(utf8)]=0;
    src=srcBuffer;
    target=targetBuffer;
    ucnv_convertEx(cnv2, cnv1, &target, targetBuffer+sizeof(targetBuffer), &src, NULL,
                   NULL, NULL, NULL, NULL, TRUE, TRUE, &errorCode);
    if( errorCode!=U_ZERO_ERROR ||
        target-targetBuffer!=sizeof(shiftJIS) ||
        *target!=0 ||
        memcmp(targetBuffer, shiftJIS, sizeof(shiftJIS))!=0
    ) {
        log_err("ucnv_convertEx(simple UTF-8 -> Shift_JIS) fails: %s - writes %d bytes, expect %d\n",
                u_errorName(errorCode), target-targetBuffer, sizeof(shiftJIS));
    }

    /* NUL-terminated source and U_STRING_NOT_TERMINATED_WARNING */
    errorCode=U_AMBIGUOUS_ALIAS_WARNING;
    memset(targetBuffer, 0xff, sizeof(targetBuffer));
    src=srcBuffer;
    target=targetBuffer;
    ucnv_convertEx(cnv2, cnv1, &target, targetBuffer+sizeof(shiftJIS), &src, NULL,
                   NULL, NULL, NULL, NULL, TRUE, TRUE, &errorCode);
    if( errorCode!=U_STRING_NOT_TERMINATED_WARNING ||
        target-targetBuffer!=sizeof(shiftJIS) ||
        *target!=(char)0xff ||
        memcmp(targetBuffer, shiftJIS, sizeof(shiftJIS))!=0
    ) {
        log_err("ucnv_convertEx(simple UTF-8 -> Shift_JIS) fails: %s, expect U_STRING_NOT_TERMINATED_WARNING - writes %d bytes, expect %d\n",
                u_errorName(errorCode), target-targetBuffer, sizeof(shiftJIS));
    }

    /* bad arguments */
    errorCode=U_MESSAGE_PARSE_ERROR;
    src=srcBuffer;
    target=targetBuffer;
    ucnv_convertEx(cnv2, cnv1, &target, targetBuffer+sizeof(targetBuffer), &src, NULL,
                   NULL, NULL, NULL, NULL, TRUE, TRUE, &errorCode);
    if(errorCode!=U_MESSAGE_PARSE_ERROR) {
        log_err("ucnv_convertEx(U_MESSAGE_PARSE_ERROR) sets %s\n", u_errorName(errorCode));
    }

    /* pivotLimit==pivotStart */
    errorCode=U_ZERO_ERROR;
    pivotSource=pivotTarget=pivotBuffer;
    ucnv_convertEx(cnv2, cnv1, &target, targetBuffer+sizeof(targetBuffer), &src, NULL,
                   pivotBuffer, &pivotSource, &pivotTarget, pivotBuffer, TRUE, TRUE, &errorCode);
    if(errorCode!=U_ILLEGAL_ARGUMENT_ERROR) {
        log_err("ucnv_convertEx(pivotLimit==pivotStart) sets %s\n", u_errorName(errorCode));
    }

    /* *pivotSource==NULL */
    errorCode=U_ZERO_ERROR;
    pivotSource=NULL;
    ucnv_convertEx(cnv2, cnv1, &target, targetBuffer+sizeof(targetBuffer), &src, NULL,
                   pivotBuffer, &pivotSource, &pivotTarget, pivotBuffer+1, TRUE, TRUE, &errorCode);
    if(errorCode!=U_ILLEGAL_ARGUMENT_ERROR) {
        log_err("ucnv_convertEx(*pivotSource==NULL) sets %s\n", u_errorName(errorCode));
    }

    /* *source==NULL */
    errorCode=U_ZERO_ERROR;
    src=NULL;
    pivotSource=pivotBuffer;
    ucnv_convertEx(cnv2, cnv1, &target, targetBuffer+sizeof(targetBuffer), &src, NULL,
                   pivotBuffer, &pivotSource, &pivotTarget, pivotBuffer+1, TRUE, TRUE, &errorCode);
    if(errorCode!=U_ILLEGAL_ARGUMENT_ERROR) {
        log_err("ucnv_convertEx(*source==NULL) sets %s\n", u_errorName(errorCode));
    }

    ucnv_close(cnv1);
    ucnv_close(cnv2);
}

static void
TestConvertAlgorithmic() {
    static const uint8_t
    utf8[]={
        /* 4e00           30a1              ff61              0410 */
        0xe4, 0xb8, 0x80, 0xe3, 0x82, 0xa1, 0xef, 0xbd, 0xa1, 0xd0, 0x90
    },
    shiftJIS[]={
        0x88, 0xea, 0x83, 0x40, 0xa1, 0x84, 0x40
    },
  /*errorTarget[]={*/
        /*
         * expected output when converting shiftJIS[] from UTF-8 to Shift-JIS:
         * SUB, SUB, 0x40, SUB, SUB, 0x40
         */
  /* 0x81, 0xa1, 0x81, 0xa1, 0x40, 0x81, 0xa1, 0x81, 0xa1, 0x40*/
  /*},*/
    utf16[]={
        0xfe, 0xff /* BOM only, no text */
    },
    utf32[]={
        0xff, 0xfe, 0, 0 /* BOM only, no text */
    };

    char target[100], utf8NUL[100], shiftJISNUL[100];

    UConverter *cnv;
    UErrorCode errorCode;

    int32_t length;

    errorCode=U_ZERO_ERROR;
    cnv=ucnv_open("Shift-JIS", &errorCode);
    if(U_FAILURE(errorCode)) {
        log_data_err("unable to open a Shift-JIS converter - %s\n", u_errorName(errorCode));
        ucnv_close(cnv);
        return;
    }

    memcpy(utf8NUL, utf8, sizeof(utf8));
    utf8NUL[sizeof(utf8)]=0;
    memcpy(shiftJISNUL, shiftJIS, sizeof(shiftJIS));
    shiftJISNUL[sizeof(shiftJIS)]=0;

    /*
     * The to/from algorithmic convenience functions share a common implementation,
     * so we need not test all permutations of them.
     */

    /* length in, not terminated out */
    errorCode=U_ZERO_ERROR;
    length=ucnv_fromAlgorithmic(cnv, UCNV_UTF8, target, sizeof(shiftJIS), (const char *)utf8, sizeof(utf8), &errorCode);
    if( errorCode!=U_STRING_NOT_TERMINATED_WARNING ||
        length!=sizeof(shiftJIS) ||
        memcmp(target, shiftJIS, length)!=0
    ) {
        log_err("ucnv_fromAlgorithmic(UTF-8 -> Shift-JIS) fails (%s expect U_STRING_NOT_TERMINATED_WARNING), returns %d expect %d\n",
                u_errorName(errorCode), length, sizeof(shiftJIS));
    }

    /* terminated in and out */
    memset(target, 0x55, sizeof(target));
    errorCode=U_STRING_NOT_TERMINATED_WARNING;
    length=ucnv_toAlgorithmic(UCNV_UTF8, cnv, target, sizeof(target), shiftJISNUL, -1, &errorCode);
    if( errorCode!=U_ZERO_ERROR ||
        length!=sizeof(utf8) ||
        memcmp(target, utf8, length)!=0
    ) {
        log_err("ucnv_toAlgorithmic(Shift-JIS -> UTF-8) fails (%s expect U_ZERO_ERROR), returns %d expect %d\n",
                u_errorName(errorCode), length, sizeof(shiftJIS));
    }

    /* empty string, some target buffer */
    errorCode=U_STRING_NOT_TERMINATED_WARNING;
    length=ucnv_toAlgorithmic(UCNV_UTF8, cnv, target, sizeof(target), shiftJISNUL, 0, &errorCode);
    if( errorCode!=U_ZERO_ERROR ||
        length!=0
    ) {
        log_err("ucnv_toAlgorithmic(empty string -> UTF-8) fails (%s expect U_ZERO_ERROR), returns %d expect 0\n",
                u_errorName(errorCode), length);
    }

    /* pseudo-empty string, no target buffer */
    errorCode=U_ZERO_ERROR;
    length=ucnv_fromAlgorithmic(cnv, UCNV_UTF16, target, 0, (const char *)utf16, 2, &errorCode);
    if( errorCode!=U_STRING_NOT_TERMINATED_WARNING ||
        length!=0
    ) {
        log_err("ucnv_fromAlgorithmic(UTF-16 only BOM -> Shift-JIS) fails (%s expect U_STRING_NOT_TERMINATED_WARNING), returns %d expect 0\n",
                u_errorName(errorCode), length);
    }

    errorCode=U_ZERO_ERROR;
    length=ucnv_fromAlgorithmic(cnv, UCNV_UTF32, target, 0, (const char *)utf32, 4, &errorCode);
    if( errorCode!=U_STRING_NOT_TERMINATED_WARNING ||
        length!=0
    ) {
        log_err("ucnv_fromAlgorithmic(UTF-32 only BOM -> Shift-JIS) fails (%s expect U_STRING_NOT_TERMINATED_WARNING), returns %d expect 0\n",
                u_errorName(errorCode), length);
    }

    /* bad arguments */
    errorCode=U_MESSAGE_PARSE_ERROR;
    length=ucnv_fromAlgorithmic(cnv, UCNV_UTF16, target, 0, (const char *)utf16, 2, &errorCode);
    if(errorCode!=U_MESSAGE_PARSE_ERROR) {
        log_err("ucnv_fromAlgorithmic(U_MESSAGE_PARSE_ERROR) sets %s\n", u_errorName(errorCode));
    }

    /* source==NULL */
    errorCode=U_ZERO_ERROR;
    length=ucnv_fromAlgorithmic(cnv, UCNV_UTF16, target, 0, NULL, 2, &errorCode);
    if(errorCode!=U_ILLEGAL_ARGUMENT_ERROR) {
        log_err("ucnv_fromAlgorithmic(source==NULL) sets %s\n", u_errorName(errorCode));
    }

    /* illegal alg. type */
    errorCode=U_ZERO_ERROR;
    length=ucnv_fromAlgorithmic(cnv, (UConverterType)99, target, 0, (const char *)utf16, 2, &errorCode);
    if(errorCode!=U_ILLEGAL_ARGUMENT_ERROR) {
        log_err("ucnv_fromAlgorithmic(illegal alg. type) sets %s\n", u_errorName(errorCode));
    }
ucnv_close(cnv);
}

static void TestLMBCSMaxChar(void) {
    static const struct {
        int8_t maxSize;
        const char *name;
    } converter[] = {
        /* some non-LMBCS converters - perfect test setup here */
        { 1, "US-ASCII"},
        { 1, "ISO-8859-1"},

        { 2, "UTF-16"},
        { 2, "UTF-16BE"},
        { 3, "UTF-8"},
        { 3, "CESU-8"},
        { 3, "SCSU"},
        { 4, "UTF-32"},
        { 4, "UTF-7"},
        { 4, "IMAP-mailbox-name"},
        { 4, "BOCU-1"},

        { 1, "windows-1256"},
        { 2, "Shift-JIS"},
        { 2, "ibm-16684"},
        { 3, "ibm-930"},
        { 3, "ibm-1390"},
        { 4, "*test3"},
        { 16,"*test4"},

        { 4, "ISCII"},
        { 4, "HZ"},

        { 3, "ISO-2022"},
        { 3, "ISO-2022-KR"},
        { 6, "ISO-2022-JP"},
        { 8, "ISO-2022-CN"},

        /* LMBCS */
        { 3, "LMBCS-1"},
        { 3, "LMBCS-2"},
        { 3, "LMBCS-3"},
        { 3, "LMBCS-4"},
        { 3, "LMBCS-5"},
        { 3, "LMBCS-6"},
        { 3, "LMBCS-8"},
        { 3, "LMBCS-11"},
        { 3, "LMBCS-16"},
        { 3, "LMBCS-17"},
        { 3, "LMBCS-18"},
        { 3, "LMBCS-19"}
    };
    int32_t idx;

    for (idx = 0; idx < LENGTHOF(converter); idx++) {
        UErrorCode status = U_ZERO_ERROR;
        UConverter *cnv = cnv_open(converter[idx].name, &status);
        if (U_FAILURE(status)) {
            continue;
        }
        if (converter[idx].maxSize != ucnv_getMaxCharSize(cnv)) {
            log_err("error: ucnv_getMaxCharSize(%s) expected %d, got %d\n",
                converter[idx].name, converter[idx].maxSize, ucnv_getMaxCharSize(cnv));
        }
        ucnv_close(cnv);
    }

    /* mostly test that the macro compiles */
    if(UCNV_GET_MAX_BYTES_FOR_STRING(1, 2)<10) {
        log_err("error UCNV_GET_MAX_BYTES_FOR_STRING(1, 2)<10\n");
    }
}


static void TestJ1968(void) {
    UErrorCode err = U_ZERO_ERROR;
    UConverter *cnv;
    char myConvName[] = "My really really really really really really really really really really really"
                          " really really really really really really really really really really really"
                          " really really really really really really really really long converter name";
    UChar myConvNameU[sizeof(myConvName)];

    u_charsToUChars(myConvName, myConvNameU, sizeof(myConvName));

    err = U_ZERO_ERROR;
    myConvNameU[UCNV_MAX_CONVERTER_NAME_LENGTH+1] = 0;
    cnv = ucnv_openU(myConvNameU, &err);
    if (cnv || err != U_ILLEGAL_ARGUMENT_ERROR) {
        log_err("1U) Didn't get U_ILLEGAL_ARGUMENT_ERROR as expected %s\n", u_errorName(err));
    }

    err = U_ZERO_ERROR;
    myConvNameU[UCNV_MAX_CONVERTER_NAME_LENGTH] = 0;
    cnv = ucnv_openU(myConvNameU, &err);
    if (cnv || err != U_ILLEGAL_ARGUMENT_ERROR) {
        log_err("2U) Didn't get U_ILLEGAL_ARGUMENT_ERROR as expected %s\n", u_errorName(err));
    }

    err = U_ZERO_ERROR;
    myConvNameU[UCNV_MAX_CONVERTER_NAME_LENGTH-1] = 0;
    cnv = ucnv_openU(myConvNameU, &err);
    if (cnv || err != U_FILE_ACCESS_ERROR) {
        log_err("3U) Didn't get U_FILE_ACCESS_ERROR as expected %s\n", u_errorName(err));
    }


    
    
    err = U_ZERO_ERROR;
    cnv = ucnv_open(myConvName, &err);
    if (cnv || err != U_ILLEGAL_ARGUMENT_ERROR) {
        log_err("1) Didn't get U_ILLEGAL_ARGUMENT_ERROR as expected %s\n", u_errorName(err));
    }

    err = U_ZERO_ERROR;
    myConvName[UCNV_MAX_CONVERTER_NAME_LENGTH] = ',';
    cnv = ucnv_open(myConvName, &err);
    if (cnv || err != U_ILLEGAL_ARGUMENT_ERROR) {
        log_err("2) Didn't get U_ILLEGAL_ARGUMENT_ERROR as expected %s\n", u_errorName(err));
    }

    err = U_ZERO_ERROR;
    myConvName[UCNV_MAX_CONVERTER_NAME_LENGTH-1] = ',';
    cnv = ucnv_open(myConvName, &err);
    if (cnv || err != U_FILE_ACCESS_ERROR) {
        log_err("3) Didn't get U_FILE_ACCESS_ERROR as expected %s\n", u_errorName(err));
    }

    err = U_ZERO_ERROR;
    myConvName[UCNV_MAX_CONVERTER_NAME_LENGTH-1] = ',';
    strncpy(myConvName + UCNV_MAX_CONVERTER_NAME_LENGTH, "locale=", 7);
    cnv = ucnv_open(myConvName, &err);
    if (cnv || err != U_ILLEGAL_ARGUMENT_ERROR) {
        log_err("4) Didn't get U_ILLEGAL_ARGUMENT_ERROR as expected %s\n", u_errorName(err));
    }

    /* The comma isn't really a part of the converter name. */
    err = U_ZERO_ERROR;
    myConvName[UCNV_MAX_CONVERTER_NAME_LENGTH] = 0;
    cnv = ucnv_open(myConvName, &err);
    if (cnv || err != U_FILE_ACCESS_ERROR) {
        log_err("5) Didn't get U_FILE_ACCESS_ERROR as expected %s\n", u_errorName(err));
    }

    err = U_ZERO_ERROR;
    myConvName[UCNV_MAX_CONVERTER_NAME_LENGTH-1] = ' ';
    cnv = ucnv_open(myConvName, &err);
    if (cnv || err != U_ILLEGAL_ARGUMENT_ERROR) {
        log_err("6) Didn't get U_ILLEGAL_ARGUMENT_ERROR as expected %s\n", u_errorName(err));
    }

    err = U_ZERO_ERROR;
    myConvName[UCNV_MAX_CONVERTER_NAME_LENGTH-1] = 0;
    cnv = ucnv_open(myConvName, &err);
    if (cnv || err != U_FILE_ACCESS_ERROR) {
        log_err("7) Didn't get U_FILE_ACCESS_ERROR as expected %s\n", u_errorName(err));
    }

}

static void
testSwap(const char *name, UBool swap) {
    /*
     * Test Unicode text.
     * Contains characters that are the highest for some of the
     * tested conversions, to make sure that the ucnvmbcs.c code that modifies the
     * tables copies the entire tables.
     */
    static const UChar text[]={
        0x61, 0xd, 0x62, 0xa, 0x4e00, 0x3000, 0xfffd, 0xa, 0x20, 0x85, 0xff5e, 0x7a
    };

    UChar uNormal[32], uSwapped[32];
    char normal[32], swapped[32];
    const UChar *pcu;
    UChar *pu;
    char *pc;
    int32_t i, normalLength, swappedLength;
    UChar u;
    char c;

    const char *swappedName;
    UConverter *cnv, *swapCnv;
    UErrorCode errorCode;

    /* if the swap flag is FALSE, then the test encoding is not EBCDIC and must not swap */

    /* open both the normal and the LF/NL-swapping converters */
    strcpy(swapped, name);
    strcat(swapped, UCNV_SWAP_LFNL_OPTION_STRING);

    errorCode=U_ZERO_ERROR;
    swapCnv=ucnv_open(swapped, &errorCode);
    cnv=ucnv_open(name, &errorCode);
    if(U_FAILURE(errorCode)) {
        log_data_err("TestEBCDICSwapLFNL error: unable to open %s or %s (%s)\n", name, swapped, u_errorName(errorCode));
        goto cleanup;
    }

    /* the name must contain the swap option if and only if we expect the converter to swap */
    swappedName=ucnv_getName(swapCnv, &errorCode);
    if(U_FAILURE(errorCode)) {
        log_err("TestEBCDICSwapLFNL error: ucnv_getName(%s,swaplfnl) failed (%s)\n", name, u_errorName(errorCode));
        goto cleanup;
    }

    pc=strstr(swappedName, UCNV_SWAP_LFNL_OPTION_STRING);
    if(swap != (pc!=NULL)) {
        log_err("TestEBCDICSwapLFNL error: ucnv_getName(%s,swaplfnl)=%s should (%d) contain 'swaplfnl'\n", name, swappedName, swap);
        goto cleanup;
    }

    /* convert to EBCDIC */
    pcu=text;
    pc=normal;
    ucnv_fromUnicode(cnv, &pc, normal+LENGTHOF(normal), &pcu, text+LENGTHOF(text), NULL, TRUE, &errorCode);
    normalLength=(int32_t)(pc-normal);

    pcu=text;
    pc=swapped;
    ucnv_fromUnicode(swapCnv, &pc, swapped+LENGTHOF(swapped), &pcu, text+LENGTHOF(text), NULL, TRUE, &errorCode);
    swappedLength=(int32_t)(pc-swapped);

    if(U_FAILURE(errorCode)) {
        log_err("TestEBCDICSwapLFNL error converting to %s - (%s)\n", name, u_errorName(errorCode));
        goto cleanup;
    }

    /* compare EBCDIC output */
    if(normalLength!=swappedLength) {
        log_err("TestEBCDICSwapLFNL error converting to %s - output lengths %d vs. %d\n", name, normalLength, swappedLength);
        goto cleanup;
    }
    for(i=0; i<normalLength; ++i) {
        /* swap EBCDIC LF/NL for comparison */
        c=normal[i];
        if(swap) {
            if(c==0x15) {
                c=0x25;
            } else if(c==0x25) {
                c=0x15;
            }
        }

        if(c!=swapped[i]) {
            log_err("TestEBCDICSwapLFNL error converting to %s - did not swap properly, output[%d]=0x%02x\n", name, i, (uint8_t)swapped[i]);
            goto cleanup;
        }
    }

    /* convert back to Unicode (may not roundtrip) */
    pc=normal;
    pu=uNormal;
    ucnv_toUnicode(cnv, &pu, uNormal+LENGTHOF(uNormal), (const char **)&pc, normal+normalLength, NULL, TRUE, &errorCode);
    normalLength=(int32_t)(pu-uNormal);

    pc=normal;
    pu=uSwapped;
    ucnv_toUnicode(swapCnv, &pu, uSwapped+LENGTHOF(uSwapped), (const char **)&pc, normal+swappedLength, NULL, TRUE, &errorCode);
    swappedLength=(int32_t)(pu-uSwapped);

    if(U_FAILURE(errorCode)) {
        log_err("TestEBCDICSwapLFNL error converting from %s - (%s)\n", name, u_errorName(errorCode));
        goto cleanup;
    }

    /* compare EBCDIC output */
    if(normalLength!=swappedLength) {
        log_err("TestEBCDICSwapLFNL error converting from %s - output lengths %d vs. %d\n", name, normalLength, swappedLength);
        goto cleanup;
    }
    for(i=0; i<normalLength; ++i) {
        /* swap EBCDIC LF/NL for comparison */
        u=uNormal[i];
        if(swap) {
            if(u==0xa) {
                u=0x85;
            } else if(u==0x85) {
                u=0xa;
            }
        }

        if(u!=uSwapped[i]) {
            log_err("TestEBCDICSwapLFNL error converting from %s - did not swap properly, output[%d]=U+%04x\n", name, i, uSwapped[i]);
            goto cleanup;
        }
    }

    /* clean up */
cleanup:
    ucnv_close(cnv);
    ucnv_close(swapCnv);
}

static void
TestEBCDICSwapLFNL() {
    static const struct {
        const char *name;
        UBool swap;
    } tests[]={
        { "ibm-37", TRUE },
        { "ibm-1047", TRUE },
        { "ibm-1140", TRUE },
        { "ibm-930", TRUE },
        { "iso-8859-3", FALSE }
    };

    int i;

    for(i=0; i<LENGTHOF(tests); ++i) {
        testSwap(tests[i].name, tests[i].swap);
    }
}
