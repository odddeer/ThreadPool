#include <iostream>
#include <chrono>
#include <thread>
using namespace std;

#include "threadpool.h"

/*
 有些场景，是希望能够获取线程执行任务得到返回值
 举例：
 1 + ... + 30000的和
 thread1 1+...+10000
 thread2 10001+...+20000
 thread3 20001+...+30000

 main thread: 给每一个线程分配计算的区间，并等待他们算完返回结果，合并最终的结果即可

*/

using uLong = unsigned long long;

class MyTask : public Task
{
public:
    MyTask(int begin, int end)
        : begin_(begin)
        , end_(end)
    {}
    // 问题一：怎么设计run函数的返回值，可以表示任意的类型 //这里run是虚函数的实现，不能把父类搞成模板
    Any run()
    {
        std::cout<<"tid:"<<std::this_thread::get_id()
                 <<"begin!"<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));//每个任务至少3s
        uLong sum=0;
        for(uLong i=begin_; i<=end_;i++)
            sum+=i;
        std::cout<<"tid:"<<std::this_thread::get_id()
                 <<"end!"<<std::endl;
        return sum;
    }
private:
    int begin_;
    int end_;
};

int main()
{
    {
        ThreadPool pool;
        // 开始启动线程池
        pool.start(2);//设置初始4个线程
        Result res1=pool.submitTask(std::make_shared<MyTask>(1,100000000));
        Result res2=pool.submitTask(std::make_shared<MyTask>(100000001,200000000));
        pool.submitTask(std::make_shared<MyTask>(100000001,200000000));
        pool.submitTask(std::make_shared<MyTask>(100000001,200000000));
        pool.submitTask(std::make_shared<MyTask>(100000001,200000000));
//        uLong sum1 = res1.get().cast_<uLong>(); 在没有等待执行结果的基础上，如果任务执行时间有点长，到出作用域}
//                                                还没结束，将会导致任务在while都没进的情况下直接结束，没做任务
//                                                如何实现做完任务再析构呢？
//        cout<<sum1<<endl;
    }
    cout<<"main over!"<<endl;
#if 0
    // 问题：ThreadPool 对象析构以后，怎么样把线程池相关的线程资源全部回收？
    {
        ThreadPool pool;// 池对象出了{}析构掉，但是线程还在等条件变量notempty，notfull等等
        // 用户自己设置线程池的工作模式
        pool.setMode(PoolMode::MODE_CACHED);
        // 开启线程池
        pool.start(4);//保证设置属性在start之前，通过start把运行转态设为true，外包一个函数在set中，运行就返回避免接着运行

        //

        // 如何设计这里的Result机制呢
        Result res1=pool.submitTask(std::make_shared<MyTask>(1,100000000));
        Result res2=pool.submitTask(std::make_shared<MyTask>(100000001,200000000));
        Result res3=pool.submitTask(std::make_shared<MyTask>(200000001,300000000));
        pool.submitTask(std::make_shared<MyTask>(200000001,300000000));
        pool.submitTask(std::make_shared<MyTask>(200000001,300000000));
        pool.submitTask(std::make_shared<MyTask>(200000001,300000000));//6个任务，初始4个线程，阈值线程10，会开新线程

        uLong sum1 = res1.get().cast_<uLong>(); // get返回了一个Any类型，怎么转成具体的类型呢？
        uLong sum2 = res2.get().cast_<uLong>();
        uLong sum3 = res3.get().cast_<uLong>();

        // Master - Slave线程模型
        // Master线程用来分解任务，然后给各个Slave线程分配任务
        // 等待各个Slave线程执行完任务，返回结果
        // Master线程合并各个任务结果，输出
        cout<<(sum1+sum2+sum3)<<endl;
    }

//    pool.submitTask(std::make_shared<MyTask>(1,10000));
//    pool.submitTask(std::make_shared<MyTask>(10001,20000));
//    pool.submitTask(std::make_shared<MyTask>(20001,30000));


    //
    //std::this_thread::sleep_for(std::chrono::seconds(5));//主线程别停止，等5s看输出
    getchar();
#endif
    return 0;
}