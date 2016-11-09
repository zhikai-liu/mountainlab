#include <QString>
#include <QtTest>
#include <objectregistry.h>

class TestClass : public QObject {
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue)
public:
    TestClass() {}
    int value() const { return m_value; }
    void setValue(int v) { m_value = v; }

private:
    int m_value;
};

class TestSubclass : public TestClass {
    Q_OBJECT
};

class ObjectRegistryTest : public QObject {
    Q_OBJECT

public:
    ObjectRegistryTest();

private Q_SLOTS:
    void testInstance();
    void testEmpty();
    void testAddObject();
    void testAutoReleasedObject();
    void testRemoveObject();
    void testGetObject();
    void testGetObjects();
    void testGetObjectByName();
    void testGetObjectByClassName();
    void testGetObjectPredicate();
    void testGetObjectsPredicate();
};

/*
 * Cleaner class removes object from the registry
 * in case any functionality is broken
 */
class Cleaner {
public:
    Cleaner(QObject* o)
        : m_object(o)
    {
    }
    ~Cleaner()
    {
        if (m_object)
            ObjectRegistry::removeObject(m_object);
    }

private:
    QPointer<QObject> m_object;
};

ObjectRegistryTest::ObjectRegistryTest()
{
}

void ObjectRegistryTest::testInstance()
{

    QVERIFY2(ObjectRegistry::instance() == nullptr, "ObjectRegistry instance invalid");
    {
        ObjectRegistry registry;
        QVERIFY2(ObjectRegistry::instance() != 0, "ObjectRegistry instance unavailable");
        // registry goies out of scope and is destroyed
    }
    QVERIFY2(ObjectRegistry::instance() == nullptr, "ObjectRegistry instance pointer not cleared");
}

void ObjectRegistryTest::testEmpty()
{
    ObjectRegistry registry;
    // check that the registry is empty
    QObject* o = ObjectRegistry::getObject<QObject>();
    QCOMPARE(o, (QObject*)0);
    QList<QObject*> list = ObjectRegistry::getObjects<QObject>();
    QVERIFY(list.isEmpty());
    QVERIFY(ObjectRegistry::allObjects().isEmpty());
}

void ObjectRegistryTest::testAddObject()
{
    QTest::ignoreMessage(QtWarningMsg, "No ObjectRegistry instance present");
    ObjectRegistry::addObject(this);
    ObjectRegistry registry;
    QVERIFY(ObjectRegistry::allObjects().isEmpty());
    QSignalSpy spy(ObjectRegistry::instance(), SIGNAL(objectAdded(QObject*)));
    QPointer<TestClass> obj = new TestClass;
    QPointer<TestClass> obj2 = new TestClass;
    Cleaner cleaner(obj);
    Cleaner cleaner2(obj2);
    ObjectRegistry::addObject(obj);
    QCOMPARE(ObjectRegistry::allObjects().size(), 1);
    ObjectRegistry::addObject(obj2);
    QCOMPARE(spy.count(), 2);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).value<QObject*>(), (QObject*)obj);
    args = spy.takeFirst();
    QCOMPARE(args.at(0).value<QObject*>(), (QObject*)obj2);
}

void ObjectRegistryTest::testAutoReleasedObject()
{
    QTest::ignoreMessage(QtWarningMsg, "No ObjectRegistry instance present");
    ObjectRegistry::addAutoReleasedObject(this);
    QPointer<TestClass> obj = new TestClass;
    {
        ObjectRegistry registry;
        QVERIFY(ObjectRegistry::allObjects().isEmpty());
        ObjectRegistry::addAutoReleasedObject(obj);
        QVERIFY(ObjectRegistry::allObjects().size() == 1);
    }
    // at this point the object should be deleted
    QVERIFY2(obj.isNull(), "Auto released object was not released");
}

void ObjectRegistryTest::testRemoveObject()
{
    QTest::ignoreMessage(QtWarningMsg, "No ObjectRegistry instance present");
    ObjectRegistry::removeObject(this);
    ObjectRegistry registry;
    QVERIFY(ObjectRegistry::allObjects().isEmpty());
    QPointer<TestClass> obj = new TestClass;
    Cleaner cleaner(obj);
    QPointer<TestClass> protObj = obj;
    QSignalSpy aboutToRemoveSpy(ObjectRegistry::instance(), SIGNAL(objectAboutToBeRemoved(QObject*)));
    QSignalSpy removedSpy(ObjectRegistry::instance(), SIGNAL(objectRemoved(QObject*)));
    ObjectRegistry::addObject(obj);
    QCOMPARE(ObjectRegistry::allObjects().size(), 1);
    ObjectRegistry::removeObject(obj);
    QCOMPARE(ObjectRegistry::allObjects().size(), 0);
    QCOMPARE(aboutToRemoveSpy.count(), 1);
    QCOMPARE(removedSpy.count(), 1);
    QList<QVariant> args = aboutToRemoveSpy.takeFirst();
    QCOMPARE(args.at(0).value<QObject*>(), (QObject*)obj);
    args = removedSpy.takeFirst();
    QCOMPARE(args.at(0).value<QObject*>(), (QObject*)obj);
    // after removing, obj should be a valid object
    QCOMPARE(protObj.data(), obj.data());
}

