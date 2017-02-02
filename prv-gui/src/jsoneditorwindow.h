#ifndef JSONEDITORWINDOW_H
#define JSONEDITORWINDOW_H

#include <QMainWindow>
#include "jsonmerger.h"

namespace Ui {
class JsonEditorWindow;
}

class JsonEditorWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit JsonEditorWindow(const QString& path,
        JsonMerger* merger,
        QWidget* parent = 0);
    ~JsonEditorWindow();

private slots:
    void updateResult(const QString& result);
    void saveUserJson();
    void loadFiles();

private:
    Ui::JsonEditorWindow* ui;
    QString m_path;
    JsonMerger* m_merger;
};

#endif // JSONEDITORWINDOW_H
