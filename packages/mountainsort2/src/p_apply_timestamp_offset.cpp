/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 2/23/2017
*******************************************************/

#include "p_apply_timestamp_offset.h"
#include <mda.h>

bool p_apply_timestamp_offset(QString firings_path, QString firings_out_path, double timestamp_offset)
{
    Mda firings(firings_path);
    for (int i = 0; i < firings.N2(); i++) {
        firings.setValue(firings.value(1, i) + timestamp_offset, 1, i);
    }
    return firings.write64(firings_out_path);
}
