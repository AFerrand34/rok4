/*
 * Copyright © (2011) Institut national de l'information
 *                    géographique et forestière
 *
 * Géoportail SAV <geop_services@geoportail.fr>
 *
 * This software is a computer program whose purpose is to publish geographic
 * data using OGC WMS and WMTS protocol.
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software.  You can  use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 *
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#ifndef DATA_SOURCE_H
#define DATA_SOURCE_H

#include <stdint.h>// pour uint8_t
#include <cstddef> // pour size_t
#include <string>  // pour std::string
#include <cstring> // pour memcpy
#include <algorithm>

#include "Logger.h"

/**
 * Interface abstraite permetant d'encapsuler une source de données.
 * La gestion mémoire des données est à la charge des classes d'implémentation.
 */
class DataSource {
public:

    /** Destructeur virtuel */
    virtual ~DataSource() {}

    /**
     * Donne un accès direct mémoire en lecture aux données. Les données pointées sont en lecture seule.
     *
     * @return size Taille des données en octets (0 en cas d'échec)
     * @return Pointeur vers les données qui ne doit pas être utilisé après destruction ou libération des données (0 en cas d'échec)
     *
     */
    virtual const uint8_t* getData ( size_t &size ) = 0;

    /**
     * Libère les données mémoire allouées.
     *
     * Le pointeur obtenu par getData() ne doit plus être utilisé après un appel à releaseData().
     * Le choix de libérer effectivement les données est laissé à l'implémentation, un nouvel appel
     * à getData() doit pouvoir être possible après libération même si ce n'est pas la logique voulue.
     * Dans ce cas, la classe doit recharger en mémoire les données libérées.
     *
     * @return true en cas de succès.
     */
    virtual bool releaseData() = 0;

    /**
     * Indique le type MIME associé à la donnée source.
     */
    virtual std::string getType() = 0;

    /**
     * Indique le statut Http associé à la donnée source.
     */
    virtual int getHttpStatus() = 0;
    
    /**
     * Indique l'encodage Http associé à la donnée source.
     */
    virtual std::string getEncoding() = 0;
};


/**
 * Interface abstraite permetant d'encapsuler un flux de données.
 */
class DataStream {
public:

    /** Destructeur virtuel */
    virtual ~DataStream() {}

    /**
     * Lit les prochaines données du flux. Tout octet ne peut être lu qu'une seule fois.
     *
     * Copie au plus size octets de données non lues dans buffer. La valeur de retour indique le nombre
     * d'octets effectivement lus.
     *
     * Une valeur de retour 0 n'indique pas forcément la fin du flux, en effet il peut ne pas y avoir
     * assez de place dans buffer pour écrire les données. Ce genre de limitation est spécifique à chaque
     * classe filles qui peut pour des commodités d'implémentation ne pas vouloir tronquer certains blocs
     * de données.
     *
     * @param buffer Pointeur cible.
     * @param size Espace disponible dans buffer en octets.
     * @return Nombre d'octets effectivement récupérés.
     */
    virtual size_t read ( uint8_t *buffer, size_t size ) = 0;

    /**
     * Indique la fin du flux. read() renverra systématiquement 0 lorsque la fin du flux est atteinte.
     *
     * @return true s'il n'y a plus de données à lire.
     */
    virtual bool eof() = 0;

    /**
     * Indique le type MIME associé au flux.
     */
    virtual std::string getType() = 0;

    /**
     * Indique le statut Http associé au flux.
     */
    virtual int getHttpStatus() = 0;
    
    /**
     * Indique l'encodage associé au flux.
     */
    virtual std::string getEncoding() = 0;
};



/**
 * Classe transformant un DataStream en DataSource.
 */
class BufferedDataSource : public DataSource {
private:
    std::string type;
    std::string encoding;
    int httpStatus;
    size_t dataSize;
    uint8_t* data;
public:
    /**
     * Constructeur.
     * Le paramètre dataStream est complètement lu. Il est donc inutilisable par la suite.
     */
    BufferedDataSource ( DataStream& dataStream );

    /** Destructeur **/
    virtual ~BufferedDataSource() {
        delete[] data;
    }

