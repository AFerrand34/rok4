/*
 * Copyright © (2011-2013) Institut national de l'information
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
 * \file Style.cpp
 * \~french
 * \brief Implémentation de la classe Style modélisant les styles.
 * \~english
 * \brief Implement the Style Class handling style definition
 */

#include "Style.h"
#include "Logger.h"
#include "intl.h"
#include "config.h"

Style::Style ( const StyleXML& s ) {
    LOGGER_DEBUG ( _ ( "Nouveau Style : " ) << s.id );
    this->id = s.id.c_str();
    this->titles = s.titles;
    this->abstracts = s.abstracts;
    this->keywords = s.keywords;
    this->legendURLs = s.legendURLs;
    this->palette = s.palette;
    if ( s.angle >= 0 && s.angle < 360 ) {
        estompage = true;
    }
}

Style::~Style() {

}

inline std::string Style::getId() { return id; }
inline std::vector<std::string> Style::getTitles() { return titles; }
inline std::vector<std::string> Style::getAbstracts() { return abstracts; }
inline std::vector<Keyword>* Style::getKeywords() { return &keywords; }
inline std::vector<LegendURL> Style::getLegendURLs() { return legendURLs; }
inline Palette* Style::getPalette() { return &palette; }
inline bool Style::isEstompage() { return estompage; }
inline int Style::getAngle() { return angle; }
inline float Style::getExaggeration() { return exaggeration; }
inline uint8_t Style::getCenter() { return center; }
