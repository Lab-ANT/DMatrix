#ifndef CHANGER_H
#define CHANGER_H

#include "sketch.hpp"
#include "datatypes.hpp"

typedef std::unordered_map<key_tp, val_tp*, key_tp_hash, key_tp_eq> groundmap;

template <class S>
class HeavyChanger {

public:
    HeavyChanger(int depth, int length, int width, int lgn);

    ~HeavyChanger();

    void Update(uint32_t * key, val_tp val);

    val_tp ChangeinSketch();

//    void Querychangeredge(val_tp thresh, myvector& result);
//
//    void Querychangernode(val_tp thresh, myvector& result);
//
//    void Changedhitteredge(myvector& oldresult, myvector& curresult); // The number of changed heavy hitters on edge between two adjacent epoch
//
//    void Changedhitternode(myvector& oldresult, myvector& curresult, unsigned int oldnum, unsigned int curnum);

    void Reset();


    S* GetCurSketch();

    S* GetOldSketch();

private:
    S* old_sk;

    S* cur_sk;

    int lgn_;
};

template <class S>
HeavyChanger<S>::HeavyChanger(int depth, int length, int width, int lgn) {
    old_sk = new S(depth, length, width, lgn);
    cur_sk = new S(depth, length, width, lgn);
    lgn_ = lgn;
}


template <class S>
HeavyChanger<S>::~HeavyChanger() {
    delete old_sk;
    delete cur_sk;
}

template <class S>
void HeavyChanger<S>::Update(uint32_t * key, val_tp val) {
    cur_sk->Update(key, val);
}

template <class S>
val_tp HeavyChanger<S>::ChangeinSketch() {
    val_tp diff;
    S* S2[2];
    memcpy(&S2[0], &old_sk, sizeof(old_sk));
    memcpy(&S2[1], &cur_sk, sizeof(cur_sk));
    diff = cur_sk->Change(S2);
    return diff;
}

