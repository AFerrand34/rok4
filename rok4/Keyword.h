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

#ifndef KEYWORD_H
#define KEYWORD_H
#include <string>
#include <map>

typedef std::pair<std::string,std::string> attribute;

class Keyword {
private:
    std::string content;
    std::map<std::string,std::string> attributes;
public:
    Keyword ( std::string content, std::map<std::string,std::string> attributes );
    Keyword ( const Keyword &origKW );
    Keyword& operator= (Keyword const& other);
    
    inline const std::string getContent() const {
        return content;
    }
    inline const std::map<std::string,std::string>* getAttributes() const {
        return &attributes;
    }
    
    inline bool hasAttributes() const {
        return !(attributes.empty());
    }
    
    bool operator== ( const Keyword& other ) const;
    bool operator!= ( const Keyword& other ) const;

    virtual ~Keyword();

};

#endif // KEYWORD_H