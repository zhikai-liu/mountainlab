/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/8/2017
*******************************************************/

#include "mlconfigpage.h"

MLConfigPage::MLConfigPage()
{
}

MLConfigPage::~MLConfigPage()
{
    qDeleteAll(m_questions);
    m_questions.clear();
}

int MLConfigPage::questionCount() const
{
    return m_questions.count();
}

MLConfigQuestion* MLConfigPage::questions(int num) const
{
    return m_questions[num];
}

void MLConfigPage::addQuestion(MLConfigQuestion* question)
{
    m_questions << question;
}
