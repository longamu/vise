/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

==== Copyright:

The library belongs to Relja Arandjelovic and the University of Oxford.
No usage or redistribution is allowed without explicit permission.
*/

#include "dataset.h"

#include <iostream>
#include <stdio.h>
#include <stdexcept>

#include <cstring>

#include "util.h"



dataset::dataset( const char fileName[], const char aPrefix[], std::string aFindPath, std::string aReplacePath ) : prefix(aPrefix), findPath(aFindPath), replacePath(aReplacePath) {
    
    int db_rc;
    
    static bool isConfigured= false;
    
    if (!isConfigured){
        db_rc= sqlite3_config( SQLITE_CONFIG_SERIALIZED ); //sqlite3 worries about mutexs, but maybe slow?
        if( db_rc ){
            std::cerr<< "SQLITE error: Can't config SQLITE_CONFIG_SERIALIZED: "<< sqlite3_errmsg(db) << "\n";
            throw std::runtime_error( "Can't open database" );
        }
        isConfigured= true;
    }
    
    int threadSafe= sqlite3_threadsafe();
    if (threadSafe==0){
        std::cout<<"\n\n\tsqlite3 not thread safe!\n\n";
    }
    
    db_rc = sqlite3_open_v2( fileName, &db, SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_PRIVATECACHE, NULL );
    if( db_rc ){
        std::cerr<< "SQLITE error: Can't open database: "<< sqlite3_errmsg(db) << "\n";
        sqlite3_close(db);
        throw std::runtime_error( "Can't open database" );
    }
    
}



dataset::~dataset(){
    sqlite3_close(db);
}



uint32_t
dataset::getNumDoc(){
    
    sqlite3_stmt *pStmt_numDoc;
    char getNumDoc_text[200]; sprintf( getNumDoc_text, "select count(*) from %s", prefix );
    sqlite3_prepare_v2( db, getNumDoc_text, -1, &pStmt_numDoc, 0);
    
    sqlite3_step( pStmt_numDoc );
    
    uint32_t numDoc= sqlite3_column_int(pStmt_numDoc, 0);
    sqlite3_finalize( pStmt_numDoc );
    
    return numDoc;
}



uint32_t
dataset::getDocID( const char hash[] ){
    
    sqlite3_stmt *pStmt_docID;
    char getDocID_text[200]; sprintf( getDocID_text, "select rowid from %s where hash=?", prefix );
    sqlite3_prepare_v2( db, getDocID_text, -1, &pStmt_docID, 0);
    
    sqlite3_bind_text( pStmt_docID, 1, hash, -1, SQLITE_STATIC );
    sqlite3_step( pStmt_docID );
    
    uint32_t rowid= sqlite3_column_int(pStmt_docID, 0);
    sqlite3_finalize( pStmt_docID );
    
    return rowid-1;
}



std::string
dataset::getHash( uint32_t docID ){
    
    sqlite3_stmt *pStmt_hash;
    //TODO: remove the need for prefix by looking up the only table (like in engine_3),
    // also remove from document_map then
    char getHash_text[200]; sprintf( getHash_text, "select hash from %s where rowid=?", prefix );
    sqlite3_prepare_v2( db, getHash_text, -1, &pStmt_hash, 0);
    
    sqlite3_bind_int64( pStmt_hash, 1, docID+1 );
    sqlite3_step( pStmt_hash );
    
    const char *db_hash= (char *)sqlite3_column_text(pStmt_hash, 0); // will get destroyed by sqlite so need to copy
    std::string hashS( db_hash );
    sqlite3_finalize( pStmt_hash );
    
    return hashS;
}



std::string
dataset::getFn( uint32_t docID ){
    
    sqlite3_stmt *pStmt_fn;
    char getFn_text[200]; sprintf( getFn_text, "select abspath from %s where rowid=?", prefix );
    sqlite3_prepare_v2( db, getFn_text, -1, &pStmt_fn, 0);
    
    sqlite3_bind_int64( pStmt_fn, 1, docID+1 );
    sqlite3_step( pStmt_fn );
    
    const char *abspath= (char *)sqlite3_column_text(pStmt_fn, 0); // will get destroyed by sqlite so need to copy
    std::string abspathS( abspath );
    
    sqlite3_finalize( pStmt_fn );
    
    if (findPath.size()>0) {
        std::string::size_type pos= abspathS.find(findPath);
        if (pos != std::string::npos) {
            abspathS.replace( pos, findPath.size(), replacePath );
        }
    }
    
    return util::expandUser(abspathS);
    
}



std::pair<uint32_t, uint32_t>
dataset::getWidthHeight( uint32_t docID ){
    
    sqlite3_stmt *pStmt_wh;
    char getWH_text[200]; sprintf( getWH_text, "select res_x,res_y from %s where rowid=?", prefix );
    sqlite3_prepare_v2( db, getWH_text, -1, &pStmt_wh, 0);
    
    sqlite3_bind_int64( pStmt_wh, 1, docID+1 );
    sqlite3_step( pStmt_wh );
    
    int w= sqlite3_column_int(pStmt_wh, 0);
    int h= sqlite3_column_int(pStmt_wh, 1);
    
    sqlite3_finalize( pStmt_wh );
    return std::make_pair(w,h);
}
