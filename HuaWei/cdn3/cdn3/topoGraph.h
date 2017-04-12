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
	int from, to, capacity, flow, cost;//��㡢���㡢������ʵ����������������
	Graph_Edge(void){};
	Graph_Edge(int u, int v, int c, int f, int w) :
		from(u), to(v), capacity(c), flow(f), cost(w){}
};
struct ConsumerNode{
    int consumerNode;//���ѽڵ���
    int netNode;//������������
    int flowNeed;//��������
};

class Graph{
public:
    int nodeNum,edgeNum,consumerNum;//�����
	vector<Graph_Edge> G[MAX_NODE];//�������ڽӱ�
	vector<ConsumerNode> consumers;//���ѽڵ�����
    map<int,int> netToConsumer;//����ڵ㵽���ѽڵ��ӳ��

    //����������
    int serverCost;

    //�����·
	//Graph_Edge p[MAX_NODE]; // ��ǰ�ڵ㵥Դ���·�е���һ����
	//int d[MAX_NODE]; // ��Դ���·����

    void init(int n,int m,int k){
        nodeNum=n;
        edgeNum=m;
        consumerNum=k;
        for(int i=0;i<nodeNum;i++){
            G[i].clear();
        }
        consumers.clear();

    }

    //��ȡͼ
    void createGraph(char * topo[MAX_EDGE_NUM]){

        string line=topo[0];
        stringstream ss(line);
        int nodeNum;//�����
        int edgeNum;//����
        int consumerNum;//���ѽ����
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
			netToConsumer[consumer.netNode] = consumer.consumerNode;//�������������ѽ��ӳ��
            consumers.push_back(consumer);
        }
    }
};

#endif // GRAPH_H
