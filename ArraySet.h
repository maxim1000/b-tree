#pragma once
#include <functional>
#include <vector>
template<typename Value>
class ArraySet
{
public:
    class Iterator;
    void insert(const Value&);
    void erase(const Value&);
    bool contains(const Value&) const;
    void enumerate(const std::function<void(const Value&)>&) const;
private:
    std::vector<Value> values_;
};
template<typename Value>
void ArraySet<Value>::insert(const Value &value)
{
    values_.push_back(value);
}
template<typename Value>
void ArraySet<Value>::erase(const Value &value)
{
    for(auto position=values_.begin();position!=values_.end();++position)
    {
        if(*position==value)
        {
            std::swap(*position,values_.back());
            values_.pop_back();
            break;
        }
    }
}
template<typename Value>
bool ArraySet<Value>::contains(const Value &value) const
{
    for(const auto &existingValue:values_)
        if(existingValue==value)
            return true;
    return false;
}
template<typename Value>
void ArraySet<Value>::enumerate(const std::function<void(const Value&)> &processor) const
{
    for(const auto &value:values_)
        processor(value);
}
