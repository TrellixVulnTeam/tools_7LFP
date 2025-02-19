/***************************************************************************
  qgsoracleprovider.cpp  -  QGIS data provider for Oracle layers
                             -------------------
    begin                : August 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsmessageoutput.h"
#include "qgsmessagelog.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorlayerimport.h"
#include "qgslogger.h"

#include "qgsprovidercountcalcevent.h"
#include "qgsproviderextentcalcevent.h"

#include "qgsoracleprovider.h"
#include "qgsoracletablemodel.h"
#include "qgsoraclesourceselect.h"
#include "qgsoracledataitems.h"
#include "qgsoraclefeatureiterator.h"
#include "qgsoracleconnpool.h"

#include <QSqlRecord>
#include <QSqlField>
#include <QMessageBox>

#include "ocispatial/wkbptr.h"

const QString ORACLE_KEY = "oracle";
const QString ORACLE_DESCRIPTION = "Oracle data provider";

QgsOracleProvider::QgsOracleProvider( QString const & uri )
    : QgsVectorDataProvider( uri )
    , mValid( false )
    , mPrimaryKeyType( pktUnknown )
    , mDetectedGeomType( QGis::WKBUnknown )
    , mRequestedGeomType( QGis::WKBUnknown )
    , mHasSpatialIndex( false )
    , mSpatialIndexName( QString::null )
    , mShared( new QgsOracleSharedData )
{
  static int geomMetaType = -1;
  if ( geomMetaType < 0 )
    geomMetaType = qRegisterMetaType<QOCISpatialGeometry>();

  QgsDebugMsg( QString( "URI: %1 " ).arg( uri ) );

  mUri = QgsDataSourceURI( uri );

  // populate members from the uri structure
  mOwnerName = mUri.schema();
  mTableName = mUri.table();
  mGeometryColumn = mUri.geometryColumn();
  mSqlWhereClause = mUri.sql();
  mSrid = mUri.srid().toInt();
  mRequestedGeomType = mUri.wkbType();
  mUseEstimatedMetadata = mUri.useEstimatedMetadata();
  mIncludeGeoAttributes = mUri.hasParam( "includegeoattributes" ) ? mUri.param( "includegeoattributes" ) == "true" : false;

  mConnection = QgsOracleConn::connectDb( mUri );
  if ( !mConnection )
  {
    return;
  }

  if ( mOwnerName.isEmpty() && mTableName.startsWith( "(" ) && mTableName.endsWith( ")" ) )
  {
    mIsQuery = true;
    mQuery = mTableName;
    mTableName = "";
  }
  else
  {
    mIsQuery = false;

    if ( mOwnerName.isEmpty() )
    {
      mOwnerName = mConnection->currentUser();
    }

    if ( !mOwnerName.isEmpty() )
    {
      mQuery += quotedIdentifier( mOwnerName ) + ".";
    }

    if ( !mTableName.isEmpty() )
    {
      mQuery += quotedIdentifier( mTableName );
    }
  }

  QgsDebugMsg( QString( "Connection info is %1" ).arg( mUri.connectionInfo() ) );
  QgsDebugMsg( QString( "Geometry column is: %1" ).arg( mGeometryColumn ) );
  QgsDebugMsg( QString( "Owner is: %1" ).arg( mOwnerName ) );
  QgsDebugMsg( QString( "Table name is: %1" ).arg( mTableName ) );
  QgsDebugMsg( QString( "Query is: %1" ).arg( mQuery ) );
  QgsDebugMsg( QString( "Where clause is: %1" ).arg( mSqlWhereClause ) );
  QgsDebugMsg( QString( "SRID is: %1" ).arg( mSrid ) );

  // no table/query passed, the provider could be used to get tables
  if ( mQuery.isEmpty() )
  {
    return;
  }

  if ( !hasSufficientPermsAndCapabilities() )
  {
    return;
  }

  // get geometry details
  if ( !getGeometryDetails() ) // gets srid, geometry and data type
  {
    // the table is not a geometry table
    mValid = false;
    disconnectDb();
    return;
  }

  mLayerExtent.setMinimal();
  mFeaturesCounted = -1;

  // set the primary key
  if ( !determinePrimaryKey() )
  {
    mValid = false;
    disconnectDb();
    return;
  }

  //fill type names into sets
  mNativeTypes
  // integer types
  << QgsVectorDataProvider::NativeType( tr( "Whole number" ), "number(10,0)", QVariant::Int )
  << QgsVectorDataProvider::NativeType( tr( "Whole big number" ), "number(20,0)", QVariant::LongLong )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (numeric)" ), "number", QVariant::Double, 1, 38, 0, 38 )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (decimal)" ), "double precision", QVariant::Double )

  // floating point
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (real)" ), "binary_float", QVariant::Double )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (double)" ), "binary_double", QVariant::Double )

  // string types
  << QgsVectorDataProvider::NativeType( tr( "Text, fixed length (char)" ), "CHAR", QVariant::String, 1, 255 )
  << QgsVectorDataProvider::NativeType( tr( "Text, limited variable length (varchar2)" ), "VARCHAR2", QVariant::String, 1, 255 )
  << QgsVectorDataProvider::NativeType( tr( "Text, unlimited length (long)" ), "LONG", QVariant::String )

  // date type
  << QgsVectorDataProvider::NativeType( tr( "Date" ), "DATE", QVariant::Date, 38, 38, 0, 0 )
  << QgsVectorDataProvider::NativeType( tr( "Date & Time" ), "TIMESTAMP(6)", QVariant::DateTime, 38, 38, 6, 6 )
  ;

  QString key;
  switch ( mPrimaryKeyType )
  {
    case pktRowId:
      key = "ROWID";
      break;

    case pktInt:
    case pktFidMap:
    {
      Q_ASSERT( mPrimaryKeyType != pktInt || mPrimaryKeyAttrs.size() == 1 );

      QString delim;
      Q_FOREACH ( int idx, mPrimaryKeyAttrs )
      {
        Q_ASSERT( idx >= 0 && idx < mAttributeFields.size() );
        key += delim + mAttributeFields[ idx ].name();
        delim = ",";
      }
    }
    break;
    case pktUnknown:
      mValid = false;
      break;
  }

  if ( mValid )
  {
    mUri.setKeyColumn( key );
    setDataSourceUri( mUri.uri() );
  }
  else
  {
    disconnectDb();
  }
}

QgsOracleProvider::~QgsOracleProvider()
{
  QgsDebugMsg( "deconstructing." );

  disconnectDb();
}

QgsAbstractFeatureSource *QgsOracleProvider::featureSource() const
{
  return new QgsOracleFeatureSource( this );
}

void QgsOracleProvider::disconnectDb()
{
  if ( mConnection )
    mConnection->disconnect();
  mConnection = 0;
}

bool QgsOracleProvider::exec( QSqlQuery &qry, QString sql )
{
  QgsDebugMsgLevel( QString( "SQL: %1" ).arg( sql ), 4 );

  qry.setForwardOnly( true );

  bool res = qry.exec( sql );
  if ( !res )
  {
    QgsDebugMsg( QString( "SQL: %1\nERROR: %2" )
                 .arg( qry.lastQuery() )
                 .arg( qry.lastError().text() ) );
  }

  return res;
}

QString QgsOracleProvider::storageType() const
{
  return "Oracle database with locator/spatial extension";
}

#if QT_VERSION < 0x050000
static bool operator<( const QVariant &a, const QVariant &b )
{
  if ( a.isNull() || b.isNull() )
    return false;

  if ( a.type() == b.type() )
  {
    switch ( a.type() )
    {
      case QVariant::Int:
      case QVariant::Char:
        return a.toInt() < b.toInt();

      case QVariant::Double:
        return a.toDouble() < b.toDouble();

      case QVariant::LongLong:
        return a.toLongLong() < b.toLongLong();

      case QVariant::List:
      {
        QList<QVariant> al = a.toList();
        QList<QVariant> bl = b.toList();

        int i, n = qMin( al.size(), bl.size() );
        for ( i = 0; i < n && al[i] == bl[i]; i++ )
          ;

        if ( i == n )
          return al.size() < bl.size();
        else
          return al[i] < bl[i];
      }
      break;

      case QVariant::StringList:
      {
        QStringList al = a.toStringList();
        QStringList bl = b.toStringList();

        int i, n = qMin( al.size(), bl.size() );
        for ( i = 0; i < n && al[i] == bl[i]; i++ )
          ;

        if ( i == n )
          return al.size() < bl.size();
        else
          return al[i] < bl[i];
      }
      break;

      case QVariant::Date:
        return a.toDate() < b.toDate();

      case QVariant::Time:
        return a.toTime() < b.toTime();

      case QVariant::DateTime:
        return a.toDateTime() < b.toDateTime();

      case QVariant::Bool:
        return a.toBool() < b.toBool();

      case QVariant::UInt:
        return a.toUInt() < b.toUInt();

      case QVariant::ULongLong:
        return a.toULongLong() < b.toULongLong();

      default:
        break;
    }
  }

  return a.canConvert( QVariant::String ) && b.canConvert( QVariant::String ) && a.toString() < b.toString();
}
#endif


QString QgsOracleProvider::pkParamWhereClause() const
{
  QString whereClause;

  switch ( mPrimaryKeyType )
  {
    case pktInt:
    case pktFidMap:
    {
      Q_ASSERT( mPrimaryKeyAttrs.size() >= 1 );

      QString delim = "";
      for ( int i = 0; i < mPrimaryKeyAttrs.size(); i++ )
      {
        int idx = mPrimaryKeyAttrs[i];
        const QgsField &fld = field( idx );

        whereClause += delim + QString( "%1=?" ).arg( mConnection->fieldExpression( fld ) );
        delim = " AND ";
      }
    }
    break;

    case pktRowId:
      return "ROWID=?";
      break;

    case pktUnknown:
      Q_ASSERT( !"FAILURE: Primary key unknown" );
      whereClause = "NULL IS NOT NULL";
      break;
  }

  if ( !mSqlWhereClause.isEmpty() )
  {
    if ( !whereClause.isEmpty() )
      whereClause += " AND ";

    whereClause += "(" + mSqlWhereClause + ")";
  }

  return whereClause;
}

void QgsOracleProvider::appendPkParams( QgsFeatureId fid, QSqlQuery &qry ) const
{
  switch ( mPrimaryKeyType )
  {
    case pktInt:
      QgsDebugMsgLevel( QString( "addBindValue pk %1" ).arg( FID_TO_STRING( fid ) ), 4 );
      qry.addBindValue( FID_TO_STRING( fid ) );
      break;

    case pktRowId:
    case pktFidMap:
    {
      QVariant pkValsVariant = mShared->lookupKey( fid );
      if ( !pkValsVariant.isNull() )
      {
        Q_FOREACH ( const QVariant &v, pkValsVariant.toList() )
        {
          QgsDebugMsgLevel( QString( "addBindValue pk %1" ).arg( FID_TO_STRING( fid ) ), 4 );
          qry.addBindValue( v );
        }
      }
      else
      {
        QgsDebugMsg( QString( "key values for fid %1 not found." ).arg( fid ) );
        for ( int i = 0; i < mPrimaryKeyAttrs.size(); i++ )
        {
          QgsDebugMsgLevel( QString( "addBindValue pk NULL" ).arg( fid ), 4 );
          qry.addBindValue( QVariant() );
        }
      }
      break;
    }
    break;

    case pktUnknown:
      QgsDebugMsg( "Unknown key type" );
      break;
  }
}


QString QgsOracleUtils::whereClause( QgsFeatureId featureId, const QgsFields& fields, QgsOraclePrimaryKeyType primaryKeyType, const QList<int>& primaryKeyAttrs, QSharedPointer<QgsOracleSharedData> sharedData )
{
  QString whereClause;

  switch ( primaryKeyType )
  {
    case pktInt:
      Q_ASSERT( primaryKeyAttrs.size() == 1 );
      whereClause = QString( "%1=%2" ).arg( QgsOracleConn::quotedIdentifier( fields[ primaryKeyAttrs[0] ].name() ) ).arg( featureId );
      break;

    case pktRowId:
    case pktFidMap:
    {
      QVariant pkValsVariant = sharedData->lookupKey( featureId );
      if ( !pkValsVariant.isNull() )
      {
        QList<QVariant> pkVals = pkValsVariant.toList();

        if ( primaryKeyType == pktFidMap )
        {
          Q_ASSERT( pkVals.size() == primaryKeyAttrs.size() );

          QString delim = "";
          for ( int i = 0; i < primaryKeyAttrs.size(); i++ )
          {
            int idx = primaryKeyAttrs[i];
            const QgsField &fld = fields[ idx ];

            whereClause += delim + QString( "%1=%2" ).arg( QgsOracleConn::fieldExpression( fld ) ).arg( QgsOracleConn::quotedValue( pkVals[i], fld.type() ) );
            delim = " AND ";
          }
        }
        else
        {
          whereClause += QString( "ROWID=%1" ).arg( QgsOracleConn::quotedValue( pkVals[0].toString() ) );
        }
      }
      else
      {
        QgsDebugMsg( QString( "FAILURE: Key values for feature %1 not found." ).arg( featureId ) );
        whereClause = "NULL IS NOT NULL";
      }
    }
    break;

    case pktUnknown:
      Q_ASSERT( !"FAILURE: Primary key unknown" );
      whereClause = "NULL IS NOT NULL";
      break;
  }

  return whereClause;
}

QString QgsOracleUtils::whereClause( QgsFeatureIds featureIds, const QgsFields &fields, QgsOraclePrimaryKeyType primaryKeyType, const QList<int> &primaryKeyAttrs, QSharedPointer<QgsOracleSharedData> sharedData )
{
  QStringList whereClauses;
  Q_FOREACH ( const QgsFeatureId featureId, featureIds )
  {
    whereClauses << whereClause( featureId, fields, primaryKeyType, primaryKeyAttrs, sharedData );
  }
  return whereClauses.isEmpty() ? "" : whereClauses.join( " OR " ).prepend( "(" ).append( ")" );
}

QString QgsOracleUtils::andWhereClauses( const QString& c1, const QString& c2 )
{
  if ( c1.isEmpty() )
    return c2;
  if ( c2.isEmpty() )
    return c1;

  return QString( "(%1) AND (%2)" ).arg( c1 ).arg( c2 );
}

QString QgsOracleProvider::whereClause( QgsFeatureId featureId ) const
{
  return QgsOracleUtils::whereClause( featureId, mAttributeFields, mPrimaryKeyType, mPrimaryKeyAttrs, mShared );
}

void QgsOracleProvider::setExtent( QgsRectangle& newExtent )
{
  mLayerExtent.setXMaximum( newExtent.xMaximum() );
  mLayerExtent.setXMinimum( newExtent.xMinimum() );
  mLayerExtent.setYMaximum( newExtent.yMaximum() );
  mLayerExtent.setYMinimum( newExtent.yMinimum() );
}

/**
 * Return the feature type
 */
