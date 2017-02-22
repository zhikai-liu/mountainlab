#include "p_create_firings.h"

#include "mda.h"

bool p_create_firings(QString event_times, QString labels, QString firings_out, int central_channel)
{
    Mda ET(event_times);
    Mda LA(labels);

    long L=ET.totalSize();

    Mda firings(3,L);
    for (long i=0; i<L; i++) {
        firings.setValue(central_channel,0,i);
        firings.setValue(ET.value(i),1,i);
        firings.setValue(LA.value(i),2,i);
    }

    return firings.write64(firings_out);
}
