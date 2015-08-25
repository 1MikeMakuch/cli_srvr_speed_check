

//#ifdef WIN32
//#include "stdafx.h"
//#endif

#include "cli_srvr.h"

void
usage (  )
{
	fprintf ( stderr, "usage: cli -h host -p port -n nrequests -s size(max %d) -d\n", MAXDATASIZE );
	exit ( 1 );
}

int 
main ( int argc, char *argv[] )
{
	int sockfd;

	struct hostent *he;
	struct sockaddr_in their_addr;	/* connector's address information */
	int port = 0;
	int nreqs = 0;
	int reqsize = 1;
	int i;
	fd_set r, w;
	int debugging = 0;
	double xmitted = 0;
	char ch;
	char *host = NULL;
    char sbuf[MAXDATASIZE];

	MYTIMET tv_start, tv_end;

	while ( ( ch = getopt ( argc, argv, "?h:dp:n:s:" ) ) != EOF ) {
		switch ( ch ) {
		case 'h':
			host = optarg;
			break;
		case 'p':
			port = atoi ( optarg );
			break;
		case 'n':
			nreqs = atoi ( optarg );
			break;
		case 's':
			reqsize = atoi ( optarg );
			break;
		case 'd':
			debugging = 1;
			break;
		case '?':
			usage (  );
			exit ( 1 );
		}
	}
	if ( !host || !port || !nreqs || !reqsize || reqsize > MAXDATASIZE) {
		usage (  );
	}

#ifdef WIN32
	WORD		wVersionRequested = MAKEWORD(1, 1);  
	WSADATA		wsaData; 
	int			res; 
 	res = WSAStartup(wVersionRequested, &wsaData); 
	if (LOBYTE(wsaData.wVersion)!= 1 || HIBYTE(wsaData.wVersion)!= 1) 
	{ 
	//-- Version 1.1 is not supported, abort
		WSACleanup(); 
		return 1; 
	} 
#endif

	if ( ( he = gethostbyname ( host ) ) == NULL ) {	/* get the host info */
		perror ( "gethostbyname" );
		usage (  );
		exit ( 1 );
	}
	if ( ( sockfd = socket ( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
		perror ( "socket" );
		usage (  );
		exit ( 1 );
	}
	their_addr.sin_family = AF_INET;	/* host byte order */
	their_addr.sin_port = htons ( port );	/* short, network byte order */
	their_addr.sin_addr = *( ( struct in_addr * ) he->h_addr );
	memset( & (their_addr.sin_zero ), 0, 8);

	if ( connect ( sockfd, ( struct sockaddr * ) & their_addr, \
				   sizeof ( struct sockaddr ) ) == -1 ) {
		perror ( "connect" );
		closesocket ( sockfd );
		exit ( 1 );
	}

// 	if ( ( numbytes = recv ( sockfd, buf, MAXDATASIZE, 0 ) ) == -1 ) {
// 		perror ( "recv" );
// 		close ( sockfd );
// 		exit ( 1 );
// 	}
// 	buf[numbytes] = '\0';
// 
// 	printf ( "Received: %s", buf );

	for ( i = 0; i < reqsize; ++i ) {
		sbuf[i] = 'x';
	}
	sbuf[i] = 0;
	int sbuflen = reqsize;

	///////////////////////////////////////////////////////////////////////
	// 1st tell server how much data to read
	///////////////////////////////////////////////////////////////////////

	MYULONG bytes2send = reqsize * nreqs;
	MYULONG nb2s = htonl(bytes2send);
	int sizeofb2s = sizeof(MYULONG);
	int nsent = send ( sockfd, (const char*)&nb2s, sizeofb2s, 0 );
	if (nsent != sizeofb2s) {
		perror("send bytes2send");
		closesocket(sockfd);
		exit(1);
	}

	FD_ZERO ( &r );
	FD_ZERO ( &w );
	FD_SET ( sockfd, &r );
	FD_SET ( sockfd, &w );

	///////////////////////////////////////////////////////////////////////
	// now send it
	///////////////////////////////////////////////////////////////////////

	MYTIME(&tv_start);
	for ( i = 0; i < nreqs; ++i ) {

// 		logit("about to send");
		if ( ( nsent = send ( sockfd, sbuf, sbuflen, 0 ) ) == -1 ) {
			perror ( "send" );
			closesocket ( sockfd );
			exit ( 1 );
		}
// 		logit("sent");
		if ( i < nreqs )
			xmitted += nsent;
	}
// 	ready = select ( FD_SETSIZE, &r, NULL, NULL, NULL );
// 	if ( ready < 1 ) {
// 		printf ( "no recv select\n" );
// 	} else {


	///////////////////////////////////////////////////////////////////////
	// now read what the server says it read
	///////////////////////////////////////////////////////////////////////

	int nreceived;
	MYULONG nr;

// 	logit("waiting for server to say what it got");
	if ( ( nreceived = recv ( sockfd, (char*)&nr, MAXDATASIZE, 0 ) ) == -1 ) {
		perror ( "recv" );
		closesocket ( sockfd );
		exit ( 1 );
	}
	MYTIME(&tv_end);

	closesocket ( sockfd );
	if (nreceived != sizeofb2s) {
		printf("recv didn't get %d byte reply from srvr:%d",sizeofb2s, nreceived);
	}

	MYULONG srvr_recvd = ntohl(nr);
	double srvr_recvd_d = srvr_recvd;

	printf ( "reqs: %d, bytes sent: %g, bytes srvr received: %g\n", 
		nreqs, xmitted, srvr_recvd_d );
	if (srvr_recvd != bytes2send) {
		printf("!!! server received:%d != bytes2send:%d\n", 
			srvr_recvd, bytes2send);
	}

	double dursecs;
#ifdef WIN32
	dursecs = difftime(tv_end,tv_start);
#else
	dursecs = tv_end.tv_sec - tv_start.tv_sec;
#endif

	double bps;
	if (dursecs < 1) dursecs = 1;
	bps = srvr_recvd / dursecs;
	printf("round trip secs: %g, bytes/sec:%g\n", dursecs, bps);

#ifdef WIN32
	WSACleanup();
#endif

	return 0;
}
