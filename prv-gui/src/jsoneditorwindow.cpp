#include "jsoneditorwindow.h"
#include "ui_jsoneditorwindow.h"

#include <QFile>

JsonEditorWindow::JsonEditorWindow(const QString& path,
    JsonMerger* merger,
    QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::JsonEditorWindow)
    , m_path(path)
    , m_merger(merger)
{
    ui->setupUi(this);

    // ui mods
    QPalette p = ui->defaultJsonEdit->palette();
    p.setColor(QPalette::Base, p.color(QPalette::Window));
    ui->defaultJsonEdit->setPalette(p);

    p = ui->resultEdit->palette();
    p.setColor(QPalette::Base, p.color(QPalette::Window));
    ui->resultEdit->setPalette(p);

    ui->actionSave->setIcon(QIcon::fromTheme("document-save"));
    ui->actionQuit->setIcon(QIcon::fromTheme("application-exit"));

    // text edits content
    loadFiles();
    ui->defaultJsonEdit->setPlainText(m_merger->defaultJson());
    ui->userJsonEdit->setPlainText(m_merger->userJson());
    ui->resultEdit->setPlainText(m_merger->resultJson());

    // connections
    connect(ui->actionSave, &QAction::triggered, this, &JsonEditorWindow::saveUserJson);
    connect(m_merger, &JsonMerger::resultChanged, this, &JsonEditorWindow::updateResult);
    connect(ui->userJsonEdit, &QPlainTextEdit::textChanged,
        [this]() {
            if (m_merger)
                m_merger->setUserJson(ui->userJsonEdit->toPlainText());
        });
    connect(m_merger, &JsonMerger::parsingFailed,
        [this](const QString& msg) {
            ui->statusbar->showMessage(QStringLiteral("JSON parsing failed: ") + msg);
        });
}

JsonEditorWindow::~JsonEditorWindow()
{
    delete ui;
}

void JsonEditorWindow::updateResult(const QString& result)
{
    ui->resultEdit->setPlainText(result);
    ui->statusbar->clearMessage();
}

void JsonEditorWindow::saveUserJson()
{
    QFile userJson(m_path + "/mountainlab.user.json");
    if (!userJson.open(QFile::WriteOnly | QFile::Truncate | QFile::Text)) {
        qFatal("Cannot save mountainlan.user.json");
    }
    userJson.write(ui->userJsonEdit->toPlainText().toUtf8());
    userJson.close();
    ui->statusbar->showMessage("File mountainlab.user.json saved!", 3000);
}

void JsonEditorWindow::loadFiles()
{
    QFile defaultJson(m_path + "/mountainlab.default.json");
    if (!defaultJson.open(QFile::ReadOnly | QFile::Text)) {
        qFatal("Cannot open mountainlab.default.json from the given path.");
    }
    m_merger->setDefaultJson(defaultJson.readAll());
    defaultJson.close();
    QFile userJson(m_path + "/mountainlab.user.json");
    if (userJson.open(QFile::ReadOnly | QFile::Text)) {
        m_merger->setUserJson(userJson.readAll());
        userJson.close();
    }
}
