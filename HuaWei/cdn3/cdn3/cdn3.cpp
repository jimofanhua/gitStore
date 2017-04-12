// cdn.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "deploy.h"
#include "io.h"
#include "lib_time.h"
#include <iostream>
using namespace std;
int main(int argc, char *argv[])
{
    print_time("Begin");
    char *topo[MAX_EDGE_NUM];
    int line_num;

    char *topo_file = "caseC0.txt";

    line_num = read_file(topo, MAX_EDGE_NUM, topo_file);

    printf("line num is :%d \n", line_num);
    if (line_num == 0)
    {
        printf("Please input valid topo file.\n");
        return -1;
    }

    char *result_file = "result.txt";

    deploy_server(topo, line_num, result_file);

    release_buff(topo, line_num);

    print_time("End");

	return 0;
}

