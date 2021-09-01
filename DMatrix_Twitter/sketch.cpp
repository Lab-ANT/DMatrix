#include "sketch.hpp"

DMatrix::DMatrix(int depth, int length, int width, int lgn) {

    mv_.depth = depth;
    mv_.length = length;
    mv_.width = width;
    mv_.lgn = lgn;
    mv_.sum = 0;
    mv_.counts = new SBucket *[depth*length*width];
    for (int i = 0; i < depth*length*width; i++) {
        mv_.counts[i] = (SBucket*)calloc(1, sizeof(SBucket));
        memset(mv_.counts[i], 0, sizeof(SBucket));
        mv_.counts[i]->key[0] = '\0';
    }

    mv_.hash = new unsigned long[depth];
    mv_.scale = new unsigned long[depth];
    mv_.hardner = new unsigned long[depth];
    char name[] = "DMatrix";
    unsigned long seed = AwareHash((unsigned char*)name, strlen(name), 13091204281, 228204732751, 6620830889);

    unsigned long seed1 = seed;
    unsigned long seed2 = seed + 100;
    unsigned long seed3 = seed + 200;

    /*for (int i = 0; i < depth; i++) {
        mv_.hash[i] = GenHashSeed(seed++);
    }
    for (int i = 0; i < depth; i++) {
        mv_.scale[i] = GenHashSeed(seed++);
    }
    for (int i = 0; i < depth; i++) {
        mv_.hardner[i] = GenHashSeed(seed++);
    }*/

    for (int i = 0; i < depth; i++) {
        mv_.hash[i] = GenHashSeed(seed1++);
    }
    for (int i = 0; i < depth; i++) {
        mv_.scale[i] = GenHashSeed(seed2++);
    }
    for (int i = 0; i < depth; i++) {
        mv_.hardner[i] = GenHashSeed(seed3++);
    }
}

DMatrix::~DMatrix() {
    for (int i = 0; i < mv_.depth*mv_.width*mv_.length; i++) {
        free(mv_.counts[i]);
    }
    delete [] mv_.hash;
    delete [] mv_.scale;
    delete [] mv_.hardner;
    delete [] mv_.counts;
}


void DMatrix::Update(uint32_t * key, val_tp val) {
    mv_.sum += val;
    //unsigned long bucket = 0;
    unsigned long row = 0;
    unsigned long col = 0;
    int keylen = mv_.lgn;
    for (int i = 0; i < mv_.depth; i++) {
        row = MurmurHash64A(key, keylen/2, mv_.hardner[i]) % mv_.width;
        col = MurmurHash64A(key+1, keylen/2, mv_.hardner[i]) % mv_.length;
        int index = i*mv_.width*mv_.length + row*mv_.length + col;
        DMatrix::SBucket *sbucket = mv_.counts[index];
        sbucket->sum += val;
        if (sbucket->key[0] == '\0') {
            memcpy(sbucket->key, key, keylen);
            sbucket->count = val;
        } else if(memcmp(key, sbucket->key, keylen) == 0) {
            sbucket->count += val;
        } else {
            sbucket->count -= val;
            if (mv_likely(sbucket->count < 0)) {
                memcpy(sbucket->key, key, keylen);
                sbucket->count = -sbucket->count;
            }
        }
    }
}

// Query on edge weight
val_tp DMatrix::Queryedgeweight(uint32_t* key) {
    return Up_estimate(key);
}

// Query on node weight (src)
val_tp DMatrix::Querynodeweight(uint32_t* key) {
    val_tp ret = 0;
    for (int j = 0; j < mv_.depth; j++) {
        unsigned long row = MurmurHash64A(key, mv_.lgn/2, mv_.hardner[j]) % mv_.width;
        val_tp value = 0;
        for (int i = 0; i < mv_.length; i++){
            int index = j*mv_.width*mv_.length + row*mv_.length + i;
            if (memcmp(key, mv_.counts[index]->key, mv_.lgn/2) == 0) {
                value += (mv_.counts[index]->sum + mv_.counts[index]->count)/2;
        } else value += (mv_.counts[index]->sum - mv_.counts[index]->count)/2;
        }
        if (j == 0) ret = value;
        else ret = std::min(ret, value);
    }
    return ret;
}

