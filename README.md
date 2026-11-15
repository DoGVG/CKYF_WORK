base:基础部分
advance:选做的进阶部分
500ms周期：我理解的是500ms内完成一次亮灭交替,所以250ms翻转一次电平


对于base:
task1:while循环开始之前，有四次基于hal_delay的闪烁
task2:  启动定时器1中断，250ms翻转电平一次（关于10ms周期选择：最初想尝试依靠tim1的10ms周期，通过分频2实现20 ms消抖，分频25实现电平翻转但消抖失败了，改回了haldelay阻塞式，tim1相关配置懒得更改了）
task3:exti下降沿触发中断，haldelay20消抖（systick优先级1，高于两个中断，不会卡死），开启led默认是闪烁

对于advace:


由于个子任务对单点按钮处理结果不同，所以使用pb0按钮exti中断触发，进行两个任务模式的切换
pa7使用轮询（放exti里开500ms窗口也许有点危险（？），进行led状态切换
tim1 5hz中断进行闪烁

task5:MODE0
task6:MODE1
