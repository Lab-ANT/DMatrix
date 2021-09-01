#ifndef MVKETCH_H
#define MVKETCH_H
#include <vector>
#include <unordered_set>
#include <utility>
#include <cstring>
#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <arpa/inet.h>
#include "datatypes.hpp"
extern "C"
{
#include "hash.h"
#include "util.h"
}



class TCM {

    typedef struct SBUCKET_type {
        val_tp sum;

        //long count;

        //uint32_t key[2];

    } SBucket;

    struct MV_type {

        //Counter to count total degree
        val_tp sum;
        //Counter table
        SBucket **counts;

        //Outer sketch depth, length and width
        int depth;
        int length;
        int width;

        //# key word bits
        int lgn;

        unsigned long *hash, *scale, *hardner;
    };


    public:
    TCM(int depth, int length, int width, int lgn);

    ~TCM();

    void Update(uint32_t * key, val_tp value);

    val_tp Queryedgeweight(uint32_t* key);

    val_tp Querynodeweight(uint32_t* key);

    //void Queryheavyhitteredge(val_tp thresh, myvector& results);

    //void Queryheavyhitternode(val_tp thresh, myvector& resultsnode);

    bool BFS(uint32_t* key, myvector& path);

    val_tp Change(TCM** mv_arr);

    //void Queryheavychangeredge(val_tp thresh, myvector& results);

    //void Queryheavychangernode(val_tp thresh, myvector& results);

    //val_tp Low_estimate(uint32_t* key);

    //val_tp Up_estimate(uint32_t* key);

    //val_tp Low_estimatenode(uint32_t* key);

    val_tp GetCount();

    void Reset();

    //void Print();

    //void FindSuper(mymap& super);

    private:

    void SetBucket(int row, int column, val_tp sum, long count, uint32_t * key);

    TCM::SBucket** GetTable();

    MV_type mv_;
};

#endif
