#ifndef BESTFIT_H_INCLUDED
#define BESTFIT_H_INCLUDED

#include "Fitter.h"

class BestFit : public Fitter
{
    public:
        /// Construction
        /// @param cflag	initial status of check-mode
        /// @param type		name of this algorithm (default=BestFit)
        BestFit(bool cflag = false, const char* type = "{ BestFit }") : Fitter(cflag, type) {};
        ~BestFit();

        // Added
        Area *alloc(int wanted) override;       	///< Application vraagt om geheugen
        void  free(Area *)      override;		    ///< Application geeft een Area weer terug aan geheugenbeheer
        void  setSize(int size);                    ///< Set memory block size

    protected:

        /// The "Memory"
        AreaList areas;     /// Typedeffed std::list<Area*>


        /// Iterator for the freelist/arealist
        ALiterator iter;    /// Typedeffed std::list<Area*>::iterator

        /// This function is called when the searcher can not find space.
        /// It tries to reclaim fragmented space by merging adjacent free areas.
        /// @returns true if adjacent areas could be merged
        bool	  reclaim()               override;


        /// This is the actual function that searches for free space
        Area 	*searcher(int wanted)     override;

        virtual  void	updateStats();	///< update resource map statistics

};


#endif // BESTFIT_H_INCLUDED
