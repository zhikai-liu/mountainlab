#include "mvthreadmanager.h"
#include <QTimer>
#include <qglobalstatic.h>
#include <QDebug>

class MVThreadManagerPrivate {
public:
    MVThreadManager* q;

    QList<QThread*> m_threads;
    int m_max_simultaneous_threads = 1;
};

MVThreadManager::MVThreadManager()
{
    d = new MVThreadManagerPrivate;
    d->q = this;
    QTimer::singleShot(2000, this, SLOT(slot_timer()));
    d->m_max_simultaneous_threads = QThread::idealThreadCount();
}

MVThreadManager::~MVThreadManager()
{
    delete d;
}

void MVThreadManager::addThread(QThread* thread)
{
    d->m_threads.insert(0, thread);
    QObject::connect(thread, SIGNAL(finished()), this, SLOT(slot_thread_finished()), Qt::QueuedConnection);
    QTimer::singleShot(0, this, SLOT(slot_handle_threads()));
}

Q_GLOBAL_STATIC(MVThreadManager, theInstance)
MVThreadManager* MVThreadManager::globalInstance()
{
    return theInstance;
}

void MVThreadManager::slot_timer()
{
    slot_handle_threads();
    QTimer::singleShot(2000, this, SLOT(slot_timer()));
}

void MVThreadManager::slot_handle_threads()
{
    int num_running = 0;
    for (int i = 0; i < d->m_threads.count(); i++) {
        if (d->m_threads[i]->isRunning()) {
            num_running++;
            if (num_running >= d->m_max_simultaneous_threads) {
                break;
            }
        }
    }
    for (int i = 0; i < d->m_threads.count(); i++) {
        if (num_running < d->m_max_simultaneous_threads) {
            if (!d->m_threads[i]->isRunning()) {
                d->m_threads[i]->start();
                num_running++;
            }
        }
    }
}

void MVThreadManager::slot_thread_finished()
{
    QThread* thr = qobject_cast<QThread*>(sender());
    if (thr == 0) {
        qWarning() << "Thread object is null in slot_thread_finished()";
    }
    d->m_threads.removeAll(thr);
    QTimer::singleShot(0, this, SLOT(slot_handle_threads()));
}
