/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:	Nanne Hemstra
 Date:		May 2007
________________________________________________________________________

-*/
static const char* rcsID = "$Id: visscenecoltab.cc,v 1.18 2010-04-27 12:17:58 cvskarthika Exp $";

#include "visscenecoltab.h"

#include "coltabindex.h"
#include "coltabmapper.h"
#include "coltabsequence.h"
#include "axislayout.h"
#include "scaler.h"
#include "iopar.h"

#include "LegendKit.h"
#include <Inventor/SbColor.h>

mCreateFactoryEntry( visBase::SceneColTab );

namespace visBase
{

const char* SceneColTab::sizestr = "Size";
const char* SceneColTab::posstr = "Pos";
 

SceneColTab::SceneColTab()
    : VisualObjectImpl( false )
    , legendkit_(new LegendKit)
{
    addChild( legendkit_ );
    legendkit_->ref();
    legendkit_->size = SbVec2s(20,150);
    legendkit_->setDiscreteMode( true );
    legendkit_->enableBackground( false );
    setPos( SceneColTab::BottomLeft );
    setLegendColor( Color(170,170,170) );
    setColTabSequence( ColTab::Sequence("") );
}


SceneColTab::~SceneColTab()
{
    removeChild( legendkit_ );
    legendkit_->unref();
}


void SceneColTab::setLegendColor( const Color& col )
{
#define col2f(rgb) float(col.rgb())/255
    legendkit_->setTickAndLinesColor( SbColor(col2f(r),col2f(g),col2f(b)) );
}


void SceneColTab::setColTabSequence( const ColTab::Sequence& ctseq )
{
    if ( sequence_==ctseq )
	return;
    
    sequence_ = ctseq;
    updateVis();
}


void SceneColTab::setSize( int w, int h )
{
    legendkit_->size[0] = w;
    legendkit_->size[1] = h;
    updateVis();
}


void SceneColTab::setPos( Pos pos )
{
    pos_ = pos; 

    if ( pos_ ==  TopLeft )
    {  
	legendkit_->istop = true;
	legendkit_->isleft = true;
    }
    else if ( pos_ == TopRight )
    {
	legendkit_->istop = true;
	legendkit_->isleft = false;
    }
    else if ( pos_ == BottomLeft )
    {
        legendkit_->istop = false;
        legendkit_->isleft = true;
    }
    else if ( pos_ == BottomRight )
    {
	legendkit_->istop = false;
        legendkit_->isleft = false;
    }
    updateVis();
}


Geom::Size2D<int> SceneColTab::getSize() const
{
    Geom::Size2D<int> sz;
    sz.setWidth( legendkit_->size[0] );
    sz.setHeight( legendkit_->size[1] );
    return sz;
}


void SceneColTab::updateVis()
{
    if ( !isOn() )
	return;

    const int nrcols = 256;
    legendkit_->clearColors();
    ColTab::IndexedLookUpTable table( sequence_, nrcols );
    for ( int idx=0; idx<nrcols; idx++ )
    {
	Color col = table.colorForIndex( idx );
	od_uint32 val = ( (unsigned int)(col.r()&0xff) << 24 ) |
		        ( (unsigned int)(col.g()&0xff) << 16 ) |
		        ( (unsigned int)(col.b()&0xff) <<  8 ) |
		        (col.t()&0xff);
	legendkit_->addDiscreteColor( double(idx)/256., val );
    }
    
    legendkit_->clearTicks();
    AxisLayout al; al.setDataRange( rg_ );
    LinScaler scaler( rg_.start, 0, rg_.stop, 1 );

    const int upto = abs( mNINT((al.stop-al.sd.start)/al.sd.step) );
    for ( int idx=0; idx<=upto; idx++ )
    {
	const float val = al.sd.start + idx*al.sd.step;
	const float normval = scaler.scale( val );
	if ( normval>=0 && normval<=1 )
	    legendkit_->addBigTick( normval, val );
    }

    legendkit_->minvalue = toString( rg_.start );
    legendkit_->maxvalue = toString( rg_.stop );
}


void SceneColTab::setColTabMapperSetup( const ColTab::MapperSetup& ms )
{
    Interval<float> rg( ms.start_, ms.start_+ms.width_ );
    if ( rg==rg_ )
	return;
    
    rg_ = rg;
    updateVis();
}


void SceneColTab::turnOn( bool yn )
{
    VisualObjectImpl::turnOn( yn );
    updateVis();
}


int SceneColTab::usePar( const IOPar& iopar )
{
    int res = VisualObjectImpl::usePar( iopar );
    if ( res != 1 ) return res;

    //setLegendColor
    
    int w, h;
    if ( !iopar.get( sizestr, w, h ) )
	return -1;

    setSize( w, h );

/*    enum Pos pos;
    if ( !iopar.get( posstr, pos ) )
	return -1;

    setPos( pos );
*/
    return 1;
}


void SceneColTab::fillPar( IOPar& iopar, TypeSet<int>& saveids ) const
{
    VisualObjectImpl::fillPar( iopar, saveids );

    Geom::Size2D<int> size = getSize();
    iopar.set( sizestr, size.width(), size.height() );

  /*  enum Pos pos = getPos();
    iopar.set( posstr, pos );*/
}

} // namespace visBase
