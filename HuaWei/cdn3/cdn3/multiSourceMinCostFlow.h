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
typedef pair<int, int> P;//first������̾��룬second���涥��ı��
//��С�������ı߽ṹ
struct MinCostFlow_Edge
{
	int to, cap, cost;//�յ㣬������ָ���������еģ������ã�����߱��
	MinCostFlow_Edge(int t, int c, int cc) :to(t), cap(c), cost(cc) {}
};
//
struct noSatisfyConsumerNode
{
	int netNode;
	int noNeedFlow;
};  
class MinCostFlow{//��С��������ָ������������
public:
	int nodeNum;//������
	vector<MinCostFlow_Edge> G[MAX_NODE];//ͼ���ڽӱ�
	int nodeCostToStart[MAX_NODE];//�ڵ㵽��ǰԴ�ڵ����С����
	int distToStart[MAX_NODE];//��Դ�ڵ����̾���
	int preNode[MAX_NODE];//���·�еĸ����
	int preEdge[MAX_NODE];//���·�еĸ���
	int pathCost[MAX_NODE];
	vector<noSatisfyConsumerNode> leaveCom;
	Graph graph;//������ͼ

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

	//��ȡMinCostFlow(ת��Graph��MinCostFlow��G)
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

	//ͨ�������ѽڵ�������������ķ�ʽ��ʼ��һ������Ⱥ
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
				
					cout << "��ʼ��2:" << res << endl;
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
				
					cout << "��ʼ��1:" << res << endl;
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
				
					cout << "��ʼ��2:" << res << endl;
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
	//��Դ�����С�������㷨
	int multiSourceMinCostFlow(const vector<int> &servers, const vector<int> &consumerNetNodes, map<int, int> comFlowNeed,
                          int flow,vector<vector<int> > &minCostPath,int &m,double blCost,double serverCost,int fwqNum){

		 vector<MinCostFlow_Edge> G1[MAX_NODE];
		 copy(this->G, this->G + MAX_NODE, G1);

		 fill(pathCost,pathCost+MAX_NODE,0);

		 leaveCom.clear();
         int superServerNode=nodeNum;//����Դ�ڵ�

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

		 //����Դ��ÿ�������������ߣ�����0����������
         for(int i=0;i<servers.size();i++){
			 addEdge(G1, superServerNode, servers[i], INF, 0);
         }

         int superConsumerNetNode=nodeNum+1;//������ڵ�

		 //��������ÿ�����ѽ�������������㽨���ߣ�����0������Ϊ���ѽڵ������
         /*for(int i=0;i<consumerNetNodes.size();i++){
             addEdge(G1,consumerNetNodes[i],superConsumerNetNode,graph.consumers[i].flowNeed,0);
         }*/
		 for (int i = 0; i<isNotServer.size(); i++){
			 addEdge(G1, isNotServer[i], superConsumerNetNode, comFlowNeed[isNotServer[i]], 0);
		 }
		 //�浱ǰ����С��·���Ľڵ�
		 stack<int> minCostPathStack;

		 //��ʾ��ǰ����·���ǵڼ���
         int pathNum=0;

		 //����һ���������
		 int flowNeed = flow;
         int minCost = 0;

		 //��ʼ����С������·������
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

		 //��ʼ�����ڵ㵽��ǰԴ�ڵ����С����
		 fill(nodeCostToStart, nodeCostToStart + nodeNum + 2, 0);

		 //flow>0ʱ����Ҫ��������
		 while (flow>0)
         {
			 //ʹ�����ȶ��п��԰���ǰ�ڵ㵽Դ�ڵ�ľ����������
             priority_queue<P, vector<P>, greater<P> >q;

			 //�����ʼ��ΪINF
			 fill(distToStart, distToStart + nodeNum + 2, INF);

			 distToStart[superServerNode] = 0;
			 q.push(P(0, superServerNode));

			 //���ε��ó���Դ�����������С������
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
					if (e.cap>0 && distToStart[e.to]>distToStart[v] + e.cost)//�ɳڲ���
					{
						distToStart[e.to] = distToStart[v] + e.cost ;
						preNode[e.to] = v;//���¸����
						preEdge[e.to] = i;//���¸��߱��
						if (!inq[e.to])
						{
							q.push(P(distToStart[e.to], e.to));
						}
						
					}
				}
			}

			 //���dist[t]���ǳ�ʼʱ���INF����ô˵��s-t����ͨ��������������
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

			 //����nodeCostToStart
     //        for (int j = 0; j<nodeNum+2; j++)
				 //nodeCostToStart[j] += distToStart[j];

			 int d = flow;
			 //bellman
			 int h = 0;
			 for (int x = superConsumerNetNode; x != superServerNode; x = preNode[x]){
				 minCostPathStack.push(x);
				 //��t�����������·����s�ҿɸĽ���
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
			 //ȥ���������
			 minCostPath[pathNum].erase(end - 1);
			 //�õ��������ѽڵ������ڵ�
			 node = minCostPath[pathNum][minCostPath[pathNum].size() - 1];
			 //�������ѽ��
			 minCostPath[pathNum].push_back(this->graph.netToConsumer[node]);
             if(d<=flowNeed){
				 minCostPath[pathNum].push_back(d);//ʵ������
             }
             else{
				 minCostPath[pathNum].push_back(flowNeed);
             }
			 pathNum++;
			 flow -= d;
			 //h[t]��ʾ��̾����ͬʱ��Ҳ�������������·�ϵķ���֮�ͣ���������d���ɵõ�������������ķ���
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
				 //�޸Ĳ���ֵ
                 e.cap -= d;

             }

         }
		 return (minCost + fwqNum*serverCost);
    }
};


#endif // MINCOSTFLOW_H
