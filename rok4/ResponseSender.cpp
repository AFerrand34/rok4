#include "ResponseSender.h"
#include "ServiceException.h"
#include "Message.h"
#include <iostream>
#include "Logger.h"
#include <stdio.h>
#include <string.h> // pour strlen
#include <sstream> // pour les stringstream

/**
 * Methode commune pour generer l'entete HTTP en fonction du status code HTTP
 */
std::string genStatusHeader(int statusCode) {
	// Creation de l'en-tete
	std::stringstream out;
	out << statusCode;
	std::string statusHeader= "Status: "+out.str()+" "+ServiceException::getStatusCodeAsReasonPhrase(statusCode)+"\r\n" ;
	return statusHeader ;
}

/**
 * Methode commune pour afficher les codes d'erreur FCGI
 */

void displayFCGIError(int error)
{
	if (error>0)
		LOGGER_ERROR("Code erreur : "<<error); // Erreur errno(2) (Cf. manpage )
        else if (error==FCGX_UNSUPPORTED_VERSION)
        	LOGGER_ERROR("Version FCGI non supportee");
        else if (error==FCGX_UNSUPPORTED_VERSION)
        	LOGGER_ERROR("Erreur de protocole");
        else if (error==FCGX_CALL_SEQ_ERROR)
        	LOGGER_ERROR("Erreur de parametre");
        else if (error==FCGX_UNSUPPORTED_VERSION)
        	LOGGER_ERROR("Preconditions non remplies");
        else
        	LOGGER_ERROR("Erreur inconnue");
}

/**
 * Copie d'une source de données dans le flux de sortie de l'objet request de type FCGX_Request
 * @return -1 en cas d'erreur
 * @return 0 sinon
 */

int ResponseSender::sendresponse(DataSource* source, FCGX_Request* request)
{
	// Creation de l'en-tete
	std::string statusHeader= genStatusHeader(source->getHttpStatus()) ;
	FCGX_PutStr(statusHeader.data(),statusHeader.size(),request->out);
	FCGX_PutStr("Content-Type: ",14,request->out);
	FCGX_PutStr(source->getType().c_str(), strlen(source->getType().c_str()),request->out);
	FCGX_PutStr("\r\n\r\n",4,request->out);
	// Copie dans le flux de sortie
	size_t buffer_size;

	const uint8_t *buffer = source->getData(buffer_size);

	int wr = 0;
	// Ecriture iterative de la source de donnees dans le flux de sortie
	while(wr < buffer_size) {
		// Taille ecrite dans le flux de sortie
		int w = FCGX_PutStr((char*)(buffer + wr), buffer_size,request->out);
		if(w < 0) {
			LOGGER_ERROR("Echec d'écriture dans le flux de sortie de la requête FCGI " << request->requestId);
			displayFCGIError(FCGX_GetError(request->out));
			delete source;
			delete[] buffer;
			return -1;
		}
		wr += w;
	}
	delete source;
	return 0;
}


/**
 * Copie d'un flux d'entree dans le flux de sortie de l'objet request de type FCGX_Request
 * @return -1 en cas d'erreur
 * @return 0 sinon
 */
int ResponseSender::sendresponse(DataStream* stream, FCGX_Request* request)
{
	// Creation de l'en-tete
	std::string statusHeader= genStatusHeader(stream->getHttpStatus());
	FCGX_PutStr(statusHeader.data(),statusHeader.size(),request->out);
	FCGX_PutStr("Content-Type: ",14,request->out);
	FCGX_PutStr(stream->getType().c_str(), strlen(stream->getType().c_str()),request->out);
	FCGX_PutStr("\r\n\r\n",4,request->out);
	// Copie dans le flux de sortie
	uint8_t *buffer = new uint8_t[2 << 20];
	size_t size_to_read = 2 << 20;
	int pos = 0;

	// Ecriture progressive du flux d'entree dans le flux de sortie
	while(true) {
		// Recuperation d'une portion du flux d'entree

		size_t read_size = stream->read(buffer, size_to_read);

		LOGGER_DEBUG(read_size);

		if (read_size==0)
			break;
		int wr = 0;
		// Ecriture iterative de la portion du flux d'entree dans le flux de sortie
		while(wr < read_size) {
			// Taille ecrite dans le flux de sortie
			int w = FCGX_PutStr((char*)(buffer + wr), read_size,request->out);
			if(w < 0) {
				LOGGER_ERROR("Echec d'écriture dans le flux de sortie de la requête FCGI " << request->requestId);
				displayFCGIError(FCGX_GetError(request->out));
				delete stream;
				delete[] buffer;
				return -1;
			}
			wr += w;
		}
		if(wr != read_size) {
			LOGGER_DEBUG( "Nombre incorrect d'octets ecrits dans le flux de sortie" );
			delete stream;
			delete[] buffer;
			break;
		}
		pos += read_size;
	}

	delete stream;
	delete[] buffer;
	return 0;
}
