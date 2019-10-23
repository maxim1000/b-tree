#pragma once
#include <functional>
#include <vector>
template<typename Value>
class HatSet
{
public:
    class Iterator;
    HatSet(size_t minChunkSize,size_t maxChunkSize);
    void insert(const Value&);
    void erase(const Value&);
    bool contains(const Value&) const;
    void enumerate(const std::function<void(const Value&)>&) const;
private:
    using Chunk=std::vector<Value>;
    size_t minChunkSize_,maxChunkSize_;
    std::vector<Chunk> chunks_;
    size_t findChunkIndex(const Value&) const;
    void splitChunkIfNeeded(size_t chunkIndex);
    static size_t findIndexForValue(const Chunk&,const Value &value);//returns first element>=value
};
///////////////////////////////////////////////////////////////////////////////
template<typename Value>
HatSet<Value>::HatSet(size_t minChunkSize,size_t maxChunkSize)
    :minChunkSize_(minChunkSize)
    ,maxChunkSize_(maxChunkSize)
{}
template<typename Value>
void HatSet<Value>::insert(const Value &value)
{
    if(chunks_.empty())
    {
        chunks_.emplace_back();
        chunks_.back().emplace_back(value);
        return;
    }
    const auto chunkIndex=findChunkIndex(value);
    auto &chunk=chunks_[chunkIndex];
    const auto index=findIndexForValue(chunk,value);
    if(index==chunk.size() || chunk[index]!=value)
    {
        chunk.insert(chunk.begin()+index,value);
        splitChunkIfNeeded(chunkIndex);
    }
}
template<typename Value>
void HatSet<Value>::erase(const Value &value)
{
    if(chunks_.empty())
        return;
    const auto chunkIndex=findChunkIndex(value);
    auto &chunk=chunks_[chunkIndex];
    const auto index=findIndexForValue(chunk,value);
    if(index<chunk.size() && chunk[index]==value)
    {
        chunk.erase(chunk.begin()+index);
        if(chunk.empty())
            chunks_.erase(chunks_.begin()+chunkIndex);
    }
}
template<typename Value>
bool HatSet<Value>::contains(const Value &value) const
{
    if(chunks_.empty())
        return false;
    const auto &chunk=chunks_[findChunkIndex(value)];
    const auto index=findIndexForValue(chunk,value);
    return (index<chunk.size() && chunk[index]==value);
}
template<typename Value>
void HatSet<Value>::enumerate(const std::function<void(const Value&)> &processor) const
{
    for(const auto &chunk:chunks_)
        for(const auto &value:chunk)
            processor(value);
}
template<typename Value>
size_t HatSet<Value>::findChunkIndex(const Value &value) const
{
    size_t index=0;
    while(index+1<chunks_.size())
    {
        if(chunks_[index+1].front()>value)
            break;
        else
            ++index;
    }
    return index;
}
template<typename Value>
void HatSet<Value>::splitChunkIfNeeded(size_t chunkIndex)
{
    if(chunks_[chunkIndex].size()>maxChunkSize_)
    {
        chunks_.emplace(chunks_.begin()+chunkIndex+1);
        auto &source=chunks_[chunkIndex];
        auto &destination=chunks_[chunkIndex+1];
        destination.assign(source.begin()+source.size()/2,source.end());
        source.erase(source.begin()+source.size()/2,source.end());
    }
}
template<typename Value>
size_t HatSet<Value>::findIndexForValue(const Chunk &chunk,const Value &value)
{
    size_t current=chunk.size();
    size_t step=chunk.size();
    while(step>0)
    {
        if(current<step || chunk[current-step]<value)
            step/=2;
        else
            current-=step;
    }
    return current;
}