QGis::WkbType QgsOracleProvider::geometryType() const
{
  return mRequestedGeomType != QGis::WKBUnknown ? mRequestedGeomType : mDetectedGeomType;
}

const QgsField &QgsOracleProvider::field( int index ) const
{
  if ( index < 0 || index >= mAttributeFields.size() )
  {
    QgsMessageLog::logMessage( tr( "FAILURE: Field %1 not found." ).arg( index ), tr( "Oracle" ) );
    throw OracleFieldNotFound();
  }

  return mAttributeFields[ index ];
}

QgsFeatureIterator QgsOracleProvider::getFeatures( const QgsFeatureRequest& request )
{
  if ( !mValid )
  {
    QgsMessageLog::logMessage( tr( "Read attempt on an invalid oracle data source" ), tr( "Oracle" ) );
    return QgsFeatureIterator();
  }

  return QgsFeatureIterator( new QgsOracleFeatureIterator( new QgsOracleFeatureSource( this ), true, request ) );
}

/**
 * Return the number of fields
 */
uint QgsOracleProvider::fieldCount() const
{
  return mAttributeFields.size();
}

const QgsFields &QgsOracleProvider::fields() const
{
  return mAttributeFields;
}

QString QgsOracleProvider::dataComment() const
{
  return mDataComment;
}

bool QgsOracleProvider::loadFields()
{
  mAttributeFields.clear();
  mDefaultValues.clear();

  QSqlQuery qry( *mConnection );

  QMap<QString, QString> comments;
  QMap<QString, QString> types;
  QMap<QString, QVariant> defvalues;

  if ( !mIsQuery )
  {
    QgsDebugMsg( QString( "Loading fields for table %1" ).arg( mTableName ) );

    if ( exec( qry, QString( "SELECT comments FROM all_tab_comments WHERE owner=%1 AND table_name=%2" )
               .arg( quotedValue( mOwnerName ) )
               .arg( quotedValue( mTableName ) ) ) )
    {
      if ( qry.next() )
        mDataComment = qry.value( 0 ).toString();
      else if ( exec( qry, QString( "SELECT comments FROM all_mview_comments WHERE owner=%1 AND mview_name=%2" )
                      .arg( quotedValue( mOwnerName ) )
                      .arg( quotedValue( mTableName ) ) ) )
      {
        if ( qry.next() )
          mDataComment = qry.value( 0 ).toString();
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Loading comment for table %1.%2 failed [%3]" )
                                 .arg( mOwnerName )
                                 .arg( mTableName )
                                 .arg( qry.lastError().text() ),
                                 tr( "Oracle" ) );
    }

    qry.finish();

    if ( exec( qry, QString( "SELECT column_name,comments FROM all_col_comments t WHERE t.owner=%1 AND t.table_name=%2 AND t.column_name<>%3" )
               .arg( quotedValue( mOwnerName ) )
               .arg( quotedValue( mTableName ) )
               .arg( quotedValue( mGeometryColumn ) ) ) )
    {
      while ( qry.next() )
      {
        comments.insert( qry.value( 0 ).toString(), qry.value( 1 ).toString() );
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Loading comment for columns of table %1.%2 failed [%3]" ).arg( mOwnerName ).arg( mTableName ).arg( qry.lastError().text() ), tr( "Oracle" ) );
    }

    qry.finish();

    if ( exec( qry, QString( "SELECT"
                             " t.column_name"
                             ",CASE WHEN t.data_type_owner IS NULL THEN t.data_type ELSE t.data_type_owner||'.'||t.data_type END"
                             ",t.data_precision"
                             ",t.data_scale"
                             ",t.char_length"
                             ",t.char_used"
                             ",t.data_default"
                             " FROM all_tab_columns t"
                             " WHERE t.owner=%1 AND t.table_name=%2%3%4"
                             " ORDER BY t.column_id" )
               .arg( quotedValue( mOwnerName ) )
               .arg( quotedValue( mTableName ) )
               .arg( mGeometryColumn.isEmpty() ? "" : QString( " AND t.column_name<>%1 " ).arg( quotedValue( mGeometryColumn ) ) )
               .arg( mIncludeGeoAttributes ? "" : " AND (t.data_type_owner<>'MDSYS' OR t.data_type<>'SDO_GEOMETRY')" )
             ) )
    {
      while ( qry.next() )
      {
        QString name      = qry.value( 0 ).toString();
        QString type      = qry.value( 1 ).toString();
        int prec          = qry.value( 2 ).toInt();
        int scale         = qry.value( 3 ).toInt();
        int clength       = qry.value( 4 ).toInt();
        bool cused        = qry.value( 5 ).toString() == "C";
        QVariant defValue = qry.value( 6 );

        if ( type == "CHAR" || type == "VARCHAR2" || type == "VARCHAR" )
        {
          types.insert( name, QString( "%1(%2 %3)" ).arg( type ).arg( clength ).arg( cused ? "CHAR" : "BYTE" ) );
        }
        else if ( type == "NCHAR" || type == "NVARCHAR2" || type == "NVARCHAR" )
        {
          types.insert( name, QString( "%1(%2)" ).arg( type ).arg( clength ) );
        }
        else if ( type == "NUMBER" )
        {
          if ( scale == 0 )
          {
            types.insert( name, QString( "%1(%2)" ).arg( type ).arg( prec ) );
          }
          else
          {
            types.insert( name, QString( "%1(%2,%3)" ).arg( type ).arg( prec ).arg( scale ) );
          }
        }
        else
        {
          types.insert( name, type );
        }

        defvalues.insert( name, defValue );
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Loading field types for table %1.%2 failed [%3]" )
                                 .arg( mOwnerName )
                                 .arg( mTableName )
                                 .arg( qry.lastError().text() ),
                                 tr( "Oracle" ) );
    }

    if ( !mGeometryColumn.isEmpty() )
    {
      if ( exec( qry, QString( "SELECT i.index_name,i.domidx_opstatus"
                               " FROM all_indexes i"
                               " JOIN all_ind_columns c ON i.owner=c.index_owner AND i.index_name=c.index_name AND c.column_name=%3"
                               " WHERE i.table_owner=%1 AND i.table_name=%2 AND i.ityp_owner='MDSYS' AND i.ityp_name='SPATIAL_INDEX'" )
                 .arg( quotedValue( mOwnerName ) )
                 .arg( quotedValue( mTableName ) )
                 .arg( quotedValue( mGeometryColumn ) ) ) )
      {
        if ( qry.next() )
        {
          mSpatialIndexName = qry.value( 0 ).toString();
          if ( qry.value( 1 ).toString() != "VALID" )
          {
            QgsMessageLog::logMessage( tr( "Invalid spatial index %1 on column %2.%3.%4 found - expect poor performance." )
                                       .arg( mSpatialIndexName )
                                       .arg( mOwnerName )
                                       .arg( mTableName )
                                       .arg( mGeometryColumn ),
                                       tr( "Oracle" ) );
          }
          else
          {
            QgsDebugMsg( QString( "Valid spatial index %1 found" ).arg( mSpatialIndexName ) );
            mHasSpatialIndex = true;
          }
        }
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Probing for spatial index on column %1.%2.%3 failed [%4]" )
                                   .arg( mOwnerName )
                                   .arg( mTableName )
                                   .arg( mGeometryColumn )
                                   .arg( qry.lastError().text() ),
                                   tr( "Oracle" ) );
      }
    }

    mEnabledCapabilities |= QgsVectorDataProvider::CreateSpatialIndex;
  }

  if ( !mGeometryColumn.isEmpty() )
  {
    if ( !mHasSpatialIndex )
    {
      mHasSpatialIndex = qry.exec( QString( "SELECT %2 FROM %1 WHERE sdo_filter(%2,mdsys.sdo_geometry(2003,%3,NULL,mdsys.sdo_elem_info_array(1,1003,3),mdsys.sdo_ordinate_array(-1,-1,1,1)))='TRUE'" )
                                   .arg( mQuery )
                                   .arg( quotedIdentifier( mGeometryColumn ) )
                                   .arg( mSrid < 1 ? "NULL" : QString::number( mSrid ) ) );
    }

    if ( !mHasSpatialIndex )
    {
      QgsMessageLog::logMessage( tr( "No spatial index on column %1 found - expect poor performance." )
                                 .arg( mGeometryColumn ),
                                 tr( "Oracle" ) );
    }
  }

  qry.finish();

  if ( !exec( qry, QString( "SELECT * FROM %1 WHERE 1=0" ).arg( mQuery ) ) )
  {
    QgsMessageLog::logMessage( tr( "Retrieving fields from '%1' failed [%2]" ).arg( mQuery ).arg( qry.lastError().text() ), tr( "Oracle" ) );
    return false;
  }

  QSqlRecord record = qry.record();

  for ( int i = 0; i < record.count(); i++ )
  {
    QSqlField field = record.field( i );

    if ( field.name() == mGeometryColumn )
      continue;

    if ( !mIsQuery && !types.contains( field.name() ) )
      continue;

    QVariant::Type type = field.type();

    if ( types.value( field.name() ) == "DATE" )
    {
      // date types are incorrectly detected as datetime
      type = QVariant::Date;
    }

    mAttributeFields.append( QgsField( field.name(), type, types.value( field.name() ), field.length(), field.precision(), comments.value( field.name() ) ) );
    mDefaultValues.append( defvalues.value( field.name(), QVariant() ) );
  }

  return true;
}

