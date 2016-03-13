#include "BestFit.h"



Area* BestFit::alloc(int wanted)
{
    require(wanted > 0);		// has to be "something",
	require(wanted <= size);	// but not more than can exist

	if(areas.empty()){
        return 0;
    }

    Area *memblockp = searcher(wanted);

    if (memblockp){
        /// Geheugen verkregen
        return memblockp;
    }

        /// Check voor fragmentatie


        /// Anders geeft 0 terug
}

/// Internal
Area* BestFit::searcher(int wanted)
{

    /// Pre-requirements
    require(wanted > 0);
    require(wanted <= size);
    require(!areas.empty());

    /// Get reference to the front element
    Area* bestfit = areas.front();

    for(ALiterator iter2 = areas.begin(); iter != areas.end(); ++iter)
    {
        Area* mem = *iter;

        ///Search for the best fitting block
        if (mem->getSize() < bestfit->getSize() && wanted < mem->getSize())
        {
            /// Found memory
            bestfit = mem->split(wanted);
        }
    }

    if (bestfit)
    {
        return bestfit;
    }

    /// TODO

    return 0;
}

void BestFit::free(Area* area)
{

}

void BestFit::setSize(int new_size)
// Initializes how much memory we own
{
	require(areas.empty());					// prevent changing the size when the freelist is nonempty
	Fitter::setSize(new_size);
	areas.push_back(new Area(0, new_size));	// and create the first free area (i.e. "all")

}

bool BestFit::reclaim()
{

}



