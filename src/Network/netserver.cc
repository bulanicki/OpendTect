/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:	Nanne Hemstra
 Date:		March 2009
________________________________________________________________________

-*/
static const char* rcsID mUsedVar = "$Id$";

#include "netserver.h"

#include "envvars.h"
#include "genc.h"
#include "netsocket.h"
#include "separstr.h"
#include "systeminfo.h"

#include "qtcpservercomm.h"
#include <QHostAddress>
#include <QString>
#include <QLocalServer>

#include "hiddenparam.h"


static Threads::Atomic<PortNr_Type> lastusableport_ = 0;


PortNr_Type Network::getNextCandidatePort()
{
    if ( lastusableport_ == 0 )
	lastusableport_ = (PortNr_Type)GetEnvVarIVal( "OD_START_PORT", 20049 );
    return lastusableport_ + 1;
}


bool Network::isLocal( SpecAddr spec )
{
    return spec == Local;
}


PortNr_Type Network::getUsablePort( PortNr_Type portnr )
{
    uiRetVal uirv;
    return getUsablePort( uirv, portnr, 100 );
}


PortNr_Type Network::getUsablePort( uiRetVal& uirv, PortNr_Type portnr,
				    int nrtries )
{
    uiString errmsg;
    if ( portnr == 0 )
	portnr = getNextCandidatePort();

    for ( int idx=0; idx<nrtries; idx++, portnr++ )
    {
	if ( isPortFree(portnr,&errmsg) )
	    { lastusableport_ = portnr; return portnr; }
    }

    uirv = errmsg;
    return 0;
}


static HiddenParam<Network::Authority,BufferString*>
					netauthservernmmgr_( nullptr );


Network::Authority::Authority( const char* host, PortNr_Type port,
							bool resolveipv6 )
    : qhost_(*new QString)
    , qhostaddr_(*new QHostAddress)
{
    netauthservernmmgr_.setParam( this, new BufferString() );
    setHost( host, resolveipv6 );
    setPort( port );
}


Network::Authority::Authority( const BufferString& servernm )
    : qhost_(*new QString)
    , qhostaddr_(*new QHostAddress)
{
    netauthservernmmgr_.setParam( this, new BufferString() );
    localFromString( servernm );
}


Network::Authority::Authority( const Authority& oth )
    : qhost_(*new QString)
    , qhostaddr_(*new QHostAddress)
{
    *this = oth;
}


Network::Authority::~Authority()
{
    delete &qhost_;
    delete &qhostaddr_;
    netauthservernmmgr_.removeAndDeleteParam( this );
}


Network::Authority& Network::Authority::operator=( const Authority& oth )
{
    if ( &oth == this )
	return *this;

    userinfo_ = oth.userinfo_;
    port_ = oth.port_;
    qhost_ = oth.qhost_;
    qhostaddr_ = oth.qhostaddr_;
    hostisaddress_ = oth.hostisaddress_;
    setServerName( oth.getServerName() );
    return *this;
}


bool Network::Authority::operator==( const Authority& oth ) const
{
    if ( &oth == this )
	return true;

    return port_ == oth.port_ &&
	   qhostaddr_ == oth.qhostaddr_ &&
	   qhost_ == oth.qhost_ &&
	   hostisaddress_ == oth.hostisaddress_ &&
	   userinfo_ == oth.userinfo_ &&
	   *netauthservernmmgr_.getParam( this ) ==
	   *netauthservernmmgr_.getParam( &oth );
}


void Network::Authority::setServerName( const char* nm )
{
    netauthservernmmgr_.getParam( this )->set( nm );
}


BufferString Network::Authority::getServerName() const
{
    return *netauthservernmmgr_.getParam( this );
}

bool Network::Authority::isUsable() const
{
    if ( isLocal() )
	return true;

    return hasAssignedPort();
}


Network::Authority Network::Authority::getLocal( const char* servernm )
{
    return Authority( BufferString(servernm) );
}


