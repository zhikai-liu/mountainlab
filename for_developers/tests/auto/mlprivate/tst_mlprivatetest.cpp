#include <QString>
#include <QtTest>
#include "mlprivate.h"

class TestClass;
class TestClassPrivate : public MLPrivate<TestClass> {
public:
    TestClassPrivate(bool& del, TestClass* qq)
        : MLPrivate(qq)
        , value(42)
        , m_del(del)
    {
    }
    ~TestClassPrivate()
    {
        m_del = true;
    }
    int value;

private:
    bool& m_del;
};

class TestClass : public MLPublic<TestClassPrivate> {
public:
    TestClass(bool& del)
        : MLPublic(new TestClassPrivate(del, this))
    {
    }
    int value() const { return d->value; }
    void setValue(int v) { d->value = v; }
};

class MLPrivateTest : public QObject {
    Q_OBJECT

public:
    MLPrivateTest();

private Q_SLOTS:
    void testDeletion();
    void testAccess();
};

MLPrivateTest::MLPrivateTest()
{
}

void MLPrivateTest::testDeletion()
{
    bool deleted = false;
    {
        TestClass object(deleted);
        object.setValue(0); // make sure it doesn't get optimized out
    }
    QCOMPARE(deleted, true);
}

void MLPrivateTest::testAccess()
{
    bool unused;
    TestClass object(unused);
    QVERIFY2(object.value() == 42, "private component not initialized");
    object.setValue(24);
    QVERIFY2(object.value() == 24, "private component immutable");
}

QTEST_APPLESS_MAIN(MLPrivateTest)

#include "tst_mlprivatetest.moc"