bool QgsOracleProvider::hasSufficientPermsAndCapabilities()
{
  QgsDebugMsg( "Checking for permissions on the relation" );

  mEnabledCapabilities = QgsVectorDataProvider::SelectAtId | QgsVectorDataProvider::SelectGeometryAtId;

  QSqlQuery qry( *mConnection );
  if ( !mIsQuery )
  {
    if ( mConnection->currentUser() == mOwnerName )
    {
      // full set of privileges for the owner
      mEnabledCapabilities |= QgsVectorDataProvider::DeleteFeatures
                              |  QgsVectorDataProvider::ChangeAttributeValues
                              |  QgsVectorDataProvider::AddFeatures
                              |  QgsVectorDataProvider::AddAttributes
                              |  QgsVectorDataProvider::DeleteAttributes
                              |  QgsVectorDataProvider::ChangeGeometries
                              |  QgsVectorDataProvider::RenameAttributes
                              ;
    }
    else if ( exec( qry, QString( "SELECT privilege FROM all_tab_privs WHERE table_schema=%1 AND table_name=%2 AND privilege IN ('DELETE','UPDATE','INSERT','ALTER TABLE')" )
                    .arg( quotedValue( mOwnerName ) ).arg( quotedValue( mTableName ) ) ) )
    {
      // check grants
      while ( qry.next() )
      {
        QString priv = qry.value( 0 ).toString();

        if ( priv == "DELETE" )
        {
          mEnabledCapabilities |= QgsVectorDataProvider::DeleteFeatures;
        }
        else if ( priv == "UPDATE" )
        {
          mEnabledCapabilities |= QgsVectorDataProvider::ChangeAttributeValues;
        }
        else if ( priv == "INSERT" )
        {
          mEnabledCapabilities |= QgsVectorDataProvider::AddFeatures;
        }
        else if ( priv == "ALTER TABLE" )
        {
          mEnabledCapabilities |= QgsVectorDataProvider::AddAttributes | QgsVectorDataProvider::DeleteAttributes | QgsVectorDataProvider::RenameAttributes;
        }
      }

      if ( !mGeometryColumn.isNull() )
      {
        if ( exec( qry, QString( "SELECT 1 FROM all_col_privs WHERE table_schema=%1 AND table_name=%2 AND column_name=%3 AND privilege='UPDATE'" )
                   .arg( quotedValue( mOwnerName ) )
                   .arg( quotedValue( mTableName ) )
                   .arg( quotedValue( mGeometryColumn ) ) ) )
        {
          if ( qry.next() )
            mEnabledCapabilities |= QgsVectorDataProvider::ChangeGeometries;
        }
        else
        {
          QgsMessageLog::logMessage( tr( "Unable to determine geometry column access privileges for column %1.%2.\nThe error message from the database was:\n%3.\nSQL: %4" )
                                     .arg( mQuery )
                                     .arg( mGeometryColumn )
                                     .arg( qry.lastError().text() )
                                     .arg( qry.lastQuery() ),
                                     tr( "Oracle" ) );
        }
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Unable to determine table access privileges for the table %1.\nThe error message from the database was:\n%2.\nSQL: %3" )
                                 .arg( mQuery )
                                 .arg( qry.lastError().text() )
                                 .arg( qry.lastQuery() ),
                                 tr( "Oracle" ) );
    }
  }
  else
  {
    // Check if the sql is a select query
    if ( !mQuery.startsWith( "(" ) && !mQuery.endsWith( ")" ) )
    {
      QgsMessageLog::logMessage( tr( "The custom query is not a select query." ), tr( "Oracle" ) );
      return false;
    }

    if ( !exec( qry, QString( "SELECT * FROM %1 WHERE 1=0" ).arg( mQuery ) ) )
    {
      QgsMessageLog::logMessage( tr( "Unable to execute the query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                                 .arg( qry.lastError().text() )
                                 .arg( qry.lastQuery() ), tr( "Oracle" ) );
      return false;
    }
  }

  qry.finish();

  return true;
}

bool QgsOracleProvider::determinePrimaryKey()
{
  if ( !loadFields() )
  {
    return false;
  }

  // check to see if there is an unique index on the relation, which
  // can be used as a key into the table. Primary keys are always
  // unique indices, so we catch them as well.

  QSqlQuery qry( *mConnection );
  if ( !mIsQuery )
  {

    if ( !exec( qry, QString( "SELECT column_name"
                              " FROM all_ind_columns a"
                              " JOIN all_constraints b ON a.index_name=constraint_name AND a.index_owner=b.owner"
                              " WHERE b.constraint_type='P' AND b.owner=%1 AND b.table_name=%2" )
                .arg( quotedValue( mOwnerName ) ).arg( quotedValue( mTableName ) ) ) )
    {
      QgsMessageLog::logMessage( tr( "Unable to execute the query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                                 .arg( qry.lastError().text() )
                                 .arg( qry.lastQuery() ), tr( "Oracle" ) );
      return false;
    }

    bool isInt = true;

    while ( qry.next() )
    {
      QString name = qry.value( 0 ).toString();

      int idx = mAttributeFields.indexFromName( name );
      if ( idx < 0 )
      {
        QgsMessageLog::logMessage( tr( "Primary key field %1 not found in %2" ).arg( name ).arg( mQuery ), tr( "Oracle" ) );
        return false;
      }

      const QgsField &fld = mAttributeFields.at( idx );

      if ( isInt &&
           fld.type() != QVariant::Int &&
           fld.type() != QVariant::LongLong &&
           !( fld.type() == QVariant::Double && fld.precision() == 0 ) )
        isInt = false;

      mPrimaryKeyAttrs << idx;
    }

    if ( mPrimaryKeyAttrs.size() > 0 )
    {
      mPrimaryKeyType = ( mPrimaryKeyAttrs.size() == 1 && isInt ) ? pktInt : pktFidMap;
    }
    else if ( !exec( qry, QString( "SELECT 1 FROM all_tables WHERE owner=%1 AND table_name=%2" ).arg( quotedValue( mOwnerName ) ).arg( quotedValue( mTableName ) ) ) )
    {
      QgsMessageLog::logMessage( tr( "Unable to execute the query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                                 .arg( qry.lastError().text() )
                                 .arg( qry.lastQuery() ), tr( "Oracle" ) );
    }
    else if ( qry.next() )
    {
      // is table
      mPrimaryKeyType = pktRowId;
    }
    else
    {
      QString primaryKey = mUri.keyColumn();
      mPrimaryKeyType = pktUnknown;

      if ( !primaryKey.isEmpty() )
      {
        int idx = fieldNameIndex( primaryKey );

        if ( idx >= 0 )
        {
          const QgsField &fld = mAttributeFields.at( idx );

          if ( mUseEstimatedMetadata || uniqueData( mQuery, primaryKey ) )
          {
            mPrimaryKeyType = ( fld.type() == QVariant::Int || fld.type() == QVariant::LongLong || ( fld.type() == QVariant::Double && fld.precision() == 0 ) ) ? pktInt : pktFidMap;
            mPrimaryKeyAttrs << idx;
          }
          else
          {
            QgsMessageLog::logMessage( tr( "Primary key field '%1' for view not unique." ).arg( primaryKey ), tr( "Oracle" ) );
          }
        }
        else
        {
          QgsMessageLog::logMessage( tr( "Key field '%1' for view not found." ).arg( primaryKey ), tr( "Oracle" ) );
        }
      }
      else
      {
        QgsMessageLog::logMessage( tr( "No key field for view given." ), tr( "Oracle" ) );
      }
    }
  }
  else
  {
    QString primaryKey = mUri.keyColumn();
    int idx = fieldNameIndex( mUri.keyColumn() );

    if ( idx >= 0 && (
           mAttributeFields[idx].type() == QVariant::Int ||
           mAttributeFields[idx].type() == QVariant::LongLong ||
           mAttributeFields[idx].type() == QVariant::Double
         ) )
    {
      if ( mUseEstimatedMetadata || uniqueData( mQuery, primaryKey ) )
      {
        mPrimaryKeyType = pktInt;
        mPrimaryKeyAttrs << idx;
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "No key field for query given." ), tr( "Oracle" ) );
      mPrimaryKeyType = pktUnknown;
    }
  }

  qry.finish();

  mValid = mPrimaryKeyType != pktUnknown;

  return mValid;
}

bool QgsOracleProvider::uniqueData( QString query, QString colName )
{
  Q_UNUSED( query );
  // Check to see if the given column contains unique data
  QSqlQuery qry( *mConnection );

  QString table = mQuery;
  if ( !mSqlWhereClause.isEmpty() )
  {
    table += " WHERE " + mSqlWhereClause;
  }

  QString sql = QString( "SELECT (SELECT count(distinct %1) FROM %2)-(SELECT count(%1) FROM %2) FROM dual" )
                .arg( quotedIdentifier( colName ) )
                .arg( mQuery );

  if ( !exec( qry, sql ) || !qry.next() )
  {
    QgsMessageLog::logMessage( tr( "Unable to execute the query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                               .arg( qry.lastError().text() )
                               .arg( qry.lastQuery() ), tr( "Oracle" ) );
    return false;
  }

  return qry.value( 0 ).toInt() == 0;
}

// Returns the minimum value of an attribute
QVariant QgsOracleProvider::minimumValue( int index )
{
  if ( !mConnection )
    return QVariant( QString::null );

  try
  {
    // get the field name
    const QgsField &fld = field( index );
    QString sql = QString( "SELECT min(%1) FROM %2" )
                  .arg( quotedIdentifier( fld.name() ) )
                  .arg( mQuery );

    if ( !mSqlWhereClause.isEmpty() )
    {
      sql += QString( " WHERE %1" ).arg( mSqlWhereClause );
    }

    QSqlQuery qry( *mConnection );

    if ( !exec( qry, sql ) )
    {
      QgsMessageLog::logMessage( tr( "Unable to execute the query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                                 .arg( qry.lastError().text() )
                                 .arg( qry.lastQuery() ), tr( "Oracle" ) );
      return QVariant( QString::null );
    }

    if ( qry.next() )
    {
      return qry.value( 0 );
    }
  }
  catch ( OracleFieldNotFound )
  {
    ;
  }
  return QVariant( QString::null );
}

// Returns the list of unique values of an attribute
void QgsOracleProvider::uniqueValues( int index, QList<QVariant> &uniqueValues, int limit )
{
  if ( !mConnection )
    return;

  uniqueValues.clear();

  try
  {
    // get the field name
    const QgsField &fld = field( index );
    QString sql = QString( "SELECT DISTINCT %1 FROM %2" )
                  .arg( quotedIdentifier( fld.name() ) )
                  .arg( mQuery );

    if ( !mSqlWhereClause.isEmpty() )
    {
      sql += QString( " WHERE %1" ).arg( mSqlWhereClause );
    }

    sql +=  QString( " ORDER BY %1" )
            .arg( quotedIdentifier( fld.name() ) );

    if ( limit >= 0 )
    {
      sql = QString( "SELECT * FROM (%1) WHERE rownum<=%2" ).arg( sql ).arg( limit );
    }

    QSqlQuery qry( *mConnection );

    if ( !exec( qry, sql ) )
    {
      QgsMessageLog::logMessage( tr( "Unable to execute the query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                                 .arg( qry.lastError().text() )
                                 .arg( qry.lastQuery() ), tr( "Oracle" ) );
      return;
    }

    while ( qry.next() )
    {
      uniqueValues.append( qry.value( 0 ) );
    }
  }
  catch ( OracleFieldNotFound )
  {
  }
}

// Returns the maximum value of an attribute
QVariant QgsOracleProvider::maximumValue( int index )
{
  if ( !mConnection )
    return QVariant();

  try
  {
    // get the field name
    const QgsField &fld = field( index );
    QString sql = QString( "SELECT max(%1) FROM %2" )
                  .arg( quotedIdentifier( fld.name() ) )
                  .arg( mQuery );

    if ( !mSqlWhereClause.isEmpty() )
    {
      sql += QString( " WHERE %1" ).arg( mSqlWhereClause );
    }

    QSqlQuery qry( *mConnection );

    if ( !exec( qry, sql ) )
    {
      QgsMessageLog::logMessage( tr( "Unable to execute the query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                                 .arg( qry.lastError().text() )
                                 .arg( qry.lastQuery() ), tr( "Oracle" ) );
      return QVariant( QString::null );
    }

    if ( qry.next() )
    {
      return qry.value( 0 );
    }
  }
  catch ( OracleFieldNotFound )
  {
    ;
  }

  return QVariant( QString::null );
}


bool QgsOracleProvider::isValid()
{
  return mValid;
}

QVariant QgsOracleProvider::defaultValue( int fieldId )
{
  return mDefaultValues.value( fieldId, QVariant() );
}

QString QgsOracleProvider::paramValue( QString fieldValue, const QString &defaultValue ) const
{
  if ( fieldValue.isNull() )
  {
    return QString::null;
  }
  else if ( fieldValue == defaultValue && !defaultValue.isNull() )
  {
    QSqlQuery qry( *mConnection );
    if ( !exec( qry, QString( "SELECT %1 FROM dual" ).arg( defaultValue ) ) || !qry.next() )
    {
      throw OracleException( tr( "Evaluation of default value failed" ), qry );
    }

    return qry.value( 0 ).toString();
  }
  else
  {
    return fieldValue;
  }
}


bool QgsOracleProvider::addFeatures( QgsFeatureList &flist )
{
  if ( flist.size() == 0 )
    return true;

  if ( mIsQuery || !mConnection )
    return false;

  bool returnvalue = true;


  QSqlDatabase db( *mConnection );

  try
  {
    QSqlQuery ins( db ), getfid( db );

    if ( !db.transaction() )
    {
      throw OracleException( tr( "Could not start transaction" ), db );
    }

    // Prepare the INSERT statement
    QString insert = QString( "INSERT INTO %1(" ).arg( mQuery );
    QString values = ") VALUES (";
    QString delim = "";

    QStringList defaultValues;
    QList<int> fieldId;

    if ( !mGeometryColumn.isNull() )
    {
      insert += quotedIdentifier( mGeometryColumn );
      values += "?";
      delim = ",";
    }

    if ( mPrimaryKeyType == pktInt || mPrimaryKeyType == pktFidMap )
    {
      QString keys, kdelim = "";

      Q_FOREACH ( int idx, mPrimaryKeyAttrs )
      {
        const QgsField &fld = field( idx );
        insert += delim + quotedIdentifier( fld.name() );
        keys += kdelim + quotedIdentifier( fld.name() );
        values += delim + "?";
        delim = ",";
        kdelim = ",";
        fieldId << idx;
        defaultValues << defaultValue( idx ).toString();
      }

      if ( !getfid.prepare( QString( "SELECT %1 FROM %2 WHERE ROWID=?" ).arg( keys ).arg( mQuery ) ) )
      {
        throw OracleException( tr( "Could not prepare get feature id statement" ), getfid );
      }
    }

    QgsAttributes attributevec = flist[0].attributes();

    // look for unique attribute values to place in statement instead of passing as parameter
    // e.g. for defaults
    for ( int idx = 0; idx < qMin( attributevec.size(), mAttributeFields.size() ); ++idx )
    {
      QVariant v = attributevec[idx];
      if ( !v.isValid() )
        continue;

      if ( fieldId.contains( idx ) )
        continue;

      const QgsField &fld = mAttributeFields[idx];

      QgsDebugMsgLevel( "Checking field against: " + fld.name(), 4 );

      if ( fld.name().isEmpty() || fld.name() == mGeometryColumn )
        continue;

      int i;
      for ( i = 1; i < flist.size(); i++ )
      {
        QgsAttributes attributevec = flist[i].attributes();
        QVariant v2 = attributevec[idx];

        if ( !v2.isValid() )
          break;

        if ( v2 != v )
          break;
      }

      insert += delim + quotedIdentifier( fld.name() );

      QString defVal = defaultValue( idx ).toString();

      if ( i == flist.size() )
      {
        if ( v == defVal )
        {
          if ( defVal.isNull() )
          {
            values += delim + "NULL";
          }
          else
          {
            values += delim + defVal;
          }
        }
        else if ( fld.typeName().endsWith( ".SDO_GEOMETRY" ) )
        {
          values += delim + "?";
        }
        else
        {
          values += delim + quotedValue( v, mAttributeFields[idx].type() );
        }
      }
      else
      {
        // value is not unique => add parameter
        values += delim + "?";
        defaultValues.append( defVal );
        fieldId.append( idx );
      }

      delim = ",";
    }

    insert += values + ")";

    QgsDebugMsgLevel( QString( "SQL prepare: %1" ).arg( insert ), 4 );
    if ( !ins.prepare( insert ) )
    {
      throw OracleException( tr( "Could not prepare insert statement" ), ins );
    }

    for ( QgsFeatureList::iterator features = flist.begin(); features != flist.end(); ++features )
    {
      QgsAttributes attributevec = features->attributes();

      QgsDebugMsgLevel( QString( "insert feature %1" ).arg( features->id() ), 4 );

      if ( !mGeometryColumn.isNull() )
      {
        appendGeomParam( features->geometry(), ins );
      }

      for ( int i = 0; i < fieldId.size(); i++ )
      {
        QVariant value = attributevec[ fieldId[i] ];

        QString v;
        if ( !value.isValid() )
        {
          const QgsField &fld = field( fieldId[i] );
          v = paramValue( defaultValues[i], defaultValues[i] );
          features->setAttribute( fieldId[i], convertValue( fld.type(), v ) );
        }
        else
        {
          v = paramValue( value.toString(), defaultValues[i] );

          if ( v != value.toString() )
          {
            const QgsField &fld = field( fieldId[i] );
            features->setAttribute( fieldId[i], convertValue( fld.type(), v ) );
          }
        }

        QgsDebugMsgLevel( QString( "addBindValue: %1" ).arg( v ), 4 );
        ins.addBindValue( v );
      }

      if ( !ins.exec() )
        throw OracleException( tr( "Could not insert feature %1" ).arg( features->id() ), ins );

      if ( mPrimaryKeyType == pktRowId )
      {
        features->setFeatureId( mShared->lookupFid( QList<QVariant>() << QVariant( ins.lastInsertId() ) ) );
        QgsDebugMsgLevel( QString( "new fid=%1" ).arg( features->id() ), 4 );
      }
      else if ( mPrimaryKeyType == pktInt || mPrimaryKeyType == pktFidMap )
      {
        getfid.addBindValue( QVariant( ins.lastInsertId() ) );
        if ( !getfid.exec() || !getfid.next() )
          throw OracleException( tr( "Could not retrieve feature id %1" ).arg( features->id() ), getfid );

        int col = 0;
        Q_FOREACH ( int idx, mPrimaryKeyAttrs )
        {
          const QgsField &fld = field( idx );

          QVariant v = getfid.value( col++ );
          if ( v.type() != fld.type() )
            v = QgsVectorDataProvider::convertValue( fld.type(), v.toString() );
          features->setAttribute( idx, v );
        }
      }
    }

    ins.finish();

    if ( !db.commit() )
    {
      throw OracleException( tr( "Could not commit transaction" ), db );
    }

    // update feature ids
    if ( mPrimaryKeyType == pktInt || mPrimaryKeyType == pktFidMap )
    {
      for ( QgsFeatureList::iterator features = flist.begin(); features != flist.end(); ++features )
      {
        QgsAttributes attributevec = features->attributes();

        if ( mPrimaryKeyType == pktInt )
        {
          features->setFeatureId( STRING_TO_FID( attributevec[ mPrimaryKeyAttrs[0] ] ) );
        }
        else
        {
          QList<QVariant> primaryKeyVals;

          Q_FOREACH ( int idx, mPrimaryKeyAttrs )
          {
            primaryKeyVals << attributevec[ idx ];
          }

          features->setFeatureId( mShared->lookupFid( QVariant( primaryKeyVals ) ) );
        }
        QgsDebugMsgLevel( QString( "new fid=%1" ).arg( features->id() ), 4 );
      }
    }

    mFeaturesCounted += flist.size();
  }
  catch ( OracleException &e )
  {
    QgsDebugMsg( QString( "Oracle error: %1" ).arg( e.errorMessage() ) );
    pushError( tr( "Oracle error while adding features: %1" ).arg( e.errorMessage() ) );
    if ( !db.rollback() )
    {
      QgsMessageLog::logMessage( tr( "Could not rollback transaction" ), tr( "Oracle" ) );
    }
    returnvalue = false;
  }

  return returnvalue;
}

bool QgsOracleProvider::deleteFeatures( const QgsFeatureIds & id )
{
  bool returnvalue = true;

  if ( mIsQuery || !mConnection )
    return false;

  QSqlDatabase db( *mConnection );

  try
  {
    QSqlQuery qry( db );

    if ( !db.transaction() )
    {
      throw OracleException( tr( "Could not start transaction" ), db );
    }

    for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
    {
      QString sql = QString( "DELETE FROM %1 WHERE %2" )
                    .arg( mQuery ).arg( whereClause( *it ) );
      QgsDebugMsg( "delete sql: " + sql );

      if ( !exec( qry, sql ) )
        throw OracleException( tr( "Deletion of feature %1 failed" ).arg( *it ), qry );

      mShared->removeFid( *it );
    }

    qry.finish();

    if ( !db.commit() )
    {
      throw OracleException( tr( "Could not commit transaction" ), db );
    }

    mFeaturesCounted -= id.size();
  }
  catch ( OracleException &e )
  {
    pushError( tr( "Oracle error while deleting features: %1" ).arg( e.errorMessage() ) );
    if ( !db.rollback() )
      QgsMessageLog::logMessage( tr( "Could not rollback transaction" ), tr( "Oracle" ) );
    returnvalue = false;
  }

  return returnvalue;
}

bool QgsOracleProvider::addAttributes( const QList<QgsField> &attributes )
{
  bool returnvalue = true;

  if ( mIsQuery || !mConnection )
    return false;

  QSqlDatabase db( *mConnection );

  try
  {
    QSqlQuery qry( db );

    if ( !db.transaction() )
    {
      throw OracleException( tr( "Could not start transaction" ), db );
    }

    for ( QList<QgsField>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter )
    {
      QString type = iter->typeName().toLower();
      if ( type == "char" || type == "varchar2" )
      {
        type = QString( "%1(%2 char)" ).arg( type ).arg( iter->length() );
      }
      else if ( type == "number" )
      {
        if ( iter->precision() > 0 )
        {
          type = QString( "%1(%2,%3)" ).arg( type ).arg( iter->length() ).arg( iter->precision() );
        }
        else
        {
          type = QString( "%1(%2,%3)" ).arg( type ).arg( iter->length() );
        }
      }

      QString sql = QString( "ALTER TABLE %1 ADD %2 %3" )
                    .arg( mQuery )
                    .arg( quotedIdentifier( iter->name() ) )
                    .arg( type );
      QgsDebugMsg( sql );

      if ( !exec( qry, sql ) )
        throw OracleException( tr( "Adding attribute %1 failed" ).arg( iter->name() ), qry );

      if ( !iter->comment().isEmpty() )
      {
        sql = QString( "COMMENT ON COLUMN %1.%2 IS %3" )
              .arg( mQuery )
              .arg( quotedIdentifier( iter->name() ) )
              .arg( quotedValue( iter->comment() ) );
        if ( !exec( qry, sql ) )
          throw OracleException( tr( "Setting comment on %1 failed" ).arg( iter->name() ), qry );
      }

      qry.finish();

    }

    if ( !db.commit() )
    {
      throw OracleException( tr( "Could not commit transaction" ), db );
    }
  }
  catch ( OracleException &e )
  {
    pushError( tr( "Oracle error while adding attributes: %1" ).arg( e.errorMessage() ) );
    if ( !db.rollback() )
      QgsMessageLog::logMessage( tr( "Could not rollback transaction" ), tr( "Oracle" ) );
    returnvalue = false;
  }

  if ( !loadFields() )
  {
    QgsMessageLog::logMessage( tr( "Could not reload fields." ), tr( "Oracle" ) );
  }

  return returnvalue;
}

bool QgsOracleProvider::deleteAttributes( const QgsAttributeIds& ids )
{
  bool returnvalue = true;

  if ( mIsQuery || !mConnection )
    return false;

  QSqlDatabase db( *mConnection );

  try
  {
    QSqlQuery qry( db );

    if ( !db.transaction() )
    {
      throw OracleException( tr( "Could not start transaction" ), db );
    }

    qry.finish();

    QList<int> idsList = ids.values();
    qSort( idsList.begin(), idsList.end(), qGreater<int>() );

    Q_FOREACH ( int id, idsList )
    {
      const QgsField &fld = mAttributeFields.at( id );

      QString sql = QString( "ALTER TABLE %1 DROP COLUMN %2" )
                    .arg( mQuery )
                    .arg( quotedIdentifier( fld.name() ) );

      //send sql statement and do error handling
      if ( !exec( qry, sql ) )
        throw OracleException( tr( "Dropping column %1 failed" ).arg( fld.name() ), qry );

      //delete the attribute from mAttributeFields
      mAttributeFields.remove( id );
      mDefaultValues.removeAt( id );
    }

    if ( !db.commit() )
    {
      throw OracleException( tr( "Could not commit transaction" ), db );
    }
  }
  catch ( OracleException &e )
  {
    pushError( tr( "Oracle error while deleting attributes: %1" ).arg( e.errorMessage() ) );
    if ( !db.rollback() )
    {
      QgsMessageLog::logMessage( tr( "Could not rollback transaction" ), tr( "Oracle" ) );
    }
    returnvalue = false;
  }

  if ( !loadFields() )
  {
    QgsMessageLog::logMessage( tr( "Could not reload fields." ), tr( "Oracle" ) );
  }

  return returnvalue;
}

bool QgsOracleProvider::renameAttributes( const QgsFieldNameMap &renamedAttributes )
{
  if ( mIsQuery || !mConnection )
    return false;

  QgsFieldNameMap::const_iterator renameIt = renamedAttributes.constBegin();
  for ( ; renameIt != renamedAttributes.constEnd(); ++renameIt )
  {
    int fieldIndex = renameIt.key();
    if ( fieldIndex < 0 || fieldIndex >= mAttributeFields.count() )
    {
      pushError( tr( "Invalid attribute index: %1" ).arg( fieldIndex ) );
      return false;
    }
    if ( mAttributeFields.indexFromName( renameIt.value() ) >= 0 )
    {
      //field name already in use
      pushError( tr( "Error renaming field %1: name '%2' already exists" ).arg( fieldIndex ).arg( renameIt.value() ) );
      return false;
    }
  }

  QSqlDatabase db( *mConnection );

  bool returnvalue = true;

  try
  {
    QSqlQuery qry( db );

    if ( !db.transaction() )
    {
      throw OracleException( tr( "Could not start transaction" ), db );
    }

    qry.finish();

    for ( renameIt = renamedAttributes.constBegin(); renameIt != renamedAttributes.constEnd(); ++renameIt )
    {
      QString src( mAttributeFields.at( renameIt.key() ).name() );

      if ( !exec( qry, QString( "ALTER TABLE %1 RENAME COLUMN %2 TO %3" )
                  .arg( mQuery,
                        quotedIdentifier( src ),
                        quotedIdentifier( renameIt.value() ) ) ) )
      {
        throw OracleException( tr( "Renaming column %1 to %2 failed" )
                               .arg( quotedIdentifier( src ),
                                     quotedIdentifier( renameIt.value() ) ),
                               qry );
      }
    }

    if ( !db.commit() )
    {
      throw OracleException( tr( "Could not commit transaction" ), db );
    }
  }
  catch ( OracleException &e )
  {
    pushError( tr( "Oracle error while renaming attributes: %1" ).arg( e.errorMessage() ) );
    if ( !db.rollback() )
    {
      QgsMessageLog::logMessage( tr( "Could not rollback transaction" ), tr( "Oracle" ) );
    }
    returnvalue = false;
  }

  if ( !loadFields() )
  {
    QgsMessageLog::logMessage( tr( "Could not reload fields." ), tr( "Oracle" ) );
    returnvalue = false;
  }

  return returnvalue;
}


bool QgsOracleProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  bool returnvalue = true;

  if ( mIsQuery || !mConnection )
    return false;

  if ( attr_map.isEmpty() )
    return true;

  QSqlDatabase db( *mConnection );

  try
  {
    QSqlQuery qry( db );

    if ( !db.transaction() )
    {
      throw OracleException( tr( "Could not start transaction" ), db );
    }

    // cycle through the features
    for ( QgsChangedAttributesMap::const_iterator iter = attr_map.begin(); iter != attr_map.end(); ++iter )
    {
      QgsFeatureId fid = iter.key();

      // skip added features
      if ( FID_IS_NEW( fid ) )
        continue;

      const QgsAttributeMap& attrs = iter.value();
      if ( attrs.isEmpty() )
        continue;

      QString sql = QString( "UPDATE %1 SET " ).arg( mQuery );

      bool pkChanged = false;
      QList<int> geometryParams;

      // cycle through the changed attributes of the feature
      QString delim;
      for ( QgsAttributeMap::const_iterator siter = attrs.begin(); siter != attrs.end(); ++siter )
      {
        try
        {
          const QgsField &fld = field( siter.key() );

          pkChanged = pkChanged || mPrimaryKeyAttrs.contains( siter.key() );

          sql += delim + QString( "%1=" ).arg( quotedIdentifier( fld.name() ) );
          delim = ",";

          if ( fld.typeName().endsWith( ".SDO_GEOMETRY" ) )
          {
            if ( mConnection->hasSpatial() )
              sql += QString( "SDO_UTIL.FROM_WKTGEOMETRY(%1)" ).arg( quotedValue( siter->toString() ) );
            else
            {
              geometryParams << siter.key();
              sql += "?";
            }
          }
          else
          {
            sql += quotedValue( *siter, fld.type() );
          }
        }
        catch ( OracleFieldNotFound )
        {
          // Field was missing - shouldn't happen
        }
      }

      sql += QString( " WHERE %1" ).arg( whereClause( fid ) );

      if ( !qry.prepare( sql ) )
      {
        throw OracleException( tr( "Could not prepare update statement." ), qry );
      }

      Q_FOREACH ( int idx, geometryParams )
      {
        QgsGeometry *g;
        if ( !attrs[idx].isNull() )
        {
          g = QgsGeometry::fromWkt( attrs[ idx ].toString() );
        }
        else
        {
          g = new QgsGeometry();
        }

        appendGeomParam( g, qry );

        delete g;
      }

      if ( !qry.exec() )
        throw OracleException( tr( "Update of feature %1 failed" ).arg( iter.key() ), qry );

      qry.finish();

      // update feature id map if key was changed
      if ( pkChanged && mPrimaryKeyType == pktFidMap )
      {
        QVariant v = mShared->removeFid( fid );

        QList<QVariant> k = v.toList();

        for ( int i = 0; i < mPrimaryKeyAttrs.size(); i++ )
        {
          int idx = mPrimaryKeyAttrs[i];
          if ( !attrs.contains( idx ) )
            continue;

          k[i] = attrs[ idx ];
        }

        mShared->insertFid( fid, k );
      }
    }

    if ( !db.commit() )
    {
      throw OracleException( tr( "Could not commit transaction" ), db );
    }
  }
  catch ( OracleException &e )
  {
    pushError( tr( "Oracle error while changing attributes: %1" ).arg( e.errorMessage() ) );
    if ( !db.rollback() )
    {
      QgsMessageLog::logMessage( tr( "Could not rollback transaction" ), tr( "Oracle" ) );
    }
    returnvalue = false;
  }

  return returnvalue;
}

void QgsOracleProvider::appendGeomParam( const QgsGeometry *geom, QSqlQuery &qry ) const
{
  QOCISpatialGeometry g;

  wkbPtr ptr;
  ptr.ucPtr = geom ? ( unsigned char * ) geom->asWkb() : 0;
  g.isNull = !ptr.ucPtr;
  g.gtype = -1;
  g.srid  = mSrid < 1 ? -1 : mSrid;

  if ( !g.isNull )
  {
    ptr.ucPtr++;  // skip endianness

    g.eleminfo.clear();
    g.ordinates.clear();

    QString expr;
    int iOrdinate = 1;
    QGis::WkbType type = ( QGis::WkbType ) * ptr.iPtr++;
    int dim = 2;

    switch ( type )
    {
      case QGis::WKBPoint25D:
        dim = 3;
      case QGis::WKBPoint:
        g.srid  = mSrid;
        g.gtype = SDO_GTYPE( dim, gtPoint );
        g.x = *ptr.dPtr++;
        g.y = *ptr.dPtr++;
        g.z = dim == 3 ? *ptr.dPtr++ : 0.0;
        break;

      case QGis::WKBLineString25D:
      case QGis::WKBMultiLineString25D:
        dim = 3;
      case QGis::WKBLineString:
      case QGis::WKBMultiLineString:
      {
        g.gtype = SDO_GTYPE( dim, gtLine );
        int nLines = 1;
        if ( type == QGis::WKBMultiLineString25D || type == QGis::WKBMultiLineString )
        {
          g.gtype = SDO_GTYPE( dim, gtMultiLine );
          nLines = *ptr.iPtr++;
          ptr.ucPtr++; // Skip endianness of first linestring
          ptr.iPtr++;  // Skip type of first linestring
        }

        for ( int iLine = 0; iLine < nLines; iLine++ )
        {
          g.eleminfo << iOrdinate << 2 << 1;

          for ( int i = 0, n = *ptr.iPtr++; i < n; i++ )
          {
            g.ordinates << *ptr.dPtr++;
            g.ordinates << *ptr.dPtr++;
            if ( dim == 3 )
              g.ordinates << *ptr.dPtr++;
            iOrdinate  += dim;
          }

          ptr.ucPtr++; // Skip endianness of next linestring
          ptr.iPtr++;  // Skip type of next linestring
        }
      }
      break;

      case QGis::WKBPolygon25D:
      case QGis::WKBMultiPolygon25D:
        dim = 3;
      case QGis::WKBPolygon:
      case QGis::WKBMultiPolygon:
      {
        g.gtype = SDO_GTYPE( dim, gtPolygon );
        int nPolygons = 1;
        if ( type == QGis::WKBMultiPolygon25D || type == QGis::WKBMultiPolygon )
        {
          g.gtype = SDO_GTYPE( dim, gtMultiPolygon );
          nPolygons = *ptr.iPtr++;

          ptr.ucPtr++; // Skip endianness of first polygon
          ptr.iPtr++;  // Skip type of first polygon
        }

        for ( int iPolygon = 0; iPolygon < nPolygons; iPolygon++ )
        {
          for ( int iRing = 0, nRings = *ptr.iPtr++; iRing < nRings; iRing++ )
          {
            g.eleminfo << iOrdinate << ( iRing == 0 ? 1003 : 2003 ) << 1;

            // TODO: verify ring orientation
            for ( int i = 0, n = *ptr.iPtr++; i < n; i++ )
            {
              g.ordinates << *ptr.dPtr++;
              g.ordinates << *ptr.dPtr++;
              if ( dim == 3 )
                g.ordinates << *ptr.dPtr++;
              iOrdinate  += dim;
            }
          }

          ptr.ucPtr++; // Skip endianness of next polygon
          ptr.iPtr++;  // Skip type of next polygon
        }
      }
      break;

      case QGis::WKBMultiPoint25D:
        dim = 3;
      case QGis::WKBMultiPoint:
      {
        g.gtype = SDO_GTYPE( dim, gtMultiPoint );
        int n = *ptr.iPtr++;

        g.eleminfo << 1 << 1 << n;

        for ( int i = 0; i < n; i++ )
        {
          ptr.ucPtr++; // Skip endianness of point
          ptr.iPtr++;  // Skip type of point

          g.ordinates << *ptr.dPtr++;
          g.ordinates << *ptr.dPtr++;
          if ( dim == 3 )
            g.ordinates << *ptr.dPtr++;
        }
      }
      break;

      case QGis::WKBUnknown:
      case QGis::WKBNoGeometry:
        g.isNull = true;
        break;
    }
  }

  QgsDebugMsgLevel( QString( "addBindValue geometry: isNull=%1 gtype=%2 srid=%3 p=%4,%5,%6 eleminfo=%7 ordinates=%8" )
                    .arg( g.isNull )
                    .arg( g.gtype )
                    .arg( g.srid )
                    .arg( g.x ).arg( g.y ).arg( g.z )
                    .arg( g.eleminfo.size() )
                    .arg( g.ordinates.size() )
                    , 4 );
  qry.addBindValue( QVariant::fromValue( g ) );
}

bool QgsOracleProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{

  if ( mIsQuery || mGeometryColumn.isNull() || !mConnection )
    return false;

  QSqlDatabase db( *mConnection );

  bool returnvalue = true;

  try
  {
    QSqlQuery qry( db );

    if ( !db.transaction() )
    {
      throw OracleException( tr( "Could not start transaction" ), db );
    }

    QString update = QString( "UPDATE %1 SET %2=? WHERE %3" )
                     .arg( mQuery )
                     .arg( quotedIdentifier( mGeometryColumn ) )
                     .arg( pkParamWhereClause() );
    QgsDebugMsgLevel( QString( "SQL prepare: %1" ).arg( update ), 4 );
    if ( !qry.prepare( update ) )
    {
      throw OracleException( tr( "Could not prepare update statement." ), qry );
    }

    for ( QgsGeometryMap::const_iterator iter = geometry_map.constBegin();
          iter != geometry_map.constEnd();
          ++iter )
    {
      appendGeomParam( &iter.value(), qry );
      appendPkParams( iter.key(), qry );

      if ( !qry.exec() )
        throw OracleException( tr( "Update of feature %1 failed" ).arg( iter.key() ), qry );
    }

    qry.finish();

    if ( !db.commit() )
    {
      throw OracleException( tr( "Could not commit transaction" ), db );
    }
  }
  catch ( OracleException &e )
  {
    pushError( tr( "Oracle error while changing geometry values: %1" ).arg( e.errorMessage() ) );
    if ( !db.rollback() )
    {
      QgsMessageLog::logMessage( tr( "Could not rollback transaction" ), tr( "Oracle" ) );
    }
    returnvalue = false;
  }

  QgsDebugMsg( "exiting." );

  return returnvalue;
}

int QgsOracleProvider::capabilities() const
{
  return mEnabledCapabilities;
}

bool QgsOracleProvider::setSubsetString( const QString& theSQL, bool updateFeatureCount )
{
  if ( !mConnection )
    return false;

  QString prevWhere = mSqlWhereClause;

  mSqlWhereClause = theSQL.trimmed();

  QString sql = QString( "SELECT * FROM %1 WHERE " ).arg( mQuery );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += "(" + mSqlWhereClause + ") AND ";
  }

  sql += "1=0";

  QSqlQuery qry( *mConnection );
  if ( !exec( qry, sql ) )
  {
    pushError( qry.lastError().text() );
    mSqlWhereClause = prevWhere;
    qry.finish();
    return false;
  }
  qry.finish();

  if ( mPrimaryKeyType == pktInt && !uniqueData( mQuery, mAttributeFields[ mPrimaryKeyAttrs[0] ].name() ) )
  {
    mSqlWhereClause = prevWhere;
    return false;
  }

  // Update datasource uri too
  mUri.setSql( theSQL );
  // Update yet another copy of the uri. Why are there 3 copies of the
  // uri? Perhaps this needs some rationalisation.....
  setDataSourceUri( mUri.uri() );

  if ( updateFeatureCount )
  {
    mFeaturesCounted = -1;
  }
  mLayerExtent.setMinimal();

  emit dataChanged();

  return true;
}

/**
 * Return the feature count
 */
long QgsOracleProvider::featureCount() const
{
  if ( mFeaturesCounted >= 0 || !mConnection )
    return mFeaturesCounted;

  // get total number of features
  QString sql;

  // use estimated metadata even when there is a where clause,
  // although we get an incorrect feature count for the subset
  // - but make huge dataset usable.
  if ( !mIsQuery && mUseEstimatedMetadata )
  {
    sql = QString( "SELECT num_rows FROM all_tables WHERE owner=%1 AND table_name=%2" )
          .arg( quotedValue( mOwnerName ) )
          .arg( quotedValue( mTableName ) );
  }
  else
  {
    sql = QString( "SELECT count(*) FROM %1" ).arg( mQuery );

    if ( !mSqlWhereClause.isEmpty() )
    {
      sql += " WHERE " + mSqlWhereClause;
    }
  }

  QSqlQuery qry( *mConnection );
  if ( exec( qry, sql ) && qry.next() )
  {
    mFeaturesCounted = qry.value( 0 ).toInt();
  }
  qry.finish();

  QgsDebugMsg( "number of features: " + QString::number( mFeaturesCounted ) );

  return mFeaturesCounted;
}

QgsRectangle QgsOracleProvider::extent()
{
  if ( mGeometryColumn.isNull() || !mConnection )
    return QgsRectangle();

  if ( mLayerExtent.isEmpty() )
  {
    QString sql;
    QSqlQuery qry( *mConnection );
    bool ok = false;

    if ( !mIsQuery )
    {
      if ( mUseEstimatedMetadata )
      {
        if ( exec( qry, QString( "SELECT sdo_lb,sdo_ub FROM mdsys.all_sdo_geom_metadata m, table(m.diminfo) WHERE owner=%1 AND table_name=%2 AND column_name=%3 AND sdo_dimname='X'" )
                   .arg( quotedValue( mOwnerName ) )
                   .arg( quotedValue( mTableName ) )
                   .arg( quotedValue( mGeometryColumn ) ) ) && qry.next() )
        {
          mLayerExtent.setXMinimum( qry.value( 0 ).toDouble() );
          mLayerExtent.setXMaximum( qry.value( 1 ).toDouble() );

          if ( exec( qry, QString( "SELECT sdo_lb,sdo_ub FROM mdsys.all_sdo_geom_metadata m, table(m.diminfo) WHERE owner=%1 AND table_name=%2 AND column_name=%3 AND sdo_dimname='Y'" )
                     .arg( quotedValue( mOwnerName ) )
                     .arg( quotedValue( mTableName ) )
                     .arg( quotedValue( mGeometryColumn ) ) )  && qry.next() )
          {
            mLayerExtent.setYMinimum( qry.value( 0 ).toDouble() );
            mLayerExtent.setYMaximum( qry.value( 1 ).toDouble() );
            return mLayerExtent;
          }
        }
      }

      if ( mHasSpatialIndex && ( mUseEstimatedMetadata || mSqlWhereClause.isEmpty() ) )
      {
        sql = QString( "SELECT SDO_TUNE.EXTENT_OF(%1,%2) FROM dual" )
              .arg( quotedValue( QString( "%1.%2" ).arg( mOwnerName ).arg( mTableName ) ) )
              .arg( quotedValue( mGeometryColumn ) );

        ok = exec( qry, sql );
      }
    }

    if ( !ok )
    {
      sql = QString( "SELECT SDO_AGGR_MBR(%1) FROM %2" ).arg( quotedIdentifier( mGeometryColumn ) ).arg( mQuery );

      if ( !mSqlWhereClause.isEmpty() )
        sql += QString( " WHERE %1" ).arg( mSqlWhereClause );

      ok = exec( qry, sql );
    }

    if ( ok && qry.next() )
    {
      QByteArray *ba = static_cast<QByteArray*>( qry.value( 0 ).data() );
      unsigned char *copy = new unsigned char[ba->size()];
      memcpy( copy, ba->constData(), ba->size() );
      QgsGeometry g;
      g.fromWkb( copy, ba->size() ); // take ownership
      mLayerExtent = g.boundingBox();
      QgsDebugMsg( "extent: " + mLayerExtent.toString() );
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Could not retrieve extents: %1\nSQL: %2" ).arg( qry.lastError().text() ).arg( qry.lastQuery() ), tr( "Oracle" ) );
    }
  }

  return mLayerExtent;
}

void QgsOracleProvider::updateExtents()
{
  mLayerExtent.setMinimal();
}

bool QgsOracleProvider::getGeometryDetails()
{
  if ( mGeometryColumn.isNull() )
  {
    mDetectedGeomType = QGis::WKBNoGeometry;
    mValid = true;
    return true;
  }

  QString ownerName = mOwnerName;
  QString tableName = mTableName;
  QString geomCol = mGeometryColumn;

  QSqlQuery qry( *mConnection );
  if ( mIsQuery )
  {
    if ( !exec( qry, QString( "SELECT %1 FROM %2 WHERE 1=0" ).arg( quotedIdentifier( mGeometryColumn ) ).arg( mQuery ) ) )
    {
      QgsMessageLog::logMessage( tr( "Could not execute query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                                 .arg( qry.lastError().text() )
                                 .arg( qry.lastQuery() ), tr( "Oracle" ) );
      mValid = false;
      return false;
    }

    ownerName = "";
    tableName = mQuery;
  }

  int detectedSrid = -1;
  QGis::WkbType detectedType = QGis::WKBUnknown;
  mHasSpatialIndex = false;

  if ( mIsQuery )
  {
    detectedSrid = mSrid;
    detectedType = mRequestedGeomType;
  }

  if ( !ownerName.isEmpty() )
  {
    if ( exec( qry, QString( "SELECT srid FROM mdsys.all_sdo_geom_metadata WHERE owner=%1 AND table_name=%2 AND column_name=%3" )
               .arg( quotedValue( ownerName ) )
               .arg( quotedValue( tableName ) )
               .arg( quotedValue( geomCol ) ) ) )
    {
      if ( qry.next() )
      {
        detectedSrid = qry.value( 0 ).toInt();
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Could not retrieve SRID of %1.\nThe error message from the database was:\n%2.\nSQL: %3" )
                                   .arg( mQuery )
                                   .arg( qry.lastError().text() )
                                   .arg( qry.lastQuery() ), tr( "Oracle" ) );
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Could not determine SRID of %1.\nThe error message from the database was:\n%2.\nSQL: %3" )
                                 .arg( mQuery )
                                 .arg( qry.lastError().text() )
                                 .arg( qry.lastQuery() ), tr( "Oracle" ) );
    }

    if ( exec( qry, QString( mUseEstimatedMetadata
                             ?  "SELECT DISTINCT gtype FROM (SELECT t.%1.sdo_gtype AS gtype FROM %2 t WHERE t.%1 IS NOT NULL AND rownum<1000) WHERE rownum<=2"
                             :  "SELECT DISTINCT t.%1.sdo_gtype FROM %2 t WHERE t.%1 IS NOT NULL AND rownum<=2" ).arg( quotedIdentifier( geomCol ) ).arg( mQuery ) ) )
    {
      if ( qry.next() )
      {
        detectedType = QgsOracleConn::wkbTypeFromDatabase( qry.value( 0 ).toInt() );
        if ( qry.next() )
        {
          detectedType = QGis::WKBUnknown;
        }
      }
      else
      {
        detectedType = QGis::WKBUnknown;
        QgsMessageLog::logMessage( tr( "%1 has no valid geometry types.\nSQL: %2" )
                                   .arg( mQuery )
                                   .arg( qry.lastQuery() ), tr( "Oracle" ) );
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Could not determine geometry type of %1.\nThe error message from the database was:\n%2.\nSQL: %3" )
                                 .arg( mQuery )
                                 .arg( qry.lastError().text() )
                                 .arg( qry.lastQuery() ), tr( "Oracle" ) );
    }
  }

  if ( detectedType == QGis::WKBUnknown || detectedSrid <= 0 )
  {
    QgsOracleLayerProperty layerProperty;

    if ( !mIsQuery )
    {
      layerProperty.ownerName = ownerName;
      layerProperty.tableName = tableName;
      layerProperty.geometryColName = mGeometryColumn;
      layerProperty.types << detectedType;
      layerProperty.srids << detectedSrid;

      QString delim = "";

      if ( !mSqlWhereClause.isEmpty() )
      {
        layerProperty.sql += delim + "(" + mSqlWhereClause + ")";
        delim = " AND ";
      }

      mConnection->retrieveLayerTypes( layerProperty, mUseEstimatedMetadata, false );

      Q_ASSERT( layerProperty.types.size() == layerProperty.srids.size() );
    }

    if ( layerProperty.types.isEmpty() )
    {
      // no data - so take what's requested
      if ( mRequestedGeomType == QGis::WKBUnknown )
      {
        QgsMessageLog::logMessage( tr( "Geometry type and srid for empty column %1 of %2 undefined." ).arg( mGeometryColumn ).arg( mQuery ) );
      }

      detectedType = QGis::WKBUnknown;
      detectedSrid = -1;
    }
    else
    {
      // requested type && srid is available
      if ( mRequestedGeomType == QGis::WKBUnknown || layerProperty.types.contains( mRequestedGeomType ) )
      {
        if ( layerProperty.size() == 1 )
        {
          // only what we requested is available
          detectedType = layerProperty.types.at( 0 );
          detectedSrid = layerProperty.srids.at( 0 );
        }
        else
        {
          // we need to filter
          detectedType = QGis::WKBUnknown;
          detectedSrid = -1;
        }
      }
      else
      {
        // geometry type undetermined or not unrequested
        QgsMessageLog::logMessage( tr( "Feature type or srid for %1 of %2 could not be determined or was not requested." ).arg( mGeometryColumn ).arg( mQuery ) );
        detectedType = QGis::WKBUnknown;
        detectedSrid = -1;
      }
    }
  }

  mDetectedGeomType = detectedType;
  if ( detectedSrid != -1 )
    mSrid = detectedSrid;

  QgsDebugMsg( QString( "Detected Oracle SRID is %1" ).arg( mSrid ) );
  QgsDebugMsg( QString( "Detected type is %1" ).arg( mDetectedGeomType ) );
  QgsDebugMsg( QString( "Requested type is %1" ).arg( mRequestedGeomType ) );

  mValid = ( mDetectedGeomType != QGis::WKBUnknown || mRequestedGeomType != QGis::WKBUnknown );

  if ( !mValid )
    return false;


  // store whether the geometry includes measure value
  if ( detectedType == QGis::WKBPoint25D || detectedType == QGis::WKBMultiPoint25D ||
       detectedType == QGis::WKBLineString25D || detectedType == QGis::WKBMultiLineString25D ||
       detectedType == QGis::WKBPolygon25D || detectedType == QGis::WKBMultiPolygon25D )
  {
    // explicitly disable adding new features and editing of geometries
    // as this would lead to corruption of measures
    QgsMessageLog::logMessage( tr( "Editing and adding disabled for 2D+ layer (%1; %2)" ).arg( mGeometryColumn ).arg( mQuery ) );
    mEnabledCapabilities &= ~( QgsVectorDataProvider::ChangeGeometries | QgsVectorDataProvider::AddFeatures );
  }

  QgsDebugMsg( QString( "Feature type name is %1" ).arg( QGis::featureType( geometryType() ) ) );

  return mValid;
}

bool QgsOracleProvider::createSpatialIndex()
{
  if ( !mConnection )
    return false;

  QSqlQuery qry( *mConnection );

  if ( !crs().geographicFlag() )
  {
    // TODO: make precision configurable
    QgsRectangle r( extent() );
    if ( !exec( qry, QString( "UPDATE mdsys.user_sdo_geom_metadata SET diminfo=mdsys.sdo_dim_array("
                              "mdsys.sdo_dim_element('X', %1, %2, 0.001),"
                              "mdsys.sdo_dim_element('Y', %3, %4, 0.001)"
                              ") WHERE table_name=%5 AND column_name=%6" )
                .arg( qgsDoubleToString( r.xMinimum() ) ).arg( qgsDoubleToString( r.xMaximum() ) )
                .arg( qgsDoubleToString( r.yMinimum() ) ).arg( qgsDoubleToString( r.yMaximum() ) )
                .arg( quotedValue( mTableName ) )
                .arg( quotedValue( mGeometryColumn ) ) )
       )
    {
      QgsMessageLog::logMessage( tr( "Could not update metadata for %1.%2.\nSQL:%3\nError: %4" )
                                 .arg( mTableName )
                                 .arg( mGeometryColumn )
                                 .arg( qry.lastQuery() )
                                 .arg( qry.lastError().text() ),
                                 tr( "Oracle" ) );
      return false;
    }

    if ( qry.numRowsAffected() == 0 )
    {
      if ( !exec( qry, QString( "INSERT INTO mdsys.user_sdo_geom_metadata(table_name,column_name,srid,diminfo) VALUES (%1,%2,%3,mdsys.sdo_dim_array("
                                "mdsys.sdo_dim_element('X', %4, %5, 0.001),"
                                "mdsys.sdo_dim_element('Y', %6, %7, 0.001)"
                                "))" )
                  .arg( quotedValue( mTableName ) )
                  .arg( quotedValue( mGeometryColumn ) )
                  .arg( mSrid < 1 ? "NULL" : QString::number( mSrid ) )
                  .arg( qgsDoubleToString( r.xMinimum() ) ).arg( qgsDoubleToString( r.xMaximum() ) )
                  .arg( qgsDoubleToString( r.yMinimum() ) ).arg( qgsDoubleToString( r.yMaximum() ) )
                ) )
      {
        QgsMessageLog::logMessage( tr( "Could not insert metadata for %1.%2.\nSQL:%3\nError: %4" )
                                   .arg( quotedValue( mTableName ) )
                                   .arg( quotedValue( mGeometryColumn ) )
                                   .arg( qry.lastQuery() )
                                   .arg( qry.lastError().text() ),
                                   tr( "Oracle" ) );
        return false;
      }
    }
  }
  else
  {
    QgsDebugMsg( "geographic CRS" );
  }

  if ( !mHasSpatialIndex )
  {
    int n = 0;
    if ( exec( qry, QString( "SELECT coalesce(substr(max(index_name),10),'0') FROM all_indexes WHERE index_name LIKE 'QGIS_IDX_%' ESCAPE '#' ORDER BY index_name" ) ) &&
         qry.next() )
    {
      n = qry.value( 0 ).toInt() + 1;
    }

    if ( !exec( qry, QString( "CREATE INDEX QGIS_IDX_%1 ON %2.%3(%4) INDEXTYPE IS MDSYS.SPATIAL_INDEX PARALLEL" )
                .arg( n, 10, 10, QChar( '0' ) )
                .arg( quotedIdentifier( mOwnerName ) )
                .arg( quotedIdentifier( mTableName ) )
                .arg( quotedIdentifier( mGeometryColumn ) ) ) )
    {
      QgsMessageLog::logMessage( tr( "Creation spatial index failed.\nSQL:%1\nError: %2" )
                                 .arg( qry.lastQuery() )
                                 .arg( qry.lastError().text() ),
                                 tr( "Oracle" ) );
      return false;
    }

    mSpatialIndexName = QString( "QGIS_IDX_%1" ).arg( n, 10, 10, QChar( '0' ) );
  }
  else
  {
    if ( !exec( qry, QString( "ALTER INDEX %1 REBUILD" ).arg( mSpatialIndexName ) ) )
    {
      QgsMessageLog::logMessage( tr( "Rebuild of spatial index failed.\nSQL:%1\nError: %2" )
                                 .arg( qry.lastQuery() )
                                 .arg( qry.lastError().text() ),
                                 tr( "Oracle" ) );
      return false;
    }
  }

  return true;
}

bool QgsOracleProvider::convertField( QgsField &field )
{
  QString fieldType = "VARCHAR2(2047)"; //default to string
  int fieldSize = field.length();
  int fieldPrec = field.precision();
  switch ( field.type() )
  {
    case QVariant::LongLong:
      fieldType = "NUMBER(20,0)";
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::DateTime:
      fieldType = "TIMESTAMP";
      fieldPrec = -1;
      break;


    case QVariant::Time:
    case QVariant::String:
      fieldType = "VARCHAR2(2047)";
      fieldPrec = -1;
      break;

    case QVariant::Date:
      fieldType = "DATE";
      fieldPrec = -1;
      break;

    case QVariant::Int:
      fieldType = "NUMBER(10,0)";
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::Double:
      if ( fieldSize <= 0 || fieldPrec <= 0 )
      {
        fieldType = "BINARY_DOUBLE";
        fieldSize = -1;
        fieldPrec = -1;
      }
      else
      {
        fieldType = QString( "NUMBER(%1,%2)" ).arg( fieldSize ).arg( fieldPrec );
      }
      break;

    default:
      return false;
  }

  field.setTypeName( fieldType );
  field.setLength( fieldSize );
  field.setPrecision( fieldPrec );
  return true;
}

QgsVectorLayerImport::ImportError QgsOracleProvider::createEmptyLayer(
  const QString& uri,
  const QgsFields &fields,
  QGis::WkbType wkbType,
  const QgsCoordinateReferenceSystem *srs,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdxMap,
  QString *errorMessage,
  const QMap<QString, QVariant> *options )
{
  Q_UNUSED( wkbType );
  Q_UNUSED( options );

  // populate members from the uri structure
  QgsDataSourceURI dsUri( uri );
  QString ownerName = dsUri.schema();

  QgsDebugMsg( QString( "Connection info is: %1" ).arg( dsUri.connectionInfo() ) );

  // create the table
  QgsOracleConn *conn = QgsOracleConn::connectDb( dsUri );
  if ( !conn )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Connection to database failed" );
    return QgsVectorLayerImport::ErrConnectionFailed;
  }

  if ( ownerName.isEmpty() )
  {
    ownerName = conn->currentUser();
  }

  if ( ownerName.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "No owner name found" );
    return QgsVectorLayerImport::ErrInvalidLayer;
  }

  QString tableName = dsUri.table();
  QString geometryColumn = dsUri.geometryColumn();

  QString primaryKey = dsUri.keyColumn();
  QString primaryKeyType;

  QString ownerTableName = quotedIdentifier( ownerName ) + "." + quotedIdentifier( tableName );

  QgsDebugMsg( QString( "Geometry column is: %1" ).arg( geometryColumn ) );
  QgsDebugMsg( QString( "Owner is: %1" ).arg( ownerName ) );
  QgsDebugMsg( QString( "Table name is: %1" ).arg( tableName ) );

  // get the pk's name and type

  // if no pk name was passed, define the new pk field name
  if ( primaryKey.isEmpty() )
  {
    int index = 0;
    QString pk = primaryKey = "id";
    while ( fields.indexFromName( primaryKey ) >= 0 )
    {
      primaryKey = QString( "%1_%2" ).arg( pk ).arg( index++ );
    }
  }
  else
  {
    int idx = fields.indexFromName( primaryKey );
    if ( idx >= 0 )
    {
      QgsField fld = fields[idx];
      if ( convertField( fld ) )
      {
        primaryKeyType = fld.typeName();
      }
    }
  }

  QSqlDatabase db( *conn );
  QSqlQuery qry( db );
  bool created = false;
  try
  {
    if ( !db.transaction() )
    {
      throw OracleException( tr( "Could not start transaction" ), db );
    }

    if ( !exec( qry, QString( "SELECT 1 FROM all_tables WHERE owner=%1 AND table_name=%2" )
                .arg( quotedValue( ownerName ) )
                .arg( quotedValue( tableName ) )
              ) )
    {
      throw OracleException( tr( "Could not determine table existence." ), qry );
    }

    bool exists = qry.next();

    if ( exists )
    {
      if ( overwrite )
      {
        // delete the table if exists, then re-create it
        if ( !exec( qry, QString( "DROP TABLE %1" ).arg( ownerTableName ) ) )
        {
          throw OracleException( tr( "Table %1 could not be dropped." ).arg( ownerTableName ), qry );
        }
      }
      else
      {
        throw OracleException( tr( "Table %1 already exists." ).arg( ownerTableName ), qry );
      }
    }

    QString sql = QString( "CREATE TABLE %1(" ).arg( ownerTableName );
    QString delim;

    if ( !primaryKey.isEmpty() && !primaryKeyType.isEmpty() )
    {
      sql += QString( "%1 %2 PRIMARY KEY" ).arg( quotedIdentifier( primaryKey ) ).arg( primaryKeyType );
      delim = ",";
    }

    // create geometry column
    sql += QString( "%1%2 MDSYS.SDO_GEOMETRY)" ).arg( delim ).arg( quotedIdentifier( geometryColumn ) );
    delim = ",";

    if ( !exec( qry, sql ) )
    {
      throw OracleException( tr( "Table creation failed." ), qry );
    }

    created = true;

    // TODO: make precision configurable
    QString diminfo;
    if ( srs->geographicFlag() )
    {
      diminfo = "mdsys.sdo_dim_array("
                "mdsys.sdo_dim_element('Longitude', -180, 180, 0.001),"
                "mdsys.sdo_dim_element('Latitude', -90, 90, 0.001)"
                ")";
    }
    else
    {
      diminfo = "mdsys.sdo_dim_array("
                "mdsys.sdo_dim_element('X', NULL, NULL, 0.001),"
                "mdsys.sdo_dim_element('Y', NULL, NULL, 0.001)"
                ")";
    }

    int srid = 0;
    QStringList parts = srs->authid().split( ":" );
    if ( parts.size() == 2 )
    {
      // apparently some EPSG codes don't have the auth_name setup in cs_srs
      if ( !exec( qry, QString( "SELECT srid FROM mdsys.cs_srs WHERE coalesce(auth_name,'EPSG')=%1 AND auth_srid=%2" )
                  .arg( quotedValue( parts[0] ) )
                  .arg( quotedValue( parts[1] ) ) ) )
      {
        throw OracleException( tr( "Could not lookup authid %1:%2" ).arg( parts[0] ).arg( parts[1] ), qry );
      }

      if ( qry.next() )
      {
        srid = qry.value( 0 ).toInt();
      }
    }

    if ( srid == 0 )
    {
      QgsDebugMsg( QString( "%1:%2 not found in mdsys.cs_srs - trying WKT" ).arg( parts[0] ).arg( parts[1] ) );

      QString wkt = srs->toWkt();
      if ( !exec( qry, QString( "SELECT srid FROM mdsys.cs_srs WHERE wktext=%1" ).arg( quotedValue( wkt ) ) ) )
      {
        throw OracleException( tr( "Could not lookup WKT." ), qry );
      }

      if ( qry.next() )
      {
        srid = qry.value( 0 ).toInt();
      }
      else
      {
        if ( !exec( qry, "SELECT max(srid)+1 FROM sdo_coord_ref_system" ) || !qry.next() )
        {
          throw OracleException( tr( "Could not determine new srid." ), qry );
        }

        srid = qry.value( 0 ).toInt();

        QString sql;

        if ( !exec( qry, QString( "INSERT"
                                  " INTO sdo_coord_ref_system(srid,coord_ref_sys_name,coord_ref_sys_kind,legacy_wktext,is_valid,is_legacy,information_source)"
                                  " VALUES (%1,%2,%3,%4,'TRUE','TRUE','GDAL/OGR via QGIS')" )
                    .arg( srid )
                    .arg( quotedValue( srs->description() ) )
                    .arg( quotedValue( srs->geographicFlag() ? "GEOGRAPHIC2D" : "PROJECTED" ) )
                    .arg( quotedValue( wkt ) ) ) )
        {
          throw OracleException( tr( "CRS not found and could not be created." ), qry );
        }
      }
    }

    if ( !exec( qry, QString( "INSERT INTO mdsys.user_sdo_geom_metadata(table_name,column_name,srid,diminfo) VALUES (%1,%2,%3,%4)" )
                .arg( quotedValue( tableName.toUpper() ) )
                .arg( quotedValue( geometryColumn.toUpper() ) )
                .arg( srid )
                .arg( diminfo ) ) )
    {
      throw OracleException( tr( "Could not insert metadata." ), qry );
    }

    if ( !db.commit() )
    {
      QgsMessageLog::logMessage( tr( "Could not commit transaction" ), tr( "Oracle" ) );
    }
  }
  catch ( OracleException &e )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Creation of data source %1 failed: \n%2" )
                      .arg( ownerTableName )
                      .arg( e.errorMessage() );

    if ( db.rollback() )
    {
      QgsMessageLog::logMessage( tr( "Could not rollback transaction" ), tr( "Oracle" ) );
    }

    if ( created )
    {
      if ( !exec( qry, QString( "DROP TABLE %1" ).arg( ownerTableName ) ) )
      {
        QgsMessageLog::logMessage( tr( "Drop created table %1 failed.\nSQL:%2\nError: %3" )
                                   .arg( qry.lastQuery() )
                                   .arg( qry.lastError().text() ), tr( "Oracle" ) );
      }
    }

    conn->disconnect();

    return QgsVectorLayerImport::ErrCreateLayer;
  }

  conn->disconnect();

  QgsDebugMsg( QString( "layer %1 created" ).arg( ownerTableName ) );

  // use the provider to edit the table
  dsUri.setDataSource( ownerName, tableName, geometryColumn, QString(), primaryKey );
  QgsOracleProvider *provider = new QgsOracleProvider( dsUri.uri() );
  if ( !provider->isValid() )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Loading of the layer %1 failed" ).arg( ownerTableName );

    delete provider;
    return QgsVectorLayerImport::ErrInvalidLayer;
  }

  QgsDebugMsg( "layer loaded" );

  // add fields to the layer
  if ( oldToNewAttrIdxMap )
    oldToNewAttrIdxMap->clear();

  if ( fields.size() > 0 )
  {
    int offset = 0;

    QSet<QString> names;
    QList<QgsField> launderedFields;
    for ( int i = 0; i < fields.size(); i++ )
    {
      QgsField fld = fields[i];

      QString name = fld.name().left( 30 ).toUpper();

      if ( names.contains( name ) )
      {
        int j;
        int n;
        for ( j = 1, n = 10; j < 3; j++, n *= 10 )
        {
          int k;
          for ( k = 0; k < n && names.contains( name ); k++ )
          {
            name = QString( "%1%2" ).arg( name.left( 30 - j ) ).arg( k, j, 10, QChar( '0' ) );
          }

          if ( k < n )
            break;
        }

        if ( j == 3 )
        {
          if ( errorMessage )
            *errorMessage = QObject::tr( "Field name clash found (%1 not remappable)" ).arg( fld.name() );

          delete provider;
          return QgsVectorLayerImport::ErrAttributeTypeUnsupported;
        }
      }

      if ( fld.name() == geometryColumn )
        geometryColumn = name;

      fld.setName( name );
      launderedFields << fld;
      names << name;
    }

    // get the list of fields
    QList<QgsField> flist;
    for ( int i = 0; i < launderedFields.size(); i++ )
    {
      QgsField fld = launderedFields[i];

      if ( fld.name() == primaryKey )
      {
        oldToNewAttrIdxMap->insert( i, 0 );
        continue;
      }

      if ( fld.name() == geometryColumn )
      {
        QgsDebugMsg( "Found a field with the same name of the geometry column. Skip it!" );
        continue;
      }

      if ( !convertField( fld ) )
      {
        if ( errorMessage )
          *errorMessage = QObject::tr( "Unsupported type for field %1" ).arg( fld.name() );

        delete provider;
        return QgsVectorLayerImport::ErrAttributeTypeUnsupported;
      }

      QgsDebugMsg( QString( "Field #%1 name %2 type %3 typename %4 width %5 precision %6" )
                   .arg( i )
                   .arg( fld.name() ).arg( QVariant::typeToName( fld.type() ) ).arg( fld.typeName() )
                   .arg( fld.length() ).arg( fld.precision() ) );

      flist.append( fld );
      if ( oldToNewAttrIdxMap )
        oldToNewAttrIdxMap->insert( i, offset++ );
    }

    if ( !provider->addAttributes( flist ) )
    {
      if ( errorMessage )
        *errorMessage = QObject::tr( "Creation of fields failed" );

      delete provider;
      return QgsVectorLayerImport::ErrAttributeCreationFailed;
    }

    QgsDebugMsg( "Done creating fields" );
  }
  else
  {
    QgsDebugMsg( "No fields created." );
  }

  delete provider;

  return QgsVectorLayerImport::NoError;
}

