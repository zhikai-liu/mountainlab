/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/30/2016
*******************************************************/

#include "taskprogressview.h"
#include "taskprogress.h"
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QtDebug>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QDialog>
#include <QShortcut>
#include <QApplication>
#include <QClipboard>
#include <QDesktopWidget>
#include <QStandardItemModel>
#include <QTimer>

class TaskProgressViewDelegate : public QStyledItemDelegate {
public:
    TaskProgressViewDelegate(QObject* parent = 0)
        : QStyledItemDelegate(parent)
    {
    }
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        if (index.parent().isValid())
            return QStyledItemDelegate::sizeHint(option, index);
        QSize sh = QStyledItemDelegate::sizeHint(option, index);
        sh.setHeight(sh.height() * 2);
        return sh;
    }
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        if (index.internalId() != 0xDEADBEEF || index.column() != 0) {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }
        QStringList tags = index.data(TaskManager::TaskProgressModel::TagsRole).toStringList();
        QIcon icon;
        if (tags.contains("calculate")) {
            QPixmap px(":/images/calculator.png");
            icon.addPixmap(px);
        }
        else if (tags.contains("download")) {
            QPixmap px(":/images/down.png");
            icon.addPixmap(px);
        }
        else if (tags.contains("process")) {
            QPixmap px(":/images/process.png");
            icon.addPixmap(px);
        }
        QStyleOptionViewItem opt = option;
        opt.text = "";
        opt.displayAlignment = Qt::AlignTop | Qt::AlignLeft;
        opt.decorationPosition = QStyleOptionViewItem::Left;
        opt.decorationAlignment = Qt::AlignTop | Qt::AlignLeft;
        opt.features |= QStyleOptionViewItem::HasDecoration;
        opt.icon = icon;
        QStyledItemDelegate::paint(painter, opt, index);
        qreal progress = index.data(Qt::UserRole).toDouble();
        if (progress < 1.0) {
            QPen p = painter->pen();
            QFont f = painter->font();
            p.setColor((option.state & QStyle::State_Selected) ? Qt::white : Qt::darkGray);
            f.setPointSize(f.pointSize() - 3);
            QFontMetrics smallFm(f);
            int elapsedWidth = smallFm.width("MMMMM");
            QStyleOptionProgressBar progOpt;
            progOpt.initFrom(opt.widget);
            progOpt.rect = opt.rect;
            progOpt.minimum = 0;
            progOpt.maximum = 100;
            progOpt.progress = progress * 100;
            progOpt.rect.setTop(progOpt.rect.center().y());
            progOpt.rect.adjust(4 + elapsedWidth + 4, 2, -4, -2);
            if (option.widget) {
                option.widget->style()->drawControl(QStyle::CE_ProgressBar, &progOpt, painter, option.widget);
            }
            painter->save();
            painter->setPen(p);
            painter->setFont(f);
            QRect r = opt.rect;
            r.setTop(option.rect.center().y());
            r.adjust(4, 2, -4, -2);
            r.setRight(r.left() + elapsedWidth);

            qreal duration = index.data(Qt::UserRole + 1).toDateTime().msecsTo(QDateTime::currentDateTime()) / 1000.0;
            if (duration < 100)
                painter->drawText(r, Qt::AlignRight | Qt::AlignVCenter, QString("%1s").arg(duration, 0, 'f', 2));
            else
                painter->drawText(r, Qt::AlignRight | Qt::AlignVCenter, QString("%1s").arg(qRound(duration)));
            painter->restore();
        }
        else {
            painter->save();
            QPen p = painter->pen();
            QFont f = painter->font();
            p.setColor((option.state & QStyle::State_Selected) ? Qt::white : Qt::darkGray);
            f.setPointSize(f.pointSize() - 2);
            painter->setPen(p);
            painter->setFont(f);
            QRect r = option.rect;
            r.setTop(option.rect.center().y());
            r.adjust(4, 2, -4, -2);

            qreal duration = index.data(Qt::UserRole + 1).toDateTime().msecsTo(index.data(Qt::UserRole + 2).toDateTime()) / 1000.0;
            painter->drawText(r, Qt::AlignLeft | Qt::AlignVCenter, QString("Completed in %1s").arg(duration));
            painter->restore();
        }
    }
};