val_tp DMatrix::Querynodeweight2(uint32_t* key) {
    val_tp ret = 0;
    for (int j = 0; j < mv_.depth; j++) {
        unsigned long row = MurmurHash64A(key, mv_.lgn/2, mv_.hardner[j]) % mv_.width;
        val_tp value = 0;
        for (int i = 0; i < mv_.length; i++){
            int index = j*mv_.width*mv_.length + row*mv_.length + i;
            value += mv_.counts[index]->sum;
        }
        if (j == 0) ret = value;
        else ret = std::min(ret, value);
    }
    return ret;
}

void DMatrix::Queryheavyhitteredge(val_tp thresh, std::vector<std::pair<key_tp, val_tp> >&results) {

    myset res;
    for (int i = 0; i < mv_.length*mv_.width*mv_.depth; i++) {
        if (mv_.counts[i]->sum > thresh) {
            key_tp reskey;
            memcpy(reskey.key, mv_.counts[i]->key, mv_.lgn);
            res.insert(reskey);
        }
    }
    val_tp maxval = 0, sumval = 0;
    for (auto it = res.begin(); it != res.end(); it++) {
        val_tp resval = 0;
        for (int j = 0; j < mv_.depth; j++) {
            unsigned long row = MurmurHash64A((*it).key, mv_.lgn/2, mv_.hardner[j]) % mv_.width;
            unsigned long col = MurmurHash64A((*it).key+1, mv_.lgn/2, mv_.hardner[j]) % mv_.length;
            int index = j*mv_.width*mv_.length + row*mv_.length + col;
            val_tp tempval = 0;
            if (memcmp(mv_.counts[index]->key, (*it).key, mv_.lgn) == 0) {
                tempval = (mv_.counts[index]->sum + mv_.counts[index]->count)/2;
            } else {
                tempval = (mv_.counts[index]->sum - mv_.counts[index]->count)/2;
            }
            if (j == 0) resval = tempval;
            else resval = std::min(tempval, resval);
        }
        if (resval > thresh ) {
            key_tp key;
            memcpy(key.key, (*it).key, mv_.lgn);
            std::pair<key_tp, val_tp> node;
            node.first = key;
            node.second = resval;
            results.push_back(node);
            maxval = std::max(maxval, resval);
            sumval += resval;
        }
    }
    //std::cout << "[Feature b] Number of heavy edge = " << results.size() << std::endl;
    // Extract Features of Max value of heavy hitters on edge weight/frequency
//    for (unsigned int i = 0; i < results.size(); i++){
//        if (results.at(i).second == maxval){
//            struct in_addr src, dst;
//            long addrsrc, addrdst;
//            addrsrc = htonl(results.at(i).first.key[0]);
//            addrdst = htonl(results.at(i).first.key[1]);
//            memcpy(&src, &addrsrc, 4);
//            memcpy(&dst, &addrdst, 4);
//            std::cout << "[Feature c] Maximum estimation value of edge weight = " << maxval
//            << " and the corresponding edge = " << inet_ntoa(src) << " -- " << inet_ntoa(dst) << std::endl;
//        }
//    }
    //std::cout << "[Feature d] Sum of the heavy edges' weight = " << sumval << std::endl;
}