QgsCoordinateReferenceSystem QgsOracleProvider::crs()
{
  QgsCoordinateReferenceSystem srs;

  if ( !mConnection )
    return srs;

  QSqlQuery qry( *mConnection );

  // apparently some EPSG codes don't have the auth_name setup in cs_srs
  if ( exec( qry, QString( "SELECT coalesce(auth_name,'EPSG'),auth_srid,wktext FROM mdsys.cs_srs WHERE srid=%1" ).arg( mSrid ) ) )
  {
    if ( qry.next() )
    {
      if ( qry.value( 0 ).toString() == "EPSG" )
      {
        srs.createFromOgcWmsCrs( QString( "EPSG:%1" ).arg( qry.value( 1 ).toString() ) );
      }
      else
      {
        srs.createFromWkt( qry.value( 2 ).toString() );
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Oracle SRID %1 not found." ).arg( mSrid ), tr( "Oracle" ) );
    }
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Lookup of Oracle SRID %1 failed.\nSQL:%2\nError:%3" )
                               .arg( mSrid )
                               .arg( qry.lastQuery() )
                               .arg( qry.lastError().text() ),
                               tr( "Oracle" ) );
  }

  return srs;
}

QString QgsOracleProvider::subsetString()
{
  return mSqlWhereClause;
}

