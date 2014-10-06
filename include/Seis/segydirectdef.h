#ifndef segydirectdef_h
#define segydirectdef_h

/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:	Bert
 Date:		Jul 2008
 RCS:		$Id$
________________________________________________________________________

-*/

#include "seismod.h"
#include "segyfiledata.h"
#include "bufstringset.h"
#include "executor.h"
#include "od_iosfwd.h"

class IOObj;
namespace Seis { class PosIndexer; }
namespace PosInfo { class CubeData; class Line2DData; }


namespace SEGY {

class FileSpec;
class Scanner;
class FileDataSet;
class PosKeyList;


mExpClass(Seis) DirectDef
{
public:

    			DirectDef();			//!< Create empty
    			DirectDef(const char*);		//!< Read from file
			~DirectDef();
    bool		isEmpty() const;

			//Functions to read/query
    bool		readFromFile(const char*);
    const IOPar*	segyPars() const;
    FixedString		fileName(int idx) const;
    FileDataSet::TrcIdx	find(const Seis::PosKey&,bool chkoffs) const;
    FileDataSet::TrcIdx	findOcc(const Seis::PosKey&,int occ) const;
    			//!< will not look at offset

    			//Functions to write
    void		setData(FileDataSet&);
    bool		writeHeadersToFile(const char*);
    			/*!<Write the headers. After calling, the fds should
			    be dumped into the stream. */
    od_ostream*		getOutputStream()	{ return outstream_; }
    bool		writeFootersToFile();
    			/*!<After fds has been dumped, write the 
			    remainder of the file */

    const FileDataSet&	fileDataSet() const	{ return *fds_; }
    const char*		errMsg() const		{ return errmsg_.str(); }

    static const char*	sKeyDirectDef();
    static const char*	sKeyFileType();
    static const char*	sKeyNrFiles();
    static const char*	sKeyInt64DataChar();
    static const char*	sKeyInt32DataChar();
    static const char*	sKeyFloatDataChar();

    static const char*	get2DFileName(const char*,const char*);
    static const char*	get2DFileName(const char*,Pos::GeomID);


    const PosInfo::CubeData&	cubeData() const { return cubedata_; }
    const PosInfo::Line2DData&	lineData() const { return linedata_; }


protected:
    void		getPosData(PosInfo::CubeData&) const;
    void		getPosData(PosInfo::Line2DData&) const;

    PosInfo::CubeData&	 cubedata_;
    PosInfo::Line2DData& linedata_;

    const FileDataSet*	fds_;
    FileDataSet*	myfds_;
    SEGY::PosKeyList*	keylist_;
    Seis::PosIndexer*	indexer_;

    mutable BufferString errmsg_;

    od_ostream*		outstream_;
    od_stream_Pos	offsetstart_;
    od_stream_Pos	datastart_;
    od_stream_Pos	textparstart_;
    od_stream_Pos	cubedatastart_;
    od_stream_Pos	indexstart_;
};


/*!Scans a file and creates an index file that can be read by OD. */
mExpClass(Seis) FileIndexer : public Executor
{
public:
    			FileIndexer(const MultiID& mid,bool isvol,
				    const FileSpec&,bool is2d,const IOPar&);
    			~FileIndexer();

    int                 nextStep();

    const char*         message() const;
    od_int64            nrDone() const;
    od_int64            totalNr() const;
    const char*         nrDoneText() const;

    const Scanner*	scanner() const { return scanner_; }

protected:

    IOObj*		ioobj_;
    BufferString	linename_;

    Scanner*		scanner_;
    BufferString	msg_;
    DirectDef*		directdef_;
    bool		is2d_;
    bool		isvol_;

};


}; //Namespace

#endif

