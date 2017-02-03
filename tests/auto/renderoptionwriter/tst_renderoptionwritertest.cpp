#include <QString>
#include <QtTest>
#include <QFont>
#include "renderable.h"

class RenderOptionWriterTest : public QObject
{
    Q_OBJECT

public:
    RenderOptionWriterTest();

private Q_SLOTS:
//    void testCase1_data();
    void testCase1();
    void testReader();
};

RenderOptionWriterTest::RenderOptionWriterTest()
{
}

//void RenderOptionWriterTest::testCase1_data()
//{
//    QTest::addColumn<QString>("data");
//    QTest::newRow("0") << QString();
//}

class FakeRenderable : public Renderable {
public:
    RenderOptionSet* createOptionSet(const QString &name) const {
        return optionSet(name);
    }
};

void RenderOptionWriterTest::testCase1()
{
    FakeRenderable builder;
    RenderOptionSet* set = builder.createOptionSet("name");
    RenderOptionBase* opt = set->addOption<int>("integerOpt", 0);
    opt->setValue(7);
    opt = set->addOption<QString>("stringOpt", QString());
    RenderOptionSet* sub = set->addSubSet("sub");
    sub->addOption<QFont>("font", QFont());

    JsonRenderOptionWriter writer;
    writer.write(set);
    qDebug() << writer.result();
    delete set;
}

void RenderOptionWriterTest::testReader()
{
    JsonRenderOptionReader reader;
    FakeRenderable builder;
    RenderOptionSet* set = builder.createOptionSet("root");
    set->addOption<int>("opt1", 42);
    QJsonDocument doc = QJsonDocument::fromJson("{ \"options\": [ { \"name\": \"opt1\", \"value\": 37}  ] }");
    QVERIFY(set->value("opt1").toInt() == 42);
    reader.read(doc.object(), set);
    QCOMPARE(set->value("opt1").toInt(), 37);
}

QTEST_APPLESS_MAIN(RenderOptionWriterTest)

#include "tst_renderoptionwritertest.moc"
