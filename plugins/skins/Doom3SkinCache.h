#ifndef DOOM3SKINCACHE_H_
#define DOOM3SKINCACHE_H_

#include "Doom3ModelSkin.h"

#include "modelskin.h"
#include "parser/DefTokeniser.h"

#include <map>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace skins
{

/**
 * Implementation of ModelSkinCache interface for Doom 3 skin management.
 */
class Doom3SkinCache 
: public ModelSkinCache
{
	// Table of named skin objects
	typedef std::map<std::string, 
					 boost::shared_ptr<Doom3ModelSkin> > NamedSkinMap;
	NamedSkinMap _namedSkins;

	// List of all skins
	StringList _allSkins;
	
	// Map between model paths and a vector of names of the associated skins,
	// which are contained in the main NamedSkinMap.
	typedef std::map<std::string, std::vector<std::string> > ModelSkinMap;
	ModelSkinMap _modelSkins;

	// Flag to indicate skin module realised. The module is realised when all
	// of the skins are loaded, which does not happen until the first call to
	// getSkinsForModel(), getAllSkins() or capture().
	bool _realised;

	// Empty Doom3ModelSkin to return if a named skin is not found
	Doom3ModelSkin _nullSkin;

private:

	// Load and parse the skin files, populating internal data structures.
	// Function may be called more than once, will do nothing if already
	// realised.
	void realise();
	
	// Parse an individual skin declaration and add it to the maps
	void parseSkin(parser::DefTokeniser& tokeniser);

public:

	typedef ModelSkinCache Type;
	STRING_CONSTANT(Name, "*");

	/* Constructor.
	 */
	Doom3SkinCache()
	: _realised(false)
	{}

	/* Return pointer to implementation (this).
	 */
	ModelSkinCache* getTable() {
		return this;
	}

	/* Return a specific named skin. If the named skin cannot be found, return
	 * the empty (null) skin with no remaps.
	 */
	ModelSkin& capture(const std::string& name) {
		realise();
		NamedSkinMap::iterator i = _namedSkins.find(name);
		if (i != _namedSkins.end())
			return *(i->second); // dereference shared_ptr
		else
			return _nullSkin;
	}
  
	/* Get the vector of skin names corresponding to the given model.
	 */
	const StringList& getSkinsForModel(const std::string& model) {
		realise();
		return _modelSkins[model];
	}
	
	/* Return a complete list of skins.
	 */
	const StringList& getAllSkins() {
		realise();
		return _allSkins;
	}
	
	/* Parse the provided string as a .skin file, and add all skins found within
	 * to the internal data structures.
	 */
	void parseFile(const std::string& contents);	
  
};


}

#endif /*DOOM3SKINCACHE_H_*/
