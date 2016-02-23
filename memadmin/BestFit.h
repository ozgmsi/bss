#ifndef BESTFIT_H
#define BESTFIT_H

#include "Fitter.h"

class BestFit : public Fitter
{
    public:
        BestFit(bool cflag=false, const char *type = "BestFit (lazy)")
            : Fitter(cflag, type){}

        ~BestFit();

        void	setSize(int new_size);
        Area	*alloc(int wanted);
        void	free(Area *ap);

    protected:
        Area    *searcher(int);
        bool    reclaim();
        void    dump();
        void    updateStats();

        std::list<Area *> areas;

    private:
};

#endif // BESTFIT_H
