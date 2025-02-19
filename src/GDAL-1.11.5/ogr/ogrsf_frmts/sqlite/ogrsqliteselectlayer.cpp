/******************************************************************************
 * $Id: ogrsqliteselectlayer.cpp 30273 2015-09-11 07:59:52Z rouault $
 *
 * Project:  OpenGIS Simple Features Reference Implementation
 * Purpose:  Implements OGRSQLiteSelectLayer class, layer access to the results
 *           of a SELECT statement executed via ExecuteSQL().
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2004, Frank Warmerdam
 * Copyright (c) 2010-2014, Even Rouault <even dot rouault at mines-paris dot org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

#include "cpl_conv.h"
#include "ogr_sqlite.h"
#include "swq.h"
#include "ogr_p.h"

CPL_CVSID("$Id: ogrsqliteselectlayer.cpp 30273 2015-09-11 07:59:52Z rouault $");
/************************************************************************/
/*                        OGRSQLiteSelectLayer()                        */
/************************************************************************/

OGRSQLiteSelectLayer::OGRSQLiteSelectLayer( OGRSQLiteDataSource *poDSIn,
                                            CPLString osSQLIn,
                                            sqlite3_stmt *hStmtIn,
                                            int bUseStatementForGetNextFeature,
                                            int bEmptyLayer,
                                            int bAllowMultipleGeomFields )

{
    poDS = poDSIn;

    iNextShapeId = 0;
    poFeatureDefn = NULL;
    bAllowResetReadingEvenIfIndexAtZero = FALSE;
    this->bAllowMultipleGeomFields = bAllowMultipleGeomFields;

    std::set<CPLString> aosEmpty;
    BuildFeatureDefn( "SELECT", hStmtIn, NULL, aosEmpty );

    if( bUseStatementForGetNextFeature )
    {
        hStmt = hStmtIn;
        bDoStep = FALSE;

        // Try to extract SRS from first geometry
        for( int iField = 0;
             !bEmptyLayer && iField < poFeatureDefn->GetGeomFieldCount();
             iField ++)
        {
            OGRSQLiteGeomFieldDefn* poGeomFieldDefn =
                poFeatureDefn->myGetGeomFieldDefn(iField);
            if( wkbFlatten(poGeomFieldDefn->GetType()) != wkbUnknown )
                continue;

            int nBytes;
            if( sqlite3_column_type( hStmt, poGeomFieldDefn->iCol ) == SQLITE_BLOB &&
                (nBytes = sqlite3_column_bytes( hStmt, poGeomFieldDefn->iCol )) > 39 )
            {
                const GByte* pabyBlob = (const GByte*)sqlite3_column_blob( hStmt, poGeomFieldDefn->iCol );
                int eByteOrder = pabyBlob[1];
                if( pabyBlob[0] == 0x00 &&
                    (eByteOrder == wkbNDR || eByteOrder == wkbXDR) &&
                    pabyBlob[38] == 0x7C )
                {
                    int nSRSId;
                    memcpy(&nSRSId, pabyBlob + 2, 4);
#ifdef CPL_LSB
                    if( eByteOrder != wkbNDR)
                        CPL_SWAP32PTR(&nSRSId);
#else
                    if( eByteOrder == wkbNDR)
                        CPL_SWAP32PTR(&nSRSId);
#endif
                    CPLPushErrorHandler(CPLQuietErrorHandler);
                    OGRSpatialReference* poSRS = poDS->FetchSRS( nSRSId );
                    CPLPopErrorHandler();
                    if( poSRS != NULL )
                    {
                        poGeomFieldDefn->nSRSId = nSRSId;
                        poGeomFieldDefn->SetSpatialRef(poSRS);
                    }
                    else
                        CPLErrorReset();
                }
#ifdef SQLITE_HAS_COLUMN_METADATA
                else if( iField == 0 )
                {
                    const char* pszTableName = sqlite3_column_table_name( hStmt, poGeomFieldDefn->iCol );
                    if( pszTableName != NULL )
                    {
                        OGRSQLiteLayer* poLayer = (OGRSQLiteLayer*)
                                        poDS->GetLayerByName(pszTableName);
                        if( poLayer != NULL &&  poLayer->GetLayerDefn()->GetGeomFieldCount() > 0)
                        {
                            OGRSQLiteGeomFieldDefn* poSrcGFldDefn =
                                poLayer->myGetLayerDefn()->myGetGeomFieldDefn(0);
                            poGeomFieldDefn->nSRSId = poSrcGFldDefn->nSRSId;
                            poGeomFieldDefn->SetSpatialRef(poSrcGFldDefn->GetSpatialRef());
                        }
                    }
                }
#endif
            }
        }
    }
    else
        sqlite3_finalize( hStmtIn );

    osSQLBase = osSQLIn;
    osSQLCurrent = osSQLIn;
    this->bEmptyLayer = bEmptyLayer;
    bSpatialFilterInSQL = TRUE;
}

