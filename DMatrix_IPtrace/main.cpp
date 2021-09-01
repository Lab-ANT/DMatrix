#include "main.hpp"
#include "adaptor.hpp"
#include <unordered_map>
#include <utility>
#include "util.h"
#include "datatypes.hpp"
#include <iomanip>
#include "RrConfig.h"

int main(int argc, char* argv[]) {
    rr::RrConfig config;
    if(config.ReadConfig("config_IPtrace.ini") == false) {
	std::cout << "Error loading configuration file." << std::endl;
	return 1;
    }
    int num = 1;
    for(;;) {
	std::string dm_config = "DMatrix_" + std::to_string(num);
	if(config.ReadString(dm_config.c_str(), "name", "") == "") {
	    std::cout << "End at the " << num << "-th configuration " << dm_config << "." << std::endl;
	    return 1;
	}
	std::cout << config.ReadString(dm_config.c_str(), "name", "") << ":" << std::endl;
	num++;

	//Dataset list
        const char* filenames = config.ReadString(dm_config.c_str(), "filenames", "").c_str();
	//const char* filenames = "iptraces.txt";
        unsigned long long buf_size = 5000000000;

        //Heavy-hitter, heavy-changer and reachability threshold
        double alpha = config.ReadFloat(dm_config.c_str(), "alpha", 0);
	double beta = config.ReadFloat(dm_config.c_str(), "beta", 0); 
	double gamma = config.ReadFloat(dm_config.c_str(), "gamma", 0);
	//double alpha = 0.02, beta = 0.03, gamma = 0.01;
	std::cout << "alpha = " << alpha << "; beta = " << beta << "; gamma = " << gamma << ";" << std::endl;

        //DMatrix parameters
	int mv_length = config.ReadInt(dm_config.c_str(), "length", 0);
        int mv_width = config.ReadInt(dm_config.c_str(), "width", 0);
	int mv_depth = config.ReadInt(dm_config.c_str(), "depth", 0);
	std::cout << "depth: " << mv_depth << std::endl;
	std::cout << "length * width: " << mv_length << " * " << mv_width << std::endl;
	//int mv_length = new_w2[n_idx];
        //int mv_width = new_w2[n_idx];
        //int mv_depth = 9;
	//std::cout << "length * width: " << mv_length << " * " << mv_width << std::endl;

	int nodenum = config.ReadInt(dm_config.c_str(), "nodenum", 0);

        int numfile = 0;

        // Average relative error for weight-based query
        double reedge = 0, renode = 0, resubgraph = 0;
        double areedge = 0, arenode = 0, aresubgraph = 0;
        // Precision, recall for rechability query
        double prerechability = 0, avrprerechability = 0, rerechability = 0, avrrerechability = 0;
    
        // Precision, recall, average relative error for heavy-key query
        // Heavy-hitter edge
        double prehitteredge=0, rehitteredge=0, reerhitteredge=0;
        double avrprehitteredge=0, avrrehitteredge=0, avrreerhitteredge=0;
        // Heavy-hitter node
        double prehitternode=0, rehitternode=0, reerhitternode=0;
        double avrprehitternode=0, avrrehitternode=0, avrreerhitternode=0;
        // Heavy-changer edge
        double prechangeredge=0, rechangeredge=0, reerchangeredge=0;
        double avrprechangeredge=0, avrrechangeredge=0, avrreerchangeredge=0;
        // Heavy-changer node
        double prechangernode=0, rechangernode=0, reerchangernode=0;
        double avrprechangernode=0, avrrechangernode=0, avrreerchangernode=0;

        // Time cost
        double updatetime=0, avrupdatetime=0, querytime1=0, avrquerytime1=0, querytime2=0, avrquerytime2=0, querytime3=0, avrquerytime3=0;
        double querytime4=0, avrquerytime4=0, querytime5=0, avrquerytime5=0, querytime6=0, avrquerytime6=0, querytime7=0, avrquerytime7=0;
        double querytime8=0, avrquerytime8=0;


        std::string file;

        std::ifstream tracefiles(filenames);
        if (!tracefiles.is_open()) {
            std::cout << "Error opening file" << std::endl;
            return -1;
        }

        groundmap groundedgetmp, groundnodetmp;
        mymap groundedge, groundnode;
        tuple_t t;
        val_tp diffsum = 0;

        myvector heavyhitteredge, heavyhitternode, oldheavyhitteredge, oldheavyhitternode, paths;

        //DMatrix
        HeavyChanger<DMatrix>* heavychangermv = new HeavyChanger<DMatrix>(mv_depth, mv_length, mv_width, LGN);

	//std::unordered_map<uint32_t,int> MAP2;
        for (std::string file; getline(tracefiles, file);) {
            //Load traces and get ground

            Adaptor* adaptor =  new Adaptor(file, buf_size);
            std::cout << "[Dataset]: " << file << std::endl;

            //Get ground
            adaptor->Reset();

            //Reset gounrdtmp;
            for (auto it = groundedgetmp.begin(); it != groundedgetmp.end(); it++) {
                it->second[1] = it->second[0];
                it->second[0] = 0;
            }

            for (auto it = groundnodetmp.begin(); it != groundnodetmp.end(); it++) {
                it->second[1] = it->second[0];
                it->second[0] = 0;
            }

            //Insert into ground table
            val_tp sum = 0;
            memset(&t, 0, sizeof(tuple_t));
            while(adaptor->GetNext(&t) == 1) {
                sum += t.size;
                key_tp key;
                memcpy(key.key, &(t.key), LGN);
                if (groundedgetmp.find(key) != groundedgetmp.end()) {
                    groundedgetmp[key][0] += t.size;
                } else {
                    val_tp* valtuple = new val_tp[2]();
                    groundedgetmp[key] = valtuple;
                    groundedgetmp[key][0] += t.size;
                }

                key_tp keynode;
                // Src nodes
                memcpy(keynode.key, &(t.key.src_ip), LGN/2);
                if (groundnodetmp.find(keynode) != groundnodetmp.end()) {
                    groundnodetmp[keynode][0] += t.size;
                } else {
		    /*if(MAP2.find(keynode.key[0]) == MAP2.end())
		    {
	    	        MAP2.insert(std::pair<uint32_t,int>(keynode.key[0],1));
		    }
		    else
		    {
		        MAP2[keynode.key[0]]++;
		    }*/
                    val_tp* valtuple = new val_tp[2]();
                    groundnodetmp[keynode] = valtuple;
                    groundnodetmp[keynode][0] += t.size;
                }

    //            // Dst node
    //            memcpy(keynode.key, &(t.key.dst_ip), LGN/2);
    //            if (groundnodetmp.find(keynode) != groundnodetmp.end()) {
    //                groundnodetmp[keynode][0] += t.size;
    //            } else {
    //                val_tp* valtuple = new val_tp[2]();
    //                groundnodetmp[keynode] = valtuple;
    //                groundnodetmp[keynode][0] += t.size;
    //            }
            }
            //std::cout << "[Feature a] Sum of the edge weight = " << sum << std::endl;
	    /*int ccccc = 0;
	    for(auto it = MAP2.begin();it != MAP2.end();it++)
	    {
		if(it->second>=2)
		{
		    ccccc++;
		}
	    }
	    std::cout << "numbers: " << ccccc << std::endl;*/

	    //std::cout << "edge numbers: " << groundedgetmp.size() << std::endl;
            if (numfile != 0) {
                groundedge.clear();
                groundnode.clear();
                val_tp oldval, newval, diff;
                diffsum = 0;
                //Get sum of edge weight change
                for (auto it = groundedgetmp.begin(); it != groundedgetmp.end(); it++) {
                    oldval = it->second[0];
                    newval = it->second[1];
                    diff = newval > oldval ? newval - oldval : oldval - newval;
                    diffsum += diff;
                }

                //Get the ground truth of heavy changer
                for (auto it = groundedgetmp.begin(); it != groundedgetmp.end(); it++) {
                    newval = it->second[0];
                    oldval = it->second[1];
                    diff = newval > oldval ? newval - oldval : oldval - newval;
                    if (diff > beta * diffsum){
                        groundedge[it->first] = diff;
                    }
                }

                //Get the ground truth of heavy changer on node
                for (auto it = groundnodetmp.begin(); it != groundnodetmp.end(); it++) {
                    newval = it->second[0];
                    oldval = it->second[1];
                    diff = newval > oldval ? newval - oldval : oldval - newval;
                    if (diff > beta * diffsum){
                        groundnode[it->first] = diff;
                    }
                }
            }

            //Update DMatrix
            uint64_t t1, t2;
            adaptor->Reset();
            heavychangermv->Reset();
            DMatrix* cursk = (DMatrix*)heavychangermv->GetCurSketch();
            t1 = now_us();
            while(adaptor->GetNext(&t) == 1) {
                key_tp key;
                memcpy(key.key, &(t.key), LGN);
                cursk->Update(key.key, (val_tp)t.size);
            }
            t2 = now_us();
            updatetime = (double)(t2-t1)/1000000000;
            avrupdatetime += updatetime;


            // Query for edge weight
            val_tp edgeerror = 0, edgecount = 0;
            t1 = now_us();
            for (auto it = groundedgetmp.begin(); it != groundedgetmp.end(); it++) {
                if (it->second[0] > 0){
                edgecount++;
                val_tp esedgeweight = cursk->Queryedgeweight((uint32_t*)(*it).first.key);
                val_tp error = esedgeweight > it->second[0] ? esedgeweight - it->second[0] : it->second[0] - esedgeweight;
                edgeerror += error*1.0/it->second[0];
                }
            }
            t2 = now_us();
            querytime1 = (double)(t2-t1)/(1000000000*edgecount);
            avrquerytime1 += querytime1;
            reedge = edgeerror*1.0/edgecount;
            areedge += reedge;

            // Query for node weight (src)
            val_tp nodeerror = 0, nodecount = 0;
            t1 = now_us();
            for (auto it = groundnodetmp.begin(); it != groundnodetmp.end(); it++) {
                if (it->second[0] > 0){
                nodecount++;
                val_tp esnodeweight = cursk->Querynodeweight((uint32_t*)(*it).first.key);
                val_tp error = esnodeweight > it->second[0] ? esnodeweight - it->second[0] : it->second[0] - esnodeweight;
                nodeerror += error*1.0/it->second[0];
                }
            }
            t2 = now_us();
            querytime2 = (double)(t2-t1)/(1000000000*nodecount);
            avrquerytime2 += querytime2;
            renode = nodeerror*1.0/nodecount;
            arenode += renode;

            // Qurey for subgraph weight (10 nodes)
            //int nodenum = 10, subgraphnode = 0;
            int subgraphnode = 0;
            key_tp nodekey[nodenum];
            for (auto it = groundnodetmp.begin(); it != groundnodetmp.end(); it++) {
                if (subgraphnode == nodenum){break;}
                if (it->second[0] > 0){
                    memcpy(nodekey[subgraphnode].key, it->first.key, LGN/2);
                    subgraphnode++;
                }
            }
            val_tp essubgraph = 0, realsubgraph = 0;
            key_tp edgekey;
            t1 = now_us();
            for (int i = 0; i<nodenum-1; i++){
                for (int j=i+1; j<nodenum; j++){
                    memcpy(edgekey.key, nodekey[i].key, LGN/2);
                    memcpy(edgekey.key+1, nodekey[j].key, LGN/2);
                    essubgraph += cursk -> Queryedgeweight((uint32_t*)edgekey.key);
                    for (auto it = groundedgetmp.begin(); it != groundedgetmp.end(); it++) {
                        if (memcmp(it->first.key, edgekey.key, LGN) == 0) {
                        realsubgraph += it->second[0];
                    }
                    }
            }
            }
            t2 = now_us();
            querytime3 = (double)(t2-t1)/1000000000;
            avrquerytime3 += querytime3;
            if (realsubgraph != 0){
            resubgraph = (essubgraph - realsubgraph)*1.0/realsubgraph;
            } else {resubgraph = essubgraph - realsubgraph;}   
            aresubgraph += resubgraph;     
        
            // Query for heavy-hitter edge
            val_tp thresholdhitter = alpha * sum;
            t1 = now_us();
            cursk->Queryheavyhitteredge(thresholdhitter, heavyhitteredge);
            t2 = now_us();
            querytime4 = (double)(t2-t1)/1000000000;
            avrquerytime4 += querytime4;
            int tp = 0, cnt = 0, fp = 0;
            reerhitteredge = 0;
            for (auto it = groundedgetmp.begin(); it != groundedgetmp.end(); it++) {
                if (it->second[0] > thresholdhitter) {
                    cnt++;
                    for (auto res = heavyhitteredge.begin(); res != heavyhitteredge.end(); res++) {
                        if (memcmp(it->first.key, res->first.key, LGN) == 0) {
                            reerhitteredge += abs(long(res->second - it->second[0]))*1.0/it->second[0];
                            tp++;
                        }
                    }
                }
            }
            prehitteredge = tp*1.0/heavyhitteredge.size();
            rehitteredge = tp*1.0/cnt;
            reerhitteredge = reerhitteredge/tp;
            avrprehitteredge += prehitteredge;
            avrrehitteredge += rehitteredge;
            avrreerhitteredge += reerhitteredge;

            // Qurey for heavy-hitter node
            t1 = now_us();
            cursk->Queryheavyhitternode(thresholdhitter, heavyhitternode);
            t2 = now_us();
            querytime5 = (double)(t2-t1)/1000000000;
            avrquerytime5 += querytime5;
            // Eliminate the condition that src and dst are both heavy hitters
    //        mymap resultstmp;
    //        for (auto res = heavyhitternode.begin(); res != heavyhitternode.end(); res++){
    //            if (resultstmp.find(res->first) != resultstmp.end()) {
    //                resultstmp[res->first] += res->second;
    //            } else {
    //                resultstmp[res->first] = res->second;
    //               }
    //        }
            tp = 0;
            cnt = 0;
            reerhitternode = 0;
            for (auto it = groundnodetmp.begin(); it != groundnodetmp.end(); it++) {
                if (it->second[0] > thresholdhitter) {
                    cnt++;
                    for (auto res = heavyhitternode.begin(); res != heavyhitternode.end(); res++) {
                        if (memcmp(it->first.key, res->first.key, LGN/2) == 0) {
                            reerhitternode += abs(long(res->second - it->second[0]))*1.0/it->second[0];
                            tp++;
                        }
                    }
                }
            }
            prehitternode = tp*1.0/heavyhitternode.size();
            rehitternode = tp*1.0/cnt;
            reerhitternode = reerhitternode/tp;
            avrprehitternode += prehitternode;
            avrrehitternode += rehitternode;
            avrreerhitternode += reerhitternode;
        
            // Query for path reachability (use nodes in subgraph query)
            tp = 0;
            fp = 0;
            cnt = 0;
            t1 = now_us();
            paths.clear();
            cursk->Queryheavyhitteredge(gamma*sum, paths);
            for (int i = 0; i<nodenum-1; i++){
                for (int j=i+1; j<nodenum; j++){
                    memcpy(edgekey.key, nodekey[i].key, LGN/2);
                    memcpy(edgekey.key+1, nodekey[j].key, LGN/2);
                    for (auto it = groundedgetmp.begin(); it != groundedgetmp.end(); it++) {
                        if (memcmp(it->first.key, edgekey.key, LGN) == 0 && it->second[0]>gamma*sum) {  //!!!
                        cnt++;
                        if (cursk->BFS((uint32_t*)edgekey.key, paths)){tp++;}
                    } else {if (cursk->BFS((uint32_t*)edgekey.key, paths)){fp++;}}
                    }
            }
            }
            t2 = now_us();
            int querynum = nodenum * (nodenum - 1)/2;
            querytime6 = (double)(t2-t1)/(1000000000*querynum);
            avrquerytime6 += querytime6;
            if (tp == tp + fp){
            prerechability = 1;
            } else {prerechability = tp*1.0/tp + fp;}
            if (tp == cnt){rerechability = 1;}
            else {rerechability = tp*1.0/cnt;}
            avrprerechability += prerechability;
            avrrerechability += rerechability;

            //std::cout << std::setfill(' ');
            //std::cout << std::setw(30) << std::left << "RError. edge weight" << std::setw(30) << std::left << "RError. node weight"
            //<< std::setw(30) << std::left << "RError. subgraph" << std:: endl;
            //std::cout << std::setw(30) << std::left << reedge << std::setw(30) << std::left << renode
            //<< std::setw(30) << std::left << resubgraph << std:: endl;
            //std::cout << std::setw(30)<< std::left << "PR. heavy-hitter edge" << std::setw(30) << std::left << "RE. heavy-hitter edge"
            //<< std::setw(30) <<  "RError. heavy-hitter edge" << std::endl;
            //std::cout << std::setw(30)<< std::left << prehitteredge << std::setw(30) << std::left << rehitteredge
            //<< std::setw(30) <<  reerhitteredge << std::endl;
            //std::cout << std::setw(30)<< std::left << "PR. heavy-hitter node" << std::setw(30) << std::left << "RE. heavy-hitter node"
            //<< std::setw(30) <<  "RError. heavy-hitter node" << std::endl;
            //std::cout << std::setw(30)<< std::left << prehitternode << std::setw(30) << std::left << rehitternode
            //<< std::setw(30) <<  reerhitternode << std::endl;
            //std::cout << std::setw(30)<< std::left << "PR. path reachability" << std::setw(30) << std::left << "RE. path reachability" << std::endl;
            //std::cout << std::setw(30)<< std::left << prerechability << std::setw(30) << std::left << rerechability << std::endl;


            if (numfile != 0) {
                val_tp diffinsketch = heavychangermv->ChangeinSketch();
                //std::cout << "[Feature h] Sum of the changes between two adjacent epoch in sketch= " << diffinsketch << std::endl;            
                myvector heavychangeredge, heavychangernode;
                heavychangeredge.clear();
                heavychangernode.clear();

                //Query for heavy changer edge
                val_tp thresholdchanger = beta * diffinsketch;
                t1 = now_us();
                heavychangermv->Querychangeredge(thresholdchanger, heavychangeredge);
                t2 = now_us();
                querytime7 = (double)(t2-t1)/1000000000;
                avrquerytime7 += querytime7;
                int tp = 0;
                reerchangeredge = 0;
                for (auto it = groundedge.begin(); it != groundedge.end(); it++) {
                        for (auto res = heavychangeredge.begin(); res != heavychangeredge.end(); res++) {
                            if (memcmp(it->first.key, res->first.key, LGN) == 0) {
                                reerchangeredge += abs(long(res->second - it->second))*1.0/it->second;
                                tp++;
                            }
                        }
                }
                prechangeredge = tp*1.0/heavychangeredge.size();
                rechangeredge = tp*1.0/groundedge.size();
                reerchangeredge = reerchangeredge/tp;
                avrprechangeredge += prechangeredge; avrrechangeredge += rechangeredge; avrreerchangeredge += reerchangeredge;
            

                //Query for heavy changer on node
                t1 = now_us();
                heavychangermv->Querychangernode(thresholdchanger, heavychangernode);
                t2 = now_us();
                querytime8 = (double)(t2-t1)/1000000000;
                avrquerytime8 += querytime8;
    //            // Eliminate the condition that src and dst are both heavy hitters
    //            mymap resultschangetmp;
    //            for (auto res = resultschange.begin(); res != resultschange.end(); res++){
    //                if (resultschangetmp.find(res->first) != resultschangetmp.end()) {
    //                    resultschangetmp[res->first] += res->second;
    //                } else {
    //                    resultschangetmp[res->first] = res->second;
    //                   }
    //            }
                tp = 0, cnt = 0;
                reerchangernode = 0;
                for (auto it = groundnode.begin(); it != groundnode.end(); it++) {
                        for (auto res = heavychangernode.begin(); res != heavychangernode.end(); res++) {
                            if (memcmp(it->first.key, res->first.key, LGN/2) == 0) {
                                reerchangernode += abs(long(res->second - it->second))*1.0/it->second;
                                tp++;
                            }
                        }
                }
                prechangernode = tp*1.0/heavychangernode.size();
                rechangernode = tp*1.0/groundnode.size();
                reerchangernode = reerchangernode/tp;
                avrprechangernode += prechangernode; avrrechangernode += rechangernode; avrreerchangernode += reerchangernode;

                
                //std::cout << std::setw(30) << std::left << "PR. heavy-changer edge" << std::setw(30) << std::left << "RE. heavy-changer edge"
                //<< std::setw(30) << std::left << "RError heavy-changer edge" << std::endl;
                //std::cout << std::setw(30) << std::left << prechangeredge << std::setw(30) << std::left << rechangeredge
                //<< std::setw(30) << std::left << reerchangeredge << std::endl;
                //std::cout << std::setw(30) << std::left << "PR. heavy-changer node" << std::setw(30) << std::left << "RE. heavy-changer node"
                //<< std::setw(30) << std::left << "RError heavy-changer node" << std::endl;
                //std::cout << std::setw(30) << std::left << prechangernode << std::setw(30) << std::left << rechangernode
                //<< std::setw(30) << std::left << reerchangernode << std::endl;


                //The number of changed heavy edge/node between two adjacent epoch
    //            heavychangermv->Changedheavy(oldheavyhitteredge, heavyhitteredge);
    //
    //            heavychangermv->ChangedheavyNode(oldheavyhitternode, heavyhitternode, oldsrcnodeNo, cursrcnodeNo);
            }

    //        oldheavyhitteredge = heavyhitteredge;
    //        oldheavyhitternode = heavyhitternode;
    //        oldsrcnodeNo = cursrcnodeNo;
            heavyhitteredge.clear();
            heavyhitternode.clear();
    //        mymap super;
    //        cursk->FindSuper(super);
    //        //std::cout << "[Feature q] The number of super-spreader = " << super.size();
    //        if (super.size() > 0){
    //            for (auto res = super.begin(); res != super.end(); res++){
    //            if (res->second == 0){
    //            struct in_addr src;
    //            long addrsrc;
    //            addrsrc = htonl(res->first.key[0]);
    //            memcpy(&src, &addrsrc, 4);
    //            //std::cout << " with a corresponding node (src) = " << inet_ntoa(src);
    //            }else{
    //            struct in_addr dst;
    //            long addrdst;
    //            addrdst = htonl(res->first.key[0]);
    //            memcpy(&dst, &addrdst, 4);
    //            //std::cout << " with a corresponding node (dst) = " << inet_ntoa(dst);
    //            }
    //            }
    //            std::cout << std::endl;
    //        }


            numfile++;
            delete adaptor;
        }

        //Delete
        for (auto it = groundedgetmp.begin(); it != groundedgetmp.end(); it++) {
            delete [] it->second;
        }
        delete heavychangermv;

        std::cout << "-----------------------------------------------   Summary    -------------------------------------------------------" << std::endl;
	
        std::cout << std::setw(30) << std::left << "ARError. edge weight" << std::setw(30) << std::left << "ARError. node weight"
        << std::setw(30) << std::left << "ARError. subgraph" << std:: endl;
        std::cout << std::setw(30) << std::left << areedge/numfile << std::setw(30) << std::left << arenode/numfile
        << std::setw(30) << std::left << aresubgraph/numfile << std:: endl;
        std::cout << std::setw(30)<< std::left << "APR. heavy-hitter edge" << std::setw(30) << std::left << "ARE. heavy-hitter edge"
        << std::setw(30) <<  "ARError. heavy-hitter edge" << std::endl;
        std::cout << std::setw(30)<< std::left << avrprehitteredge/numfile << std::setw(30) << std::left << avrrehitteredge/numfile
        << std::setw(30) <<  avrreerhitteredge/numfile << std::endl;
        std::cout << std::setw(30)<< std::left << "APR. heavy-hitter node" << std::setw(30) << std::left << "ARE. heavy-hitter node"
        << std::setw(30) <<  "ARError. heavy-hitter node" << std::endl;
        std::cout << std::setw(30)<< std::left << avrprehitternode/numfile << std::setw(30) << std::left << avrrehitternode/numfile
        << std::setw(30) <<  avrreerhitternode/numfile << std::endl;
        std::cout << std::setw(30)<< std::left << "APR. path reachability" << std::setw(30) << std::left << "ARE. path reachability" << std::endl;
        std::cout << std::setw(30)<< std::left << avrprerechability/numfile << std::setw(30) << std::left << avrrerechability/numfile << std::endl;

        std::cout << std::setw(30) << std::left << "APR. heavy-changer edge" << std::setw(30) << std::left << "ARE. heavy-changer edge"
        << std::setw(30) << std::left << "ARError" << std::endl;
        std::cout << std::setw(30) << std::left << avrprechangeredge/(numfile-1) << std::setw(30) << std::left << avrrechangeredge/(numfile-1)
        << std::setw(30) << std::left << avrreerchangeredge/(numfile-1) << std::endl;
        std::cout << std::setw(30) << std::left << "APR. heavy-changer node" << std::setw(30) << std::left << "ARE. heavy-changer node"
        << std::setw(30) << std::left << "ARError" << std::endl;
        std::cout << std::setw(30) << std::left << avrprechangernode/(numfile-1) << std::setw(30) << std::left << avrrechangernode/(numfile-1)
        << std::setw(30) << std::left << avrreerchangernode/(numfile-1) << std::endl;

        std::cout << std::setw(30) << std::left << "AvrT. Update " << std::setw(30) << std::left << "AvrT. edge weight"
        << std::setw(30) << std::left << "AvrT. node weight" << std::setw(30) << std::left << "AvrT. subgraph(10 nodes)"
        << std::setw(30) << std::left << "AvrT. heavy-hitter edge" << std::endl;
        std::cout << std::setw(30) << std::left << avrupdatetime/numfile << std::setw(30) << std::left << avrquerytime1/numfile
        << std::setw(30) << std::left << avrquerytime2/numfile << std::setw(30) << std::left << avrquerytime3/numfile
        << std::setw(30) << std::left << avrquerytime4/numfile << std::endl;

        std::cout << std::setw(30) << std::left << "AvrT. heavy-hitter node" << std::setw(30) << std::left << "AvrT. reachability"
        << std::setw(30) << std::left << "AvrT. heavy-changer edge" << std::setw(30) << std::left << "AvrT. heavy-changer node" << std::endl;
        std::cout << std::setw(30) << std::left << avrquerytime5/numfile << std::setw(30) << std::left << avrquerytime6/numfile
        << std::setw(30) << std::left << avrquerytime7/(numfile-1) << std::setw(30) << std::left << avrquerytime8/(numfile-1) << std::endl;
    }
}