bool Network::Authority::addressIsValid() const
{
    return !qhostaddr_.isNull();
}


bool Network::Authority::portIsFree( uiString* errmsg ) const
{
    if ( !hasAssignedPort() )
    {
	if ( errmsg )
	    *errmsg = tr("Network port is not assigned.");
	return false;
    }

    return isPortFree( port_, errmsg );
}


void Network::Authority::fromString( const char* str, bool resolveipv6 )
{
    FixedString hoststr( str );
    if ( hoststr.contains('@') )
    {
	const SeparString usersep( hoststr, '@' );
	userinfo_.set( usersep[0] );
	hoststr = usersep.size() > 1 ? hoststr.find('@')+1 : nullptr;
    }
    else
	userinfo_.setEmpty();

    if ( hoststr.contains('[') )
    {
	BufferString hostnm( hoststr );
	hoststr = hostnm.find(']') + 1;
	hostnm.replace( '[', ' ' ).replace( ']', '\0' );
	hostnm.trimBlanks();
	setHost( hostnm );
	if ( hoststr.contains(':') )
	{
	    const SeparString hostsep( hoststr, ':' );
	    setPort( hostsep.size() > 1 ?
		     mCast(PortNr_Type, hostsep[1].toInt()) : 0 );
	}
	else
	    setPort( 0 );
    }
    else
    {
	if ( hoststr.contains(':') )
	{
	    const SeparString hostsep( hoststr, ':' );
	    setHost( hostsep[0], resolveipv6 );
	    setPort( hostsep.size() > 1 ?
		 mCast(PortNr_Type, hostsep[1].toInt()) : 0 );
	}
	else
	{
	    setHost( hoststr, resolveipv6 );
	    setPort( 0 );
	}
    }
}


void Network::Authority::localFromString( const char* str )
{
    setServerName( str );
    qhostaddr_.setAddress( QHostAddress::LocalHost );
    port_ = 0;
}


BufferString Network::Authority::toString( bool external ) const
{
    BufferString ret( userinfo_ );
    if ( !ret.isEmpty() )
	ret.add( "@" );
    ret.add( getHost(external) );
    if ( port_ > 0 )
	ret.add( ":" ).add( port_ );
    return ret;
}


BufferString Network::Authority::getHost( bool external ) const
{
    BufferString ret;
    if ( (external && isLocal()) )
	return ret;

    if ( isLocal() )
	return Socket::sKeyLocalHost();

    if ( external && hostisaddress_ )
    {
	const BufferString ipaddr( qhostaddr_.toString() );
	ret.set( System::hostName(ipaddr) );
	if ( ret.isEmpty() )
	    ret.set( GetLocalHostName() );
    }
    else if ( addressIsValid() && !external )
    {
	const bool ipv6 =
			qhostaddr_.protocol() == QAbstractSocket::IPv6Protocol;
	if ( ipv6 )
	    ret.add( "[" );
	ret.add( qhostaddr_.toString() );
	if ( ipv6 )
	    ret.add( "]" );
    }
    else
	ret.add( qhost_ );

    return ret;
}


void Network::Authority::setHost( const char* nm, bool resolveipv6 )
{
    setHostAddress( nm, resolveipv6 );
}


void Network::Authority::setFreePort( uiRetVal& uirv )
{
    setPort( getUsablePort(uirv) );
}