//template <class S>
//void HeavyChanger<S>::Querychangeredge(val_tp thresh, myvector& result) {
//    myvector res1, res2;
//    cur_sk->Queryheavychangeredge(thresh, res1);
//    old_sk->Queryheavychangeredge(thresh, res2);
//    myset reset;
//    for (auto it = res1.begin(); it != res1.end(); it++) {
//        reset.insert(it->first);
//    }
//    for (auto it = res2.begin(); it != res2.end(); it++) {
//        reset.insert(it->first);
//    }
//    val_tp old_low, old_up;
//    val_tp new_low, new_up;
//    val_tp diff1, diff2;
//    val_tp change;
////    val_tp maxchange = 0, sumchange = 0;
//    for (auto it = reset.begin(); it != reset.end(); it++) {
//        old_low = old_sk->Low_estimate((uint32_t*)(*it).key);
//        old_up = old_sk->Up_estimate((uint32_t*)(*it).key);
//        new_low = cur_sk->Low_estimate((uint32_t*)(*it).key);
//        new_up = cur_sk->Up_estimate((uint32_t*)(*it).key);
//        diff1 = new_up > old_low ? new_up - old_low : old_low - new_up;
//        diff2 = new_low > old_up ? new_low - old_up : old_up - new_low;
//        change = diff1 > diff2 ? diff1 : diff2;
//        if (change > thresh) {
//            key_tp key;
//            memcpy(key.key, it->key, lgn_);
//            std::pair<key_tp, val_tp> cand;
//            cand.first = key;
//            cand.second = change;
//            result.push_back(cand);
////            maxchange = std::max(maxchange, change);
////            sumchange += change;
//        }
//    }
//    //std::cout << "[Feature i] Number of heavy changers on edge weight = " << result.size() << std::endl;
//
////    for (unsigned int i = 0; i < result.size(); i++){
////        if (result.at(i).second == maxchange){
////            struct in_addr src, dst;
////            long addrsrc, addrdst;
////            addrsrc = htonl(result.at(i).first.key[0]);
////            addrdst = htonl(result.at(i).first.key[1]);
////            memcpy(&src, &addrsrc, 4);
////            memcpy(&dst, &addrdst, 4);
//////            std::cout << "[Feature j] Maximum estimation value of heavy changers on edge weight = " << maxchange
//////            << " and the corresponding edge = " << inet_ntoa(src) << " -- " << inet_ntoa(dst) << std::endl;
////        }
////    }
//    //std::cout << "[Feature k] Sum of the heavy changers' weight (edge) = " << sumchange << std::endl;
//}
//
////Query for heavy changer on node
//template <class S>
//void HeavyChanger<S>::Querychangernode(val_tp thresh, myvector& result) {
//    //std::cout << "[Feature l.1] Number of heavy changers on node (src) = " << result.size() << std::endl;
//    //std::cout << "[Feature m.1] Maximum estimation value of heavy changers on node (src) = " << srcmaxchange
//    //          << " and the corresponding source = " << inet_ntoa(src) << std::endl;
//    //std::cout << "[Feature n.1] Sum of the heavy changers' weight (dst) = " << srcsumchange << std::endl;
//    //std::cout << "[Feature l.2] Number of heavy changers on node (dst) = " << result.size()-srcnodesize << std::endl;
//    //std::cout << "[Feature m.2] Maximum estimation value of heavy changers on node (dst) = " << dstmaxchange
//    //          << " and the corresponding destination = " << inet_ntoa(dst) << std::endl;
//    //std::cout << "[Feature n.2] Sum of the heavy changers' weight (dst) = " << dstsumchange << std::endl;
//    myvector res1, res2;
//    cur_sk->Queryheavychangernode(thresh, res1);
//    old_sk->Queryheavychangernode(thresh, res2);
//    myset reset;
//    for (auto it = res1.begin(); it != res1.end(); it++) {
//        reset.insert(it->first);
//    }
//    for (auto it = res2.begin(); it != res2.end(); it++) {
//        reset.insert(it->first);
//    }
//    val_tp old_low, old_up;
//    val_tp new_low, new_up;
//    val_tp diff1, diff2;
//    val_tp change;
//    for (auto it = reset.begin(); it != reset.end(); it++) {
//        old_low = old_sk->Low_estimatenode((uint32_t*)(*it).key);
//        old_up = old_sk->Querynodeweight((uint32_t*)(*it).key);
//        new_low = cur_sk->Low_estimatenode((uint32_t*)(*it).key);
//        new_up = cur_sk->Querynodeweight((uint32_t*)(*it).key);
//        diff1 = new_up > old_low ? new_up - old_low : old_low - new_up;
//        diff2 = new_low > old_up ? new_low - old_up : old_up - new_low;
//        change = diff1 > diff2 ? diff1 : diff2;
//        if (change > thresh) {
//            key_tp key;
//            memcpy(key.key, it->key, lgn_/2);
//            std::pair<key_tp, val_tp> cand;
//            cand.first = key;
//            cand.second = change;
//            result.push_back(cand);
//        }
//    }
//}
//
//template <class S>
//void HeavyChanger<S>::Changedhitteredge(myvector& oldresult, myvector& curresult) {
//    mymap totalre;
//    for (unsigned int i = 0; i < oldresult.size(); i++){
//        if (totalre.find(oldresult.at(i).first) != totalre.end()) {
//            totalre[oldresult.at(i).first] += 1;
//        } else {
//            totalre[oldresult.at(i).first] = 0;
//            }
//    }
//    for (unsigned int i = 0; i < curresult.size(); i++){
//        if (totalre.find(curresult.at(i).first) != totalre.end()) {
//            totalre[curresult.at(i).first] += 1;
//        } else {
//            totalre[curresult.at(i).first] = 0;
//            }
//    }
//    val_tp changecount = 0;
//    for (auto it = totalre.begin(); it != totalre.end(); it++){
//        if (it->second == 0){
//            changecount++;
//        }
//        }
//    //std::cout << "[Feature o] The number of changed heavy hitters on edge between two adjacent epoch = " << changecount << std::endl;
//}
//
//template <class S>
//void HeavyChanger<S>::Changedhitternode(myvector& oldresult, myvector& curresult, unsigned int oldnum, unsigned int curnum) {
//    mymap totalresrc, totalredst;
//    for (unsigned int i = 0; i < oldnum; i++){
//        if (totalresrc.find(oldresult.at(i).first) != totalresrc.end()) {
//            totalresrc[oldresult.at(i).first] += 1;
//        } else {
//            totalresrc[oldresult.at(i).first] = 0;
//            }
//    }
//    for (unsigned int i = 0; i < curnum; i++){
//        if (totalresrc.find(curresult.at(i).first) != totalresrc.end()) {
//            totalresrc[curresult.at(i).first] += 1;
//        } else {
//            totalresrc[curresult.at(i).first] = 0;
//            }
//    }
//    val_tp changecount = 0;
//    for (auto it = totalresrc.begin(); it != totalresrc.end(); it++){
//        if (it->second == 0){
//            changecount++;
//        }
//        }
//    //std::cout << "[Feature p.1] The number of changed heavy hitters on node (src) between two adjacent epoch = " << changecount << std::endl;
//
//    for (unsigned int i = oldnum; i < oldresult.size(); i++){
//        if (totalredst.find(oldresult.at(i).first) != totalredst.end()) {
//            totalredst[oldresult.at(i).first] += 1;
//        } else {
//            totalredst[oldresult.at(i).first] = 0;
//            }
//    }
//    for (unsigned int i = curnum; i < curresult.size(); i++){
//        if (totalredst.find(curresult.at(i).first) != totalredst.end()) {
//            totalredst[curresult.at(i).first] += 1;
//        } else {
//            totalredst[curresult.at(i).first] = 0;
//            }
//    }
//    val_tp changecountdst = 0;
//    for (auto it = totalredst.begin(); it != totalredst.end(); it++){
//        if (it->second == 0){
//            changecountdst++;
//        }
//        }
//    //std::cout << "[Feature p.2] The number of changed heavy hitters on node (dst) between two adjacent epoch = " << changecount << std::endl;
//}


template <class S>
void HeavyChanger<S>::Reset() {
    old_sk->Reset();
    S* temp = old_sk;
    old_sk = cur_sk;
    cur_sk = temp;
}

template <class S>
S* HeavyChanger<S>::GetCurSketch() {
    return cur_sk;
}

template <class S>
S* HeavyChanger<S>::GetOldSketch() {
    return old_sk;
}

#endif
