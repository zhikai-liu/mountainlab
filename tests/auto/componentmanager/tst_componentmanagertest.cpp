#include <QString>
#include <QtTest>
#include "componentmanager.h"
#include "icomponent.h"
#include <functional>

class TestComponent : public IComponent {
    Q_OBJECT
    using TestFunc = std::function<bool(const TestComponent*)>;

public:
    TestComponent(QObject* parent = 0)
        : IComponent(parent)
        , m_name("TestComponent")
    {
    }
    TestComponent(const QString& n = "TestComponent", QObject* parent = 0)
        : IComponent(parent)
        , m_name(n)
    {
    }
    QString name() const { return m_name; }
    QStringList dependencies() const { return m_deps; }
    bool initialize()
    {
        if (m_initFunc)
            return m_initFunc(this);
        return true;
    }
    void extensionsReady()
    {
        if (m_extFunc)
            m_extFunc(this);
    }
    void uninitialize()
    {
        if (m_uninitFunc)
            m_uninitFunc(this);
    }

    void setInitializeFunction(const TestFunc& f)
    {
        m_initFunc = f;
    }
    void setExtReadyFunction(const TestFunc& f)
    {
        m_extFunc = f;
    }
    void setUninitializeFunction(const TestFunc& f)
    {
        m_uninitFunc = f;
    }
    void setDependencies(const QStringList& deps)
    {
        m_deps = deps;
    }

private:
    QString m_name;
    QStringList m_deps;
    TestFunc m_initFunc;
    TestFunc m_extFunc;
    TestFunc m_uninitFunc;
};

class ComponentManagerTest : public QObject {
    Q_OBJECT

public:
    ComponentManagerTest();

private Q_SLOTS:
    void testLoadSingle();
    void testUnLoadOnDestruct();
    void testInitFailed();
    void testLoadTwo();
    void testDependency();
    void testDependency_data();
};

ComponentManagerTest::ComponentManagerTest()
{
}

void ComponentManagerTest::testLoadSingle()
{
    ComponentManager manager;
    TestComponent* component = new TestComponent(&manager);
    bool initCalled = false;
    bool extCalled = false;
    bool uninitCalled = false;
    component->setInitializeFunction([&initCalled](const TestComponent*) {
        initCalled = true; return true;
    });
    component->setExtReadyFunction([&extCalled](const TestComponent*) {
        extCalled = true; return true;
    });
    component->setUninitializeFunction([&uninitCalled](const TestComponent*) {
        uninitCalled = true; return true;
    });
    manager.addComponent(component);
    manager.loadComponents();
    QVERIFY(manager.loadedComponents().size() == 1);
    QVERIFY2(initCalled == true, "initialize not called");
    QVERIFY2(extCalled == true, "extensionsInitialized not called");
    QVERIFY2(uninitCalled == false,
        "uninitialize() should not be called "
        "before plugin is uninitialized");
    manager.unloadComponents();
    QVERIFY2(uninitCalled == true, "uninitialize not called");
    QVERIFY(manager.loadedComponents().size() == 0);
}

void ComponentManagerTest::testUnLoadOnDestruct()
{
    bool uninitCalled = false;
    {
        ComponentManager manager;
        TestComponent* component = new TestComponent(&manager);
        component->setUninitializeFunction(
            [&uninitCalled](const TestComponent*) {
            uninitCalled = true; return true;
            });
        manager.addComponent(component);
        manager.loadComponents();
        QVERIFY(manager.loadedComponents().size() == 1);
    }
    QVERIFY2(uninitCalled == true, "not unloaded on manager destruction");
}

void ComponentManagerTest::testInitFailed()
{
    ComponentManager manager;
    TestComponent* component = new TestComponent(&manager);
    bool initCalled = false;
    bool extCalled = false;
    bool uninitCalled = false;
    component->setInitializeFunction([&initCalled](const TestComponent*) {
        initCalled = true; return false;
    });
    component->setExtReadyFunction([&extCalled](const TestComponent*) {
        extCalled = true; return true;
    });
    component->setUninitializeFunction([&uninitCalled](const TestComponent*) {
        uninitCalled = true; return true;
    });
    manager.addComponent(component);
    manager.loadComponents();
    QVERIFY(manager.loadedComponents().size() == 0);
    QVERIFY2(initCalled == true, "initialize not called");
    QVERIFY2(extCalled == false, "extensionsInitialized should not be called");
    manager.unloadComponents();
    QVERIFY2(uninitCalled == false, "uninitialize should not be called");
}

