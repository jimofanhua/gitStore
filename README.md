# 华为软件精英设计大赛 2017
==========================================================================
> 从参加到初赛结束一共坚持了大概半个月，其中10天的时间都在西北赛区前64，但是就在初赛截止前一天，突然莫名的窜出20多个黑马，导致最后无缘入围前64，
与华为校招绿卡也就这样插肩而过。

我们的思路是上层采用遗传算法，下层采用最小费用流作为评价函数。由于最小费用流计算一次几乎需要0.2秒的时间，虽然后期加入了缓存机制，但是提升并没有特别明显，
特别是在高级测试用例上，遗传算法还是跑不了多少代。然而前期的重点都放在了遗传算法的改进上，觉着最小费用流已经是非常经典的算法了，没有改进的空间了，哎，失败就失败
在了这个地方，后来才发现大家几乎都改用了ZKW（基于最小费用流的改进算法），比赛结束后尝试用了下ZKW，发现速度确实提升了不少，可惜已经被淘汰，遗憾而归!

不过我们算法中的初始化种群策略还是比较值得借鉴的，采用了一些小的技巧，只需用差不多1秒的时间就可以求得一个比暴力好很多的解。


