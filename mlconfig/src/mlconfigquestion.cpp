/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/8/2017
*******************************************************/

class MLConfigQuestionPrivate {
public:
    MLConfigQuestion* q;
};

MLConfigQuestion::MLConfigQuestion()
{
    d = new MLConfigQuestionPrivate;
    d->q = this;
}

MLConfigQuestion::~MLConfigQuestion()
{
    delete d;
}