QString QgsOracleProvider::getTableName()
{
  return mTableName;
}

size_t QgsOracleProvider::layerCount() const
{
  return 1;                   // XXX need to return actual number of layers
} // QgsOracleProvider::layerCount()


QString  QgsOracleProvider::name() const
{
  return ORACLE_KEY;
} //  QgsOracleProvider::name()

QString  QgsOracleProvider::description() const
{
  return ORACLE_DESCRIPTION;
} //  QgsOracleProvider::description()

/**
 * Class factory to return a pointer to a newly created
 * QgsOracleProvider object
 */
QGISEXTERN QgsOracleProvider * classFactory( const QString *uri )
{
  return new QgsOracleProvider( *uri );
}
/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return QSqlDatabase::isDriverAvailable( "QOCISPATIAL" ) ? ORACLE_KEY : 0;
}
/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return QSqlDatabase::isDriverAvailable( "QOCISPATIAL" ) ? ORACLE_DESCRIPTION : 0;
}
/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}

QGISEXTERN QgsOracleSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl )
{
  return new QgsOracleSourceSelect( parent, fl );
}

QGISEXTERN int dataCapabilities()
{
  return QgsDataProvider::Database;
}

QGISEXTERN QgsDataItem *dataItem( QString thePath, QgsDataItem *parentItem )
{
  Q_UNUSED( thePath );
  return new QgsOracleRootItem( parentItem, "Oracle", "oracle:" );
}

