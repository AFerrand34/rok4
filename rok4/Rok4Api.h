#ifndef _ROK4API
#define _ROK4API

/**
* \file Rok4Api.h
* \brief Interface de l'API de ROK4
*/

#ifdef __cplusplus

#include "Rok4Server.h"

extern "C" {

#else

#include <stdint.h>

// Types
typedef void Rok4Server;

#endif

typedef struct{
        int status;
        char* type;
        char* content;
} HttpResponse;

typedef struct{
	char* filename;
	uint32_t posoff;
	uint32_t possize;
} TileRef;

// Functions

Rok4Server* rok4InitServer(const char* serverConfigFile);
HttpResponse* rok4GetWMTSCapabilities(const char* hostname, const char* path, Rok4Server* server);
HttpResponse* rok4GetTile(const char* query, const char* hostname, const char* path, Rok4Server* server);
HttpResponse* rok4GetTileReferences(const char* query, const char* hostname, const char* path, Rok4Server* server, TileRef* tileRef);
void rok4KillServer(Rok4Server* server);

#ifdef __cplusplus
}
#endif

#endif
