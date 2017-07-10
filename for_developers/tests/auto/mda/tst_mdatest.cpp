#include <QString>
#include <QtTest>
#include "mda/mda.h"
#include <objectregistry.h>

using VD = QVector<double>;

class MdaTest : public QObject {
    Q_OBJECT

public:
    MdaTest();

private Q_SLOTS:
    void mda_constructor();
    void mda_constructor_data();
    void allocate();
    void allocate_data();
    void get1();
    void get1_data();
    void invalid_readfile();

private:
    ObjectRegistry m_registry; // prevent warnings about missing registry
};

MdaTest::MdaTest()
{
}

void MdaTest::mda_constructor()
{
    QFETCH(bigint, N1);
    QFETCH(bigint, N2);
    QFETCH(bigint, N3);
    QFETCH(bigint, N4);
    QFETCH(bigint, N5);
    QFETCH(bigint, N6);
    QFETCH(int, ndims);
    QFETCH(bigint, totalSize);

    Mda mda(N1, N2, N3, N4, N5, N6);

    QCOMPARE(mda.N1(), N1);
    QCOMPARE(mda.N2(), N2);
    QCOMPARE(mda.N3(), N3);
    QCOMPARE(mda.N4(), N4);
    QCOMPARE(mda.N5(), N5);
    QCOMPARE(mda.N6(), N6);

    QCOMPARE(mda.size(0), N1);
    QCOMPARE(mda.size(1), N2);
    QCOMPARE(mda.size(2), N3);
    QCOMPARE(mda.size(3), N4);
    QCOMPARE(mda.size(4), N5);
    QCOMPARE(mda.size(5), N6);

    //    QEXPECT_FAIL("000000", "Needs to be fixed", Continue);
    //    QEXPECT_FAIL("negative", "Needs to be fixed", Continue);
    //    QEXPECT_FAIL("negative total size", "Needs to be fixed", Continue);
    QCOMPARE(mda.ndims(), ndims);

    //    QEXPECT_FAIL("negative", "Needs to be fixed", Continue);
    QCOMPARE(mda.totalSize(), totalSize);
    if (totalSize == 0) {
        QVERIFY2(mda.dataPtr() == nullptr, "For invalid dims no data should be allocated");
    }
}

void MdaTest::mda_constructor_data()
{
    QTest::addColumn<bigint>("N1");
    QTest::addColumn<bigint>("N2");
    QTest::addColumn<bigint>("N3");
    QTest::addColumn<bigint>("N4");
    QTest::addColumn<bigint>("N5");
    QTest::addColumn<bigint>("N6");
    QTest::addColumn<int>("ndims");
    QTest::addColumn<bigint>("totalSize");

    QTest::newRow("111111")
        << bigint(1) << bigint(1) << bigint(1) << bigint(1) << bigint(1) << bigint(1)
        << 2 << bigint(1);
    QTest::newRow("123456")
        << bigint(1) << bigint(2) << bigint(3) << bigint(4) << bigint(5) << bigint(6)
        << 6 << bigint(720);
    QTest::newRow("123451")
        << bigint(1) << bigint(2) << bigint(3) << bigint(4) << bigint(5) << bigint(1)
        << 5 << bigint(120);
    QTest::newRow("222212")
        << bigint(2) << bigint(2) << bigint(2) << bigint(2) << bigint(1) << bigint(2)
        << 6 << bigint(32);

    // 0 and negative values do not make any sense, total size should be 0
    QTest::newRow("000000")
        << bigint(0) << bigint(0) << bigint(0) << bigint(0) << bigint(0) << bigint(0)
        << 0 << bigint(0);
    QTest::newRow("negative")
        << -bigint(1) << -bigint(1) << -bigint(1) << -bigint(1) << -bigint(1) << -bigint(1)
        << 0 << bigint(0);
    QTest::newRow("negative total size")
        << bigint(1) << bigint(2) << bigint(3) << -bigint(4) << bigint(5) << bigint(6)
        << 0 << bigint(0);
}

