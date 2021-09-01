#ifndef ADAPTOR_H
#define ADAPTOR_H
#include <iostream>
#include <string.h>
#include "datatypes.hpp"

class Adaptor {


    typedef struct {
        unsigned char* databuffer;
        uint64_t cnt=0;
        uint64_t cur=0;
        unsigned char* ptr;
    } adaptor_t;

    public:
        Adaptor(std::string filename, uint64_t buffersize);

        ~Adaptor();

        int GetNext(tuple_t* t);

        void Reset();

        uint64_t GetDataSize();

        adaptor_t* data;

};
#endif
