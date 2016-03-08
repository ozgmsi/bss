#include "BestFit.h"
#include "ansi.h"

BestFit::~BestFit()
{
    while(!areas.empty())
    {
        Area *area = areas.back();
        areas.pop_back();
        delete area;
    }
}

void BestFit::setSize(int new_size)
{
    require(areas.empty());
    Fitter::setSize(new_size);
    areas.push_back(new Area(0, new_size));
}

Area *BestFit::alloc(int wanted)
{
    require(wanted > 0);
    require(wanted <= size);

    updateStats();

    if (areas.empty()){
        return 0;
    }

    Area *ap = 0;
    ap = searcher(wanted);
    if (ap){
        return ap;
    }
    if (reclaim()){
        ap = searcher(wanted);
        if (ap){
            return ap;
        }
    }

    //dump(); //DEBUG
    return 0;
}

void BestFit::free(Area *ap)
{
    require(ap != 0);
    if (cflag) {
        for (ALiterator i = areas.begin(); i != areas.end(); ++i){
            check(!ap->overlaps(*i));
        }
    }

    areas.push_back(ap);
}

Area *BestFit::searcher(int wanted)
{
    require(wanted > 0);
    require(wanted <= size);
    require(!areas.empty());

    int lowestBestFitSize = getSize();
    ALiterator bestFitPosition = areas.end();
    Area *bestFitArea = 0;

    for (ALiterator i = areas.begin(); i != areas.end(); ++i){
        Area *ap = *i;
        if (wanted <= ap->getSize() && (ap->getSize() - wanted) < lowestBestFitSize){
            lowestBestFitSize = ap->getSize() - wanted;
            bestFitPosition = i;
            bestFitArea = *i;
        }
    }

    if (bestFitPosition != areas.end()){
        ALiterator next = areas.erase(bestFitPosition);
        if (bestFitArea->getSize() > wanted){
            Area *remaining = bestFitArea->split(wanted);
            areas.insert(next, remaining);
        }

        return bestFitArea;
    }

    return 0;
}

bool BestFit::reclaim()
{
    require(!areas.empty());		// sanity check

	// Sort resource map by area address
	areas.sort(Area::orderByAddress());	// WARNING: expensive N*log(N) operation !

	// Search thru all free areas for matches between successive elements
	bool  changed = false;
	ALiterator  i = areas.begin();
	Area  *ap = *i;					// The current candidate ...
	for(++i ; i != areas.end() ;) {
		Area  *bp = *i;				// ... match it with.
		if(bp->getBase() == (ap->getBase() + ap->getSize())) {
			// Oke; bp matches ap ... [i.e. bp follows ap]
			ALiterator  next = areas.erase(i);	// remove bp from the list
			ap->join(bp);			// append area bp to ap (and destroy bp)
			++mergers;				// update statistics
			changed = true;			// we changed something
			i = next;				// revive the 'i' iterator
			// and now try match ap with whatever followed bp
		} else {
			ap = bp;				// move on to next free area
			++i;
		}
	}
	++reclaims;	// update statistics ("reclaims attempted")
	return changed;
}

void BestFit::dump()
{
    std::cerr << AC_BLUE << type << "::areas";
	for (ALiterator  i = areas.begin() ; i != areas.end() ; ++i) {
		std::cerr << ' ' << **i;
	}
	std::cerr << AA_RESET << std::endl;
}

void BestFit::updateStats()
{
    ++qcnt;									// number of 'alloc's
	qsum  += areas.size();					// length of resource map
	qsum2 += (areas.size() * areas.size());	// same: squared
}
