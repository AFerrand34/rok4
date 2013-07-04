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
 * \file FileImage.h
 ** \~french
 * \brief Définition des classes FileImage et FileImageFactory
 * \details
 * \li FileImage : gestion d'un image attachée à un fichier
 * \li FileImageFactory : usine de création d'objet FileImage
 ** \~english
 * \brief Define classes FileImage and FileImageFactory
 * \details
 * \li FileImage : manage an image linked to a file
 * \li FileImageFactory : factory to create FileImage object
 */

#ifndef FILEIMAGE_H
#define FILEIMAGE_H

#include "Image.h"
#include <string.h>
#include "Format.h"

#define IMAGE_MAX_FILENAME_LENGTH 512

/**
 * \author Institut national de l'information géographique et forestière
 * \~french
 * \brief Manipulation d'une image associée à un fichier
 */
class FileImage : public Image {

protected:
    /**
     * \~french \brief Chemin du fichier image
     * \~english \brief Path to th image file
     */
    char* filename;
    /**
     * \~french \brief Photométrie des données (rgb, gray...)
     * \~english \brief Data photometric (rgb, gray...)
     */
    Photometric::ePhotometric photometric;
    /**
     * \~french \brief Compression des données (jpeg, packbits...)
     * \~english \brief Data compression (jpeg, packbits...)
     */
    Compression::eCompression compression;
    /**
     * \~french \brief Format des canaux
     * \~english \brief Sample format
     */
    SampleFormat::eSampleFormat sampleformat;
    /**
     * \~french \brief Nombre de bits par canal
     * \~english \brief Number of bits per sample
     */
    int bitspersample;

    /** \~french
     * \brief Crée un objet FileImage à partir de tous ses éléments constitutifs
     * \details Ce constructeur n'est appelé que par les constructeurs des classes filles
     * \param[in] width largeur de l'image en pixel
     * \param[in] height hauteur de l'image en pixel
     * \param[in] resx résolution dans le sens des X
     * \param[in] resy résolution dans le sens des Y
     * \param[in] channel nombre de canaux par pixel
     * \param[in] bbox emprise rectangulaire de l'image
     * \param[in] name chemin du fichier image
     * \param[in] sampleformat format des canaux
     * \param[in] bitspersample nombre de bits par canal
     * \param[in] photometric photométrie des données
     * \param[in] compression compression des données
     ** \~english
     * \brief Create a FileImage object, from all attributes
     * \param[in] width image width, in pixel
     * \param[in] height image height, in pixel
     * \param[in] resx X wise resolution
     * \param[in] resy Y wise resolution
     * \param[in] channel number of samples per pixel
     * \param[in] bbox bounding box
     * \param[in] name path to image file
     * \param[in] sampleformat samples' format
     * \param[in] bitspersample number of bits per sample
     * \param[in] photometric data photometric
     * \param[in] compression data compression
     */
    FileImage (
        int width, int height, double resx, double resy, int channels, BoundingBox< double > bbox, char* name,
        SampleFormat::eSampleFormat sampleformat, int bitspersample, Photometric::ePhotometric photometric, Compression::eCompression compression
    );

public:

    virtual int getline ( uint8_t *buffer, int line ) = 0;
    virtual int getline ( float *buffer, int line ) = 0;

    /**
     * \~french
     * \brief Retourne une partie de l'image
     * \details On définit la partie l'image en renseignant les coordonnées du pixel en haut à gauche, ainsi que les dimensions (en pixels) de la partie voulue.
     * \param[in] offsetx coordonnée en X du pixel en haut à gauche de la partie de l'image voulue
     * \param[in] offsety coordonnée en Y du pixel en haut à gauche de la partie de l'image voulue
     * \param[in] w largeur en pixel de la partie de l'image voulue
     * \param[in] l hauteur en pixel de la partie de l'image voulue
     * \param[out] buffer buffer dans le quel stocké le résultat. Il doit être suffisemment grand.
     * \return 0 en cas de succes, -1 sinon
     */
    int getTile ( int offsetx, int offsety, int w, int l, uint8_t* buffer ) {

        int tileLineByteSize = w * getPixelByteSize();

        if ( bitspersample == 8 && sampleformat == SampleFormat::UINT ) {
            uint8_t line[width * channels];
            
            for ( int y = 0; y < l; y++ ) {
                getline (line, offsety + y);
                memcpy ( buffer + y * tileLineByteSize, line + offsetx * channels, tileLineByteSize );
            }

        } else if ( bitspersample == 32 && sampleformat == SampleFormat::FLOAT ) {

            float line[width * channels];

            for ( int y = 0; y < l; y++ ) {
                getline (line, offsety + y);
                memcpy ( buffer + y * tileLineByteSize, line + offsetx * channels, tileLineByteSize );
            }

        }
        
        return 1;
    }

