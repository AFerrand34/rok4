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
 * \file ExtendedCompoundImage.cpp
 ** \~french
 * \brief Implémentation des classes ExtendedCompoundImage, ExtendedCompoundMask et ExtendedCompoundImageFactory
 * \details
 * \li ExtendedCompoundImage : image composée d'images compatibles, superposables
 * \li ExtendedCompoundMask : masque composé, associé à une image composée
 * \li ExtendedCompoundImageFactory : usine de création d'objet ExtendedCompoundImage
 ** \~english
 * \brief Implement classes ExtendedCompoundImage, ExtendedCompoundMask and ExtendedCompoundImageFactory
 * \details
 * \li ExtendedCompoundImage : image compounded with superimpose images
 * \li ExtendedCompoundMask : compounded mask, associated with a compounded image
 * \li ExtendedCompoundImageFactory : factory to create ExtendedCompoundImage object
 */

#include "ExtendedCompoundImage.h"
#include "Logger.h"
#include "Utils.h"

#ifndef __max
#define __max(a, b)   ( ((a) > (b)) ? (a) : (b) )
#endif
#ifndef __min
#define __min(a, b)   ( ((a) < (b)) ? (a) : (b) )
#endif

template <typename T>
int ExtendedCompoundImage::_getline ( T* buffer, int line ) {
    int i;

    for ( i=0; i<width*channels; i++ ) {
        buffer[i]= ( T ) nodata[i%channels];
    }

    double y = l2y ( line );

    for ( i=0; i < ( int ) images.size(); i++ ) {
        // On ecarte les images qui ne se trouvent pas sur la ligne
        // On evite de comparer des coordonnees terrain (comparaison de flottants)
        // Les coordonnees image sont obtenues en arrondissant au pixel le plus proche

        if ( y2l ( images[i]->getYmin() ) <= line || y2l ( images[i]->getYmax() ) > line ) {
            continue;
        }
        if ( images[i]->getXmin() >= getXmax() || images[i]->getXmax() <= getXmin() ) {
            continue;
        }

        // c0 : indice de la 1ere colonne dans l'ExtendedCompoundImage de son intersection avec l'image courante
        int c0=__max ( 0,x2c ( images[i]->getXmin() ) );
        // c1-1 : indice de la derniere colonne dans l'ExtendedCompoundImage de son intersection avec l'image courante
        int c1=__min ( width,x2c ( images[i]->getXmax() ) );

        // c2 : indice de de la 1ere colonne de l'ExtendedCompoundImage dans l'image courante
        int c2=- ( __min ( 0,x2c ( images[i]->getXmin() ) ) );

        T* buffer_t = new T[images[i]->width*images[i]->channels];
        LOGGER_DEBUG ( i<<" "<<line<<" "<<images[i]->y2l ( y ) );
        images[i]->getline ( buffer_t,images[i]->y2l ( y ) );

        if ( getMask ( i ) == NULL ) {
            memcpy ( &buffer[c0*channels], &buffer_t[c2*channels], ( c1-c0 ) *channels*sizeof ( T ) );
        } else {

            uint8_t* buffer_m = new uint8_t[getMask ( i )->width];
            getMask ( i )->getline ( buffer_m,getMask ( i )->y2l ( y ) );

            for ( int j=0; j < c1-c0; j++ ) {
                if ( buffer_m[c2+j] ) {
                    if ( c2+j >= images[i]->width ) {
                        // On dépasse la largeur de l'image courante (arrondis). On passe.
                        // Une sortie pour vérifier si ce cas se représente malgré les corrections
                        LOGGER_ERROR ( "Dépassement : demande la colonne "<<c2+j+1<<" sur "<<images[i]->width );
                        continue;
                    }
                    memcpy ( &buffer[ ( c0+j ) *channels],&buffer_t[ ( c2+j ) *channels],sizeof ( T ) *channels );
                }
            }

            delete [] buffer_m;
        }
        delete [] buffer_t;
    }
    return width*channels*sizeof ( T );
}


/* Implementation de getline pour les uint8_t */
int ExtendedCompoundImage::getline ( uint8_t* buffer, int line ) {
    return _getline ( buffer, line );
}

/* Implementation de getline pour les float */
int ExtendedCompoundImage::getline ( float* buffer, int line ) {
    return _getline ( buffer, line );
}