    /** Implémentation de l'interface DataSource **/
    const uint8_t* getData ( size_t &size ) {
        size = dataSize;
        return data;
    }

    /**
     * Le buffer ne peut pas être libéré car on n'a pas de moyen de le reremplir pour un éventuel futur getData
     * @return false
     */
    bool releaseData() {
        return false;
    }

    /** @return le type du dataStream */
    std::string getType() {
        return type;
    }

    /** @return le status du dataStream */
    int getHttpStatus() {
        return httpStatus;
    }
    
     /** @return l'encodage du dataStream */
    std::string getEncoding() {
        return encoding;
    }

    /** @return la taille du buffer */
   size_t getSize() {
       return dataSize;
   }
};

/**
 * Classe de données brutes.
 */
class RawDataSource : public DataSource {
private:
    size_t dataSize;
    uint8_t* data;
public:
    /**
     * Constructeur.
     */
    RawDataSource ( uint8_t *dat, size_t dataS){
        dataSize = dataS;
        data = new uint8_t[dataSize];
        memcpy ( data, dat, dataSize );
    }

    /** Destructeur **/
    virtual ~RawDataSource() {
        delete[] data;
        data = 0;
    }

    /** Implémentation de l'interface DataSource **/
    const uint8_t* getData ( size_t &size ) {
        size = dataSize;
        return data;
    }

    /**
     * Le buffer ne peut pas être libéré car on n'a pas de moyen de le reremplir pour un éventuel futur getData
     * @return false
     */
    bool releaseData() {
        if (data)
          delete[] data;
        data = 0;
        return true;
    }

    /** @return le type du dataStream */
    std::string getType() {
        return "";
    }

    /** @return le status du dataStream */
    int getHttpStatus() {
        return 200;
    }

     /** @return l'encodage du dataStream */
    std::string getEncoding() {
        return "";
    }

    /** @return la taille du buffer */
   size_t getSize() {
       return dataSize;
   }
};

/**
 * Classe d'un flux de données brutes.
 */
class RawDataStream : public DataStream {
private:
    size_t dataSize;
    uint8_t* data;
    size_t pos;
public:
    /**
     * Constructeur.
     */
    RawDataStream ( uint8_t *dat, size_t dataS){
        dataSize = dataS;
        data = new uint8_t[dataSize];
        memcpy ( data, dat, dataSize );
        pos = 0;
    }

    /** Destructeur **/
    virtual ~RawDataStream() {
        delete[] data;
    }

    /** Implémentation de l'interface DataSource **/
    size_t read ( uint8_t *buffer, size_t size ) {
        if ( size > dataSize - pos ) size = dataSize - pos;
        memcpy ( buffer, ( uint8_t* ) ( data +pos ),size );
        pos+=size;
        return size;
    }

    bool eof() {
        return ( pos==dataSize );
    }

    /** @return le type du dataStream */
    std::string getType() {
        return "";
    }

    /** @return le status du dataStream */
    int getHttpStatus() {
        return 200;
    }

     /** @return l'encodage du dataStream */
    std::string getEncoding() {
        return "";
    }

    /** @return la taille du buffer */
   size_t getSize() {
       return dataSize;
   }
};

/**
 * Classe Transformant un DataSource en DataStream
 */
class DataStreamFromDataSource : public DataStream {
private:
    DataSource* datasource;
    size_t pos;
    const uint8_t* data;
    size_t data_size;
public:
    DataStreamFromDataSource ( DataSource* datasource ): datasource(datasource){
       data = datasource->getData(data_size);
       pos = 0;
    };
    ~DataStreamFromDataSource() {
        delete datasource;
    }
    size_t read ( uint8_t *buffer, size_t size ){
        size_t read = std::min<size_t>(data_size-pos, size);
        memcpy((void*) buffer, (void*)(data + pos) , read);
        pos += read;
        return read;
    };
    bool eof(){
        return data_size == pos;
    };
    std::string getType(){
        return datasource->getType();
    }
    int getHttpStatus(){
        return datasource->getHttpStatus();
    }
    std::string getEncoding(){
        return datasource->getEncoding();
    }
};



#endif
