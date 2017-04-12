#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <map>
#include <sstream>
using namespace  std;

#define MAX_NODE 1000
#define INF 100000000
#define MAX_SERVER 500

struct Graph_Edge
{
	int from, to, capacity, flow, cost;//入点、出点、容量、实际流量、流量单价
	Graph_Edge(void){};
	Graph_Edge(int u, int v, int c, int f, int w) :
		from(u), to(v), capacity(c), flow(f), cost(w){}
};
struct ConsumerNode{
    int consumerNode;//消费节点编号
    int netNode;//所连网络结点编号
    int flowNeed;//流量需求
};

class Graph{
public:
    int nodeNum,edgeNum,consumerNum;//结点数
	vector<Graph_Edge> G[MAX_NODE];//网络结点邻接表
	vector<ConsumerNode> consumers;//消费节点数组
    map<int,int> netToConsumer;//网络节点到消费节点的映射

    //服务器单价
    int serverCost;

    //求最短路
	//Graph_Edge p[MAX_NODE]; // 当前节点单源最短路中的上一条边
	//int d[MAX_NODE]; // 单源最短路径长

    void init(int n,int m,int k){
        nodeNum=n;
        edgeNum=m;
        consumerNum=k;
        for(int i=0;i<nodeNum;i++){
            G[i].clear();
        }
        consumers.clear();

    }

    //获取图
    void createGraph(char * topo[MAX_EDGE_NUM]){

        string line=topo[0];
        stringstream ss(line);
        int nodeNum;//结点数
        int edgeNum;//边数
        int consumerNum;//消费结点数
        ss>>nodeNum>>edgeNum>>consumerNum;
        init(nodeNum,edgeNum,consumerNum);
        serverCost=strtol(topo[2],NULL,10);
        for(int i=4;i<edgeNum+4;i++){
			Graph_Edge e;
            line=topo[i];
            stringstream ss(line);
			ss >> e.from >> e.to >> e.capacity >> e.cost;
            G[e.from].push_back(e);
        }
        for(int i=5+edgeNum;i<consumerNum+5+edgeNum;i++){
			ConsumerNode consumer;
            line=topo[i];
            stringstream ss(line);
			ss >> consumer.consumerNode >> consumer.netNode >> consumer.flowNeed;
			netToConsumer[consumer.netNode] = consumer.consumerNode;//建立网结点和消费结点映射
            consumers.push_back(consumer);
        }
    }
};

#endif // GRAPH_H