void Network::Authority::setHostAddress( const char* host, bool resolveipv6 )
{
    BufferString hostnm( host );
    hostnm.replace( '[', ' ' ).replace( ']', ' ' );
    hostnm.trimBlanks();

    qhost_ = hostnm;
    qhostaddr_.setAddress( qhost_ );

    const QAbstractSocket::NetworkLayerProtocol prtocolval =
							qhostaddr_.protocol();
    hostisaddress_ = prtocolval >
		     QAbstractSocket::UnknownNetworkLayerProtocol;

    if ( hostisaddress_ )
    {
#ifdef  __win__
	if ( prtocolval == QAbstractSocket::IPv4Protocol &&
			    qhostaddr_ == QHostAddress(QHostAddress::AnyIPv4) )
	{
	    const BufferString localaddr(
			System::hostAddress(Socket::sKeyLocalHost()) );
	    qhostaddr_.setAddress( QString(localaddr) );
	}
	else if ( prtocolval == QAbstractSocket::IPv6Protocol &&
			    qhostaddr_ == QHostAddress(QHostAddress::AnyIPv6) )
	{
	    const BufferString localaddr(
		   System::hostAddress(Socket::sKeyLocalHost(),false));
	    qhostaddr_.setAddress( QString(localaddr) );
	}
#endif
    }
    else
    {
	qhostaddr_.setAddress(
			QString(System::hostAddress(hostnm,!resolveipv6)) );

    }

    if ( qhostaddr_.isNull() )
	qhostaddr_.clear();
}


static int sockid = 0;

static int getNewID()
{
    return ++sockid;
}


static HiddenParam<Network::Server,QLocalServer*>
					netserverqlocservmgr_(nullptr);

Network::Server::Server()
    : newConnection(this)
    , readyRead(this)
    , qtcpserver_(new QTcpServer())
{
    netserverqlocservmgr_.setParam( this, nullptr );
    comm_ = new QTcpServerComm( qtcpserver_, this );
}


Network::Server::Server( bool islocal )
    : newConnection(this)
    , readyRead(this)
    , qtcpserver_(nullptr)
{
    netserverqlocservmgr_.setParam( this, nullptr );
    if ( islocal )
    {
	auto* qlocalserver = new QLocalServer();
	netserverqlocservmgr_.setParam( this, qlocalserver );
	comm_ = new QTcpServerComm( qlocalserver, this );
    }
    else
    {
	qtcpserver_ =  new QTcpServer();
	comm_ = new QTcpServerComm( qtcpserver_, this );
    }
}


Network::Server::~Server()
{
    detachAllNotifiers();
    if ( isListening() )
	close();
    delete qtcpserver_;
    netserverqlocservmgr_.removeAndDeleteParam( this );
    delete comm_;
    deepErase( sockets2bdeleted_ );
}


namespace Network {

static QHostAddress::SpecialAddress qtSpecAddr( SpecAddr specaddress )
{
    if ( specaddress == Any )
	return QHostAddress::Any;
    else if ( specaddress == IPv4 )
	return QHostAddress::AnyIPv4;
    else if ( specaddress == IPv6 )
	return QHostAddress::AnyIPv6;
    else if ( specaddress == Broadcast )
	return QHostAddress::Broadcast;
    else if ( specaddress == LocalIPv4 )
	return QHostAddress::LocalHost;
    else if ( specaddress == LocalIPv6 )
	return QHostAddress::LocalHostIPv6;

    return QHostAddress::Null;
}

};


QLocalServer* Network::Server::localServer() const
{ return netserverqlocservmgr_.getParam( this ); }

QLocalServer* Network::Server::localServer()
{ return netserverqlocservmgr_.getParam( this ); }


bool Network::Server::listen( SpecAddr specaddress, PortNr_Type prt )
{
    if ( isLocal() )
	return localServer()->listen( toString(prt) );

    return qtcpserver_->listen( QHostAddress(qtSpecAddr(specaddress)), prt );
}


bool Network::Server::listen( const char* servernm, uiRetVal& ret )
{
    if ( !isLocal() )
    {
	pErrMsg( "It needs a valid server anme to listen to" );
	return false;
    }

    const bool canlisten = localServer()->listen( servernm );

    if ( !canlisten )
	ret.set( tr("%1 server name is already in use").arg(servernm) );

    return canlisten;
}


bool Network::Server::isListening() const
{
    if ( isLocal() )
	return localServer()->isListening();

    return qtcpserver_->isListening();
}