ExtendedCompoundImage* ExtendedCompoundImageFactory::createExtendedCompoundImage (
    std::vector<Image*>& images, int* nodata, uint mirrors ) {
    
    if ( images.size() ==0 ) {
        LOGGER_ERROR ( "No source images to defined compounded image" );
        return NULL;
    }

    for ( int i=0; i<images.size()-1; i++ ) {
        if ( ! images[i]->isCompatibleWith ( images[i+1] ) ) {
            LOGGER_ERROR ( "Source images are not consistent" );
            LOGGER_ERROR ( "Image " << i );
            images[i]->print();
            LOGGER_ERROR ( "Image " << i+1 );
            images[i+1]->print();
            return NULL;
        }
    }

    // Rectangle englobant des images d entree
    double xmin=1E12, ymin=1E12, xmax=-1E12, ymax=-1E12 ;
    for ( unsigned int j=0; j<images.size(); j++ ) {
        if ( images.at ( j )->getXmin() <xmin )  xmin=images.at ( j )->getXmin();
        if ( images.at ( j )->getYmin() <ymin )  ymin=images.at ( j )->getYmin();
        if ( images.at ( j )->getXmax() >xmax )  xmax=images.at ( j )->getXmax();
        if ( images.at ( j )->getYmax() >ymax )  ymax=images.at ( j )->getYmax();
    }

    int w= ( int ) ( ( xmax-xmin ) / ( *images.begin() )->getResX() +0.5 );
    int h= ( int ) ( ( ymax-ymin ) / ( *images.begin() )->getResY() +0.5 );

    return new ExtendedCompoundImage ( w,h,images.at ( 0 )->channels,BoundingBox<double> ( xmin,ymin,xmax,ymax ),
                                       images,nodata,mirrors );
}

ExtendedCompoundImage* ExtendedCompoundImageFactory::createExtendedCompoundImage (
    int width, int height, int channels, BoundingBox<double> bbox,
    std::vector<Image*>& images, int* nodata, uint mirrors ) {
    
    if ( images.size() ==0 ) {
        LOGGER_ERROR ( "No source images to defined compounded image" );
        return NULL;
    }

    for ( int i=0; i<images.size()-1; i++ ) {
        if ( ! images[i]->isCompatibleWith ( images[i+1] ) ) {
            LOGGER_ERROR ( "Source images are not consistent" );
            LOGGER_ERROR ( "Image " << i );
            images[i]->print();
            LOGGER_ERROR ( "Image " << i+1 );
            images[i+1]->print();
            return NULL;
        }
    }

    return new ExtendedCompoundImage ( width,height,channels,bbox,images,nodata,mirrors );
}

int ExtendedCompoundMask::_getline ( uint8_t* buffer, int line ) {

    memset ( buffer,0,width );

    for ( uint i = ECI->getMirrorsNumber(); i < ECI->getImages()->size(); i++ ) {
        /* On ecarte les images qui ne se trouvent pas sur la ligne
         * On evite de comparer des coordonnees terrain (comparaison de flottants)
         * Les coordonnees image sont obtenues en arrondissant au pixel le plus proche
         */
        if ( y2l ( ECI->getImages()->at ( i )->getYmin() ) <= line || y2l ( ECI->getImages()->at ( i )->getYmax() ) > line ) {
            continue;
        }
        if ( ECI->getImages()->at ( i )->getXmin() >= getXmax() || ECI->getImages()->at ( i )->getXmax() <= getXmin() ) {
            continue;
        }

        // c0 : indice de la 1ere colonne dans l'ExtendedCompoundImage de son intersection avec l'image courante
        int c0=__max ( 0,x2c ( ECI->getImages()->at ( i )->getXmin() ) );
        // c1-1 : indice de la derniere colonne dans l'ExtendedCompoundImage de son intersection avec l'image courante
        int c1=__min ( width,x2c ( ECI->getImages()->at ( i )->getXmax() ) );

        // c2 : indice de de la 1ere colonne de l'ExtendedCompoundImage dans l'image courante
        int c2=- ( __min ( 0,x2c ( ECI->getImages()->at ( i )->getXmin() ) ) );

        if ( ECI->getMask ( i ) == NULL ) {
            memset ( &buffer[c0], 255, c1-c0 );
        } else {
            // Récupération du masque de l'image courante de l'ECI.
            uint8_t* buffer_m = new uint8_t[ECI->getMask ( i )->width];
            ECI->getMask ( i )->getline ( buffer_m,ECI->getMask ( i )->y2l ( l2y ( line ) ) );
            // On ajoute au masque actuel (on écrase si la valeur est différente de 0)
            for ( int j = 0; j < c1-c0; j++ ) {
                if ( buffer_m[c2+j] ) {
                    memcpy ( &buffer[c0+j],&buffer_m[c2+j],1 );
                }
            }
            delete [] buffer_m;
        }
    }

    return width;
}

/* Implementation de getline pour les uint8_t */
int ExtendedCompoundMask::getline ( uint8_t* buffer, int line ) {
    return _getline ( buffer, line );
}

/* Implementation de getline pour les float */
int ExtendedCompoundMask::getline ( float* buffer, int line ) {
    uint8_t* buffer_t = new uint8_t[width*channels];
    getline ( buffer_t,line );
    convert ( buffer,buffer_t,width*channels );
    delete [] buffer_t;
    return width*channels;
}