void DMatrix::Queryheavyhitternode(val_tp thresh, std::vector<std::pair<key_tp, val_tp> >&resultsnode) {
    myset res;
    for (int k = 0; k < mv_.depth; k++) {
        for (int i = 0; i < mv_.width; i++){
            val_tp rowsum = 0;
            for (int j = 0; j< mv_.length; j++){
                int index = k*mv_.width*mv_.length + i*mv_.length + j;
                rowsum += mv_.counts[index]->sum;
            }
            if (rowsum > thresh){
                key_tp reskey;
                for (int j = 0; j< mv_.length; j++){
                int index = k*mv_.width*mv_.length + i*mv_.length + j;
                memcpy(reskey.key, mv_.counts[index]->key, mv_.lgn/2);
                res.insert(reskey);
            }
            }
        }
    }


    for (auto it = res.begin(); it != res.end(); it++) {
        val_tp resval = Querynodeweight((uint32_t*)(*it).key);
        if (resval > thresh ) {
            key_tp key;
            memcpy(key.key, (*it).key, mv_.lgn/2);
            std::pair<key_tp, val_tp> node;
            node.first = key;
            node.second = resval;
            resultsnode.push_back(node);
        }
    }

    //std::cout << "[Feature e.1] Number of heavy nodes (src) = " << srcnode.size() << std::endl;
//    for (auto it = resultsnode.begin(); it != resultsnode.end(); it++) {
//        if (it->second == maxval){
//            struct in_addr src;
//            long addrsrc;
//            addrsrc = htonl(it->first.key[0]);
//            memcpy(&src, &addrsrc, 4);
//            std::cout << "[Feature f.1] Maximum value of heavy nodes' weight (src) = " << srcmaxval
//            << " and the corresponding source = " << inet_ntoa(src) << std::endl;
//        }

    //std::cout << "[Feature g.1] Sum of the heavy nodes' weight (src) = " << srcsumval << std::endl;

//    for (auto it = dstnode.begin(); it != dstnode.end(); it++) {
//        if (it->second > thresh ) {
//            key_tp key;
//            memcpy(key.key, it->first.key, mv_.lgn/2);
//            std::pair<key_tp, val_tp> node;
//            node.first = key;
//            node.second = it->second;
//            resultsnode.push_back(node);
//            dstmaxval = std::max(srcmaxval, it->second);
//            dstsumval += it->second;
//        }
//    }
//    //std::cout << "[Feature e.2] Number of heavy nodes (dst) = " << dstnode.size() << std::endl;
//    for (auto it = dstnode.begin(); it != dstnode.end(); it++) {
//        if (it->second == dstmaxval){
//            struct in_addr dst;
//            long addrdst;
//            addrdst = htonl(it->first.key[0]);
//            memcpy(&dst, &addrdst, 4);
////            std::cout << "[Feature f.2] Maximum value of heavy nodes' weight (dst) = " << dstmaxval
////            << " and the corresponding destination = " << inet_ntoa(dst) << std::endl;
//        }
//    }
//    //std::cout << "[Feature g.2] Sum of the heavy nodes' weight (dst) = " << dstsumval << std::endl;
//    return countsrc;
}

bool DMatrix::BFS(uint32_t* key, std::vector<std::pair<key_tp, val_tp> >& path){
    myset res, dstres;
    key_tp reskey;
    memcpy(reskey.key, key, mv_.lgn/2);
    res.insert(reskey);
    while (res.size()){
    dstres.clear();
    for (auto re = res.begin(); re != res.end(); re++){
        if (memcmp(key+1, re->key, mv_.lgn/2)==0){return true;}
    for (auto it = path.begin(); it != path.end(); it++) {
        if (memcmp(re->key, it->first.key, mv_.lgn/2)==0){
            memcpy(reskey.key, it->first.key+1, mv_.lgn/2);
            dstres.insert(reskey);
        }
    }
    }
    res = dstres;
    }
    return false;
}

val_tp DMatrix::Change(DMatrix** mv_arr){
    DMatrix* oldsk = mv_arr[0];
    DMatrix* cursk = mv_arr[1];
    int depth = cursk->mv_.depth, length = cursk->mv_.length, width = cursk->mv_.width;
    DMatrix::SBucket** oldtable = (oldsk)->GetTable();
    DMatrix::SBucket** curtable = (cursk)->GetTable();
    val_tp diff[depth], maxdiff=0, difftemp=0;
    for (int j = 0; j < depth; j++) {
        diff[j] = 0;
        for (int i = j*length*width; i < length*width*(j+1); i++) {
            difftemp = curtable[i]->sum > oldtable[i]->sum ? curtable[i]->sum - oldtable[i]->sum : oldtable[i]->sum - curtable[i]->sum;
            diff[j] += difftemp;
    }
    maxdiff = std::max(maxdiff, diff[j]);
    }
    return maxdiff;
}

