/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/7/2016
*******************************************************/

#ifndef isosplit2_PROCESSOR_H
#define isosplit2_PROCESSOR_H

#include "msprocessor.h"

class isosplit2_ProcessorPrivate;
class isosplit2_Processor : public MSProcessor {
public:
    friend class isosplit2_ProcessorPrivate;
    isosplit2_Processor();
    virtual ~isosplit2_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    isosplit2_ProcessorPrivate* d;
};

class isosplit2_w_ProcessorPrivate;
class isosplit2_w_Processor : public MSProcessor {
public:
    friend class isosplit2_w_ProcessorPrivate;
    isosplit2_w_Processor();
    virtual ~isosplit2_w_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    isosplit2_w_ProcessorPrivate* d;
};

#endif // isosplit2_PROCESSOR_H