#if 0
class TaskProgressModel : public QAbstractItemModel {
public:
    enum {
        ProgressRole = Qt::UserRole,
        StartTimeRole,
        EndTimeRole,
        LogRole,
        IndentedLogRole
    };
    enum {
        InvalidId = 0xDEADBEEF
    };
    TaskProgressModel(QObject* parent = 0)
        : QAbstractItemModel(parent)
    {
        m_agent = TaskProgressAgent::globalInstance();
        connect(m_agent, &TaskProgressAgent::tasksChanged, this, &TaskProgressModel::update);
        update();
    }

    QModelIndex index(int row, int column,
        const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid() && parent.internalId() != InvalidId)
            return QModelIndex();
        if (parent.isValid()) {
            return createIndex(row, column, parent.row());
        }
        return createIndex(row, column, InvalidId);
    }

    QModelIndex parent(const QModelIndex& child) const override
    {
        if (child.internalId() == InvalidId)
            return QModelIndex();
        return createIndex(child.internalId(), 0, InvalidId);
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid() && parent.internalId() != InvalidId)
            return 0;
        if (parent.isValid()) {
            if (parent.row() < 0)
                return 0;
            return m_data.at(parent.row()).log_messages.size();
        }
        return m_data.size();
    }

    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 2; // 2
        return 2;
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid())
            return QVariant();
        if (index.parent().isValid()) {
            return logData(index, role);
        }
        return taskData(index, role);
    }

    QVariant logData(const QModelIndex& index, int role = Qt::DisplayRole) const
    {
        const TaskInfo& task = m_data.at(index.internalId());
        const auto& logMessages = task.log_messages;
        auto logMessage = logMessages.at(logMessages.count() - 1 - index.row()); // newest first
        switch (role) {
        case Qt::EditRole:
        case Qt::DisplayRole:
            if (index.column() == 0)
                return logMessage.time;
            return logMessage.message;
        case Qt::UserRole:
            return logMessage.time;
        case LogRole:
            return singleLog(logMessage);
        case IndentedLogRole:
            return singleLog(logMessage, "\t");
        default:
            return QVariant();
        }
    }
    QVariant taskData(const QModelIndex& index, int role = Qt::DisplayRole) const
    {
        if (index.column() != 0)
            return QVariant();
        const TaskInfo& task = m_data.at(index.row());
        switch (role) {
        case Qt::EditRole:
        case Qt::DisplayRole:
            // modified by jfm -- 5/17/2016
            if (!task.error.isEmpty()) {
                return task.label + ": " + task.error;
            }
            else {
                return task.label;
            }
        case Qt::ToolTipRole:
            // modified by jfm -- 5/17/2016
            if (!task.description.isEmpty())
                return task.description;
            else
                return taskData(index, Qt::DisplayRole);
        case Qt::ForegroundRole: {
            // modified by jfm -- 5/17/2016
            if (!task.error.isEmpty()) {
                return QColor(Qt::red);
            }
            else {
                if (task.progress < 1)
                    return QColor(Qt::blue);
            }
            return QVariant();
        }
        case ProgressRole:
            return task.progress;
        case StartTimeRole:
            return task.start_time;
        case EndTimeRole:
            return task.end_time;
        case LogRole:
            return assembleLog(task);
        case IndentedLogRole:
            return assembleLog(task, "\t");
        }
        return QVariant();
    }

protected:
    void update()
    {
        beginResetModel();
        m_data.clear();
        m_data.append(m_agent->activeTasks());
        m_data.append(m_agent->completedTasks());
        endResetModel();
    }

    QString assembleLog(const TaskInfo& task, const QString& prefix = QString()) const
    {
        QStringList entries;
        foreach (const TaskProgressLogMessage& msg, task.log_messages) {
            entries << singleLog(msg, prefix);
        }
        return entries.join("\n");
    }
    QString singleLog(const TaskProgressLogMessage& msg, const QString& prefix = QString()) const
    {
        //return QString("%1%2: %3").arg(prefix).arg(msg.time.toString(Qt::ISODate)).arg(msg.message);
        return QString("%1%2: %3").arg(prefix).arg(msg.time.toString("yyyy-MM-dd hh:mm:ss.zzz")).arg(msg.message);
    }

private:
    QList<TaskInfo> m_data;
    TaskProgressAgent* m_agent;
};
#endif
class TaskProgressViewPrivate {
public:
    TaskProgressView* q;