// ---------------------------------------------------------------------------

QGISEXTERN QgsVectorLayerImport::ImportError createEmptyLayer(
  const QString& uri,
  const QgsFields &fields,
  QGis::WkbType wkbType,
  const QgsCoordinateReferenceSystem *srs,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdxMap,
  QString *errorMessage,
  const QMap<QString, QVariant> *options )
{
  return QgsOracleProvider::createEmptyLayer(
           uri, fields, wkbType, srs, overwrite,
           oldToNewAttrIdxMap, errorMessage, options
         );
}

QGISEXTERN bool deleteLayer( const QString& uri, QString& errCause )
{
  QgsDebugMsg( "deleting layer " + uri );

  QgsDataSourceURI dsUri( uri );
  QString ownerName = dsUri.schema();
  QString tableName = dsUri.table();
  QString geometryCol = dsUri.geometryColumn();

  QgsOracleConn *conn = QgsOracleConn::connectDb( dsUri );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  if ( ownerName != conn->currentUser() )
  {
    errCause = QObject::tr( "%1 not owner of the table %2." )
               .arg( ownerName )
               .arg( tableName );
    conn->disconnect();
    return false;
  }

  QSqlQuery qry( *conn );

  // check the geometry column count
  if ( !QgsOracleProvider::exec( qry, QString( "SELECT count(*)"
                                 " FROM user_tab_columns"
                                 " WHERE table_name=%1 AND data_type='SDO_GEOMETRY' AND data_type_owner='MDSYS'" )
                                 .arg( QgsOracleConn::quotedValue( tableName ) ) )
       || !qry.next() )
  {
    errCause = QObject::tr( "Unable to determine number of geometry columns of layer %1.%2: \n%3" )
               .arg( ownerName )
               .arg( tableName )
               .arg( qry.lastError().text() );
    conn->disconnect();
    return false;
  }

  int count = qry.value( 0 ).toInt();

  QString dropTable;
  QString cleanView;
  if ( !geometryCol.isEmpty() && count > 1 )
  {
    // the table has more geometry columns, drop just the geometry column
    dropTable = QString( "ALTER TABLE %1 DROP COLUMN %2" )
                .arg( QgsOracleConn::quotedIdentifier( tableName ) )
                .arg( QgsOracleConn::quotedIdentifier( geometryCol ) );
    cleanView = QString( "DELETE FROM mdsys.user_sdo_geom_metadata WHERE table_name=%1 AND column_name=%2" )
                .arg( QgsOracleConn::quotedValue( tableName ) )
                .arg( QgsOracleConn::quotedValue( geometryCol ) );
  }
  else
  {
    // drop the table
    dropTable = QString( "DROP TABLE %1" )
                .arg( QgsOracleConn::quotedIdentifier( tableName ) );
    cleanView = QString( "DELETE FROM mdsys.user_sdo_geom_metadata WHERE table_name=%1" )
                .arg( QgsOracleConn::quotedValue( tableName ) );
  }

  if ( !QgsOracleProvider::exec( qry, dropTable ) )
  {
    errCause = QObject::tr( "Unable to delete layer %1.%2: \n%3" )
               .arg( ownerName )
               .arg( tableName )
               .arg( qry.lastError().text() );
    conn->disconnect();
    return false;
  }

  if ( !QgsOracleProvider::exec( qry, cleanView ) )
  {
    errCause = QObject::tr( "Unable to clean metadata %1.%2: \n%3" )
               .arg( ownerName )
               .arg( tableName )
               .arg( qry.lastError().text() );
    conn->disconnect();
    return false;
  }

  conn->disconnect();
  return true;
}

