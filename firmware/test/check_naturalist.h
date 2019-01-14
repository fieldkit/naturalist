#ifndef CHECK_NATURALIST_H_INCLUDED
#define CHECK_NATURALIST_H_INCLUDED

#include "check_core.h"

namespace fk {

class CheckNaturalist : public CheckCore {
public:
    bool sht31();
    bool mpl3115a2();
    bool tsl2591();
    bool bno055();
    bool sph0645();

public:
    bool check() override;

};

}

#endif
