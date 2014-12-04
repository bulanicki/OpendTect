/*+
 * (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 * AUTHOR   : Bert
 * DATE     : Dec 2013
 * FUNCTION :
-*/

static const char* rcsID mUsedVar = "$Id$";

#include "od_iostream.h"
#include "testprog.h"
#include "file.h"
#include "filepath.h"
#include "ptrman.h"
#include "strmprov.h"
#include "thread.h"

static const BufferString tmpfnm( FilePath::getTempName("txt") );


#define mRetFail(s) { od_cout() << "Failed " << s << od_endl; }

#define mImplNumberTestFn(fnnm,decls,act,cond) \
static bool fnnm() \
{ \
    od_istream strm( tmpfnm ); \
    decls; \
    act; \
    const bool isok = (cond); \
    if ( !quiet ) \
    { \
	od_cout() << #fnnm << "[" << #cond << "]" \
		  << (isok ? " OK" : " Fail") << od_endl; \
	if ( strm.isBad() ) \
		od_cout() << "\terrmsg=" << strm.errMsg().getFullString() \
			  << od_endl; \
    } \
    return isok; \
}


mImplNumberTestFn( testIfNumberIsNormal, int i; float f,
	   strm >> i >> f,
	   i == 123 && mIsEqual(f,44.5f,1e-8) )

mImplNumberTestFn( testIfNumberIsBad, int i; float f,
	   strm >> i >> f,
	   strm.isBad() )

mImplNumberTestFn( testIfFNumberIs0, int i; float f,
	   strm >> i >> f,
	   i == 123 && f == 0 && strm.isOK() )

mImplNumberTestFn( testOnlyIntRead, int i; float f,
	   strm >> i >> f,
	   i == 123 && !strm.isOK() )


bool testPipeInput()
{
    FixedString message = "OpendTect rules";
    const BufferString command( "@echo ", message );
    StreamData streamdata = StreamProvider( command ).makeIStream();
    mRunStandardTest( streamdata.istrm,  "Creation of standard input stream");
    PtrMan<od_istream> istream = new od_istream(streamdata.istrm);

    BufferString streaminput;
    mRunStandardTest( istream->getAll( streaminput ) , "Read from pipe" );
    mRunStandardTest( streaminput==message, "Pipe content check" );

    return true;
}


bool testPipeOutput()
{
    FixedString message = "OpendTect rules";

    BufferString command = "@";
#ifdef __win__
    command.add( "echo" );
#else
    command.add( "cat" );
#endif

    command.add( " > " ).add( tmpfnm );
    StreamProvider prov( command );
    StreamData ostreamdata = prov.makeOStream();
    mRunStandardTest( ostreamdata.ostrm,  "Creation of standard output stream");
    PtrMan<od_ostream> ostream = new od_ostream(ostreamdata.ostrm);

    *ostream << message;
    ostream->close();

    ostream = 0; //Deletes everything
    Threads::sleep( 1 );

    od_istream istream( tmpfnm );
    mRunStandardTest( istream.isOK(), "Opening temporary file");
    BufferString streaminput;

    istream.getAll( streaminput );
    istream.close();
    File::remove( tmpfnm );

    mRunStandardTest( streaminput==message, "Pipe content check" );

    return true;
}


int doExit( int retval )
{
    if ( File::exists( tmpfnm ) )
	File::remove( tmpfnm );

    return ExitProgram( retval );
}


int main( int argc, char** argv )
{
    mInitTestProg();

    bool isok;
#define mDoTest(strm,content,tstfn) \
    od_ostream strm( tmpfnm ); \
    strm << content; \
    strm.close(); \
    isok = tstfn(); \
    File::remove( tmpfnm ); \
    if ( !isok ) \
	doExit( 1 )

    mDoTest(strm1,"123 44.5",testIfNumberIsNormal);
    mDoTest(strm2,"\t\n123\t\t44.5\n\n",testIfNumberIsNormal);
    mDoTest(strm3,"aap 44.5",testIfNumberIsBad);
    mDoTest(strm4," 123 noot",testIfNumberIsBad);
    mDoTest(strm5,"123\n-0.0e-22\n888",testIfFNumberIs0);
    mDoTest(strm6,"123",testOnlyIntRead);
    mDoTest(strm7,"\n123\n \n",testOnlyIntRead);

    if ( !testPipeInput() )
	doExit( 1 );

    if ( !testPipeOutput() )
    {
	if ( File::exists( tmpfnm ) )
	    File::remove( tmpfnm );

	doExit( 1 );
    }


    return doExit( 0 );
}