void ObjectRegistryTest::testGetObject()
{
    QTest::ignoreMessage(QtWarningMsg, "No ObjectRegistry instance present");
    ObjectRegistry::getObject<TestClass>();
    ObjectRegistry registry;
    QVERIFY(ObjectRegistry::allObjects().isEmpty());
    QPointer<TestClass> obj = new TestClass;
    ObjectRegistry::addObject(obj);
    QCOMPARE(ObjectRegistry::getObject<TestClass>(), obj.data());
    // get as QObject
    QCOMPARE(ObjectRegistry::getObject<QObject>(), static_cast<QObject*>(obj));
    //  try to get as wrong class
    QCOMPARE(ObjectRegistry::getObject<TestSubclass>(), static_cast<TestSubclass*>(0));
}

void ObjectRegistryTest::testGetObjects()
{
    QTest::ignoreMessage(QtWarningMsg, "No ObjectRegistry instance present");
    ObjectRegistry::getObjects<TestClass>();
    ObjectRegistry registry;
    QVERIFY(ObjectRegistry::allObjects().isEmpty());
    QPointer<TestClass> obj = new TestClass;
    ObjectRegistry::addObject(obj);
    QCOMPARE(ObjectRegistry::getObjects<TestClass>().size(), 1);
    QCOMPARE(ObjectRegistry::getObjects<TestClass>().at(0), obj.data());
    // get as QObject
    QCOMPARE(ObjectRegistry::getObjects<QObject>().size(), 1);
    QCOMPARE(ObjectRegistry::getObjects<QObject>().at(0), (QObject*)obj);
    // try to get as wrong class
    QCOMPARE(ObjectRegistry::getObjects<TestSubclass>().size(), 0);
}

void ObjectRegistryTest::testGetObjectByName()
{
    QTest::ignoreMessage(QtWarningMsg, "No ObjectRegistry instance present");
    ObjectRegistry::getObjectByName("");
    ObjectRegistry registry;
    QVERIFY(ObjectRegistry::allObjects().isEmpty());
    QPointer<TestClass> obj = new TestClass;
    obj->setObjectName("_OBJECT_REGISTRY_OBJECT_NAME_");
    ObjectRegistry::addObject(obj);
    // check we can actually retrieve the object
    QVERIFY(ObjectRegistry::getObject<TestClass>() == obj.data());
    // check if we can retrieve the object by name
    QVERIFY(ObjectRegistry::getObjectByName("_OBJECT_REGISTRY_OBJECT_NAME_") == obj.data());
    QVERIFY(ObjectRegistry::getObjectByName("_BOGUS_NAME_") == nullptr);
}

void ObjectRegistryTest::testGetObjectByClassName()
{
    QTest::ignoreMessage(QtWarningMsg, "No ObjectRegistry instance present");
    ObjectRegistry::getObjectByClassName("TestClass>");
    ObjectRegistry registry;
    QVERIFY(ObjectRegistry::allObjects().isEmpty());
    QPointer<TestClass> obj = new TestClass;
    ObjectRegistry::addObject(obj);
    // check we can actually retrieve the object
    QVERIFY(ObjectRegistry::getObject<TestClass>() == obj.data());
    // check if we can retrieve the object by name
    QVERIFY(ObjectRegistry::getObjectByClassName("TestClass") == obj.data());
    QVERIFY(ObjectRegistry::getObjectByClassName("TestSubclass") == nullptr);
}

class ValuePredicate {
public:
    ValuePredicate(int v)
        : m_v(v)
    {
    }
    bool operator()(TestClass* tc) { return tc->value() == m_v; }

private:
    int m_v;
};

void ObjectRegistryTest::testGetObjectPredicate()
{
    QTest::ignoreMessage(QtWarningMsg, "No ObjectRegistry instance present");
    ObjectRegistry::getObject<TestClass>(ValuePredicate(7));
    ObjectRegistry registry;
    QVERIFY(ObjectRegistry::allObjects().isEmpty());
    QPointer<TestClass> obj = new TestClass;
    obj->setValue(7);
    ObjectRegistry::addObject(obj);
    // check we can actually retrieve the object
    QVERIFY(ObjectRegistry::getObject<TestClass>() == obj.data());
    // retrieve by valid predicate
    TestClass* retrieved = ObjectRegistry::getObject<TestClass>(ValuePredicate(7));
    QVERIFY(retrieved == obj);
    // retrieve by invalid predicate
    retrieved = ObjectRegistry::getObject<TestClass>(ValuePredicate(6));
    QVERIFY2(retrieved == nullptr, "Predicate not satisfied, yet object returned");
}

void ObjectRegistryTest::testGetObjectsPredicate()
{
    QTest::ignoreMessage(QtWarningMsg, "No ObjectRegistry instance present");
    ObjectRegistry::getObjects<TestClass>(ValuePredicate(6));
    ObjectRegistry registry;
    QVERIFY(ObjectRegistry::allObjects().isEmpty());
    QPointer<TestClass> obj = new TestClass;
    obj->setValue(7);
    ObjectRegistry::addObject(obj);
    // check we can actually retrieve the object
    QVERIFY(ObjectRegistry::getObject<TestClass>() == obj.data());
    // retrieve by valid predicate
    QList<TestClass*> retrieved = ObjectRegistry::getObjects<TestClass>(ValuePredicate(7));
    QVERIFY(retrieved.size() == 1 && retrieved.at(0) == obj);
    // retrieve by invalid predicate
    retrieved = ObjectRegistry::getObjects<TestClass>(ValuePredicate(6));
    QVERIFY2(retrieved.size() == 0, "Predicate not satisfied, yet object returned");
}

QTEST_APPLESS_MAIN(ObjectRegistryTest)

#include "tst_objectregistrytest.moc"
