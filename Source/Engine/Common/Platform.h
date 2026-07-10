#pragma once

/*
====================================================================

 Platform abstraction layer.

 All platform-specific includes, types and small compatibility
 shims live here, so the rest of the engine can be written once
 for Windows / Linux / macOS.

====================================================================
*/

#ifdef _WIN32

	/******** Windows ********/

	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif

	// winsock2 must come before Windows.h
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <iphlpapi.h>
	#include <Windows.h>

	// addrlen type used by recvfrom/accept
	typedef int net_socklen_t;

#else

	/******** Linux / macOS (POSIX) ********/

	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <errno.h>
	#include <sys/stat.h> // mkdir
	#include <string.h>
	#include <dlfcn.h>

	// winsock compatibility: sockets are plain file descriptors
	typedef int SOCKET;
	#define INVALID_SOCKET (-1)
	#define SOCKET_ERROR   (-1)

	typedef socklen_t net_socklen_t;

	// closesocket()/ioctlsocket() map directly to POSIX calls
	#define closesocket(s) close(s)
	#define ioctlsocket(s, cmd, argp) ioctl((s), (cmd), (argp))

	// WSAStartup/WSACleanup have no POSIX equivalent - sockets are always ready
	typedef struct WSADATA { int iDummy; } WSADATA;
	#ifndef MAKEWORD
		#define MAKEWORD(low, high) (0)
	#endif
	static inline int WSAStartup(int wVersionRequested, WSADATA* lpWSAData)
	{
		(void)wVersionRequested; (void)lpWSAData;
		return 0;
	}
	static inline int WSACleanup(void) { return 0; }

	// basic Windows.h defines used across the code base
	typedef int BOOL;
	#ifndef TRUE
		#define TRUE  1
	#endif
	#ifndef FALSE
		#define FALSE 0
	#endif
	#ifndef MAX_PATH
		#define MAX_PATH 260
	#endif

	// MSVC CRT string helpers
	#include <ctype.h>
	static inline char* _strupr(char* s)
	{
		for (char* p = s; *p != '\0'; ++p)
			*p = (char)toupper((unsigned char)*p);
		return s;
	}

	// The code base uses Windows style "Dir\\File" paths in many places.
	// On POSIX we normalize backslashes to forward slashes before opening.
	#include <stdio.h>
	static inline FILE* Power_fopenPortable(const char* path, const char* mode)
	{
		char fixed[1024];
		size_t i = 0;
		for (; path[i] != '\0' && i < sizeof(fixed) - 1; ++i)
			fixed[i] = (path[i] == '\\') ? '/' : path[i];
		fixed[i] = '\0';
		return fopen(fixed, mode);
	}
	#define fopen(path, mode) Power_fopenPortable((path), (mode))

#endif
