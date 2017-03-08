/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/8/2017
*******************************************************/
#ifndef MLCONFIGQUESTION_H
#define MLCONFIGQUESTION_H

class MLConfigQuestionPrivate;
class MLConfigQuestion {
public:
    friend class MLConfigQuestionPrivate;
    MLConfigQuestion();
    virtual ~MLConfigQuestion();

private:
    MLConfigQuestionPrivate* d;
};

#endif // MLCONFIGQUESTION_H
