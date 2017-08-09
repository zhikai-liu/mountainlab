#ifndef MVTHREADMANAGER_H
#define MVTHREADMANAGER_H

#include <QThread>

class MVThreadManagerPrivate;
class MVThreadManager : public QObject {
    Q_OBJECT
public:
    enum Duration {
        ShortTerm,
        LongTerm
    };

    friend class MVThreadManagerPrivate;
    MVThreadManager();
    virtual ~MVThreadManager();

    void addThread(QThread* thread);

    static MVThreadManager* globalInstance();

private slots:
    void slot_timer();
    void slot_handle_threads();
    void slot_thread_finished();

private:
    MVThreadManagerPrivate* d;
};

#endif // MVTHREADMANAGER_H
