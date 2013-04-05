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
 * \file TiffNodataManager.h
 ** \~french
 * \brief Définition de la classe TiffNodataManager, permettant de modifier la couleur de nodata des images à canal entier
 ** \~english
 * \brief Define classe TiffNodataManager, allowing to modify nodata color for byte sample image
 */

#ifndef _TIFFNODATAMANAGER_
#define _TIFFNODATAMANAGER_

#include <stdint.h>
#include <tiff.h>

/**
 * \author Institut national de l'information géographique et forestière
 * \~french
 * \brief Manipulation de la couleur de nodata
 * \details On veut pouvoir modifier la couleur de nodata d'images à canal entier, et ce pour des images qui ne possèdent pas de masque de données. De plus, on veut avoir la possibilité de supprimer une couleur d'une image pour la réserver au nodata. Typiquement, il est des cas ou le blanc pur doit être exclusivement utilisé pour caractériser des pixels de nodata. On va alors remplacer le blanc de donnée (neige, écume, hot spot...) par du "gris clair" (254,254,254).
 *
 * On va donc définir 3 couleurs :
 * \li la couleur cible : les pixels de cette couleur sont ceux potentiellement modifiés
 * \li la nouvelle couleur de nodata
 * \li la nouvelle couleur de donnée
 *
 * Et la classe va permettre les actions suivantes :
 * \li de modifier les pixels qui touchent le bord, dont la couleur est celle cible en nouvelle couleur de non-donnée, si ils touchent le bord (expliquer après)
 * \li de modifier les pixels dont la couleur est celle cible en nouvelle couleur de donnée, si ils ne touchent pas le bord.
 *
 * Les deux actions sont bien distinctes.
 *
 * On dit qu'un pixel "touche le bord" dès lors que l'on peut relier le pixel au bord en ne passant que par des pixels dont la couleur est celle cible. Techniquement, on commence par les bord puis on se propage vers l'intérieur de l'image.
 *
 * \~ \image html manageNodata.png \~french
 *
 * Les fonctions utilisent les loggers et doivent donc être initialisés par un des appelant.
 */
class TiffNodataManager {
private:
    /**
     * \~french \brief Nombre de canaux des couleurs du manager
     * \~english \brief Number of samples in manager colors
     */
    uint16 channels;

    /**
     * \~french \brief Couleur concerné par les modifications éventuelles
     * \~english \brief Color impacted by modifications
     */
    uint8_t *targetValue;
    /**
     * \~french \brief Nouvelle couleur pour les pixels de donnée
     * \details Elle peut être la même que la couleur cible #targetValue. Dans ce cas, on ne touche pas aux pixels de donnée.
     * \~english \brief New color for data
     * \details Could be the same as #targetValue.
     */
    uint8_t *dataValue;
    /**
     * \~french \brief Nouvelle couleur pour les pixels de non-donnée
     * \details Elle peut être la même que la couleur cible #targetValue. Dans ce cas, on ne touche pas aux pixels de non-donnée.
     * \~english \brief New color for nodata
     * \details Could be the same as #targetValue.
     */
    uint8_t *nodataValue;

    /**
     * \~french \brief Doit-on commencer par supprimer la couleur cible de l'image ?
     * \details Si on veut changer la couleur pour les données, on doit totalement suppimer la couleur cible de l'image, quitte à remettre dans la couleur initiale les pixels de non-donnée.
     * \~english \brief Have we to begin to remove target color in image ?
     */
    bool removeTargetValue;
    /**
     * \~french \brief Doit-on modifier les pixels ?
     * \details Si on veut changer la couleur pour les données, on doit totalement suppimer la couleur cible de l'image, quitte à remettre dans la couleur initiale les pixels de non-donnée.
     * \~english \brief Have we to begin to remove target color in image ?
     */
    bool newNodataValue;

    /**
     * \~french \brief Identifie et change la couleur des pixels qui touchent le bord
     * \details On identifie les pixels du bord dont la couleur est #dataValue, et on les ajoute dans une pile (onstocke la position en 1 dimension dans l'image). On gère en parallèle le masque de donnée, temporaire, pour mémoriser les pixels déjà identifié comme nodata.
     *
     * Itérativement, tant que la pile n'est pas vide, on en prend la tête et on considère les 4 pixels autour. Si ils sont de la couleur #dataValue et qu'ils n'ont pas déjà été traités (on le sait grâce au masque), on les ajoute à la pile. Ainsi de suite jusqu'à vider la pile.
     *
     * \param[in,out] IM image à modifier
     * \param[in] width largeur de l'image
     * \param[in] height hauteur de l'image
     * \param[in] samplesperpixel nombre de canaux de l'image
     * \~english \brief Identify et switch color of pixels which touch sides
     * \param[in,out] IM image to modify
     * \param[in] width image's width
     * \param[in] height image's height
     * \param[in] samplesperpixel image's number of samples per pixel
     */
    void changeNodataValue ( uint8_t* IM, uint32 width , uint32 height,uint16 samplesperpixel );

public:

    /** \~french
     * \brief Crée un objet TiffNodataManager à partir des différentes couleurs
     * \details Les booléens #removeTargetValue et #newNodataValue sont déduits des couleurs, si elles sont identiques ou non.
     * \param[in] channels nombre de canaux dans les couleurs
     * \param[in] targetValue couleur cible
     * \param[in] dataValue nouvelle couleur pour les données
     * \param[in] nodataValue nouvelle couleur pour les non-données
     ** \~english
     * \brief Create a TiffNodataManager object from three characteristic colors
     * \details Booleans #removeTargetValue and #newNodataValue are deduced from colors.
     * \param[in] channels colors' number of samples
     * \param[in] targetValue color to treat
     * \param[in] dataValue new color for data
     * \param[in] nodataValue new color for nodata
     */
    TiffNodataManager ( uint16 channels, uint8_t* targetValue, uint8_t* dataValue, uint8_t* nodataValue );

    /** \~french
     * \brief Fonction de traitement du manager, effectuant les modification de l'image
     * \details Elle utilise les booléens #removeTargetValue et #newNodataValue pour déterminer le travail à faire.
     * \param[in] input chemin de l'image à modifier
     * \param[in] output chemin de l'image de sortie
     * \return Vrai en cas de réussite, faux sinon
     ** \~english
     * \brief Manager treatment function, doing image's modifications
     * \details Use booleans #removeTargetValue and #newNodataValue to define what to do.
     * \param[in] input Image's path to modify
     * \param[in] output Output image path
     * \return True if success, false otherwise
     */
    bool treatNodata ( char* input, char* output );
};

#endif // _TIFFNODATAMANAGER_