    /**
     * \~french
     * \brief Ecrit une image, à partir d'une image source
     * \details Toutes les informations nécessaires à l'écriture d'une image sont dans l'objet FileImage, sauf les données à écrire. On renseigne cela via une seconde image.
     * \param[in] pIn source des donnée de l'image à écrire
     * \return 0 en cas de succes, -1 sinon
     */
    virtual int writeImage ( Image* pIn ) = 0;

    /**
     * \~french
     * \brief Ecrit une image, à partir d'un buffer d'entiers
     * \param[in] buffer source des donnée de l'image à écrire
     * \return 0 en cas de succes, -1 sinon
     */
    virtual int writeImage ( uint8_t* buffer ) = 0;

    /**
     * \~french
     * \brief Ecrit une image, à partir d'un buffer de flottants
     * \param[in] buffer source des donnée de l'image à écrire
     * \return 0 en cas de succes, -1 sinon
     */
    virtual int writeImage ( float* buffer ) = 0;

    /**
     * \~french
     * \brief Ecrit une ligne de l'image, à partir d'un buffer d'entiers
     * \param[in] buffer source des donnée de l'image à écrire
     * \param[in] line ligne de l'image à écrire
     * \return 0 en cas de succes, -1 sinon
     */
    virtual int writeLine ( uint8_t* buffer, int line ) = 0;

    /**
     * \~french
     * \brief Ecrit une ligne de l'image, à partir d'un buffer de flottants
     * \param[in] buffer source des donnée de l'image à écrire
     * \param[in] line ligne de l'image à écrire
     * \return 0 en cas de succes, -1 sinon
     */
    virtual int writeLine ( float* buffer, int line ) = 0;

    /**
     * \~french
     * \brief Retourne le chemin du fichier image
     * \return chemin image
     * \~english
     * \brief Return the path to image file
     * \return image's path
     */
    inline char* getFilename() {
        return filename;
    }
    /**
     * \~french
     * \brief Retourne le nombre de bits par canal
     * \return nombre de bits par canal
     * \~english
     * \brief Return number of bits per sample
     * \return number of bits per sample
     */
    inline int getBitsPerSample() {
        return bitspersample;
    }
    /**
     * \~french
     * \brief Retourne la photométrie des données image (rgb, gray...)
     * \return photométie
     * \~english
     * \brief Return data photometric (rgb, gray...)
     * \return photometric
     */
    inline Photometric::ePhotometric getPhotometric() {
        return photometric;
    }
    /**
     * \~french
     * \brief Retourne la compression des données
     * \return compression
     * \~english
     * \brief Return data compression
     * \return compression
     */
    inline Compression::eCompression getCompression() {
        return compression;
    }

    /**
     * \~french
     * \brief Retourne le format des canaux (entier, flottant)
     * \return format des canaux
     * \~english
     * \brief Return sample format (integer, float)
     * \return sample format
     */
    inline SampleFormat::eSampleFormat getSampleFormat() {
        return sampleformat;
    }

    /**
     * \~french
     * \brief Retourne la taille en octet d'un pixel
     * \~english
     * \brief Return the pixel's byte size
     */
    inline int getPixelByteSize () {
        return bitspersample * channels / 8;
    }

    /**
     * \~french
     * \brief Destructeur par défaut
     * \~english
     * \brief Default destructor
     */
    ~FileImage() {
        delete [] filename;
    }

