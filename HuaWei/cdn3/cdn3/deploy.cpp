#include "stdafx.h"
#include "deploy.h"
#include "topoGraph.h"
#include "multiSourceMinCostFlow.h"
#include <iostream>
#include <fstream>
#include <time.h>
#include <algorithm> 
#include <cstdlib>
#include <ctime>
#include <string>
#include <unordered_map>
#include <sstream>
#include <vector>
using namespace std;
static bool is_break = false;
//你要完成的功能总入口
//double myrand()
//{
//	static int first = 0;
//	if(!first){
//		srand((unsigned)time(0));
//		first++;
//	}
//	return (double)rand() / RAND_MAX;
//}
string vecToKey(vector<int> t)
{
	string s="";
	for(vector<int>::iterator iter = t.begin(); iter!=t.end(); iter++)
	{
		stringstream stream;  
        stream<<*iter;  
		s+=stream.str();
	}
	return s;
}

struct HashFunc
{
	std::size_t operator()(const vector<int> &key) const
	{
		size_t size =  hash<string>()(vecToKey(key)) % 17711;
		return size;
	}
};
struct netNodeDu
{
	int netNode;
	int num;
};
void init_pop(vector<vector<int> > &population,int pop,int bit,double gl)
{

	srand((unsigned)time(0));
	vector<int> x; 
	int size=pop - population.size();
	for (int i = 0; i<size; i++)
	{
		for(int j=0;j<bit;j++)
		{
			if(myrand()<(gl/90))
				x.push_back(1);
			else
				x.push_back(0);
		}
		population.push_back(x);
		x.clear();
	}
}
void init_popZG(vector<vector<int> > &population,int pop,int bit,int initL[])
{
	srand((unsigned)time(0));
	int size=pop - population.size();
	int size1=population.size();
	for (int i = 0; i<size; i++)
	{
		population.push_back(vector<int>(initL,initL+bit));

		for(int j=0;j<4;j++)
		{
			int k=int(myrand()*(bit-1));
			while(initL[k]==0)
			{
				k=int(myrand()*(bit-1));
			}
			population[size1+i][k]=1-population[size1+i][k];
		}
	}
}
int*  selectIndiv(double eval1[],int pop)
{
	int *rs=new int[pop];
	double *eval2=new double[pop];
	double temp=0;
	for(int i=0;i<pop;i++)
	{
		rs[i]=i;
		eval2[i]=eval1[i];
	}
	for(int i=0;i<pop;i++)
	{
		for(int j=i+1;j<pop;j++)
		{
			if(eval2[i]>eval2[j])
			{
				temp=eval2[i];
				eval2[i]=eval2[j];
				eval2[j]=temp;

				temp=rs[i];
				rs[i]=rs[j];
				rs[j]=temp;
			}
		}
	}
	return rs;
}
void crossover(vector<vector<int> > &population_rw,int pop,int bit)
{
	double pc=0.95;
	char temp;
	for(int k=0;k<pop;k++)
	{
		if(pc>=myrand())
		{
			int i=0,j=0;
			while(i==j)
			{
				i=int(myrand()*(pop-1));
				j=int(myrand()*(pop-1));
			}
			int p=int(myrand()*(bit-1));
			temp=population_rw[i][p];
			population_rw[i][p]=population_rw[j][p];
			population_rw[j][p]=temp;
		}
	}
}
unordered_map<vector<int>, double,HashFunc> hashCost;
vector<vector<int> > *population=new vector<vector<int> >;
vector<vector<int> > populationTemp;
vector<vector<int> > *population_temp=new vector<vector<int> >(50);//必须大于最大的种群个数
vector<vector<int> > *tempChanged;
vector<vector<int> > externalPop;
vector<double> externalCost;
double *evalTemp;
double minpop=1e30;
double evalEver=0.0;
double pm=0.001;
double pmMax=0.1;
double pmMin=0.01;
static int hash_num = 0;
double findHaspCost(vector<int> &vec)
{ 
	
	if(hashCost.find(vec)!=hashCost.end())
	{
		if(hashCost.find(vec)->first==vec){
			hash_num++;
			return hashCost.find(vec)->second;
		}
		else
			return -1;
	}
	else
		return -1;
}
void mutation(vector<vector<int> > &population_rw,double eval1[],int pop,int bit,int mulationL[])
{
	for(int k=0;k<pop;k++)
	{
		if(eval1[k]<evalEver)
		{
			pm=pmMax-(pmMax-pmMin)*(evalEver-eval1[k])/(evalEver-minpop);
		}
		else
			pm=pmMax;
		if(bit>500)
		{
			if(pm>=myrand())
			{
				int j=int(myrand()*(bit-1));
				while(mulationL[j]==0)
				{
					j=int(myrand()*(bit-1));
				}	
				population_rw[k][j]=1-population_rw[k][j];
			}
		}
		else
		{
			if(pm>=myrand())
			{
				int j=int(myrand()*(bit-1));
				population_rw[k][j]=1-population_rw[k][j];
			}
		}	
	}
}
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
	clock_t start,finish;
	double totaltime;
	start=clock();
	srand((unsigned)time(0));
	Graph g;
	g.createGraph(topo);
	double serverCost=g.serverCost;
	MinCostFlow m;
	m.createMinCostFlow(g);
	vector<int> consumerNetNodes;
	int f=0;
	int run=1;
	int bit=g.nodeNum;
	int pop=30;
	if(bit<250)
		pop=30;
	else if(bit>=250&&bit<500)
		pop=5;
	else
		pop=5;
	double *popCost=new double[pop];
	double *eval1=new double[pop];
	int *bl=new int[bit]();
	int flowNeedAll=0;
	vector<vector<int> > minCostPathPop(1000);
	map<int, int> comFlowNeed;
	double blCost=g.consumers.size()*serverCost;
	for(int i=0;i<(int)g.consumers.size();i++)
	{
		consumerNetNodes.push_back(g.consumers[i].netNode);
		comFlowNeed.insert(make_pair(g.consumers[i].netNode, g.consumers[i].flowNeed));
		bl[g.consumers[i].netNode]=1;
		f+=g.consumers[i].flowNeed;
		flowNeedAll+=g.consumers[i].flowNeed;
		minCostPathPop[i].push_back(g.consumers[i].netNode);
		minCostPathPop[i].push_back(g.consumers[i].consumerNode);
		minCostPathPop[i].push_back(g.consumers[i].flowNeed);
	}
	vector<vector<int> > minCostPath1;
	int m1 = 5000;
	minpop=blCost;
	vector<vector<int>> s = m.initPopulationBySort(g.consumers, consumerNetNodes, comFlowNeed,f, minCostPath1, m1, minpop, serverCost, bit, pop, externalPop, externalCost);
	if(s.size()!=0)
	{
		population=&s;
	}
	else
		(*population).push_back(vector<int>(bl,bl+bit));

	double g1l=double(g.consumerNum)/g.nodeNum;

	/*externalPop.push_back(vector<int>(bl,bl+bit));
	externalCost.push_back(blCost);*/
	int externalPopNum=externalPop.size();
	for(int i=0;i<externalPopNum;i++)
	{
		if(findHaspCost(externalPop[i])==-1)
		{
			hashCost[externalPop[i]]=externalCost[i];
		}
	}
	
	
	int minpopUnchaged=0;
	int sigExte=0;
	int t=0;
	int *mulationL = new int[bit]();
	//为了降低遗传算法决策变量的维度，对于高维的根据先验知识减少一些点。
	if(bit>500)
	{	
		vector<netNodeDu> duSort;
		vector<netNodeDu> duSortLittle;
		netNodeDu nnd;
		for(int i=0;i<bit;i++)
		{
			nnd.netNode=i;
			nnd.num=0;
			duSort.push_back(nnd);
			duSortLittle.push_back(nnd);
		}
		for(int i=0;i<g.consumers.size();i++)
		{
			for(int j=0;j<g.G[g.consumers[i].netNode].size();j++)
			{
				duSort[g.G[g.consumers[i].netNode][j].to].num++;
			}
		}

		for(int i=0;i<g.nodeNum;i++)
		{
			duSortLittle[i].num=g.G[i].size();
		}


		sort(duSort.begin(), duSort.end(), [](const netNodeDu &a, const netNodeDu&b){
			return a.num > b.num;
			});
		sort(duSortLittle.begin(), duSortLittle.end(), [](const netNodeDu &a, const netNodeDu&b){
			return a.num > b.num;
			});
		vector<ConsumerNode> consumerSort = g.consumers ;
			sort(consumerSort.begin(), consumerSort.end(), [](const ConsumerNode &a, const ConsumerNode&b){
				return a.flowNeed > b.flowNeed;
			});
		int iterSortNum = 0;
		for (vector<ConsumerNode>::iterator iter = consumerSort.begin(); iter != consumerSort.end(); iter++)
		{
			if(iterSortNum<consumerSort.size()/3)
			{
				mulationL[(*iter).netNode] = 1;
				iterSortNum++;
			}
			else
				break;
		}
		iterSortNum = 0;
		for (vector<netNodeDu>::iterator iter = duSort.begin(); iter != duSort.end(); iter++)
		{
			if(iterSortNum<consumerSort.size()/2)
			{
				mulationL[(*iter).netNode] = 1;
				iterSortNum++;
			}
			else
				break;
		}
		iterSortNum = 0;
		for (vector<netNodeDu>::iterator iter = duSortLittle.begin(); iter != duSortLittle.end(); iter++)
		{
			if(iterSortNum<10)
			{
				mulationL[(*iter).netNode] = 1;
				iterSortNum++;
			}
			else
				break;
		}
		for(int i=0;i<bit;i++)
		{
			if(externalPop[0][i]!=mulationL[i]&&mulationL[i]==0)
				mulationL[i]=1;
		}
		init_popZG(*population,pop,bit,mulationL);
	}
	else
		init_pop(*population,pop,bit,g1l);

	
	populationTemp=*population;

	while(run<50000)
	{
		vector<vector<int> > servers(1000);
		vector<vector<vector<int> > > minCostPath(1000);
		for(int exNum=0;exNum<externalPopNum&&sigExte==0;exNum++)
		{
			int sji=int(myrand()*(pop-1));
			(*population)[sji]=externalPop[exNum];
		}
		if(minpopUnchaged>700&&bit<250&&sigExte==0)
		{
			sigExte=1;
			*population=populationTemp;
		}
		if(minpopUnchaged>150)
		{
			pmMax=0.4;
			pmMin=0.04;
		}
		crossover(*population,pop,bit);
		mutation(*population,eval1,pop,bit,mulationL);
		pmMax=0.1;
		pmMin=0.01;
		for(int i=0;i<pop;i++)
		{
			double costTemp=findHaspCost((*population)[i]);
			if(costTemp!=-1)
				eval1[i]=costTemp;
			else
			{
				int count=0;
				for(int j=0;j<bit;j++)
				{
					if((*population)[i][j]==1)
					{
						servers[i].push_back(j);
						count++;
					}
				}
				if(count==0)
					eval1[i]=INF;
				else
					eval1[i] = m.multiSourceMinCostFlow(servers[i], consumerNetNodes, comFlowNeed, f, minCostPath[i], m1, blCost, serverCost, count);
				hashCost[(*population)[i]]=eval1[i];
			}		
			finish=clock();
            totaltime=(double)(finish-start)/CLOCKS_PER_SEC;
            if(totaltime>88.5)
			{
                is_break = true;
                break;
            }
		}
		if(is_break){
            break;
        }
		double evalmin=1e30;	
		double sum=0.0;
		for(int i=0;i<pop;i++)
		{
			if(evalmin>eval1[i])
			{
				evalmin=eval1[i];
				t=i;
			}
			sum+=eval1[i];
		}
		evalEver=sum/pop;
		if(evalmin<minpop)
		{
			externalPop.push_back((*population)[t]);
			externalPopNum++;
			minpopUnchaged=0;
			minpop=evalmin;
			minCostPathPop=minCostPath[t];
		}
		else
			minpopUnchaged++;
		cout<<minpop<<endl;
		int *rs=new int[pop];
		//rs=roulette_wheel(eval1,run,pop);
		rs=selectIndiv(eval1,pop);
		int tempNum=0;
		for(int i=0;i<pop;i++)
		{
			if(i<int(pop/4.0))
			{
				(*population_temp)[tempNum++]=(*population)[rs[i]];
				(*population_temp)[tempNum++]=(*population)[rs[i]];
			}
			else if(i>=int(pop/4.0)&&i<=int(pop/4.0*3))
			{
				(*population_temp)[tempNum++]=(*population)[rs[i]];
			}
				
		}
		/*int tempNum=0;
		for(int i=0;i<pop;i++)
		{
			while(rs[i]--)
			{
				(*population_temp)[tempNum++]=(*population)[i];
			}
		}*/

		tempChanged=population;
		population=population_temp;
		population_temp=tempChanged;
		/*if(evalmin>evalEver/6*5)
		{
			mutation(*population,pop,bit);
		}*/
		servers.clear();
		minCostPath.clear();
		delete []rs;
		finish=clock();
		totaltime=(double)(finish-start)/CLOCKS_PER_SEC;
		if(totaltime>88.5)
			break;
		run++;
	}
	delete bl;
	(*population).clear();
	int numPath=0;
	stringstream ss;
	string str;
	for (int i = 0; i<(int)minCostPathPop.size(); i++)
	{
		if(minCostPathPop[i].size()!=0)
			numPath++;
	}
	ss<<numPath;
	str=ss.str();
	str+="\n";
	str+="\n";
	for (int i = 0; i<numPath; i++)
	{
		for(int j=0;j<(int)minCostPathPop[i].size();j++)
		{
			stringstream ss;
			ss<<minCostPathPop[i][j];
			str+=ss.str();
			if(j==(int)minCostPathPop[i].size()-1&&i<numPath-1)
				str+="\n";
			else
				str+=" ";
		}

	}
	minCostPathPop.clear();
	cout<<"最小成本为"<<minpop<<endl;
	cout<<(double)hash_num / (run * pop)<<endl; 
	cout<<run<<endl;
	const char *topo_file=(char*)str.c_str();
	write_result(topo_file, filename);

}