/************************************************************************/
/*                            ResetReading()                            */
/************************************************************************/

void OGRSQLiteSelectLayer::ResetReading()

{
    if( iNextShapeId > 0 || bAllowResetReadingEvenIfIndexAtZero )
    {
        OGRSQLiteLayer::ResetReading();
        bAllowResetReadingEvenIfIndexAtZero = FALSE;
    }
}

/************************************************************************/
/*                           GetNextFeature()                           */
/************************************************************************/

OGRFeature *OGRSQLiteSelectLayer::GetNextFeature()
{
    if( bEmptyLayer )
        return NULL;

    return OGRSQLiteLayer::GetNextFeature();
}

/************************************************************************/
/*               OGRGenSQLResultsLayerHasSpecialField()                 */
/************************************************************************/

static
int HasSpecialFields(swq_expr_node* expr, int nMinIndexForSpecialField)
{
    if (expr->eNodeType == SNT_COLUMN)
    {
        if (expr->table_index == 0)
        {
            return expr->field_index >= nMinIndexForSpecialField &&
                   expr->field_index < nMinIndexForSpecialField + SPECIAL_FIELD_COUNT;
        }
    }
    else if (expr->eNodeType == SNT_OPERATION)
    {
        for( int i = 0; i < expr->nSubExprCount; i++ )
        {
            if (HasSpecialFields(expr->papoSubExpr[i], nMinIndexForSpecialField))
                return TRUE;
        }
    }
    return FALSE;
}

/************************************************************************/
/*                         SetAttributeFilter()                         */
/************************************************************************/

OGRErr OGRSQLiteSelectLayer::SetAttributeFilter( const char *pszQuery )

{
    if( m_pszAttrQueryString == NULL && pszQuery == NULL )
        return OGRERR_NONE;

    CPLFree(m_pszAttrQueryString);
    m_pszAttrQueryString = (pszQuery) ? CPLStrdup(pszQuery) : NULL;

    bAllowResetReadingEvenIfIndexAtZero = TRUE;

    OGRFeatureQuery oQuery;

    CPLPushErrorHandler(CPLQuietErrorHandler);
    int bHasSpecialFields = (pszQuery != NULL && pszQuery[0] != '\0' &&
        oQuery.Compile( GetLayerDefn(), pszQuery ) == OGRERR_NONE &&
        HasSpecialFields((swq_expr_node*)oQuery.GetSWGExpr(), GetLayerDefn()->GetFieldCount()) );
    CPLPopErrorHandler();

    if( bHasSpecialFields || !BuildSQL() )
    {
        return OGRSQLiteLayer::SetAttributeFilter(pszQuery);
    }

    ResetReading();

    return OGRERR_NONE;
}

/************************************************************************/
/*                           GetNextFeature()                           */
/************************************************************************/

int OGRSQLiteSelectLayer::GetFeatureCount( int bForce )
{
    if( bEmptyLayer )
        return 0;

    if( m_poAttrQuery == NULL &&
        EQUALN(osSQLCurrent, "SELECT COUNT(*) FROM", strlen("SELECT COUNT(*) FROM")) &&
        osSQLCurrent.ifind(" GROUP BY ") == std::string::npos &&
        osSQLCurrent.ifind(" UNION ") == std::string::npos &&
        osSQLCurrent.ifind(" INTERSECT ") == std::string::npos &&
        osSQLCurrent.ifind(" EXCEPT ") == std::string::npos )
        return 1;

    if( m_poAttrQuery != NULL || (m_poFilterGeom != NULL && !bSpatialFilterInSQL) )
        return OGRLayer::GetFeatureCount(bForce);

    CPLString osFeatureCountSQL("SELECT COUNT(*) FROM (");
    osFeatureCountSQL += osSQLCurrent;
    osFeatureCountSQL += ")";

    CPLDebug("SQLITE", "Running %s", osFeatureCountSQL.c_str());

/* -------------------------------------------------------------------- */
/*      Execute.                                                        */
/* -------------------------------------------------------------------- */
    char *pszErrMsg = NULL;
    char **papszResult;
    int nRowCount, nColCount;
    int nResult = -1;

    if( sqlite3_get_table( poDS->GetDB(), osFeatureCountSQL, &papszResult, 
                           &nRowCount, &nColCount, &pszErrMsg ) != SQLITE_OK )
    {
        CPLDebug("SQLITE", "Error: %s", pszErrMsg);
        sqlite3_free(pszErrMsg);
        return OGRLayer::GetFeatureCount(bForce);
    }

    if( nRowCount == 1 && nColCount == 1 )
    {
        nResult = atoi(papszResult[1]);
    }

    sqlite3_free_table( papszResult );

    return nResult;
}

