/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/8/2017
*******************************************************/
#ifndef MLCONFIGPAGE_H
#define MLCONFIGPAGE_H

#include <QString>

class MLConfigPage {
public:
    MLConfigPage();
    virtual ~MLConfigPage();

    virtual QString title() = 0;
    virtual QString description() = 0;

    int questionCount() const;
    MLConfigQuestion* questions(int num) const;

protected:
    void addQuestion(MLConfigQuestion* question);

private:
    QList<MLConfigQuestion*> m_questions;
};

#endif // MLCONFIGPAGE_H
