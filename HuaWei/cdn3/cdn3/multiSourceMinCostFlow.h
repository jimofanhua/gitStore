#ifndef MINCOSTFLOW_H
#define MINCOSTFLOW_H

#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <functional>
#include <cstdio>
#include <set>
#include <time.h>
#include <cstring>
#include <map>
#include "topoGraph.h"
using namespace std;
double myrand()
{
	static int first = 0;
	if(!first){
		srand((unsigned)time(0));
		first++;
	}
	return (double)rand() / RAND_MAX;
}
typedef pair<int, int> P;//first保存最短距离，second保存顶点的编号
//最小费用流的边结构
struct MinCostFlow_Edge
{
	int to, cap, cost;//终点，容量（指残量网络中的），费用，反向边编号
	MinCostFlow_Edge(int t, int c, int cc) :to(t), cap(c), cost(cc) {}
};
//
struct noSatisfyConsumerNode
{
	int netNode;
	int noNeedFlow;
};  
class MinCostFlow{//最小费用流（指定需求流量）
public:
	int nodeNum;//顶点数
	vector<MinCostFlow_Edge> G[MAX_NODE];//图的邻接表
	int nodeCostToStart[MAX_NODE];//节点到当前源节点的最小费用
	int distToStart[MAX_NODE];//到源节点的最短距离
	int preNode[MAX_NODE];//最短路中的父结点
	int preEdge[MAX_NODE];//最短路中的父边
	int pathCost[MAX_NODE];
	vector<noSatisfyConsumerNode> leaveCom;
	Graph graph;//关联的图

	void init(int n){
		nodeNum=n;
		for(int i=0;i<nodeNum;i++){
			G[i].clear();
		}
	}

	void addEdge(int from, int to, int cap, int cost)
	{
		G[from].push_back(MinCostFlow_Edge(to, cap, cost));
		G[to].push_back(MinCostFlow_Edge(from, cap, cost));
	}

	void addEdge(vector<MinCostFlow_Edge> G1[], int from, int to, int cap, int cost){
		G1[from].push_back(MinCostFlow_Edge(to, cap, cost));
		G1[to].push_back(MinCostFlow_Edge(from, cap, cost));
	}

	//获取MinCostFlow(转存Graph到MinCostFlow的G)
	void createMinCostFlow(const Graph &graph){
		this->graph=graph;
		init(graph.nodeNum);
		for (int i = 0; i<graph.nodeNum; i++)
		{
			for(int j=0;j<graph.G[i].size();j++){
				addEdge(graph.G[i][j].from,graph.G[i][j].to,
					graph.G[i][j].capacity, graph.G[i][j].cost);

			}
		}
	}

	vector<noSatisfyConsumerNode> computeUnsatisfyConsumerNode(vector<ConsumerNode> &consumers, set<int> &servers, vector<int> &consumerNetNodes, map<int, int> comFlowNeed, int f, int &m, double blCost, double serverCost)
	{
		vector<int> serVec;
		vector<vector<int> > serMinCostPath;
		vector<noSatisfyConsumerNode> noNeedcom;
		for (set<int>::iterator iter = servers.begin(); iter != servers.end(); iter++)
		{
			serVec.push_back(*iter);
		}
		multiSourceMinCostFlow(serVec, consumerNetNodes, comFlowNeed, f, serMinCostPath, m, blCost, serverCost, serVec.size());
		map<int, int> serMap;
		for(int i =0;i<serMinCostPath.size(); i++)
		{
			if(serMinCostPath[i].size()>0)
			{
				int comNode = serMinCostPath[i][serMinCostPath[i].size()-2];
				int nowFlowNeed = serMinCostPath[i][serMinCostPath[i].size()-1];
				if(serMap.find(comNode)!=serMap.end())
				{
					serMap[comNode]+=nowFlowNeed;
				}
				else
					serMap.insert(make_pair(comNode,nowFlowNeed));
			}
			else
				break;
				
		}
		noSatisfyConsumerNode nscn;
		for (int i = 0; i < consumers.size(); i++)
		{
			if(serMap[consumers[i].consumerNode]<consumers[i].flowNeed)
			{
				nscn.netNode=consumers[i].netNode;
				nscn.noNeedFlow=consumers[i].flowNeed;
				noNeedcom.push_back(nscn);
			}
		}

		for(vector<noSatisfyConsumerNode>::iterator iter=leaveCom.begin(); iter!=leaveCom.end(); iter++)
		{
			noNeedcom.push_back(*iter);
		}
		if(noNeedcom.size()!=0)
		{
			sort(noNeedcom.begin(), noNeedcom.end(), [](const noSatisfyConsumerNode &a, const noSatisfyConsumerNode&b){
			return a.noNeedFlow > b.noNeedFlow;
			});	
		}
		return noNeedcom;
	}

