#pragma once

/* greebo: The interface of the grid system
 *
 * Use these methods to set/get the grid size of the xyviews
 */
#include <stdexcept>
#include "imodule.h"
#include <sigc++/signal.h>

enum GridSize
{
	GRID_0125 = -3,
	GRID_025 = -2,
	GRID_05 = -1,
	GRID_1 = 0,
	GRID_2 = 1,
	GRID_4 = 2,
	GRID_8 = 3,
	GRID_16 = 4,
	GRID_32 = 5,
	GRID_64 = 6,
	GRID_128 = 7,
	GRID_256 = 8,
};

namespace grid
{

inline const char* getStringForSize(GridSize size)
{
	switch (size)
	{
	case GRID_0125: return "0.125";
	case GRID_025:  return "0.25";
	case GRID_05:  return "0.5";
	case GRID_1:  return "1";
	case GRID_2:  return "2";
	case GRID_4:  return "4";
	case GRID_8:  return "8";
	case GRID_16:  return "16";
	case GRID_32:  return "32";
	case GRID_64:  return "64";
	case GRID_128:  return "128";
	case GRID_256:  return "256";
	default:
		throw new std::logic_error("Grid size not handled in switch!");
	};
}

// The space the grid is dividing. Regular map editing is using the
// World grid, while the Texture Tool is working in UV space.
enum class Space
{
    World,
    Texture,
};

}

// grid renderings
enum GridLook
{
	GRIDLOOK_LINES,
	GRIDLOOK_DOTLINES,
	GRIDLOOK_MOREDOTLINES,
	GRIDLOOK_CROSSES,
	GRIDLOOK_DOTS,
	GRIDLOOK_BIGDOTS,
	GRIDLOOK_SQUARES,
};

constexpr const char* const MODULE_GRID("Grid");

class IGridManager :
	public RegisterableModule
{
public:
	virtual ~IGridManager() {}

	virtual void setGridSize(GridSize gridSize) = 0;
	
    // Returns the grid spacing in units of the given space
    virtual float getGridSize(grid::Space = grid::Space::World) const = 0;

    // Returns the grid power of the currently active grid size
	virtual int getGridPower(grid::Space = grid::Space::World) const = 0;

    // Returns the base number the exponent is applied to (e.g. 2)
	virtual int getGridBase(grid::Space = grid::Space::World) const = 0;

	virtual void gridDown() = 0;
	virtual void gridUp() = 0;

	virtual GridLook getMajorLook() const = 0;
	virtual GridLook getMinorLook() const = 0;

    /// Signal emitted when the grid is changed
	virtual sigc::signal<void()> signal_gridChanged() const = 0;
};

// This is the accessor for the grid module
inline IGridManager& GlobalGrid()
{
	static module::InstanceReference<IGridManager> _reference(MODULE_GRID);
	return _reference;
}
