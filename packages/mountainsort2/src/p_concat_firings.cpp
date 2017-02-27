#include "p_concat_firings.h"

#include <diskreadmda.h>
#include <diskreadmda32.h>
#include <diskwritemda.h>

bool p_concat_firings(QStringList timeseries_list, QStringList firings_list, QString timeseries_out, QString firings_out)
{
    if (timeseries_list.isEmpty()) {
        qWarning() << "timeseries_list is empty.";
        return false;
    }
    if (firings_list.count()!=timeseries_list.count()) {
        qWarning() << "Mismatch between sizes of timeseries list and firings list";
        return false;
    }

    DiskReadMda32 X0(timeseries_list.value(0));
    int M=X0.N1();
    DiskReadMda F0(firings_list.value(0));
    int R=F0.N1();

    long NN=0;
    long LL=0;
    for (int i=0; i<timeseries_list.count(); i++) {
        DiskReadMda32 X1(timeseries_list.value(i));
        NN+=X1.N2();
        DiskReadMda F1(firings_list.value(i));
        LL+=F1.N2();
    }

    DiskWriteMda Y(X0.mdaioHeader().data_type,timeseries_out,M,NN);
    Mda G(R,LL);
    long n0=0;
    long i0=0;
    for (int i=0; i<timeseries_list.count(); i++) {
        printf("Timeseries %d of %d\n",i+1,timeseries_list.count());
        DiskReadMda32 X1(timeseries_list.value(i));
        Mda32 chunk;
        X1.readChunk(chunk,0,0,M,X1.N2());
        Y.writeChunk(chunk,0,n0);

        Mda F1(firings_list.value(i));
        for (long j=0; j<F1.N2(); j++) {
            for (int r=0; r<R; r++) {
                G.setValue(F1.value(r,j),r,i0+j);
            }
            G.setValue(G.value(1,i0+j)+n0,1,i0+j);
        }

        n0+=X1.N2();
        i0+=F1.N2();
    }

    G.write64(firings_out);

    return true;
}