// ----------


QgsOracleSharedData::QgsOracleSharedData()
    : mFidCounter( 0 )
{
}

QgsFeatureId QgsOracleSharedData::lookupFid( const QVariant &v )
{
  QMutexLocker locker( &mMutex );

  QMap<QVariant, QgsFeatureId>::const_iterator it = mKeyToFid.find( v );

  if ( it != mKeyToFid.constEnd() )
  {
    return it.value();
  }

  mFidToKey.insert( ++mFidCounter, v );
  mKeyToFid.insert( v, mFidCounter );

  return mFidCounter;
}

QVariant QgsOracleSharedData::removeFid( QgsFeatureId fid )
{
  QMutexLocker locker( &mMutex );

  QVariant v = mFidToKey[ fid ];
  mFidToKey.remove( fid );
  mKeyToFid.remove( v );
  return v;
}

void QgsOracleSharedData::insertFid( QgsFeatureId fid, const QVariant& k )
{
  QMutexLocker locker( &mMutex );

  mFidToKey.insert( fid, k );
  mKeyToFid.insert( k, fid );
}

QVariant QgsOracleSharedData::lookupKey( QgsFeatureId featureId )
{
  QMutexLocker locker( &mMutex );

  QMap<QgsFeatureId, QVariant>::const_iterator it = mFidToKey.find( featureId );
  if ( it != mFidToKey.constEnd() )
    return it.value();
  return QVariant();
}

