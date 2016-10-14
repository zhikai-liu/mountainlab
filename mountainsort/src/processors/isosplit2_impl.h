#ifndef ISOSPLIT2_IMPL_H
#define ISOSPLIT2_IMPL_H

#include <QString>

struct isosplit2_impl_opts {
};

namespace isosplit2_impl {

bool isosplit2(QString data_in, QString labels_out, isosplit2_impl_opts opts);
}

struct isosplit2_w_impl_opts {
};

namespace isosplit2_w_impl {

bool isosplit2_w(QString data_in, QString weights_in, QString labels_out, isosplit2_w_impl_opts opts);
}

#endif // ISOSPLIT2_IMPL_H