/************************************************************************/
/*                           ResetStatement()                           */
/************************************************************************/

OGRErr OGRSQLiteSelectLayer::ResetStatement()

{
    int rc;

    ClearStatement();

    iNextShapeId = 0;
    bDoStep = TRUE;

#ifdef DEBUG
    CPLDebug( "OGR_SQLITE", "prepare(%s)", osSQLCurrent.c_str() );
#endif

    rc = sqlite3_prepare( poDS->GetDB(), osSQLCurrent, osSQLCurrent.size(),
                          &hStmt, NULL );

    if( rc == SQLITE_OK )
    {
        return OGRERR_NONE;
    }
    else
    {
        CPLError( CE_Failure, CPLE_AppDefined, 
                  "In ResetStatement(): sqlite3_prepare(%s):\n  %s", 
                  osSQLCurrent.c_str(), sqlite3_errmsg(poDS->GetDB()) );
        hStmt = NULL;
        return OGRERR_FAILURE;
    }
}

/************************************************************************/
/*                          SetSpatialFilter()                          */
/************************************************************************/

void OGRSQLiteSelectLayer::SetSpatialFilter( int iGeomField, OGRGeometry * poGeomIn )

{
    if( iGeomField == 0 && poGeomIn == NULL && GetLayerDefn()->GetGeomFieldCount() == 0 )
    {
        /* do nothing */
    }
    else if( iGeomField < 0 || iGeomField >= GetLayerDefn()->GetGeomFieldCount() )
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                    "Invalid geometry field index : %d", iGeomField);
        return;
    }

    bAllowResetReadingEvenIfIndexAtZero = TRUE;

    m_iGeomFieldFilter = iGeomField;
    if( InstallFilter( poGeomIn ) )
    {
        BuildSQL();

        ResetReading();
    }
}

/************************************************************************/
/*                            GetBaseLayer()                            */
/************************************************************************/

OGRSQLiteLayer* OGRSQLiteSelectLayer::GetBaseLayer(size_t& i)
{
    char** papszTokens = CSLTokenizeString(osSQLBase.c_str());
    int bCanInsertFilter = TRUE;
    int nCountSelect = 0, nCountFrom = 0, nCountWhere = 0;

    for(int iToken = 0; papszTokens[iToken] != NULL; iToken++)
    {
        if (EQUAL(papszTokens[iToken], "SELECT"))
            nCountSelect ++;
        else if (EQUAL(papszTokens[iToken], "FROM"))
            nCountFrom ++;
        else if (EQUAL(papszTokens[iToken], "WHERE"))
            nCountWhere ++;
        else if (EQUAL(papszTokens[iToken], "UNION") ||
                 EQUAL(papszTokens[iToken], "JOIN") ||
                 EQUAL(papszTokens[iToken], "INTERSECT") ||
                 EQUAL(papszTokens[iToken], "EXCEPT"))
        {
            bCanInsertFilter = FALSE;
        }
    }
    CSLDestroy(papszTokens);

    if (!(bCanInsertFilter && nCountSelect == 1 && nCountFrom == 1 && nCountWhere <= 1))
    {
        CPLDebug("SQLITE", "SQL expression too complex to analyse");
        return NULL;
    }

    size_t nFromPos = osSQLBase.ifind(" from ");
    if (nFromPos == std::string::npos)
    {
        return NULL;
    }

    /* Remove potential quotes around layer name */
    char chFirst = osSQLBase[nFromPos + 6];
    int bInQuotes = (chFirst == '\'' || chFirst == '"' );
    CPLString osBaseLayerName;
    for( i = nFromPos + 6 + (bInQuotes ? 1 : 0);
         i < osSQLBase.size(); i++ )
    {
        if (osSQLBase[i] == chFirst && bInQuotes )
        {
            if( i + 1 < osSQLBase.size() &&
                osSQLBase[i + 1] == chFirst )
            {
                osBaseLayerName += osSQLBase[i];
                i++;
            }
            else
            {
                i++;
                break;
            }
        }
        else if (osSQLBase[i] == ' ' && !bInQuotes)
            break;
        else
            osBaseLayerName += osSQLBase[i];
    }

    OGRSQLiteLayer* poUnderlyingLayer = NULL;
    if( strchr(osBaseLayerName, '(') == NULL &&
        poFeatureDefn->GetGeomFieldCount() != 0 )
    {
        CPLString osNewUnderlyingTableName;
        osNewUnderlyingTableName.Printf("%s(%s)",
                                        osBaseLayerName.c_str(),
                                        poFeatureDefn->GetGeomFieldDefn(0)->GetNameRef());
        poUnderlyingLayer =
            (OGRSQLiteLayer*) poDS->GetLayerByName(osNewUnderlyingTableName);
    }
    if( poUnderlyingLayer == NULL )
        poUnderlyingLayer = (OGRSQLiteLayer*) poDS->GetLayerByName(osBaseLayerName);

    if( poUnderlyingLayer != NULL && GetSpatialRef() != NULL &&
        poUnderlyingLayer->GetSpatialRef() != NULL &&
        GetSpatialRef() != poUnderlyingLayer->GetSpatialRef() &&
        !GetSpatialRef()->IsSame(poUnderlyingLayer->GetSpatialRef()) )
    {
        CPLDebug("SQLITE", "Result layer and base layer don't have the same SRS.");
        return NULL;
    }

    return poUnderlyingLayer;
}

