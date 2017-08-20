#ifndef MLVECTOR_H
#define MLVECTOR_H

#include <vector>

template <typename T>
class MLVector : public std::vector<T> {
public:
    using std::vector<T>::vector;
    MLVector& operator<<(const T& val)
    {
        this->emplace_back(val);
        return *this;
    }
    qintptr count() const { return (qintptr)std::vector<T>::size(); } // breaks for large values
    T operator[](qintptr i) const { return std::vector<T>::operator[]((std::size_t)i); }
    T& operator[](qintptr i) { return std::vector<T>::operator[]((std::size_t)i); }
    bool isEmpty() const { return std::vector<T>::size() == 0; }
    void append(const MLVector<T>& other) { this->insert(this->end(), other.begin(), other.end()); }
    T value(qintptr i) const
    {
        if (i < 0)
            return 0;
        if (i >= this->count())
            return 0;
        return (*this)[i];
    }
};

#endif // MLVECTOR_H
