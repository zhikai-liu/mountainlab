#include <QString>
#include <QtTest>
#include "icounter.h"

class CountersTest : public QObject
{
    Q_OBJECT

public:
    CountersTest();

private Q_SLOTS:
    void testManagerEmpty();
    void testManager();
    void testIntCounter();
    void testDoubleCounter();
    void testCustomCounter();
    void testAggregateCounter();
};

CountersTest::CountersTest()
{
}

void CountersTest::testManagerEmpty()
{
    CounterManager manager;
    QVERIFY(manager.availableCounters().isEmpty());
    QVERIFY(manager.counter("bogus") == nullptr);
}

void CountersTest::testManager()
{
    CounterManager manager;
    IIntCounter counter("C1");
    QSignalSpy addSpy(&manager, SIGNAL(countersChanged()));
    manager.addCounter(&counter);
    QCOMPARE(addSpy.size(), 1);
    QCOMPARE(manager.availableCounters().size(), 1);
    QCOMPARE(manager.availableCounters(), QStringList({"C1"}));
    QCOMPARE(manager.counter("C1"), &counter);
    manager.removeCounter(&counter);
    QCOMPARE(addSpy.size(), 2);
    QCOMPARE(manager.availableCounters().size(), 0);
    QCOMPARE(manager.counter("C1"), static_cast<ICounterBase*>(nullptr));
}

void CountersTest::testIntCounter()
{
    // int uses QAtomicInteger internally
    IIntCounter counter("IntCounter");
    QCOMPARE(counter.name(), QStringLiteral("IntCounter"));
    QSignalSpy spy(&counter, SIGNAL(valueChanged()));
    QCOMPARE(counter.value(), 0);
    QCOMPARE(counter.genericValue(), QVariant(0));
    QCOMPARE(counter.label(), QVariant("0").toString());
    counter.add(42);
    QCOMPARE(counter.value(), 42);
    QVERIFY2(spy.size() == 1, "valueChanged() not emiited after add");
    counter.add(0);
    QCOMPARE(counter.value(), 42);
    QVERIFY2(spy.size() == 1, "valueChanged() emitted but the value didn't change");
    QCOMPARE(counter.genericValue(), QVariant(42));
    QCOMPARE(counter.label(), QStringLiteral("42"));
}

void CountersTest::testDoubleCounter()
{
    // double uses a QReadWriteLock internally
    IDoubleCounter counter("DoubleCounter");
    QCOMPARE(counter.name(), QStringLiteral("DoubleCounter"));
    QSignalSpy spy(&counter, SIGNAL(valueChanged()));
    QCOMPARE(counter.value(), 0.0);
    QCOMPARE(counter.genericValue(), QVariant(0.0));
    QCOMPARE(counter.label(), QVariant(0.0).toString());
    counter.add(42.5);
    QCOMPARE(counter.value(), 42.5);
    QVERIFY2(spy.size() == 1, "valueChanged() not emiited after add");
    counter.add(0.0);
    QCOMPARE(counter.value(), 42.5);
    QVERIFY2(spy.size() == 1, "valueChanged() emitted but the value didn't change");
    QCOMPARE(counter.genericValue(), QVariant(42.5));
    QCOMPARE(counter.label(), QStringLiteral("42.5"));
}

class BytesCounter : public IIntCounter {
public:
    BytesCounter(const QString &name)
        : IIntCounter(name) {}
    QString label() const
    {
        int64_t v = value();
        if ( v >= 1024*1024 ) {
            double MB = static_cast<double>(v) / (1024*1024);
            return QString("%1 MB").arg(MB);
        }
        if ( v >= 1024 ) {
            double kB = static_cast<double>(v) / (1024);
            return QString("%1 kB").arg(kB);
        }
        return QString("%1 bytes").arg(v);
    }
};



void CountersTest::testCustomCounter()
{
    BytesCounter counter("BytesCounter");
    QCOMPARE(counter.label(), QStringLiteral("0 bytes"));
    counter.add(100);
    QCOMPARE(counter.label(), QStringLiteral("100 bytes"));
    QCOMPARE(counter.add(924), 1024);
    QCOMPARE(counter.label(), QStringLiteral("1 kB"));
    QCOMPARE(counter.add(512), 1536);
    QCOMPARE(counter.label(), QStringLiteral("1.5 kB"));
    QCOMPARE(counter.add(-512), 1024);
    QCOMPARE(counter.label(), QStringLiteral("1 kB"));
    QCOMPARE(counter.add(1023*1024), 1024*1024);
    QCOMPARE(counter.label(), QStringLiteral("1 MB"));
    QCOMPARE(counter.add(512*1024), 1536*1024);
    QCOMPARE(counter.label(), QStringLiteral("1.5 MB"));
}

class AddCounter : public IAggregateCounter {
public:
    Type type() const { return Integer; }
    AddCounter(const QString &name, ICounterBase *c1, ICounterBase *c2) : IAggregateCounter (name) {
        addCounter(c1);
        addCounter(c2);
    }
    QVariant genericValue() const { return QVariant::fromValue(m_value); }
    int64_t value() const { return m_value; }
protected:
    void updateValue() {
        m_value = counters().first()->value<int>() + counters().last()->value<int>();
        emit valueChanged();
    }
private:
    int64_t m_value;
};

void CountersTest::testAggregateCounter()
{
    IIntCounter c1("C1");
    IIntCounter c2("C2");
    AddCounter sum("SUM", &c1, &c2);
    QVERIFY(c1.add(100) == 100);
    QVERIFY(c2.add(100) == 100);
    QCOMPARE(sum.value(), 200L);
    QVERIFY2(sum.add(100) == 200, "aggregate counter is read-only, value cannot be set directly");
}

QTEST_APPLESS_MAIN(CountersTest)

#include "tst_counterstest.moc"
