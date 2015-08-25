

#include "cli_srvr.h"

#define BACKLOG 10				/* how many pending connections queue will
								 * hold */

void
usage (  )
{
	fprintf ( stderr, "usage srvr -p port [-d]\n" );
	exit ( 1 );
}

int main ( int argc, char *argv[] )
{
	int port = 0;
	char msg[MAXDATASIZE];
	fd_set r, w;
	int on = 1;
	int debugging = 0;
	char ch;

	int sockfd, new_fd;			/* listen on sock_fd, new connection on
								 * new_fd */
	struct sockaddr_in my_addr;	/* my address information */
	struct sockaddr_in their_addr;	/* connector's address information */
	socklen_t sin_size;

	while ( ( ch = getopt ( argc, argv, "p:d" ) ) != EOF ) {
		switch ( ch ) {
		case 'p':
			port = atoi ( optarg );
			break;
		case 'd':
			debugging = 1;
			break;
		case '?':
			usage (  );
			break;
		}
	}
	if ( !port ) {
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




	if ( ( sockfd = socket ( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
		perror ( "socket" );
		exit ( 1 );
	}
	setsockopt ( sockfd, SOL_SOCKET, SO_REUSEADDR, ( const char * ) &on, sizeof on );

	my_addr.sin_family = AF_INET;	/* host byte order */
	my_addr.sin_port = htons ( port );	/* short, network byte order */
	my_addr.sin_addr.s_addr = htonl ( INADDR_ANY );	/* auto-fill with my IP */

	memset( & ( my_addr.sin_zero), 0, 8);

	if ( bind ( sockfd, ( struct sockaddr * ) & my_addr, sizeof ( struct sockaddr ) )
		 == -1 ) {
		perror ( "bind" );
		closesocket ( sockfd );
		exit ( 1 );
	}
	while ( 1 ) {
		long nrequests = 0;
		long xmitted = 0;
		int stopi = 0;
		if ( listen ( sockfd, BACKLOG ) == -1 ) {
			perror ( "listen" );
			closesocket ( sockfd );
			exit ( 1 );
		}
		logit("\nwaiting for connection");
		sin_size = sizeof ( struct sockaddr_in );
		if ( ( new_fd = accept ( sockfd, ( struct sockaddr * ) & their_addr,
								 &sin_size ) ) == -1 ) {
			perror ( "accept" );
			closesocket ( sockfd );
			exit ( 0 );
		}
		printf ( "server: got connection from %s\n",
				 inet_ntoa ( their_addr.sin_addr ) );
// 		if ( send ( new_fd, "Hello, world!\n", 14, 0 ) == -1 )
// 			perror ( "send" );

		FD_ZERO ( &r );
		FD_ZERO ( &w );
		FD_SET ( new_fd, &r );
		FD_SET ( new_fd, &w );

		///////////////////////////////////////////////////////////////////
		// 1st read # bytes to read from client
		///////////////////////////////////////////////////////////////////

// 		logit("waiting for # of bytes");
		MYULONG bytes2recv_nl;
		int sizeofb2r = sizeof(MYULONG); 
		printf("sizeofb2r %d\n",sizeofb2r);
		int n = recv ( new_fd, (char*)&bytes2recv_nl, sizeofb2r, 0 );
		if (n != sizeofb2r) {
			closesocket ( new_fd );
			perror ( "recv bytes2recv" );
			stopi = 1;
		}
		MYULONG bytes2recv = ntohl(bytes2recv_nl);
// 		printf("got %d bytes 2 recv\n", bytes2recv);
		MYULONG bytesreceived = 0;

		///////////////////////////////////////////////////////////////////
		// now read 'em
		///////////////////////////////////////////////////////////////////

		while (bytesreceived < bytes2recv) {
			bytesreceived += recv ( new_fd, msg, MAXDATASIZE, 0 );
// 			printf("received so far %d\n", bytesreceived);
		}
		if (bytes2recv != bytesreceived) {
			printf("!!! bytes2recv:%d != bytesreceived:%d\n",bytes2recv,bytesreceived);
		}

		///////////////////////////////////////////////////////////////////
		// now tell client how much we got
		///////////////////////////////////////////////////////////////////

		double bytesreceived_d = bytesreceived;
		printf("got %g bytes\n", bytesreceived_d);
		long bytes = htonl(bytesreceived);
		if ( ( n = send ( new_fd, (const char*)&bytes, sizeofb2r, 0 ) ) == -1 ) {
			perror ( "send" );
		}
		closesocket ( new_fd );
	}

#ifdef WIN32
	WSACleanup();
#endif

	return 0;
}
