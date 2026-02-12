//
//

#ifndef CDOWNLOAD_MANAGER_OBSERVER_H
#define CDOWNLOAD_MANAGER_OBSERVER_H

template <typename T>
class IObserver {
public:
    virtual void on_update(const T& event) = 0;  // pure virtual
    virtual ~IObserver() = default;               // destrutor virtual sempre!
};

template <typename T>
class IProducer {
public:
    virtual void add_observer(IObserver<T>* listener) = 0;
    virtual void remove_observer(IObserver<T>* listener) = 0;
    virtual void emit(const T& event) = 0;        // T, não std::string

    virtual ~IProducer() = default;
};

#endif //CDOWNLOAD_MANAGER_OBSERVER_H