	//通过对消费节点流量需求排序的方式初始化一部分种群
	vector<vector<int> >  initPopulationBySort(vector<ConsumerNode> consumers, vector<int> &consumerNetNodes, map<int, int> comFlowNeed, int f, vector<vector<int> > &minCostPath, int &m, double blCost, double serverCost, int bit, int popNum, vector<vector<int> > &externalPop, vector<double> &externalCost)
	{
		vector<vector<int>> pop;
		set<int> serversH;
		set<int> serversL;
		sort(consumers.begin(), consumers.end(), [](const ConsumerNode &a, const ConsumerNode&b){
			return a.flowNeed > b.flowNeed;
		});
		for (int i = 0; i < consumers.size()/2; i++)
		{
			serversH.insert(consumers[i].netNode);
		}

		for (int i = 0; i < consumers.size()/3; i++)
		{
			serversL.insert(consumers[i].netNode);
		}

		vector<noSatisfyConsumerNode> noNeedcomL = computeUnsatisfyConsumerNode(consumers, serversL, consumerNetNodes, comFlowNeed,f, m, blCost, serverCost);
		int count1=popNum;
		if(bit>500)
		{
			while(noNeedcomL.size()!=0)
			{
				int *initPop = new int[bit]();
				set<int> test = serversL;
				test.insert(noNeedcomL[0].netNode);
				vector<int> vec;
				for (set<int>::iterator iter = test.begin(); iter != test.end(); iter++)
				{
					vec.push_back(*iter);
					initPop[*iter] = 1;
				}
				int res = multiSourceMinCostFlow(vec, consumerNetNodes, comFlowNeed, f, minCostPath, m, blCost, serverCost, vec.size());
				if (res < blCost)
				{
				
					cout << "初始化2:" << res << endl;
					pop.push_back(vector<int>(initPop, initPop + bit));
					externalPop.push_back(vector<int>(initPop, initPop + bit));
					externalCost.push_back(res);
				}		
				serversL.insert(noNeedcomL[0].netNode);	
				noNeedcomL = computeUnsatisfyConsumerNode(consumers, serversL, consumerNetNodes, comFlowNeed,f, m, blCost, serverCost);
			}
			return pop;
		}
		else
		{
			for (int j = consumers.size()/2; j < consumers.size()&&count1>0;j++)
			{
				int *initPop = new int[bit]();
				set<int> test = serversH;
				test.insert(consumers[j].netNode);
				vector<int> vec;
				for (set<int>::iterator iter = test.begin(); iter != test.end(); iter++)
				{
					vec.push_back(*iter);
					initPop[*iter] = 1;
				}
				int res = multiSourceMinCostFlow(vec, consumerNetNodes, comFlowNeed,f, minCostPath, m, blCost, serverCost, vec.size());
				if (res < blCost)
				{
				
					cout << "初始化1:" << res << endl;
					pop.push_back(vector<int>(initPop, initPop + bit));
					externalPop.push_back(vector<int>(initPop, initPop + bit));
					externalCost.push_back(res);
					count1--;
				}		
				serversH.insert(consumers[j].netNode);
			}

			while(noNeedcomL.size()!=0)
			{
				int *initPop = new int[bit]();
				set<int> test = serversL;
				test.insert(noNeedcomL[0].netNode);
				vector<int> vec;
				for (set<int>::iterator iter = test.begin(); iter != test.end(); iter++)
				{
					vec.push_back(*iter);
					initPop[*iter] = 1;
				}
				int res = multiSourceMinCostFlow(vec, consumerNetNodes, comFlowNeed, f, minCostPath, m, blCost, serverCost, vec.size());
				if (res < blCost)
				{
				
					cout << "初始化2:" << res << endl;
					pop.push_back(vector<int>(initPop, initPop + bit));
					externalPop.push_back(vector<int>(initPop, initPop + bit));
					externalCost.push_back(res);
				}		
				serversL.insert(noNeedcomL[0].netNode);	
				noNeedcomL = computeUnsatisfyConsumerNode(consumers, serversL, consumerNetNodes, comFlowNeed,f, m, blCost, serverCost);
			}
			return pop;	
		}
			
	}
	//多源多汇最小费用流算法
	int multiSourceMinCostFlow(const vector<int> &servers, const vector<int> &consumerNetNodes, map<int, int> comFlowNeed,
                          int flow,vector<vector<int> > &minCostPath,int &m,double blCost,double serverCost,int fwqNum){

		 vector<MinCostFlow_Edge> G1[MAX_NODE];
		 copy(this->G, this->G + MAX_NODE, G1);

		 fill(pathCost,pathCost+MAX_NODE,0);

		 leaveCom.clear();
         int superServerNode=nodeNum;//超级源节点

		 vector<int> isNotServer;
		 vector<int> isServer;
		 int serNode[800] = { 0 };
		 for (int i = 0; i < servers.size(); i++)
		 {
			 serNode[servers[i]] = 1;
		 }
		 for (int j = 0; j < consumerNetNodes.size(); j++)
		 {
			 if (serNode[consumerNetNodes[j]] == 1)
			 {
				 isServer.push_back(consumerNetNodes[j]);
				 flow -= comFlowNeed[consumerNetNodes[j]];
			 }
			 else
				 isNotServer.push_back(consumerNetNodes[j]);
		 }

		 //超级源与每个服务器建立边：费用0，容量无穷
         for(int i=0;i<servers.size();i++){
			 addEdge(G1, superServerNode, servers[i], INF, 0);
         }

         int superConsumerNetNode=nodeNum+1;//超级汇节点

		 //超级汇与每个消费结点所连的网络结点建立边：费用0，容量为消费节点的需求
         /*for(int i=0;i<consumerNetNodes.size();i++){
             addEdge(G1,consumerNetNodes[i],superConsumerNetNode,graph.consumers[i].flowNeed,0);
         }*/
		 for (int i = 0; i<isNotServer.size(); i++){
			 addEdge(G1, isNotServer[i], superConsumerNetNode, comFlowNeed[isNotServer[i]], 0);
		 }
		 //存当前的最小流路径的节点
		 stack<int> minCostPathStack;

		 //表示当前增广路径是第几条
         int pathNum=0;

		 //备份一个最大需求
		 int flowNeed = flow;
         int minCost = 0;

		 //初始化最小费用流路径数组
         vector<int> temp;
		 for(int i=0;i<m;i++){

			minCostPath.push_back(temp);
		 }
		 \
		 for (int k = 0; k < isServer.size(); k++)
		 {
			 minCostPath[pathNum].push_back(isServer[k]);
			 minCostPath[pathNum].push_back(this->graph.netToConsumer[isServer[k]]);
			 minCostPath[pathNum].push_back(comFlowNeed[isServer[k]]);
			 pathNum++;
		 }

		 //初始化各节点到当前源节点的最小费用
		 fill(nodeCostToStart, nodeCostToStart + nodeNum + 2, 0);

		 //flow>0时还需要继续增广
		 while (flow>0)
         {
			 //使用优先队列可以按当前节点到源节点的距离进行排序
             priority_queue<P, vector<P>, greater<P> >q;

			 //距离初始化为INF
			 fill(distToStart, distToStart + nodeNum + 2, INF);

			 distToStart[superServerNode] = 0;
			 q.push(P(0, superServerNode));

			 //单次调用超级源到超级汇的最小费用流
            //bellman
			int inq[1000] = {0};
			inq[superServerNode] = 1;
			while (!q.empty())
			{
				P p = q.top(); q.pop();
				int v = p.second;
				inq[v] = 0;
				for (int i = 0; i<G1[v].size(); i++)
				{
					MinCostFlow_Edge &e = G1[v][i];
					if (e.cap>0 && distToStart[e.to]>distToStart[v] + e.cost)//松弛操作
					{
						distToStart[e.to] = distToStart[v] + e.cost ;
						preNode[e.to] = v;//更新父结点
						preEdge[e.to] = i;//更新父边编号
						if (!inq[e.to])
						{
							q.push(P(distToStart[e.to], e.to));
						}
						
					}
				}
			}

			 //如果dist[t]还是初始时候的INF，那么说明s-t不连通，不能再增广了
			 if (distToStart[superConsumerNetNode] == INF){

				 double costBmz = minCost / (flowNeed - flow)*flowNeed + fwqNum*serverCost;
					if(costBmz>blCost)
					{
						return costBmz;
					}
						
					else
						return blCost+10;
             }

			 if (pathNum>1500)
					return blCost+10000;

			 //更新nodeCostToStart
     //        for (int j = 0; j<nodeNum+2; j++)
				 //nodeCostToStart[j] += distToStart[j];

			 int d = flow;
			 //bellman
			 int h = 0;
			 for (int x = superConsumerNetNode; x != superServerNode; x = preNode[x]){
				 minCostPathStack.push(x);
				 //从t出发沿着最短路返回s找可改进量
				 d = min(d, G1[preNode[x]][preEdge[x]].cap);
				 //bellman
				 h += G1[preNode[x]][preEdge[x]].cost;
             }
             int node;
			 while (!minCostPathStack.empty()){
				 node = minCostPathStack.top();
				 minCostPathStack.pop();
				 minCostPath[pathNum].push_back(node);

             }
			 auto end = minCostPath[pathNum].end();
			 //去掉超级汇点
			 minCostPath[pathNum].erase(end - 1);
			 //得到连接消费节点的网络节点
			 node = minCostPath[pathNum][minCostPath[pathNum].size() - 1];
			 //加入消费结点
			 minCostPath[pathNum].push_back(this->graph.netToConsumer[node]);
             if(d<=flowNeed){
				 minCostPath[pathNum].push_back(d);//实际流量
             }
             else{
				 minCostPath[pathNum].push_back(flowNeed);
             }
			 pathNum++;
			 flow -= d;
			 //h[t]表示最短距离的同时，也代表了这条最短路上的费用之和，乘以流量d即可得到本次增广所需的费用
			 //minCost += d*nodeCostToStart[superConsumerNetNode];
			 //bellman
			 minCost += d*h;
			 //pathCost[node]+=d*nodeCostToStart[superConsumerNetNode];
			 //bellman
			 pathCost[node]+=d*h;
			 if(d==G1[preNode[node]][preEdge[node]].cap)
			 {
				 if(pathCost[node]>serverCost)
				 {
					 noSatisfyConsumerNode nscn;
					 nscn.netNode=node;
					 nscn.noNeedFlow=comFlowNeed[node];
					 leaveCom.push_back(nscn);
				 }
			 }
			 for (int x = superConsumerNetNode; x != superServerNode; x = preNode[x])
             {
				 MinCostFlow_Edge &e = G1[preNode[x]][preEdge[x]];
				 //修改残量值
                 e.cap -= d;

             }

         }
		 return (minCost + fwqNum*serverCost);
    }
};


#endif // MINCOSTFLOW_H