void MdaTest::allocate()
{
    Mda mda(1, 1, 1, 1, 1, 1);
    QVERIFY(mda.N1() == 1);
    QVERIFY(mda.N2() == 1);
    QVERIFY(mda.N3() == 1);
    QVERIFY(mda.N4() == 1);
    QVERIFY(mda.N5() == 1);
    QVERIFY(mda.N6() == 1);

    QVERIFY(mda.totalSize() == 1);

    // reallocate a different configuration

    QFETCH(bigint, N1);
    QFETCH(bigint, N2);
    QFETCH(bigint, N3);
    QFETCH(bigint, N4);
    QFETCH(bigint, N5);
    QFETCH(bigint, N6);
    QFETCH(int, ndims);
    QFETCH(bigint, totalSize);

    QCOMPARE(mda.allocate(N1, N2, N3, N4, N5, N6), true);

    QCOMPARE(mda.minimum(), 0.0); // check if data was initialized
    QCOMPARE(mda.maximum(), 0.0); // check if data was initialized

    QCOMPARE(mda.N1(), N1);
    QCOMPARE(mda.N2(), N2);
    QCOMPARE(mda.N3(), N3);
    QCOMPARE(mda.N4(), N4);
    QCOMPARE(mda.N5(), N5);
    QCOMPARE(mda.N6(), N6);

    QCOMPARE(mda.size(0), N1);
    QCOMPARE(mda.size(1), N2);
    QCOMPARE(mda.size(2), N3);
    QCOMPARE(mda.size(3), N4);
    QCOMPARE(mda.size(4), N5);
    QCOMPARE(mda.size(5), N6);

    //    QEXPECT_FAIL("000000", "Needs to be fixed", Continue);
    //    QEXPECT_FAIL("negative", "Needs to be fixed", Continue);
    //    QEXPECT_FAIL("negative total size", "Needs to be fixed", Continue);
    QCOMPARE(mda.ndims(), ndims);
    QCOMPARE(mda.totalSize(), totalSize);
    if (totalSize == 0) {
        QVERIFY2(mda.dataPtr() == nullptr, "For invalid dims no data should be allocated");
    }
}

void MdaTest::allocate_data()
{
    QTest::addColumn<bigint>("N1");
    QTest::addColumn<bigint>("N2");
    QTest::addColumn<bigint>("N3");
    QTest::addColumn<bigint>("N4");
    QTest::addColumn<bigint>("N5");
    QTest::addColumn<bigint>("N6");
    QTest::addColumn<int>("ndims");
    QTest::addColumn<bigint>("totalSize");

    //    QTest::newRow("111111")
    //            << bigint(1) << bigint(1) << bigint(1) << bigint(1) << bigint(1) << bigint(1)
    //            << 2 << bigint(1);
    QTest::newRow("123456")
        << bigint(1) << bigint(2) << bigint(3) << bigint(4) << bigint(5) << bigint(6)
        << 6 << bigint(720);
    QTest::newRow("123451")
        << bigint(1) << bigint(2) << bigint(3) << bigint(4) << bigint(5) << bigint(1)
        << 5 << bigint(120);
    QTest::newRow("222212")
        << bigint(2) << bigint(2) << bigint(2) << bigint(2) << bigint(1) << bigint(2)
        << 6 << bigint(32);

    // 0 and negative values do not make any sense, total size should be 0
    QTest::newRow("000000")
        << bigint(0) << bigint(0) << bigint(0) << bigint(0) << bigint(0) << bigint(0)
        << 0 << bigint(0);
    QTest::newRow("negative")
        << -bigint(1) << -bigint(1) << -bigint(1) << -bigint(1) << -bigint(1) << -bigint(1)
        << 0 << bigint(0);
    QTest::newRow("negative total size")
        << bigint(1) << bigint(2) << bigint(3) << -bigint(4) << bigint(5) << bigint(6)
        << 0 << bigint(0);
}

void MdaTest::get1()
{
    Mda mda(2, 2, 2);
    QVERIFY2(mda.minimum() == mda.maximum()
            && mda.minimum() == 0,
        "Data is initialized to 0");

    for (int i = 0; i < mda.totalSize(); ++i) {
        QVERIFY2(mda.get(i) == 0, "get does not return proper value");
    }
    QFETCH(VD, input);
    std::copy(input.constData(), input.constData() + mda.totalSize(), mda.dataPtr());
    for (int i = 0; i < mda.totalSize(); ++i) {
        QCOMPARE(mda.get(i), input.at(i));
    }
}

void MdaTest::get1_data()
{
    QTest::addColumn<VD>("input");
    QTest::newRow("12345678") << VD({ 1, 2, 3, 4, 5, 6, 7, 8 });
    QTest::newRow("negative") << VD(8, -1);
}

void MdaTest::invalid_readfile()
{
    Mda mda("invalidfile.mda");
    QCOMPARE(mda.ndims(), 2); // minimum to have a matrix
    QCOMPARE(mda.N1(), bigint(1));
    QCOMPARE(mda.N2(), bigint(1));
}

QTEST_APPLESS_MAIN(MdaTest)

#include "tst_mdatest.moc"
