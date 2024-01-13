#pragma once

#include <cstddef>
#include "imodule.h"

class ICounter
{
public:
    /** Destructor */
	virtual ~ICounter() {}

	/** greebo: Decrements/increments the counter.
	 */
	virtual void increment() = 0;
	virtual void decrement() = 0;

	/** greebo: Returns the current count.
	 */
	virtual std::size_t get() const = 0;
};

// Known counters
enum CounterType
{
	counterBrushes,
	counterPatches,
	counterEntities,
};

const char* const MODULE_COUNTER("Counters");

/** greebo: This abstract class defines the interface to the core application.
 * 			Use this to access methods from the main codebase in radiant/
 */
class ICounterManager :
	public RegisterableModule
{
public:
	// Returns the Counter object of the given type
	virtual ICounter& getCounter(CounterType counter) = 0;

	virtual sigc::signal<void()>& signal_countersChanged() = 0;
};

inline ICounterManager& GlobalCounters()
{
	static module::InstanceReference<ICounterManager> _reference(MODULE_COUNTER);
	return _reference;
}
