#include <QString>
#include <QtTest>
#include "signalhandler.h"

class SignalCatcher : public QObject, public QObjectList {
    Q_OBJECT
public:
    SignalCatcher(QObject* parent = nullptr)
        : QObject(parent)
    {
    }
signals:
    void triggered();
public slots:
    void trigger()
    {
        append(sender());
        emit triggered();
    }
};

class SignalHandlerTest : public QObject {
    Q_OBJECT

public:
    SignalHandlerTest();
    void kill(int sig)
    {
        ::kill(QCoreApplication::applicationPid(), sig);
    }

private Q_SLOTS:
    void basicFunction();
    void basicSlot();
    void another();
    void setFunction();
    void setSlot();
    void priorityFunction();
    void prioritySlot();
    void mixed();
    void uninstall();
    void uninstallMultiple();
    void paused();
    void resume();
};

SignalHandlerTest::SignalHandlerTest()
{
}

void SignalHandlerTest::basicFunction()
{
    SignalHandler handler;
    bool fired = false;
    handler.installHandler(SIGHUP, [&fired]() { fired = true; });
    kill(SIGHUP);
    QVERIFY(fired);
}

void SignalHandlerTest::basicSlot()
{
    SignalHandler handler;
    SignalCatcher catcher;
    handler.installHandler(SIGHUP, &catcher, SLOT(trigger()));
    kill(SIGHUP);
    QVERIFY(catcher.size() == 1);
}

void SignalHandlerTest::another()
{
    SignalHandler handler;
    bool fired = false;
    handler.installHandler(SIGUSR1, [&fired]() { fired = true; });
    kill(SIGHUP);
    QVERIFY(!fired);
}

void SignalHandlerTest::setFunction()
{
    SignalHandler handler;
    bool fired = false;
    handler.installHandler(SignalHandler::SigHangUp | SignalHandler::SigUser1, [&fired]() { fired = true; });
    kill(SIGHUP);
    QVERIFY(fired);
    fired = false;
    kill(SIGUSR1);
    QVERIFY(fired);
}

void SignalHandlerTest::setSlot()
{
    SignalHandler handler;
    SignalCatcher catcher;
    handler.installHandler(SignalHandler::SigHangUp | SignalHandler::SigUser1, &catcher, SLOT(trigger()));
    kill(SIGHUP);
    QVERIFY(catcher.size() == 1);
    kill(SIGUSR1);
    QVERIFY(catcher.size() == 2);
}

void SignalHandlerTest::priorityFunction()
{
    SignalHandler handler;
    QVector<int> fired;
    handler.installHandler(SIGHUP, [&fired]() { fired << 0; });
    handler.installHandler(SIGHUP, [&fired]() { fired << 10; }, 10);

    kill(SIGHUP);
    QVERIFY(fired.count() == 2);
    QVERIFY(fired.first() == 10);
    QVERIFY(fired.last() == 0);
}

void SignalHandlerTest::prioritySlot()
{
    SignalHandler handler;
    SignalCatcher catcher;
    SignalCatcher catcher10;
    QVector<int> fired;
    connect(&catcher, &SignalCatcher::triggered, [&fired]() { fired.append(0); });
    connect(&catcher10, &SignalCatcher::triggered, [&fired]() { fired.append(10); });
    handler.installHandler(SIGHUP, &catcher, SLOT(trigger()));
    handler.installHandler(SIGHUP, &catcher10, SLOT(trigger()), 10);
    kill(SIGHUP);
    QVERIFY(catcher.count() == 1);
    QVERIFY(catcher10.count() == 1);
    QVERIFY(fired.count() == 2);
    QVERIFY(fired.first() == 10);
    QVERIFY(fired.last() == 0);
}

void SignalHandlerTest::mixed()
{
    SignalHandler handler;
    SignalCatcher catcher;
    bool fired = false;
    handler.installHandler(SIGHUP, [&fired]() { fired = true; });
    handler.installHandler(SIGHUP, &catcher, SLOT(trigger()));
    kill(SIGHUP);
    QVERIFY(fired);
    QVERIFY(catcher.count() == 1);
}

void SignalHandlerTest::uninstall()
{
    SignalHandler handler;
    QVector<int> fired;
    size_t id = handler.installHandler(SIGHUP, [&fired]() { fired << 0; });
    kill(SIGHUP);
    QVERIFY(fired.count() == 1);
    fired.clear();
    handler.uninstallHandler(id);
    kill(SIGHUP);
    QVERIFY(fired.isEmpty());
}

void SignalHandlerTest::uninstallMultiple()
{
    SignalHandler handler;
    QVector<int> fired;
    size_t id = handler.installHandler(SIGHUP, [&fired]() { fired << 0; });
    size_t id2 = handler.installHandler(SIGHUP, [&fired]() { fired << 10; }, 10);
    Q_UNUSED(id2)
    kill(SIGHUP);
    QVERIFY(fired.count() == 2);
    fired.clear();
    handler.uninstallHandler(id);
    kill(SIGHUP);
    QVERIFY(fired.count() == 1);
}

void SignalHandlerTest::paused()
{
    SignalHandler handler;
    bool fired = false;
    handler.installHandler(SIGHUP, [&fired]() { fired = true; });
    handler.pause();
    kill(SIGHUP);
    QEXPECT_FAIL("", "pause() and resume() not implemented", Continue);
    QVERIFY(!fired);
}

void SignalHandlerTest::resume()
{
    SignalHandler handler;
    bool fired = false;
    handler.installHandler(SIGHUP, [&fired]() { fired = true; });
    handler.pause();
    kill(SIGHUP);
    QEXPECT_FAIL("", "pause() and resume() not implemented", Continue);
    QVERIFY(!fired);
    fired = false;
    handler.resume();
    kill(SIGHUP);
    QVERIFY(fired);
}

QTEST_APPLESS_MAIN(SignalHandlerTest)

#include "tst_signalhandlertest.moc"