/************************************************************************/
/*                             BuildSQL()                               */
/************************************************************************/

int OGRSQLiteSelectLayer::BuildSQL()

{
    osSQLCurrent = osSQLBase;
    bSpatialFilterInSQL = TRUE;

    size_t i = 0;
    OGRSQLiteLayer* poBaseLayer = GetBaseLayer(i);
    if (poBaseLayer == NULL)
    {
        CPLDebug("SQLITE", "Cannot find base layer");
        bSpatialFilterInSQL = FALSE;
        return FALSE;
    }

    CPLString osSpatialWhere;
    if (m_poFilterGeom != NULL)
    {
        const char* pszGeomCol =
            poFeatureDefn->GetGeomFieldDefn(m_iGeomFieldFilter)->GetNameRef();
        int nIdx = poBaseLayer->GetLayerDefn()->GetGeomFieldIndex(pszGeomCol);
        if( nIdx < 0 )
        {
            CPLDebug("SQLITE", "Cannot find field %s in base layer", pszGeomCol);
            bSpatialFilterInSQL = FALSE;
        }
        else
        {
            osSpatialWhere = poBaseLayer->GetSpatialWhere(nIdx, m_poFilterGeom);
            if (osSpatialWhere.size() == 0)
            {
                CPLDebug("SQLITE", "Cannot get spatial where clause");
                bSpatialFilterInSQL = FALSE;
            }
        }
    }

    CPLString osCustomWhere;
    if( osSpatialWhere.size() != 0 )
    {
        osCustomWhere = osSpatialWhere;
    }
    if( m_pszAttrQueryString != NULL && m_pszAttrQueryString[0] != '\0' )
    {
        if( osSpatialWhere.size() != 0)
            osCustomWhere += " AND (";
        osCustomWhere += m_pszAttrQueryString;
        if( osSpatialWhere.size() != 0)
            osCustomWhere += ")";
    }

    /* Nothing to do */
    if( osCustomWhere.size() == 0 )
        return TRUE;

    while (i < osSQLBase.size() && osSQLBase[i] == ' ')
        i ++;

    if (i < osSQLBase.size() && EQUALN(osSQLBase.c_str() + i, "WHERE ", 6))
    {
        osSQLCurrent = osSQLBase.substr(0, i + 6);
        osSQLCurrent += osCustomWhere;
        osSQLCurrent += " AND (";

        size_t nEndOfWhere = osSQLBase.ifind(" GROUP ");
        if (nEndOfWhere == std::string::npos)
            nEndOfWhere = osSQLBase.ifind(" ORDER ");
        if (nEndOfWhere == std::string::npos)
            nEndOfWhere = osSQLBase.ifind(" LIMIT ");

        if (nEndOfWhere == std::string::npos)
        {
            osSQLCurrent += osSQLBase.substr(i + 6);
            osSQLCurrent += ")";
        }
        else
        {
            osSQLCurrent += osSQLBase.substr(i + 6, nEndOfWhere - (i + 6));
            osSQLCurrent += ")";
            osSQLCurrent += osSQLBase.substr(nEndOfWhere);
        }
    }
    else if (i < osSQLBase.size() &&
             (EQUALN(osSQLBase.c_str() + i, "GROUP ", 6) ||
              EQUALN(osSQLBase.c_str() + i, "ORDER ", 6) ||
              EQUALN(osSQLBase.c_str() + i, "LIMIT ", 6)))
    {
        osSQLCurrent = osSQLBase.substr(0, i);
        osSQLCurrent += " WHERE ";
        osSQLCurrent += osCustomWhere;
        osSQLCurrent += " ";
        osSQLCurrent += osSQLBase.substr(i);
    }
    else if (i == osSQLBase.size())
    {
        osSQLCurrent = osSQLBase.substr(0, i);
        osSQLCurrent += " WHERE ";
        osSQLCurrent += osCustomWhere;
    }
    else
    {
        CPLDebug("SQLITE", "SQL expression too complex for the driver to insert attribute and/or spatial filter in it");
        bSpatialFilterInSQL = FALSE;
        return FALSE;
    }

    return TRUE;
}

