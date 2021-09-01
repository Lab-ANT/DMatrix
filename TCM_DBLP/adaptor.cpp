#include "adaptor.hpp"
#include <unordered_set>
#include <fstream>
#include <iostream>

Adaptor::Adaptor(std::string filename, uint64_t buffersize) {
    data = (adaptor_t*)calloc(1, sizeof(adaptor_t));
    data->databuffer = (unsigned char*)calloc(buffersize, sizeof(unsigned char));
    data->ptr = data->databuffer;
    data->cnt = 0;
    data->cur = 0;
    //Read twitter file
    std::ifstream infile;
    infile.open(filename);
    std::string srcdst;
    unsigned char* p = data->databuffer;
    int frequency = 1;
    while (getline(infile, srcdst))
    {
        int pos1 = srcdst.find(" ");
        int pos2 = srcdst.find("\r\n");
        std::string ip1 = srcdst.substr(0,pos1);
        std::string ip2 = srcdst.substr(pos1+1,pos2-pos1);
        int srcip = atoi(ip1.c_str());
        int dstip = atoi(ip2.c_str());
        memcpy(p, &srcip, sizeof(uint32_t));
        memcpy(p+sizeof(uint32_t), &dstip, sizeof(uint32_t));
        memcpy(p+sizeof(uint32_t)*2, &frequency, sizeof(uint16_t));
        p += sizeof(uint32_t)*2+sizeof(uint16_t);
        data->cnt++;
    }
    infile.close();
}

Adaptor::~Adaptor() {
    free(data->databuffer);
    free(data);
}

int Adaptor::GetNext(tuple_t* t) {
    if (data->cur > data->cnt) {
        return -1;
    }
    t->key.src_ip = *((uint32_t*)(data->ptr));
    t->key.dst_ip = *((uint32_t*)(data->ptr+4));
    t->size = *((uint16_t*)(data->ptr+8));
    data->cur++;
    data->ptr += 10;
    return 1;
}

void Adaptor::Reset() {
    data->cur = 0;
    data->ptr = data->databuffer;
}

uint64_t Adaptor::GetDataSize() {
    return data->cnt;
}

//int main(int argc, char* argv[]) {
//    const char* filenames = "twitterfiles";
//    unsigned long long buf_size = 5000000000;
//
//   std::string file;
//
//    std::ifstream tracefiles(filenames);
//    if (!tracefiles.is_open()) {
//        std::cout << "Error opening file" << std::endl;
//        return -1;
//    }
//
//    for (std::string file; getline(tracefiles, file);) {
//        //Load traces and get ground
//
//        Adaptor* adaptor =  new Adaptor(file, buf_size);
//        std::cout << "[Dataset]: " << file << std::endl;
//
//        //Get ground
//        adaptor->Reset();
//        tuple_t t;
//        memset(&t, 0, sizeof(tuple_t));
//        while(adaptor->GetNext(&t) == 1) {
//        }
//        }
//}
