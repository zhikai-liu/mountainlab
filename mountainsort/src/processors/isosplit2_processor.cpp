/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/7/2016
*******************************************************/

#include "isosplit2_processor.h"
#include "isosplit2.h"
#include "isosplit2_impl.h"

class isosplit2_ProcessorPrivate {
public:
    isosplit2_Processor* q;
};

isosplit2_Processor::isosplit2_Processor()
{
    d = new isosplit2_ProcessorPrivate;
    d->q = this;

    this->setName("isosplit2");
    this->setVersion("0.1");
    this->setInputFileParameters("data");
    this->setOutputFileParameters("labels");
    //this->setRequiredParameters("isocut_threshold");
}

isosplit2_Processor::~isosplit2_Processor()
{
    delete d;
}

bool isosplit2_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool isosplit2_Processor::run(const QMap<QString, QVariant>& params)
{
    isosplit2_impl_opts opts;
    //opts.clip_size = params["clip_size"].toInt();
    QString data_path = params["data"].toString();
    QString labels_path = params["labels"].toString();
    return isosplit2_impl::isosplit2(data_path, labels_path, opts);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class isosplit2_w_ProcessorPrivate {
public:
    isosplit2_w_Processor* q;
};

isosplit2_w_Processor::isosplit2_w_Processor()
{
    d = new isosplit2_w_ProcessorPrivate;
    d->q = this;

    this->setName("isosplit2");
    this->setVersion("0.1");
    this->setInputFileParameters("data", "weights");
    this->setOutputFileParameters("labels");
    //this->setRequiredParameters("isocut_threshold");
}

isosplit2_w_Processor::~isosplit2_w_Processor()
{
    delete d;
}

bool isosplit2_w_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool isosplit2_w_Processor::run(const QMap<QString, QVariant>& params)
{
    isosplit2_w_impl_opts opts;
    //opts.clip_size = params["clip_size"].toInt();
    QString data_path = params["data"].toString();
    QString weights_path = params["weights"].toString();
    QString labels_path = params["labels"].toString();
    return isosplit2_w_impl::isosplit2_w(data_path, weights_path, labels_path, opts);
}
