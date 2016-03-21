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
    areas.push_back(new Area(0, new_size)))
}

Area *BestFit::alloc(int wanted)
{
    require(wanted > 0);
    require(wanted <= areas.size());

    if (areas.empty()){
        return 0;
    }

    Area *ap = 0;
    ap = searcher(wanted);
    if (ap){
        return ap;
    }
    if (reclaim){
        ap = searcher(wanted);
        if (ap){
            return ap;
        }
    }

    //dump(); //DEBUG
    return 0;
}

void BestFit::free()
{

}

Area *BestFit::searcher(int wanted)
{
    require(wanted > 0);
    require(wanted <= areas.size());


}

bool BestFit::reclaim()
{

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
