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
    QFETCH(int, N1);
    QFETCH(int, N2);
    QFETCH(int, N3);
    QFETCH(int, N4);
    QFETCH(int, N5);
    QFETCH(int, N6);
    QFETCH(int, ndims);
    QFETCH(int, totalSize);

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
    QTest::addColumn<int>("N1");
    QTest::addColumn<int>("N2");
    QTest::addColumn<int>("N3");
    QTest::addColumn<int>("N4");
    QTest::addColumn<int>("N5");
    QTest::addColumn<int>("N6");
    QTest::addColumn<int>("ndims");
    QTest::addColumn<int>("totalSize");

    QTest::newRow("111111")
        << 1L << 1L << 1L << 1L << 1L << 1L
        << 2 << 1L;
    QTest::newRow("123456")
        << 1L << 2L << 3L << 4L << 5L << 6L
        << 6 << 720L;
    QTest::newRow("123451")
        << 1L << 2L << 3L << 4L << 5L << 1L
        << 5 << 120L;
    QTest::newRow("222212")
        << 2L << 2L << 2L << 2L << 1L << 2L
        << 6 << 32L;

    // 0 and negative values do not make any sense, total size should be 0
    QTest::newRow("000000")
        << 0L << 0L << 0L << 0L << 0L << 0L
        << 0 << 0L;
    QTest::newRow("negative")
        << -1L << -1L << -1L << -1L << -1L << -1L
        << 0 << 0L;
    QTest::newRow("negative total size")
        << 1L << 2L << 3L << -4L << 5L << 6L
        << 0 << 0L;
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

    QFETCH(int, N1);
    QFETCH(int, N2);
    QFETCH(int, N3);
    QFETCH(int, N4);
    QFETCH(int, N5);
    QFETCH(int, N6);
    QFETCH(int, ndims);
    QFETCH(int, totalSize);

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
    QTest::addColumn<int>("N1");
    QTest::addColumn<int>("N2");
    QTest::addColumn<int>("N3");
    QTest::addColumn<int>("N4");
    QTest::addColumn<int>("N5");
    QTest::addColumn<int>("N6");
    QTest::addColumn<int>("ndims");
    QTest::addColumn<int>("totalSize");

    //    QTest::newRow("111111")
    //            << 1L << 1L << 1L << 1L << 1L << 1L
    //            << 2 << 1L;
    QTest::newRow("123456")
        << 1L << 2L << 3L << 4L << 5L << 6L
        << 6 << 720L;
    QTest::newRow("123451")
        << 1L << 2L << 3L << 4L << 5L << 1L
        << 5 << 120L;
    QTest::newRow("222212")
        << 2L << 2L << 2L << 2L << 1L << 2L
        << 6 << 32L;

    // 0 and negative values do not make any sense, total size should be 0
    QTest::newRow("000000")
        << 0L << 0L << 0L << 0L << 0L << 0L
        << 0 << 0L;
    QTest::newRow("negative")
        << -1L << -1L << -1L << -1L << -1L << -1L
        << 0 << 0L;
    QTest::newRow("negative total size")
        << 1L << 2L << 3L << -4L << 5L << 6L
        << 0 << 0L;
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
    QCOMPARE(mda.N1(), 1L);
    QCOMPARE(mda.N2(), 1L);
}

QTEST_APPLESS_MAIN(MdaTest)

#include "tst_mdatest.moc"
