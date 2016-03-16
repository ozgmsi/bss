#include "BestFit.h"

BestFit::~BestFit()
{
    while(!areas.empty()){
        Area *area = areas.back();
        areas.pop_back();
        delete area;
    }
}

Area* BestFit::alloc(int wanted)
{
    require(wanted > 0);		// has to be "something",
	require(wanted <= size);	// but not more than can exist

    updateStats();              // update resource map

    /// Indien geen memory dan niks
	if(areas.empty()){
        return 0;
    }

    Area *memblockp = searcher(wanted);

    if (memblockp){
        /// Geheugen verkregen
        return memblockp;
    }

    /// Check voor fragmentatie
	if (reclaim()) {			        // could we reclaim fragmented areas
		memblockp = searcher(wanted);	// second attempt
		if (memblockp) {				// success ?
			return memblockp;
		}
	}
    /// Geen geheugen kunnen alloceren
    return 0;
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
    for(ALiterator mem_iter = areas.begin(); mem_iter != areas.end(); ++mem_iter)
    {
        Area* mem = *mem_iter;

        ///Search for the best fitting block
        if (mem->getSize() < bestfit->getSize() && wanted <= mem->getSize())
        {
            /// Found best fitting mem block
            bestfit = mem;
        }
    }

    if (bestfit && wanted <= bestfit->getSize()){
        /// Split current bestfit,
        ALiterater iter =  areas.erase(bestfit);
        areas.insert(iter,bestfit->split(wanted));    /// Add remaining space to freelist

        /// Bij teruggave moet de applicatie altijd vertellen hoe groot het gebied was
        /// Geef pointer terug
        /// C void* malloc(size_t n)  > size_t = size_type bijv. unsigned
        /// C void  free(caddr_t ptr) > caddr_r = core adress type kan bijv. void*
        return bestfit;
    }
    return 0;
}

void BestFit::free(Area* area)
{
    require(area != 0);
	if (cflag) {
		// EXPENSIVE: check for overlap with already registered free areas
		for (ALiterator  i = areas.begin() ; i != areas.end() ; ++i) {
			check(!area->overlaps(*i));    // sanity check
		}
	}
	areas.push_back(area);	// de lazy
}

void BestFit::setSize(int new_size)
// Initializes how much memory we own
{
	require(areas.empty());					// prevent changing the size when the freelist is nonempty
	Fitter::setSize(new_size);
	areas.push_back(new Area(0, new_size));	// and create the first free area (i.e. "all")

}

/// Tools
// Try to join fragmented freespace
bool BestFit::reclaim()
{
	bool  changed = false;	// did we change anything ?

	// Sort resource map by area address
	areas.sort( Area::orderByAddress() );	// WARNING: expensive N*log(N) operation !

	// Search thru all available areas for matches
	ALiterator  i = areas.begin();
	Area  *ap = *i;				// the current candidate ...
	for (++i ; i != areas.end() ;) {
		Area  *bp = *i;			// ... match ap with ...
		if (bp->getBase() == (ap->getBase() + ap->getSize()))
		{
			// oke, bp matches ap ...
			ALiterator  next = areas.erase(i);	// remove bp from the list
			ap->join(bp);			// append area bp to ap (and destroy bp)
			++mergers;				// update statistics
			changed = true;			// yes we changed something
			i = next;				// revive the 'i' iterator
		} else {
			ap = bp;				// move on to next area
			++i;
		}
	}
	if (changed) {					// iff we have changed some area's the
		iter = areas.begin();		// next search should start at the (new) front
	}
	++reclaims;						// update statistics
	return changed;
}

// Update statistics
void	BestFit::updateStats()
{
	++qcnt;									// number of 'alloc's
	qsum  += areas.size();					// length of resource map
	qsum2 += (areas.size() * areas.size());	// same: squared
}



