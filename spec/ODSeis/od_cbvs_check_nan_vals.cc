/*+
 * (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 * AUTHOR   : A.H. Bril
 * DATE     : 2000
 * RCS      : $Id: od_cbvs_check_nan_vals.cc,v 1.5 2010/04/23 05:27:20 cvsnanne Exp $
-*/

static const char* rcsID = "$Id: od_cbvs_check_nan_vals.cc,v 1.5 2010/04/23 05:27:20 cvsnanne Exp $";

#include "seistrc.h"
#include "seiscbvs.h"
#include "seisselection.h"
#include "cubesampling.h"
#include "conn.h"
#include "iostrm.h"
#include "separstr.h"
#include "file.h"
#include "filepath.h"
#include "ptrman.h"
#include "strmprov.h"
#include <iostream>
#include <math.h>

#include "prog.h"
#include "seisfact.h"


int main( int argc, char** argv )
{
    if ( argc < 1 )
    {
	std::cerr << "Usage: " << argv[0]
	          << " inpfile\n";
	std::cerr << "Format input: CBVS ; Format ouput: inl crl v [v ...]"
		  << std::endl;
	ExitProgram( 1 );
    }

    FilePath fp( argv[1] );
    
    if ( !File::exists(fp.fullPath()) )
    {
        std::cerr << fp.fullPath() << " does not exist" << std::endl;
        ExitProgram( 1 );
    }
    
    if ( !fp.isAbsolute() )
    {
        fp.insert( File::getCurrentPath() );
    }

    BufferString fname=fp.fullPath();


    PtrMan<CBVSSeisTrcTranslator> tri = CBVSSeisTrcTranslator::getInstance();
    if ( !tri->initRead( new StreamConn(fname,Conn::Read) ) )
	{ std::cerr << tri->errMsg() << std::endl; ExitProgram( 1 ); }

    fp.set( argv[2] ); 
    if ( !fp.isAbsolute() ) { fp.insert( File::getCurrentPath() ); }
    fname = fp.fullPath();

    StreamData outsd = StreamProvider( fname ).makeOStream();
    if ( !outsd.usable() )
        { std::cerr << "Cannot open output file" << std::endl; ExitProgram(1); }

    SeisTrc trc;
    int nrwr = 0;
    int nrlwr = 0;
    while ( tri->read(trc) )
    {
	const int nrcomps = trc.data().nrComponents();
	const int nrsamps = trc.size();
	Coord coord = trc.info().coord;
	for ( int isamp=0; isamp<nrsamps; isamp++ )
	{
	    for ( int icomp=0; icomp<nrcomps; icomp++ )
	    {
		if ( trc.get(isamp,icomp) > 10000 )
		{
		    std::cout<< trc.info().binid.inl << ' ' 
			     << trc.info().binid.crl << ' '
			     << trc.get(isamp,icomp) << ' '<<'\n'<< std::endl;
		}
		else if (trc.get(isamp,icomp) < -10000 )
		{
		    std::cout << trc.info().binid.inl << ' ' 
			     << trc.info().binid.crl << ' '
			     << trc.get(isamp,icomp) << ' '<<'\n'<< std::endl;
		}
		nrlwr++;
	    }
	}
	nrwr++;
    }

    ExitProgram( nrwr ? 0 : 1 ); return 0;
}
