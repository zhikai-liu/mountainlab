#ifndef ICOUNTER_H
#define ICOUNTER_H

#include <QObject>
#include <QReadWriteLock>
#include <QVariant>
#include <QHash>
#include <type_traits>

class ICounterBase : public QObject {
    Q_OBJECT
public:
    enum Type {
        Unknown,
        Integer,
        Double
    };
    Q_ENUM(Type)
    ICounterBase(const QString& name);
    virtual Type type() const = 0;
    virtual QString name() const;
    virtual QString label() const;
    virtual QVariant genericValue() const = 0;
    virtual QVariant add(const QVariant&) = 0;
    template <typename T>
    T value() const { return genericValue().value<T>(); }
signals:
    void valueChanged();

private:
    QString m_name;
};

class IAggregateCounter : public ICounterBase {
    Q_OBJECT
public:
    using ICounterBase::ICounterBase;
    QVariant add(const QVariant&);
    QList<ICounterBase*> counters() const;
    void addCounter(ICounterBase*);
    void addCounters(const QList<ICounterBase*>&);
protected slots:
    virtual void updateValue() = 0;

private:
    QList<ICounterBase*> m_counters;
};

template <typename T, bool integral = false>
class ICounterImpl : public ICounterBase {
public:
    using ICounterBase::ICounterBase;
    Type type() const { return Unknown; }
    QVariant add(const QVariant& inc)
    {
        return QVariant::fromValue<T>(add(inc.value<T>()));
    }

    T add(const T& inc)
    {
        m_lock.lockForWrite();
        T old = m_value;
        m_value += inc;
        m_lock.unlock();
        if (m_value != old)
            emit valueChanged();
        return m_value;
    }
    T value() const
    {
        QReadLocker locker(&m_lock);
        return m_value;
    }
    QVariant genericValue() const
    {
        return QVariant::fromValue<T>(value());
    }

private:
    mutable QReadWriteLock m_lock;
    T m_value;
};

template <typename T>
class ICounterImpl<T, true> : public ICounterBase {
public:
    using ICounterBase::ICounterBase;
    Type type() const { return Integer; }
    QVariant add(const QVariant& inc)
    {
        return QVariant::fromValue<T>(add(inc.value<T>()));
    }
    T add(const T& inc)
    {
        if (inc == 0)
            return m_value;
        T val = (m_value += inc);
        emit valueChanged();
        return val;
    }
    T value() const
    {
        return m_value;
    }
    QVariant genericValue() const
    {
        return QVariant::fromValue<T>(value());
    }

private:
    QAtomicInteger<T> m_value;
};

template <typename T>
using ICounter = ICounterImpl<T, std::is_integral<T>::value>;

using IIntCounter = ICounter<int>;
using IDoubleCounter = ICounter<double>;

class CounterGroup : public QObject {
    Q_OBJECT
public:
    CounterGroup(const QString& n, QObject* parent = 0)
        : QObject(parent)
        , m_name(n)
    {
    }
    void setName(const QString& n) { m_name = n; }
    QString name() const { return m_name; }
    virtual QStringList availableCounters() const = 0;
    virtual ICounterBase* counter(const QString& name) const = 0;

private:
    QString m_name;
};

class ICounterManager : public QObject {
    Q_OBJECT
public:
    virtual QStringList availableCounters() const = 0;
    virtual ICounterBase* counter(const QString& name) const = 0;

    virtual QStringList availableGroups() const = 0;
    virtual CounterGroup* group(const QString& name) const = 0;
signals:
    void countersChanged();
    void groupsChanged();

protected:
    ICounterManager(QObject* parent = 0);
};

class CounterManager : public ICounterManager {
public:
    CounterManager(QObject* parent = 0);

    QStringList availableCounters() const override;
    ICounterBase* counter(const QString& name) const override;
    void setCounters(QList<ICounterBase*> counters);
    void addCounter(ICounterBase* counter);
    void removeCounter(ICounterBase* counter);

    QStringList availableGroups() const;
    CounterGroup* group(const QString& name) const;
    void addGroup(CounterGroup* group);
    void removeGroup(CounterGroup* group);

private:
    QHash<QString, ICounterBase*> m_counters;
    QStringList m_counterNames;
    mutable QReadWriteLock m_countersLock;

    QHash<QString, CounterGroup*> m_groups;
    QStringList m_groupNames;
    mutable QReadWriteLock m_groupsLock;
};

#endif // ICOUNTER_H
