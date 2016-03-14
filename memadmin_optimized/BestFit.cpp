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

        std::cout << wanted << "kb verkregen, over : " << memblockp->getSize() << std::endl;
        areas.push_back(memblockp);
        return memblockp;
    }

        /// Check voor fragmentatie


        /// Anders geeft 0 terug
    return 0;
}

/// Internal
Area* BestFit::searcher(int wanted)
{
    /// Pre-requirements
    require(wanted > 0);
    std::cout << wanted << "Wanted" << std::endl;
    std::cout << size << "Size" << std::endl;
    require(wanted <= size);
    require(!areas.empty());

    /// Get reference to the front element
    Area* bestfit = areas.front();
    std::cout << areas.size() << std::endl;
    for(ALiterator mem_iter = areas.begin(); mem_iter != areas.end(); ++mem_iter)
    {
        Area* mem = *mem_iter;

        ///Search for the best fitting block
        if (mem->getSize() <= bestfit->getSize() && wanted < mem->getSize())
        {
        /// Found memory
            bestfit = mem;
        }
    }

    if (bestfit){
        bestfit = bestfit->split(wanted);
        size -= wanted;
        return bestfit;
    }

    // Framentatie implementeren

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