    QString shortened(QString txt, int maxlen);
};

TaskProgressView::TaskProgressView()
{
    d = new TaskProgressViewPrivate;
    d->q = this;
    setSelectionMode(ContiguousSelection);
    setItemDelegate(new TaskProgressViewDelegate(this));
    TaskManager::TaskProgressModel* model = new TaskManager::TaskProgressModel(this);
    setModel(model);
    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header()->hide();
    setExpandsOnDoubleClick(false);
    connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showLogMessages(QModelIndex)));
    QShortcut* copyToClipboard = new QShortcut(QKeySequence(QKeySequence::Copy), this);
    connect(copyToClipboard, SIGNAL(activated()), this, SLOT(copySelectedToClipboard()));
    connect(model, &QAbstractItemModel::modelReset, [this, model]() {
        for(int i = 0; i < model->rowCount(); ++i)
            this->setFirstColumnSpanned(i, QModelIndex(), true);
    });

    connect(model, &QAbstractItemModel::rowsInserted, [this, model](const QModelIndex& parent, int from, int to) {
        if (parent.isValid()) return;
        for(int i = from; i <=to; ++i)
            this->setFirstColumnSpanned(i, QModelIndex(), true);
    });
    for (int i = 0; i < model->rowCount(); ++i)
        setFirstColumnSpanned(i, QModelIndex(), true);
    connect(model, &QAbstractItemModel::dataChanged, [this, model](const QModelIndex& from, const QModelIndex& to) {
        if (from.parent().isValid()) return;
        for (int i = from.row(); i<=to.row(); ++i) {
            setFirstColumnSpanned(i, QModelIndex(), true);
        }
    });
    QTimer* timer = new QTimer(this);
    timer->start(1000);
    connect(timer, SIGNAL(timeout()), viewport(), SLOT(update()));
}

TaskProgressView::~TaskProgressView()
{
    delete d;
}

void TaskProgressView::copySelectedToClipboard()
{
    QItemSelectionModel* selectionModel = this->selectionModel();
    const auto selRows = selectionModel->selectedRows();
    if (selRows.isEmpty())
        return;
    // if first selected entry is a task, we ignore all non-tasks
    bool selectingTasks = !selRows.first().parent().isValid();
    QStringList result;
    QModelIndex lastTask;
    foreach (QModelIndex row, selRows) {
        if (selectingTasks == row.parent().isValid())
            continue;
        if (selectingTasks) {
            // for each task get the name of the task
            result << row.data().toString();
            // for each task get its log messages
            result << row.data(TaskManager::TaskProgressModel::IndentedLogRole).toString();
        }
        else {
            // for each log see if it belongs to the previos task
            // if not, add the task name to the log
            if (row.parent() != lastTask) {
                result << row.parent().data().toString();
                lastTask = row.parent();
            }
            result << row.data(TaskManager::TaskProgressModel::IndentedLogRole).toString();
        }
    }
    QApplication::clipboard()->setText(result.join("\n"));
}

void TaskProgressView::showLogMessages(const QModelIndex& index)
{
    if (index.parent().isValid())
        return;
    QWidget* dlg = new QWidget(this);
    // forcing the widget to be a window despite having a parent so that the window
    // is destroyed with the main window
    dlg->setWindowFlags(Qt::Window);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(tr("Log messages for %1").arg(index.data().toString()));
    QPlainTextEdit* te = new QPlainTextEdit;
    QFont f;
    f.setPointSize(f.pointSize() - 2);
    te->setFont(f);
    te->setReadOnly(true);
    QDialogButtonBox* bb = new QDialogButtonBox;
    QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(close()));
    QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(close()));
    bb->setStandardButtons(QDialogButtonBox::Close);
    QVBoxLayout* l = new QVBoxLayout(dlg);
    l->addWidget(te);
    l->addWidget(bb);
    te->setPlainText(index.data(TaskManager::TaskProgressModel::LogRole).toString());
    QRect r = QApplication::desktop()->screenGeometry(dlg);
    r.setWidth(r.width() / 2);
    r.setHeight(r.height() / 2);
    dlg->resize(r.size());
    dlg->show();
}

QString TaskProgressViewPrivate::shortened(QString txt, int maxlen)
{
    if (txt.count() > maxlen) {
        return txt.mid(0, maxlen - 3) + "...";
    }
    else
        return txt;
}