QGISEXTERN bool saveStyle( const QString &uri,
                           const QString &qmlStyle,
                           const QString &sldStyle,
                           const QString &styleName,
                           const QString &styleDescription,
                           const QString &uiFileContent,
                           bool useAsDefault,
                           QString &errCause )
{
  QgsDataSourceURI dsUri( uri );

  QgsOracleConn *conn = QgsOracleConn::connectDb( dsUri );
  if ( !conn )
  {
    errCause = QObject::tr( "Could not connect to database" );
    return false;
  }

  QSqlQuery qry = QSqlQuery( *conn );
  if ( !qry.exec( "SELECT COUNT(*) FROM user_tables WHERE table_name='LAYER_STYLES'" ) || !qry.next() )
  {
    errCause = QObject::tr( "Unable to check layer style existence [%1]" ).arg( qry.lastError().text() );
    conn->disconnect();
    return false;
  }
  else if ( qry.value( 0 ).toInt() == 0 )
  {
    QgsDebugMsg( "Creating layer style table." );

    if ( !qry.exec( "CREATE TABLE layer_styles("
                    "id INTEGER PRIMARY KEY,"
                    "f_table_catalog VARCHAR2(30) NOT NULL,"
                    "f_table_schema VARCHAR2(30) NOT NULL,"
                    "f_table_name VARCHAR2(30) NOT NULL,"
                    "f_geometry_column VARCHAR2(30) NOT NULL,"
                    "stylename VARCHAR2(2047),"
                    "styleqml CLOB,"
                    "stylesld CLOB,"
                    "useasdefault INTEGER,"
                    "description VARCHAR2(2047),"
                    "owner VARCHAR2(30),"
                    "ui CLOB,"
                    "update_time timestamp"
                    ")" ) )
    {
      errCause = QObject::tr( "Unable to create layer style table [%1]" ).arg( qry.lastError().text() );
      conn->disconnect();
      return false;
    }
  }

  int id;
  QString sql;

  sql = QString( "SELECT id,stylename"
                 " FROM layer_styles"
                 " WHERE f_table_catalog=%1"
                 " AND f_table_schema=%2"
                 " AND f_table_name=%3"
                 " AND f_geometry_column=%4"
                 " AND styleName=%5" )
        .arg( QgsOracleConn::quotedValue( dsUri.database() ) )
        .arg( QgsOracleConn::quotedValue( dsUri.schema() ) )
        .arg( QgsOracleConn::quotedValue( dsUri.table() ) )
        .arg( QgsOracleConn::quotedValue( dsUri.geometryColumn() ) )
        .arg( QgsOracleConn::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) );

  if ( !qry.exec( sql ) )
  {
    errCause = QObject::tr( "Unable to check style existence [%1]" ).arg( qry.lastError().text() );
    conn->disconnect();
    return false;
  }
  else if ( qry.next() )
  {
    if ( QMessageBox::question( nullptr, QObject::tr( "Save style in database" ),
                                QObject::tr( "A style named \"%1\" already exists in the database for this layer. Do you want to overwrite it?" )
                                .arg( styleName.isEmpty() ? dsUri.table() : styleName ),
                                QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
    {
      errCause = QObject::tr( "Operation aborted. No changes were made in the database" );
      conn->disconnect();
      return false;
    }

    id = qry.value( 0 ).toInt();

    sql = QString( "UPDATE layer_styles"
                   " SET update_time=(select current_timestamp from dual),"
                   "f_table_catalog=?,"
                   "f_table_schema=?,"
                   "f_table_name=?,"
                   "f_geometry_column=?,"
                   "styleName=?,"
                   "styleQML=?,"
                   "styleSLD=?,"
                   "useAsDefault=?,"
                   "description=?,"
                   "owner=?"
                   "%1"
                   " WHERE id=%2" )
          .arg( uiFileContent.isEmpty() ? "" : ",ui=?" )
          .arg( id );
  }
  else if ( qry.exec( "select coalesce(max(id)+1,0) FROM layer_styles" ) && qry.next() )
  {
    id = qry.value( 0 ).toInt();

    sql = QString( "INSERT INTO layer_styles("
                   "id,update_time,f_table_catalog,f_table_schema,f_table_name,f_geometry_column,styleName,styleQML,styleSLD,useAsDefault,description,owner%1"
                   ") VALUES ("
                   "%2,"
                   "(select current_timestamp from dual)"
                   "%3"
                   ")" )
          .arg( uiFileContent.isEmpty() ? "" : ",ui" )
          .arg( id )
          .arg( QString( ",?" ).repeated( uiFileContent.isEmpty() ? 10 : 11 ) );
  }
  else
  {
    errCause = QObject::tr( "Cannot fetch new layer style id." );
    conn->disconnect();
    return false;
  }

  if ( !qry.prepare( sql ) )
  {
    errCause = QObject::tr( "Could not prepare insert/update [%1]" ).arg( qry.lastError().text() );
    QgsDebugMsg( "prepare insert/update failed" );
    conn->disconnect();
    return false;
  }

  qry.addBindValue( dsUri.database() );
  qry.addBindValue( dsUri.schema() );
  qry.addBindValue( dsUri.table() );
  qry.addBindValue( dsUri.geometryColumn() );
  qry.addBindValue( styleName.isEmpty() ? dsUri.table() : styleName );
  qry.addBindValue( qmlStyle );
  qry.addBindValue( sldStyle );
  qry.addBindValue( useAsDefault ? 1 : 0 );
  qry.addBindValue( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription );
  qry.addBindValue( dsUri.username() );
  if ( !uiFileContent.isEmpty() )
    qry.addBindValue( uiFileContent );

  if ( !qry.exec() )
  {
    errCause = QObject::tr( "Could not execute insert/update [%1]" ).arg( qry.lastError().text() );
    QgsDebugMsg( "execute insert/update failed" );
    conn->disconnect();
    return false;
  }

  if ( useAsDefault )
  {
    if ( !qry.exec( QString( "UPDATE layer_styles"
                             " SET useasdefault=0,update_time=(select current_timestamp from dual)"
                             " WHERE f_table_catalog=%1"
                             " AND f_table_schema=%2"
                             " AND f_table_name=%3"
                             " AND f_geometry_column=%4"
                             " AND id<>%5" )
                    .arg( QgsOracleConn::quotedValue( dsUri.database() ) )
                    .arg( QgsOracleConn::quotedValue( dsUri.schema() ) )
                    .arg( QgsOracleConn::quotedValue( dsUri.table() ) )
                    .arg( QgsOracleConn::quotedValue( dsUri.geometryColumn() ) )
                    .arg( id ) ) )
    {
      errCause = QObject::tr( "Could not reset default status [%1]" ).arg( qry.lastError().text() );
      QgsDebugMsg( "execute update failed" );
      conn->disconnect();
      return false;
    }
  }

  conn->disconnect();

  return true;
}

QGISEXTERN QString loadStyle( const QString &uri, QString &errCause )
{
  QgsDataSourceURI dsUri( uri );

  QgsOracleConn *conn = QgsOracleConn::connectDb( dsUri );
  if ( !conn )
  {
    errCause = QObject::tr( "Could not connect to database" );
    return QString::null;
  }

  QSqlQuery qry( *conn );

  QString style;
  if ( !qry.exec( "SELECT COUNT(*) FROM user_tables WHERE table_name='LAYER_STYLES'" ) || !qry.next() || qry.value( 0 ).toInt() == 0 )
  {
    errCause = QObject::tr( "Unable to find layer style table [%1]" ).arg( qry.lastError().text() );
    conn->disconnect();
    return QString::null;
  }
  else if ( !qry.exec( QString( "SELECT styleQML FROM ("
                                "SELECT styleQML"
                                " FROM layer_styles"
                                " WHERE f_table_catalog=%1"
                                " AND f_table_schema=%2"
                                " AND f_table_name=%3"
                                " AND f_geometry_column=%4"
                                " ORDER BY useAsDefault DESC"
                                ") WHERE rownum=1" )
                       .arg( QgsOracleConn::quotedValue( dsUri.database() ) )
                       .arg( QgsOracleConn::quotedValue( dsUri.schema() ) )
                       .arg( QgsOracleConn::quotedValue( dsUri.table() ) )
                       .arg( QgsOracleConn::quotedValue( dsUri.geometryColumn() ) ) ) )
  {
    errCause = QObject::tr( "Could not retrieve style [%1]" ).arg( qry.lastError().text() );
  }
  else if ( !qry.next() )
  {
    errCause = QObject::tr( "Style not found" );
  }
  else
  {
    style = qry.value( 0 ).toString();
  }

  conn->disconnect();

  return style;
}

QGISEXTERN int listStyles( const QString &uri,
                           QStringList &ids,
                           QStringList &names,
                           QStringList &descriptions,
                           QString &errCause )
{
  QgsDataSourceURI dsUri( uri );

  QgsOracleConn *conn = QgsOracleConn::connectDb( dsUri );
  if ( !conn )
  {
    errCause = QObject::tr( "Could not connect to database" );
    return -1;
  }

  QSqlQuery qry( *conn );

  int res = -1;
  if ( !qry.exec( "SELECT count(*) FROM user_tables WHERE table_name='LAYER_STYLES'" ) || !qry.next() )
  {
    errCause = QObject::tr( "Could not verify existence of layer style table [%1]" ).arg( qry.lastError().text() );
  }
  else if ( qry.value( 0 ).toInt() == 0 )
  {
    errCause = QObject::tr( "Layer style table does not exist [%1]" ).arg( qry.value( 0 ).toString() );
  }
  else
  {
    if ( !qry.exec( QString( "SELECT id,styleName,description FROM layer_styles WHERE f_table_catalog=%1 AND f_table_schema=%2 AND f_table_name=%3 AND f_geometry_column=%4" )
                    .arg( QgsOracleConn::quotedValue( dsUri.database() ) )
                    .arg( QgsOracleConn::quotedValue( dsUri.schema() ) )
                    .arg( QgsOracleConn::quotedValue( dsUri.table() ) )
                    .arg( QgsOracleConn::quotedValue( dsUri.geometryColumn() ) ) ) )
    {
      errCause = QObject::tr( "No style for layer found" );
    }
    else
    {
      res = 0;
      while ( qry.next() )
      {
        ids << qry.value( 0 ).toString();
        names << qry.value( 1 ).toString();
        descriptions << qry.value( 2 ).toString();
        res++;
      }

      qry.finish();

      if ( qry.exec( QString( "SELECT id,styleName,description FROM layer_styles WHERE NOT (f_table_catalog=%1 AND f_table_schema=%2 AND f_table_name=%3 AND f_geometry_column=%4) ORDER BY update_time DESC" )
                     .arg( QgsOracleConn::quotedValue( dsUri.database() ) )
                     .arg( QgsOracleConn::quotedValue( dsUri.schema() ) )
                     .arg( QgsOracleConn::quotedValue( dsUri.table() ) )
                     .arg( QgsOracleConn::quotedValue( dsUri.geometryColumn() ) ) ) )
      {
        while ( qry.next() )
        {
          ids << qry.value( 0 ).toString();
          names << qry.value( 1 ).toString();
          descriptions << qry.value( 2 ).toString();
        }
      }
    }
  }

  conn->disconnect();

  return res;
}

QGISEXTERN QString getStyleById( const QString& uri, QString styleId, QString& errCause )
{
  QString style;
  QgsDataSourceURI dsUri( uri );

  QgsOracleConn *conn = QgsOracleConn::connectDb( dsUri );
  if ( !conn )
  {
    errCause = QObject::tr( "Could not connect to database" );
    return style;
  }

  QSqlQuery qry( *conn );

  if ( !qry.exec( QString( "SELECT styleQml FROM layer_styles WHERE id=%1" ).arg( QgsOracleConn::quotedValue( styleId ) ) ) )
  {
    errCause = QObject::tr( "Could not load layer style table [%1]" ).arg( qry.lastError().text() );
  }
  else if ( !qry.next() )
  {
    errCause = QObject::tr( "No styles found in layer table [%1]" ).arg( qry.lastError().text() );
  }
  else
  {
    style = qry.value( 0 ).toString();
  }

  conn->disconnect();

  return style;
}

// vim: set sw=2 :
