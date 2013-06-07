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

/**
 * \file FileImage.cpp
 ** \~french
 * \brief Implémentation des classes FileImage et FileImageFactory
 * \details
 * \li FileImage : image physique, attaché à un fichier
 * \li FileImageFactory : usine de création d'objet FileImage
 ** \~english
 * \brief Implement classes FileImage and FileImageFactory
 * \details
 * \li FileImage : physical image, linked to a file
 * \li FileImageFactory : factory to create FileImage object
 */

#include "FileImage.h"
#include "Logger.h"
#include "Utils.h"
#include "LibtiffImage.h"

/* ------------------------------------------------------------------------------------------------ */
/* -------------------------------------------- USINES -------------------------------------------- */

/* ----- Pour la lecture ----- */
FileImage* FileImageFactory::createImageToRead ( char* name, BoundingBox< double > bbox, double resx, double resy ) {

    // Récupération de l'extension du fichier
    char * pch;
    char extension[3];
    pch = strrchr(name,'.');

    memcpy(extension, pch, 3);

    // TIFF
    if (strncmp(extension, "tif", 3) == 0) {
        LOGGER_DEBUG("TIFF image to read");
        
        LibtiffImageFactory LTIF;
        return LTIF.createLibtiffImageToRead( name, bbox, resx, resy );
    }

    // PNG
    else if (strncmp(extension, "png", 3) == 0) {
        LOGGER_ERROR("PNG image to read : NOT YET IMPLEMENTED");
        return NULL;
    }

    // JPEG 2000
    else if (strncmp(extension, "jp2", 3) == 0) {
        LOGGER_ERROR("JPEG2000 image to read : NOT YET IMPLEMENTED");
        return NULL;
    }

    // Format inconnu en lecture
    else {
        LOGGER_ERROR("Unknown image's format, to read : " << name);
        return NULL;
    }

}

/* ----- Pour l'écriture ----- */
FileImage* FileImageFactory::createImageToWrite (
    char* name, BoundingBox<double> bbox, double resx, double resy, int width, int height, int channels,
    SampleType sampleType, Photometric::ePhotometric photometric, Compression::eCompression compression ) {

    // Récupération de l'extension du fichier
    char * pch;
    char extension[3];
    pch = strrchr(name,'.');

    memcpy(extension, pch, 3);

    // TIFF
    if (strncmp(extension, "tif", 3) == 0) {
        LOGGER_DEBUG("TIFF image to write");

        LibtiffImageFactory LTIF;
        return LTIF.createLibtiffImageToWrite(  name, bbox, resx, resy, width, height, channels, sampleType, photometric, compression, 16 );
    }

    // Format inconnu en écriture
    else {
        LOGGER_ERROR("Unknown image's format, to write : " << name);
        return NULL;
    }
    
}

/* ------------------------------------------------------------------------------------------------ */
/* ----------------------------------------- CONSTRUCTEUR ----------------------------------------- */

FileImage::FileImage ( int width,int height, double resx, double resy, int channels, BoundingBox<double> bbox,
                       char* name, SampleType sampleType, Photometric::ePhotometric photometric, Compression::eCompression compression ) :
                       
    Image ( width,height,channels,resx,resy,bbox ), ST ( sampleType ), photometric ( photometric ), compression ( compression ) {
        
    filename = new char[IMAGE_MAX_FILENAME_LENGTH];
    strcpy ( filename,name );
}
