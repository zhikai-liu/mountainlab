#include "icounter.h"
#include <QHash>
#include <objectregistry.h>

QString ICounterBase::label() const
{
    return genericValue().toString();
}

QVariant IAggregateCounter::add(const QVariant&)
{
    return genericValue();
}

QList<ICounterBase*> IAggregateCounter::counters() const
{
    return m_counters;
}

void IAggregateCounter::addCounter(ICounterBase* c)
{
    m_counters << c;
    connect(c, SIGNAL(valueChanged()), this, SLOT(updateValue()));
    updateValue();
}

void IAggregateCounter::addCounters(const QList<ICounterBase*>& list)
{
    foreach (ICounterBase* c, list) {
        m_counters << c;
        connect(c, SIGNAL(valueChanged()), this, SLOT(updateValue()));
    }
    updateValue();
}

ICounterManager::ICounterManager(QObject* parent)
    : QObject(parent)
{
}

CounterManager::CounterManager(QObject* parent)
    : ICounterManager(parent)
{
}

QStringList CounterManager::availableCounters() const
{
    QReadLocker locker(&m_countersLock);
    return m_counterNames;
}

ICounterBase* CounterManager::counter(const QString& name) const
{
    QReadLocker locker(&m_countersLock);
    return m_counters.value(name, nullptr);
}

void CounterManager::setCounters(QList<ICounterBase*> counters)
{
    QWriteLocker locker(&m_countersLock);
    for (ICounterBase* counter : counters) {
        const QString& name = counter->name();
        m_counters.insert(name, counter);
        m_counterNames.append(name);
    }
    locker.unlock();
    emit countersChanged();
}

void CounterManager::addCounter(ICounterBase* counter)
{
    const QString& name = counter->name();
    QWriteLocker locker(&m_countersLock);
    if (m_counterNames.contains(name))
        return;
    m_counters.insert(name, counter);
    m_counterNames.append(name);
    locker.unlock();
    emit countersChanged();
}

void CounterManager::removeCounter(ICounterBase* counter)
{
    const QString& name = counter->name();
    QWriteLocker locker(&m_countersLock);
    if (!m_counterNames.removeOne(name))
        return;
    m_counters.remove(name);
    locker.unlock();
    emit countersChanged();
}

void CounterManager::addGroup(CounterGroup* group)
{
    QWriteLocker locker(&m_groupsLock);
    if (m_groupNames.contains(group->name()))
        return;
    m_groups.insert(group->name(), group);
    m_groupNames.append(group->name());
    locker.unlock();
    emit groupsChanged();
}

void CounterManager::removeGroup(CounterGroup* group)
{
    QWriteLocker locker(&m_groupsLock);
    if (!m_groupNames.removeOne(group->name()))
        return;
    m_groups.remove(group->name());
    locker.unlock();
    emit groupsChanged();
}

CounterGroup* CounterManager::group(const QString& name) const
{
    QReadLocker locker(&m_groupsLock);
    return m_groups.value(name, nullptr);
}

QStringList CounterManager::availableGroups() const
{
    QReadLocker locker(&m_groupsLock);
    return m_groupNames;
}

ICounterBase::ICounterBase(const QString& name)
    : m_name(name)
{
}

QString ICounterBase::name() const
{
    return m_name;
}