    /** \~french
     * \brief Sortie des informations sur l'image
     ** \~english
     * \brief File image description output
     */
    void print() {
        LOGGER_INFO ( "" );
        LOGGER_INFO ( "------------ FileImage -------------" );
        Image::print();
        LOGGER_INFO ( "\t- File name : " << filename );
        LOGGER_INFO ( "\t- Compression : " << Compression::toString ( compression ) );
        LOGGER_INFO ( "\t- Photometric : " << Photometric::toString ( photometric ) );
        LOGGER_INFO ( "\t- Bits per sample : " << bitspersample );
        LOGGER_INFO ( "\t- Sample format : " << SampleFormat::toString ( sampleformat ) );
        LOGGER_INFO ( "" );
    }
};

/** \~ \author Institut national de l'information géographique et forestière
 ** \~french
 * \brief Usine de création d'une image associée à un fichier
 * \details Il est nécessaire de passer par cette classe pour créer des objets d'une classe fille de la classe FileImage. Cela permet de savoir de quelle classe fille instancier un objet, selon l'extension du chemin fourni.
 */
class FileImageFactory {
public:
    /** \~french
     * \brief Crée un objet FileImage, pour la lecture
     * \details On considère que les informations d'emprise et de résolutions ne sont pas présentes dans le fichier, on les précise donc à l'usine. Tout le reste sera lu dans les en-têtes. On vérifiera aussi la cohérence entre les emprise et résolutions fournies et les dimensions récupérées dans le fichier.
     * \param[in] filename chemin du fichier image
     * \param[in] bbox emprise rectangulaire de l'image
     * \param[in] resx résolution dans le sens des X.
     * \param[in] resy résolution dans le sens des Y.
     * \return un pointeur d'objet d'une classe fille de FileImage, NULL en cas d'erreur
     ** \~english
     * \brief Create a FileImage object, for reading
     * \details Bbox and resolutions are not present in the file, so we precise them. All other informations are extracted from header. We have to check consistency between provided bbox and resolutions and read image's dimensions.
     * \param[in] filename path to image file
     * \param[in] bbox bounding box
     * \param[in] resx X wise resolution.
     * \param[in] resy Y wise resolution.
     * \return a FileImage's child class object pointer, NULL if error
     */
    FileImage* createImageToRead ( char* filename, BoundingBox<double> bbox = BoundingBox<double>(0,0,0,0), double resx = -1, double resy = -1 );

    /** \~french
     * \brief Crée un objet FileImage, pour l'écriture
     * \details Toutes les méta-informations sur l'image doivent être précisées pour écrire l'en-tête de l'image. Rien n'est calculé
     * \param[in] filename chemin du fichier image
     * \param[in] bbox emprise rectangulaire de l'image
     * \param[in] resx résolution dans le sens des X.
     * \param[in] resy résolution dans le sens des Y.
     * \param[in] width largeur de l'image en pixel
     * \param[in] height hauteur de l'image en pixel
     * \param[in] channel nombre de canaux par pixel
     * \param[in] sampleformat format des canaux
     * \param[in] bitspersample nombre de bits par canal
     * \param[in] photometric photométie des données
     * \param[in] compression compression des données
     * \return un pointeur d'objet d'une classe fille de FileImage, NULL en cas d'erreur
     ** \~english
     * \brief Create an FileImage object, for writting
     * \details All informations have to be provided to be written in the header. No calculation.
     * \param[in] filename path to image file
     * \param[in] bbox bounding box
     * \param[in] resx X wise resolution.
     * \param[in] resy Y wise resolution.
     * \param[in] width image width, in pixel
     * \param[in] height image height, in pixel
     * \param[in] channel number of samples per pixel
     * \param[in] sampleformat samples' format
     * \param[in] bitspersample number of bits per sample
     * \param[in] photometric data photometric
     * \param[in] compression data compression
     * \return a FileImage's child class object pointer, NULL if error
     */
    FileImage* createImageToWrite (
        char* filename, BoundingBox<double> bbox, double resx, double resy, int width, int height,
        int channels, SampleFormat::eSampleFormat sampleformat, int bitspersample, Photometric::ePhotometric photometric, Compression::eCompression compression
    );

};


#endif