void DMatrix::Queryheavychangeredge(val_tp thresh, std::vector<std::pair<key_tp, val_tp> >&results) {

    myset res;
    for (int i = 0; i < mv_.length*mv_.width*mv_.depth; i++) {
        if (mv_.counts[i]->sum > thresh) {
            key_tp reskey;
            memcpy(reskey.key, mv_.counts[i]->key, mv_.lgn);
            res.insert(reskey);
        }
    }

    for (auto it = res.begin(); it != res.end(); it++) {
        val_tp resval = 0;
        for (int j = 0; j < mv_.depth; j++) {
            unsigned long row = MurmurHash64A((*it).key, mv_.lgn/2, mv_.hardner[j]) % mv_.width;
            unsigned long col = MurmurHash64A((*it).key+1, mv_.lgn/2, mv_.hardner[j]) % mv_.length;
            int index = j*mv_.width*mv_.length + row*mv_.length + col;
            val_tp tempval = 0;
            if (memcmp(mv_.counts[index]->key, (*it).key, mv_.lgn) == 0) {
                tempval = (mv_.counts[index]->sum + mv_.counts[index]->count)/2;
            } else {
                tempval = (mv_.counts[index]->sum - mv_.counts[index]->count)/2;
            }
            if (j == 0) resval = tempval;
            else resval = std::min(tempval, resval);
        }
        if (resval > thresh ) {
            key_tp key;
            memcpy(key.key, (*it).key, mv_.lgn);
            std::pair<key_tp, val_tp> node;
            node.first = key;
            node.second = resval;
            results.push_back(node);
        }
    }
}


void DMatrix::Queryheavychangernode(val_tp thresh, std::vector<std::pair<key_tp, val_tp> >&results) {

    myset keyset;
    for (int i = 0; i < mv_.length*mv_.width*mv_.depth; i++) {
        key_tp reskey;
        memcpy(reskey.key, mv_.counts[i]->key, mv_.lgn/2);
        keyset.insert(reskey);
    }
    for (auto it = keyset.begin(); it != keyset.end(); it++) {
        val_tp resval = Querynodeweight((uint32_t*)(*it).key);
        if (resval > thresh){
            key_tp key;
            memcpy(key.key, (*it).key, mv_.lgn/2);
            std::pair<key_tp, val_tp> node;
            node.first = key;
            node.second = resval;
            results.push_back(node);
        }
    }
}

val_tp DMatrix::Low_estimate(uint32_t* key) {
    val_tp ret = 0;
    for (int j = 0; j < mv_.depth; j++) {
        unsigned long row = MurmurHash64A(key, mv_.lgn/2, mv_.hardner[j]) % mv_.width;
        unsigned long col = MurmurHash64A(key+1, mv_.lgn/2, mv_.hardner[j]) % mv_.length;
        int index = j*mv_.width*mv_.length + row*mv_.length + col;
        val_tp lowest = 0;
        if (memcmp(key, mv_.counts[index]->key, mv_.lgn) == 0) {
            lowest = mv_.counts[index]->count;
        } else lowest = 0;
        ret = std::max(ret, lowest);
    }
    return ret;
}

val_tp DMatrix::Up_estimate(uint32_t* key) {
    val_tp ret = 0;
    for (int j = 0; j < mv_.depth; j++) {
        unsigned long row = MurmurHash64A(key, mv_.lgn/2, mv_.hardner[j]) % mv_.width;
        unsigned long col = MurmurHash64A(key+1, mv_.lgn/2, mv_.hardner[j]) % mv_.length;
        int index = j*mv_.width*mv_.length + row*mv_.length + col;
        val_tp upest = 0;

        if (memcmp(key, mv_.counts[index]->key, mv_.lgn) == 0) {
            upest = (mv_.counts[index]->sum + mv_.counts[index]->count)/2;
        } else upest = (mv_.counts[index]->sum - mv_.counts[index]->count)/2;
        if (j == 0) ret = upest;
        else ret = std::min(ret, upest);
    }
    return ret;
}

