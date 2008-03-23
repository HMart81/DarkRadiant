#ifndef DOOM3MAPFORMAT_H_
#define DOOM3MAPFORMAT_H_

#include "inode.h"
#include <vector>
#include <ostream>

class Entity;

namespace map {

/* Walker class to traverse the scene graph and write each entity
 * out to the token stream, including its member brushes.
 */

class NodeExporter : 
	public scene::NodeVisitor
{
	// Stack to hold the parent entity when writing a brush. Either
	// the parent Entity is pushed, or a NULL pointer.
	std::vector<Entity*> _entityStack;

	// Output stream to write to
	std::ostream& _outStream;
  
	// Number of entities written (map global)
	int _entityCount;
	
	// Number of brushes written for the current entity (entity local)
	int _brushCount;

	// Are we writing dummy brushes to brushless entities?
	bool _writeDummyBrushes;
	
public:
	// Constructor
	NodeExporter(std::ostream& os);
	
	// Pre-descent callback
	virtual bool pre(const scene::INodePtr& node);
  
	// Post-descent callback
	virtual void post(const scene::INodePtr& node);

private:
	void exportEntity(const Entity& entity, std::ostream& os);
};

} // namespace map

#endif /* DOOM3MAPFORMAT_H_ */