/************************************************************************/
/*                           TestCapability()                           */
/************************************************************************/

int OGRSQLiteSelectLayer::TestCapability( const char * pszCap )

{
    if (EQUAL(pszCap,OLCFastSpatialFilter))
    {
        if (osSQLCurrent != osSQLBase)
            return TRUE;

        size_t i = 0;
        OGRSQLiteLayer* poBaseLayer = GetBaseLayer(i);
        if (poBaseLayer == NULL)
        {
            CPLDebug("SQLITE", "Cannot find base layer");
            return FALSE;
        }

        OGRPolygon oFakePoly;
        const char* pszWKT = "POLYGON((0 0,0 1,1 1,1 0,0 0))";
        oFakePoly.importFromWkt((char**) &pszWKT);
        CPLString    osSpatialWhere = poBaseLayer->GetSpatialWhere(0, &oFakePoly);

        return osSpatialWhere.size() != 0;
    }
    else
        return OGRSQLiteLayer::TestCapability( pszCap );
}

/************************************************************************/
/*                             GetExtent()                              */
/************************************************************************/

OGRErr OGRSQLiteSelectLayer::GetExtent(int iGeomField, OGREnvelope *psExtent, int bForce)
{
    if( iGeomField < 0 || iGeomField >= GetLayerDefn()->GetGeomFieldCount() ||
        GetLayerDefn()->GetGeomFieldDefn(iGeomField)->GetType() == wkbNone )
    {
        if( iGeomField != 0 )
        {
            CPLError(CE_Failure, CPLE_AppDefined,
                     "Invalid geometry field index : %d", iGeomField);
        }
        return OGRERR_FAILURE;
    }

    /* Caching of extent by SQL string is interesting to speed-up the */
    /* establishment of the WFS GetCapabilities document for a MapServer mapfile */
    /* which has several layers, only differing by scale rules */
    if( iGeomField == 0 )
    {
        const OGREnvelope* psCachedExtent = poDS->GetEnvelopeFromSQL(osSQLBase);
        if (psCachedExtent)
        {
            memcpy(psExtent, psCachedExtent, sizeof(*psCachedExtent));
            return OGRERR_NONE;
        }
    }

    CPLString osSQLCommand = osSQLBase;

    /* ORDER BY are costly to evaluate and are not necessary to establish */
    /* the layer extent. */
    size_t nOrderByPos = osSQLCommand.ifind(" ORDER BY ");
    if( osSQLCommand.ifind("SELECT ") == 0 &&
        nOrderByPos != std::string::npos &&
        osSQLCommand.ifind(" LIMIT ") == std::string::npos &&
        osSQLCommand.ifind(" UNION ") == std::string::npos &&
        osSQLCommand.ifind(" INTERSECT ") == std::string::npos &&
        osSQLCommand.ifind(" EXCEPT ") == std::string::npos)
    {
        osSQLCommand.resize(nOrderByPos);

        OGRLayer* poTmpLayer = poDS->ExecuteSQL(osSQLCommand.c_str(), NULL, NULL);
        if (poTmpLayer)
        {
            OGRErr eErr = poTmpLayer->GetExtent(iGeomField, psExtent, bForce);
            poDS->ReleaseResultSet(poTmpLayer);
            return eErr;
        }
    }

    OGRErr eErr;
    if( iGeomField == 0 )
        eErr = OGRSQLiteLayer::GetExtent(psExtent, bForce);
    else
        eErr = OGRSQLiteLayer::GetExtent(iGeomField, psExtent, bForce);
    if (iGeomField == 0 && eErr == OGRERR_NONE && poDS->GetUpdate() == FALSE)
        poDS->SetEnvelopeForSQL(osSQLBase, *psExtent);
    return eErr;
}
