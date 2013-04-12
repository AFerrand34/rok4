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
 * \file manageNodata.cpp
 * \author Institut national de l'information géographique et forestière
 * \~french \brief Gère la couleur des pixels de nodata
 * \~english \brief Manage nodata pixel color
 * \~french \details Cet outil est destiné à identifier et modifier les pixels qui contiennent une certaine valeur dans une image TIFF à canaux entiers non signés sur 8 bits. Ceci est utile lorsque l'on souhaite modifier la couleur de nodata ou la supprimer des données.
 *
 * On définit deux fichiers :
 * \li l'image en entrée, à modifier
 * \li l'image en sortie
 *
 * On va également définir 3 couleurs en paramètre :
 * \li la couleur cible : les pixels de cette couleur sont ceux potentiellement modifiés
 * \li la nouvelle couleur de nodata
 * \li la nouvelle couleur de donnée
 *
 * Ainsi, les pixels contenant la couleur cible sont passés :
 * \li en nouvelle couleur de nodata si ils touchent le bord
 * \li en nouvelle couleur de donnée sinon
 *
 * \~ \image html manageNodata.png \~french
 *
 * Cet outil n'est qu'une interface permettant l'utilisation de la classe TiffNodataManager, qui réalise réellement tous les traitements.
 *
 * Le nombre de canaux du fichier en entrée et les valeurs de nodata renseignée doivent être cohérents.
 */

using namespace std;

#include "TiffNodataManager.h"
#include "Logger.h"
#include <cstdlib>
#include <iostream>
#include <string.h>
#include "../be4version.h"

/**
 * \~french
 * \brief Affiche l'utilisation et les différentes options de la commande manageNodata
 * \details L'affichage se fait dans la sortie d'erreur
 *
 * \~ \code
 * manageNodata version X.X.X
 *
 * Manage nodata pixel color in a TIFF file
 *
 * Usage: manageNodata -target <VAL> -nodata <VAL> [-data <VAL>] <INPUT FILE> <OUTPUT FILE>
 *
 * Colors are provided in decimal format, one integer value per sample
 * Parameters:
 *      -target color to modify in input image
 *      -data new color for data pixel which contained target color
 *      -nodata new color for nodata pixel, which contained target color and linked to borders
 *      -channels samples per pixel : 1, 3 or 4
 *
 * Example, to keep pure white for nodata :
 *      manageNodata -target 255,255,255 -nodata 255,255,255 -data 254,254,254 old_image.tif new_image.tif
 * \endcode
 */
void usage() {

    std::cerr << "\nmanageNodata version " << BE4_VERSION << "\n\n" <<

              "Manage nodata pixel color in a TIFF file, byte samples\n\n" <<

              "Usage: manageNodata -target <VAL> -nodata <VAL> [-data <VAL>] <INPUT FILE> <OUTPUT FILE>\n\n" <<

              "Colors are provided in decimal format, one integer value per sample\n" <<
              "Parameters:\n" <<
              "     -target color to modify in input image\n" <<
              "     -data new color for data pixel which contained target color\n" <<
              "     -nodata new color for nodata pixel, which contained target color and linked to borders\n" <<
              "     -channels samples per pixel : 1, 3 or 4\n\n" <<

              "Example, to keep pure white for nodata : \n" <<
              "     manageNodata -target 255,255,255 -nodata 255,255,255 -data 254,254,254 old_image.tif new_image.tif\n" << std::endl;
}

/**
 * \~french
 * \brief Affiche un message d'erreur, l'utilisation de la commande et sort en erreur
 * \param[in] message message d'erreur
 * \param[in] errorCode code de retour, -1 par défaut
 */
void error ( std::string message, int errorCode = -1 ) {
    LOGGER_ERROR ( message );
    usage();
    sleep ( 1 );
    exit ( errorCode );
}

/**
 ** \~french
 * \brief Fonction principale de l'outil manageNodata
 * \details Tout est contenu dans cette fonction.
 * \param[in] argc nombre de paramètres
 * \param[in] argv tableau des paramètres
 * \return code de retour, 0 en cas de succès, -1 sinon
 ** \~english
 * \brief Main function for tool manageNodata
 * \details All instrcutions are in this function.
 * \param[in] argc parameters number
 * \param[in] argv parameters array
 * \return return code, 0 if success, -1 otherwise
 */
