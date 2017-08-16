#ifndef MLVECTOR_H
#define MLVECTOR_H

template <typename T>
class MLVector : public std::vector<T> {
public:
    using std::vector<T>::vector;
    //MLVector(const std::vector<T> &v) = delete;
    //MLVector& operator=(const std::vector<T> &) = delete;
    void operator<<(T val) { this->push_back(val); }
    qintptr count() const { return (qintptr)std::vector<T>::size(); } // breaks for large values
    T operator[](qintptr i) const { return std::vector<T>::operator[]((std::size_t)i); }
    T& operator[](qintptr i) { return std::vector<T>::operator[]((std::size_t)i); }
};

#endif // MLVECTOR_H
