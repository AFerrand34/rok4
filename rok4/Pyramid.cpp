#include <cmath>
#include "Pyramid.h"
#include "Logger.h"
#include "Message.h"
#include "Grid.h"

/*

 */
DataSource* Pyramid::getTile(int x, int y, std::string tmId) {

	std::map<std::string, Level*>::const_iterator it=levels.find(tmId);

	// Cas normalement filtre en amont (exception WMTS)
	if (it==levels.end())
		LOGGER_ERROR("Erreur WMTS : niveau manquant");

	return it->second->getTile(x, y);
}

std::string Pyramid::best_level(double resolution_x, double resolution_y) {

	// TODO: A REFAIRE !!!!
	// res_level/resx ou resy ne doit pas exceder une certaine valeur
	double resolution = sqrt(resolution_x * resolution_y);

	std::map<std::string, Level*>::iterator it(levels.begin()), itend(levels.end());
	std::string best_h = it->first;
	double best = resolution_x / it->second->getRes();
	++it;
	for (;it!=itend;++it){
//		LOGGER_DEBUG("level teste:" << it->first << "\tmeilleur level:" << best_h << "\tnbre de level:" << levels.size() << "\tlevel.res:" << it->second->getRes());
		double d = resolution / it->second->getRes();
		if((best < 0.8 && d > best) ||
					(best >= 0.8 && d >= 0.8 && d < best)) {
			best = d;
			best_h = it->first;
		}
	}
	return best_h;
}


Level * Pyramid::getFirstLevel(){
	std::map<std::string, Level*>::iterator it(levels.begin());
	return it->second;
}

TileMatrixSet Pyramid::getTms(){
	return tms;
}


Image* Pyramid::getbbox(BoundingBox<double> bbox, int width, int height, CRS dst_crs) {
	// On calcule la résolution de la requete dans le crs source selon une diagonale de l'image
	double resolution_x, resolution_y;
	if(getTms().getCrs()==dst_crs){
	        resolution_x = (bbox.xmax - bbox.xmin) / width;
        	resolution_y = (bbox.ymax - bbox.ymin) / height;
	}
	else {
		Grid* grid = new Grid(width, height, bbox);
//		try
//		{
                	grid->reproject(dst_crs.getProj4Code(),getTms().getCrs().getProj4Code());
/*			throw ("toto");
		}
		catch (std::string str)
		{
			return 0;	
		}
*/		
                resolution_x = (grid->bbox.xmax - grid->bbox.xmin) / width;
                resolution_y = (grid->bbox.ymax - grid->bbox.ymin) / height;
                delete grid;		
	}
	std::string l = best_level(resolution_x, resolution_y);
        LOGGER_DEBUG( "best_level=" << l << " resolution requete=" << resolution_x << " " << resolution_y);

	if(getTms().getCrs()==dst_crs)
		return levels[l]->getbbox(bbox, width, height);
	else
		return levels[l]->getbbox(bbox, width, height, getTms().getCrs(), dst_crs);

}

//TODO: destructeur


