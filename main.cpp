#include "ArraySet.h"
#include "BTree.h"
#include "SortedArraySet.h"
#include "MultilevelHat.h"
#include "MultilevelHatWithCachedSmallest.h"
#include "HatSet.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <set>
#include <string>
template<typename Set>
void smokeTest(const Set &prototype)
{
    {//empty container doesn't call enumerator and contains no elements
        int count=0;
        auto empty=prototype;
        empty.enumerate([&](int){++count;});
        if(count!=0)
            throw std::logic_error("an empty container called processor during enumeration");
        if(empty.contains(1))
            throw std::logic_error("an empty container claims to contain '1'");
    }
    {//inserting and erasing
        auto set=prototype;
        set.insert(1);
        if(!set.contains(1))
            throw std::logic_error("'1' is not in the container after inserting it there");
        if(set.contains(2))
            throw std::logic_error("'2' is in the container when it was not inserted");
        set.erase(1);
        if(set.contains(1))
            throw std::logic_error("'1' is in the container after erasing it");
    }
    {//many elements
        auto set=prototype;
        for(int c=0;c<1000;++c)
        {
            set.insert(c);
            if(!set.contains(c))
                throw std::logic_error("an inserted value is absent in the container");
        }
        for(int c=0;c<1000;++c)
            set.erase(c);
    }
}
template<typename Set>
bool contains(const Set &set,int value)
{
    return set.contains(value);
}
bool contains(const std::set<int> &set,int value)
{
    return set.count(value)!=0;
}
template<typename Set>
void performanceTest(const Set &prototype,const std::string &title)
{
    std::cout<<"----"<<std::endl;
    std::cout<<title<<std::endl;
    std::cout<<"\t"<<"inserting(us)"<<"\t"<<"searching(us)"<<"\t"<<"erasing(us)"<<std::endl;
    for(int count=10;count<=10000000;count*=2)
    {
        std::default_random_engine engine;
        std::uniform_int_distribution<int> random;
        std::cout<<count;
        std::vector<int> values;
        for(int c=0;c<count;++c)
            values.push_back(random(engine));
        auto set=prototype;
        {//inserting
            const auto start=std::chrono::steady_clock::now();
            for(auto value:values)
                set.insert(value/2*2);
            const auto finish=std::chrono::steady_clock::now();
            const double seconds=
                std::chrono::duration_cast<std::chrono::milliseconds>(finish-start).count()/1000.;
            std::cout<<"\t"<<seconds/count*1000000;
        }
        {//searching
            const auto start=std::chrono::steady_clock::now();
            for(auto value:values)
                contains(set,value);
            const auto finish=std::chrono::steady_clock::now();
            const double seconds=
                std::chrono::duration_cast<std::chrono::milliseconds>(finish-start).count()/1000.;
            std::cout<<"\t"<<seconds/count*1000000;
        }
        {//erasing
            const auto start=std::chrono::steady_clock::now();
            for(auto value:values)
                set.erase(value/2*2);
            const auto finish=std::chrono::steady_clock::now();
            const double seconds=
                std::chrono::duration_cast<std::chrono::milliseconds>(finish-start).count()/1000.;
            std::cout<<"\t"<<seconds/count*1000000;
        }
        std::cout<<std::endl;
    }
}
int main()
{
    try
    {
        smokeTest(ArraySet<int>());
        smokeTest(SortedArraySet<int>());
        smokeTest(HatSet<int>(10,19));
        smokeTest(MultilevelHat<int>(10,19));
        smokeTest(MultilevelHatWithCachedSmallest<int>(10,19));
        smokeTest(BTree<int>(10,19));
        performanceTest(ArraySet<int>(),"array");
        performanceTest(SortedArraySet<int>(),"sorted array");
        performanceTest(HatSet<int>(10000,19999),"HAT");
        performanceTest(MultilevelHat<int>(1000,1999),"multilevel HAT");
        performanceTest(MultilevelHatWithCachedSmallest<int>(1000,1999),"multilevel HAT with cached smallest element");
        performanceTest(BTree<int>(1000,1999),"B-tree");
        performanceTest(std::set<int>(),"std::set");
    }
    catch(const std::logic_error &e)
    {
        std::cerr<<"test failed: "<<e.what()<<std::endl;
        return 1;
    }
    return 0;
}