void ComponentManagerTest::testLoadTwo()
{
    ComponentManager manager;
    TestComponent* component = new TestComponent(&manager);
    TestComponent* component2 = new TestComponent("TestComponent2", &manager);
    manager.addComponent(component);
    manager.addComponent(component2);
    manager.loadComponents();
    QCOMPARE(manager.loadedComponents().size(), 2);
}

void ComponentManagerTest::testDependency()
{
    QStringList order;
    auto func = [&order](const TestComponent* cmp) {
        order.append(cmp->name());
        return true;
    };
    QFETCH(QStringList, names);
    QFETCH(QList<QStringList>, deps);
    QFETCH(QStringList, expectedOrder);

    {
        ComponentManager manager;
        QList<TestComponent*> components;
        for (int i = 0; i < names.size(); ++i) {
            TestComponent* component = new TestComponent(names[i], &manager);
            component->setInitializeFunction(func);
            component->setDependencies(deps[i]);
            components.append(component);
        }
        QListIterator<TestComponent*> iter(components);
        while (iter.hasNext()) {
            manager.addComponent(iter.next());
        }
        manager.loadComponents();
        QCOMPARE(manager.loadedComponents().size(), expectedOrder.size());
        QCOMPARE(order, expectedOrder);
        manager.unloadComponents();
        order.clear();
    }
    // loading order should be the same
    // regardless which component was added first
    {
        ComponentManager manager;
        QList<TestComponent*> components;
        for (int i = 0; i < names.size(); ++i) {
            TestComponent* component = new TestComponent(names[i], &manager);
            component->setInitializeFunction(func);
            component->setDependencies(deps[i]);
            components.append(component);
        }
        QListIterator<TestComponent*> iter(components);
        iter.toBack();
        while (iter.hasPrevious()) {
            manager.addComponent(iter.previous());
        }

        manager.loadComponents();
        QCOMPARE(manager.loadedComponents().size(), expectedOrder.size());
        QCOMPARE(order, expectedOrder);
        manager.unloadComponents();
        order.clear();
    }
}

void ComponentManagerTest::testDependency_data()
{
    QTest::addColumn<QStringList>("names"); // component names
    QTest::addColumn<QList<QStringList> >("deps"); // component dependencies
    QTest::addColumn<QStringList>("expectedOrder"); // load order

    /*
     * When designing test cases make sure there is just one
     * load order that satisfies the requirements or else
     * the test case might fail sometimes
     */

    QTest::newRow("satisfied")
        << QStringList({ "C1", "C2" })
        << QList<QStringList>({ { "C2" }, {} })
        << QStringList({ "C2", "C1" });

    QTest::newRow("failed")
        << QStringList({ "C1", "C2" })
        << QList<QStringList>({ {}, { "D1" } })
        << QStringList({ "C1" });

    QTest::newRow("cycle")
        << QStringList({ "C1", "C2" })
        << QList<QStringList>({ { "C2" }, { "C1" } })
        << QStringList();

    QTest::newRow("int cycle")
        << QStringList({ "C1", "C2", "C3" })
        << QList<QStringList>({ { "C2" }, { "C3" }, { "C1" } })
        << QStringList();

    QTest::newRow("depend on self")
        << QStringList({ "C1" })
        << QList<QStringList>({ { "C1" } })
        << QStringList();

    QTest::newRow("depend on two")
        << QStringList({ "D1", "D2", "CMP" })
        << QList<QStringList>({ {}, { "D1" }, { "D1", "D2" } })
        << QStringList({ "D1", "D2", "CMP" });

    QTest::newRow("depend on two, one missing")
        << QStringList({ "D1", "CMP" })
        << QList<QStringList>({ {}, { "D1", "D2" } })
        << QStringList({ "D1" });
}

QTEST_APPLESS_MAIN(ComponentManagerTest)

#include "tst_componentmanagertest.moc"