int main ( int argc, char* argv[] ) {
    char* input = 0;
    char* output = 0;

    char* strTargetValue = 0;
    char* strNewNodata = 0;
    char* strNewData = 0;

    int channels = 0;

    /* Initialisation des Loggers */
    Logger::setOutput ( STANDARD_OUTPUT_STREAM_FOR_ERRORS );

    Accumulator* acc = new StreamAccumulator();
    //Logger::setAccumulator(DEBUG, acc);
    Logger::setAccumulator ( INFO , acc );
    Logger::setAccumulator ( WARN , acc );
    Logger::setAccumulator ( ERROR, acc );
    Logger::setAccumulator ( FATAL, acc );

    std::ostream &logd = LOGGER ( DEBUG );
    logd.precision ( 16 );
    logd.setf ( std::ios::fixed,std::ios::floatfield );

    std::ostream &logw = LOGGER ( WARN );
    logw.precision ( 16 );
    logw.setf ( std::ios::fixed,std::ios::floatfield );

    for ( int i = 1; i < argc; i++ ) {
        if ( !strcmp ( argv[i],"-h" ) ) {
            usage();
            exit ( 0 );
        }
        if ( !strcmp ( argv[i],"-target" ) ) {
            if ( i++ >= argc ) error ( "Error with option -target",-1 );
            strTargetValue = argv[i];
            continue;
        } else if ( !strcmp ( argv[i],"-nodata" ) ) {
            if ( i++ >= argc ) error ( "Error with option -nodata",-1 );
            strNewNodata = argv[i];
            continue;
        } else if ( !strcmp ( argv[i],"-data" ) ) {
            if ( i++ >= argc ) error ( "Error with option -data",-1 );
            strNewData = argv[i];
            continue;
        } else if ( !strcmp ( argv[i],"-channels" ) ) {
            if ( i++ >= argc ) error ( "Error with option -channels",-1 );
            channels = atoi ( argv[i] );
            continue;
        } else if ( !input ) {
            input = argv[i];
        } else if ( !output ) {
            output = argv[i];
        } else {
            error ( "Error : unknown option : " + string ( argv[i] ),-1 );
        }
    }

    if ( ! input ) error ( "Missing input file",-1 );
    if ( ! output ) error ( "Missing output file",-1 );
    if ( ! channels ) error ( "Missing number of samples per pixel",-1 );
    if ( ! strTargetValue ) error ( "Missing target color",-1 );
    if ( ! strNewNodata && ! strNewData ) error ( "What have we to do with the target color ? Precise a new nodata or data color",-1 );

    uint8_t* targetValue = new uint8_t[channels];
    uint8_t* newNodata = new uint8_t[channels];
    uint8_t* newData = new uint8_t[channels];

    // Target value interpretation
    char* charValue = strtok ( strTargetValue,"," );
    if ( charValue == NULL ) {
        error ( "Error with option -target : integer values (between 0 and 255) seperated by comma",-1 );
    }
    int value = atoi ( charValue );
    if ( value < 0 || value > 255 ) {
        error ( "Error with option -target : integer values (between 0 and 255) seperated by comma",-1 );
    }
    targetValue[0] = value;
    for ( int i = 1; i < channels; i++ ) {
        charValue = strtok ( NULL, "," );
        if ( charValue == NULL ) {
            error ( "Error with option -oldValue : integer values (between 0 and 255) seperated by comma",-1 );
        }
        value = atoi ( charValue );
        if ( value < 0 || value > 255 ) {
            error ( "Error with option -oldValue : integer values (between 0 and 255) seperated by comma",-1 );
        }
        targetValue[i] = value;
    }

    // New nodata interpretation
    if ( strNewNodata ) {
        charValue = strtok ( strNewNodata,"," );
        if ( charValue == NULL ) {
            error ( "Error with option -nodata : integer values (between 0 and 255) seperated by comma",-1 );
        }
        value = atoi ( charValue );
        if ( value < 0 || value > 255 ) {
            error ( "Error with option -nodata : integer values (between 0 and 255) seperated by comma",-1 );
        }
        newNodata[0] = value;
        for ( int i = 1; i < channels; i++ ) {
            charValue = strtok ( NULL, "," );
            if ( charValue == NULL ) {
                error ( "Error with option -nodata : integer values (between 0 and 255) seperated by comma",-1 );
            }
            value = atoi ( charValue );
            if ( value < 0 || value > 255 ) {
                error ( "Error with option -nodata : integer values (between 0 and 255) seperated by comma",-1 );
            }
            newNodata[i] = value;
        }
    }

    // New data interpretation
    if ( strNewData ) {
        charValue = strtok ( strNewData,"," );
        if ( charValue == NULL ) {
            error ( "Error with option -data : integer values (between 0 and 255) seperated by comma",-1 );
        }
        value = atoi ( charValue );
        if ( value < 0 || value > 255 ) {
            error ( "Error with option -data : integer values (between 0 and 255) seperated by comma",-1 );
        }
        newData[0] = value;
        for ( int i = 1; i < channels; i++ ) {
            charValue = strtok ( NULL, "," );
            if ( charValue == NULL ) {
                error ( "Error with option -data : integer values (between 0 and 255) seperated by comma",-1 );
            }
            value = atoi ( charValue );
            if ( value < 0 || value > 255 ) {
                error ( "Error with option -data : integer values (between 0 and 255) seperated by comma",-1 );
            }
            newData[i] = value;
        }
    } else {
        // Pas de nouvelle couleur pour la donnée : elle a la valeur de la couleur cible
        newData = targetValue;
    }

    if ( ! strNewNodata ) {
        // On ne précise pas de nouvelle couleur de non-donnée, elle est la même que la couleur cible.
        newNodata = targetValue;
    }

    TiffNodataManager TNM ( channels,targetValue,newData,newNodata );

    if ( ! TNM.treatNodata ( input,output ) ) {
        error ( "Error : unable to treat nodata for this file : " + string ( input ),-1 );
    }

    delete[] targetValue;
    delete[] newData;
    delete[] newNodata;

    return 0;
}