val_tp DMatrix::Up_estimate2(uint32_t* key) {
    val_tp ret = 0;
    for (int j = 0; j < mv_.depth; j++) {
        unsigned long row = MurmurHash64A(key, mv_.lgn/2, mv_.hardner[j]) % mv_.width;
        unsigned long col = MurmurHash64A(key+1, mv_.lgn/2, mv_.hardner[j]) % mv_.length;
        int index = j*mv_.width*mv_.length + row*mv_.length + col;
        val_tp upest = 0;
	upest = mv_.counts[index]->sum;
        if (j == 0) ret = upest;
        else ret = std::min(ret, upest);
    }
    return ret;
}

val_tp DMatrix::Low_estimatenode(uint32_t* key) {
    val_tp ret = 0;
    for (int j = 0; j < mv_.depth; j++) {
        unsigned long row = MurmurHash64A(key, mv_.lgn/2, mv_.hardner[j]) % mv_.width;
        val_tp value = 0;
        for (int i = 0; i < mv_.length; i++){
            int index = j*mv_.width*mv_.length + row*mv_.length + i;
            if (memcmp(key, mv_.counts[index]->key, mv_.lgn/2) == 0) {
                value += mv_.counts[index]->count;
        } else value += 0;
        }
        if (j == 0) ret = value;
        else ret = std::max(ret, value);
    }
    return ret;
}

val_tp DMatrix::GetCount() {
    return mv_.sum;
}

void DMatrix::Reset() {
    mv_.sum=0;
    for (int i = 0; i < mv_.depth*mv_.length*mv_.width; i++){
        mv_.counts[i]->sum = 0;
        mv_.counts[i]->count = 0;
        memset(mv_.counts[i]->key, 0, mv_.lgn);
    }
}

DMatrix::SBucket** DMatrix::GetTable() {
    return mv_.counts;
}

void DMatrix::Print() {
    struct in_addr src, dst;
    long addrsrc, addrdst;
    for (int i = 0; i < mv_.length*mv_.width*mv_.depth; i++) {
        addrsrc = htonl(mv_.counts[i]->key[0]);
        addrdst = htonl(mv_.counts[i]->key[1]);
        memcpy(&src, &addrsrc, 4);
        memcpy(&dst, &addrdst, 4);
        std::cout <<"[Bucket] Src = " << std::setw(20) << std::left << inet_ntoa(src) << "Dst = " << std::setw(20) << std::left <<
        inet_ntoa(dst) << "Sum = " << std::setw(15) << std::left << mv_.counts[i]->sum << "Count = " << mv_.counts[i]->count << std::endl;
    }
}

void DMatrix::FindSuper(mymap& super){
    unsigned int depth = mv_.depth, length = mv_.length, width = mv_.width;
    mymap srcmap[depth], dstmap[depth];

    for (unsigned int j = 0; j < depth; j++) {
        for (unsigned int i = j*length*width; i < length*width*(j+1); i++) {
        key_tp srckey, dstkey;
        memcpy(srckey.key, mv_.counts[i]->key, mv_.lgn/2);
        memcpy(dstkey.key, mv_.counts[i]->key+1, mv_.lgn/2);
        if (srcmap[j].find(srckey) != srcmap[j].end()) {
            srcmap[j][srckey] += 1;
        } else {
            srcmap[j][srckey] = 1;
            }

        if (dstmap[j].find(dstkey) != dstmap[j].end()) {
            dstmap[j][dstkey] += 1;
        } else {
            dstmap[j][dstkey] = 1;
            }
        }


        for (auto it = srcmap[j].begin(); it!= srcmap[j].end(); it++){
            if (it->second > length/2){
            super.insert(std::pair<key_tp, val_tp>(it->first, 0));
            }
        }
        for (auto it = dstmap[j].begin(); it!= dstmap[j].end(); it++){
            if (it->second > width/2){
            super.insert(std::pair<key_tp, val_tp>(it->first, 1));
            }
        }
    }

}

