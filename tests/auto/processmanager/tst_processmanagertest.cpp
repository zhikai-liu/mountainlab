#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include "qprocessmanager.h"

class ProcessManagerTest : public QObject
{
    Q_OBJECT

public:
    ProcessManagerTest();

private Q_SLOTS:
    void testEmpty();
    void testFinished();
    void testDestroy();
    void testDestroyWithParent();
    void testKill();
    void testKillLast();
    void testKillAll();
};

ProcessManagerTest::ProcessManagerTest()
{
}

void ProcessManagerTest::testEmpty()
{
    QProcessManager manager;
    QCOMPARE(manager.count(), 0);
}

void ProcessManagerTest::testFinished() {
    QProcessManager manager;
    QSharedPointer<QProcess> process = manager.start("/bin/true");
    QSignalSpy spy(process.data(), SIGNAL(finished(int)));
    QCOMPARE(manager.count(), 1);
    QVERIFY2(spy.wait(), "finished() was not emitted");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(manager.count(), 0);
}

void ProcessManagerTest::testDestroy()
{
    QProcessManager manager;
    QSharedPointer<QProcess> process = manager.start("/bin/true");
    QCOMPARE(manager.count(), 1);
    QSignalSpy spy(process.data(), SIGNAL(destroyed(QObject*)));
    process.reset(); // releasing handle
    QVERIFY2(spy.wait(), "destroy() was not emitted");
    QCOMPARE(manager.count(), 0);
}

void ProcessManagerTest::testDestroyWithParent()
{
    QProcessManager manager;
    QProcess* process = manager.start("/bin/true").data();
    process->setParent(this); // setting a parent prevents the object from being destroyed
    QCOMPARE(manager.count(), 1);
    QSignalSpy spy(process, SIGNAL(destroyed(QObject*)));
    QVERIFY2(!spy.wait(), "destroy() shouldn't be emitted");
    QCOMPARE(spy.count(), 0);
    delete process;
    QCOMPARE(spy.count(), 1); // now it is destroyed();
}

void ProcessManagerTest::testKill()
{
    QProcessManager manager;
    QSharedPointer<QProcess> process = manager.start("/bin/sleep", {"500"});
    QSignalSpy spy(process.data(), SIGNAL(finished(int)));
    QCOMPARE(manager.count(), 1);
    QTest::qWait(10);
    QVERIFY(spy.isEmpty()); // hasn't finished yet
    manager.closeLast();
    QTest::qWait(10);
    QCOMPARE(manager.count(), 0);
    QVERIFY2(!spy.isEmpty(), "finished() wasn't emitted");
}

void ProcessManagerTest::testKillLast()
{
    QProcessManager manager;
    manager.start("/bin/sleep", {"500"});
    manager.start("/bin/sleep", {"500"});
    QCOMPARE(manager.count(), 2);
    QTest::qWait(10);
    manager.closeLast();
    QTest::qWait(10);
    QCOMPARE(manager.count(), 1);
    manager.closeLast();
    QTest::qWait(10);
    QCOMPARE(manager.count(), 0);
}

void ProcessManagerTest::testKillAll()
{
    QProcessManager manager;
    QSharedPointer<QProcess> p1 = manager.start("/bin/sleep", {"500"});
    QSharedPointer<QProcess> p2 = manager.start("/bin/sleep", {"500"});
    QCOMPARE(manager.count(), 2);
    QTest::qWait(10);
    manager.closeAll();
    QTest::qWait(10);
    QCOMPARE(manager.count(), 0);
}

QTEST_MAIN(ProcessManagerTest)

#include "tst_processmanagertest.moc"