PortNr_Type Network::Server::port() const
{
    if ( isLocal() )
    {
	pErrMsg("Local server does not have a port");
	return 0;
    }
    return qtcpserver_->serverPort();
}


Network::Authority Network::Server::authority() const
{
    BufferString addr;
    if ( isLocal() )
	return Authority::getLocal(
				BufferString(localServer()->serverName()) );
    else
    {
	const QHostAddress qaddr = qtcpserver_->serverAddress();
	if ( qaddr.protocol() > QAbstractSocket::UnknownNetworkLayerProtocol )
	    addr.set( qaddr.toString() );
	return Authority( addr, port() );
    }
}


void Network::Server::close()
{
    if ( isLocal() )
	localServer()->close();
    else
	qtcpserver_->close();
}


const char* Network::Server::errorMsg() const
{
    if ( isLocal() )
	errmsg_ = localServer()->errorString().toLatin1().constData();
    else
	errmsg_ = qtcpserver_->errorString().toLatin1().constData();
    return errmsg_.buf();
}


bool Network::Server::hasPendingConnections() const
{
    if ( isLocal() )
	return localServer()->hasPendingConnections();

    return qtcpserver_->hasPendingConnections();
}

QTcpSocket* Network::Server::nextPendingConnection()
{
    return qtcpserver_->nextPendingConnection();
}


QLocalSocket* Network::Server::nextPendingLocalConnection()
{
    return localServer()->nextPendingConnection();
}


void Network::Server::notifyNewConnection()
{
    if ( !hasPendingConnections() )
	return;
    Socket* socket = nullptr;
    if ( isLocal() )
	socket = new Socket( nextPendingLocalConnection(), isLocal() );
    else
	socket = new Socket( nextPendingConnection(), isLocal() );

    socket->readyRead.notify( mCB(this,Server,readyReadCB));
    mAttachCB( socket->disconnected, Server::disconnectCB);
    sockets_ += socket;
    const int id = getNewID();
    ids_ += id;

    newConnection.trigger( id );
}


void Network::Server::readyReadCB( CallBacker* cb )
{
    mDynamicCastGet(Socket*,socket,cb);
    if ( !socket ) return;

    const int idx = sockets_.indexOf( socket );
    if ( idx<0 )
	return;

    readyRead.trigger( ids_[idx] );
}


void Network::Server::disconnectCB( CallBacker* cb )
{
    mDynamicCastGet(Socket*,socket,cb);
    if ( !socket ) return;

    //socket->readyRead.remove( mCB(this,Server,readyReadCB) );
    const int idx = sockets_.indexOf( socket );
    if ( idx>=0 )
    {
	sockets_.removeSingle( idx );
	ids_.removeSingle( idx );
    }

    const int delidx = sockets2bdeleted_.indexOf( socket );
    if ( delidx>=0 )
	sockets2bdeleted_ += socket;
}


void Network::Server::read( int id, BufferString& data ) const
{
    const Socket* socket = getSocket( id );
    if ( !socket )
	return;

    socket->read( data );
}


void Network::Server::read( int id, IOPar& par ) const
{
    const Socket* socket = getSocket( id );
    if ( !socket )
	return;

    socket->read( par );
}


int Network::Server::write( int id, const IOPar& par )
{
    Network::Socket* socket = getSocket( id );
    return socket ? socket->write( par ) : 0;
}


int Network::Server::write( int id, const char* str )
{
    Socket* socket = getSocket( id );
    return socket ? socket->write( FixedString(str) ) : 0;
}


Network::Socket* Network::Server::getSocket( int id )
{
    const int idx = ids_.indexOf( id );
    if ( idx<0 )
	return 0;

    return sockets_[idx];
}


const Network::Socket* Network::Server::getSocket( int id ) const
{ return const_cast<Server*>( this )->getSocket( id ); }


bool Network::Server::waitForNewConnection( int msec )
{
    return qtcpserver_->waitForNewConnection( msec );
}
