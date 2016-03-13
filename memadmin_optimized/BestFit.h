#ifndef BESTFIT_H_INCLUDED
#define BESTFIT_H_INCLUDED

#include "Fitter.h"

class BestFit : public Fitter
{
    public:
        /// Construction
        /// @param cflag	initial status of check-mode
        /// @param type		name of this algorithm (default=BestFit)
        BestFit(bool cflag, const char* type = "{ BestFit }") : Fitter(cflag, type) {};

        // Added
        Area *alloc(int wanted) override;       	///< Application vraagt om geheugen
        void  free(Area *)      override;		    ///< Application geeft een Area weer terug aan geheugenbeheer
        void setSize(int size);



    protected:

        /// The "Memory"
        /// std::list<Area*>
        AreaList areas;

        /// std::list<Area*>::iterator
        ALiterator iter;

        /// This function is called when the searcher can not find space.
        /// It tries to reclaim fragmented space by merging adjacent free areas.
        /// @returns true if adjacent areas could be merged
        bool	  reclaim()     override;


        /// This is the actual function that searches for free space
        Area 	*searcher(int wanted)  override;

};


#endif // BESTFIT_H_INCLUDED
