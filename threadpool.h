#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <thread>
#include <queue>
#include <memory> // 智能指针-共享指针
#include <atomic> // 原子操作
#include <mutex>
#include <condition_variable> //条件变量头文件
#include <functional>
#include <unordered_map>
#include <thread>

// Any类型：可以接收任意数据的类型
class Any
{
public:
    Any()=default;
    ~Any()=default;
    Any(const Any&)=delete;
    Any& operator=(const Any&)=delete;
    Any(Any&&) = default;
    Any& operator=(Any&&)=default;

    template<typename T>//Any(T data):base_(new Derive<T>(data)){}
    Any(T data):base_(std::make_unique<Derive<T>>(data))
    {}

    template<typename T>
    T cast_()
    {
        //我们怎么从base_找到它所指向的Derive对象，从它里面取出data成员变量
        //基类指针 =》派生类指针  四种类型强转 可识别的 RTTI
        Derive<T> *pd = dynamic_cast<Derive<T>*>(base_.get());
        if(pd == nullptr)// 如果T传进来的和本身的不对，返回指针不对，报异常
        {
            throw "type is unmatch!";
        }
        return pd->data_;
    }
private:
    // 基类类型
    class Base
    {
    public:
        virtual ~Base() = default;// default={}
    };

    // 派生类类型
    template<typename T>
    class Derive:public Base
    {
    public:
        Derive(T data):data_(data)
        {}
        T data_;
    };

    // 定义一个基(父)类的指针
    std::unique_ptr<Base> base_;
};

// 实现一个信号量类
class Semaphore
{
public:
    Semaphore(int limit=0)
        :resLimit_(limit)
        ,isExit_(false)
    {}

    //~Semaphore()=default;
    ~Semaphore()
    {
        isExit_=true;
    }

    //获取一个信号量资源
    void wait()
    {
        if(isExit_)
            return;
        // 用条件变量实现信号量
        std::unique_lock<std::mutex> lock(mtx_);
        // 等待信号量有资源，没有资源的话，会阻塞当前线程
        cond_.wait(lock,[&]()->bool{return resLimit_>0;});
        resLimit_--;
    }
    // 增加一个信号量资源
    void post()
    {
        if(isExit_)
            return;
        std::unique_lock<std::mutex> lock(mtx_);
        resLimit_++;
        cond_.notify_all();//通知其他线程
    }
private:
    std::atomic_bool isExit_;// linux和windows下的不同，在linux下的析构函数
    int resLimit_;// 信号量多少个为满
    std::mutex mtx_;
    std::condition_variable cond_;
};

// Task类的提前声明
class Task;

// 实现接收提交到线程池的task任务执行完成后的返回值类型Result
class Result
{
public:
    Result(std::shared_ptr<Task> task, bool isValid=true);
    ~Result()=default;

    // 问题一：setVal方法，如何获取任务执行完的返回值
    void setVal(Any any);
    // 问题二：get方法，用户调用这个方法获取task的返回值
    Any get();
private:
    Any any_; // 存储任务的返回值
    Semaphore sem_; //线程通信信号量
    std::shared_ptr<Task> task_;// 用该只能指针指着的Task对象，不会提前析构掉，因为还有这里指着
    std::atomic_bool isValid_;//如果任务提交失败，后面结果需要知道该情况以确定是否阻塞等待线程结果
};

// 任务抽象基类
class Task
{
public:
    Task();
    ~Task();

    void exec();// 通过对run进行封装，来
    void setResult(Result* res);// 通过传入一个Result类对象指针，来接收run的结果
    // y用户可以自定义任务类型，从Task继承，重写run方法，实现各种任务处理（多态）
    //virtual void run()=0;
    virtual Any run()=0;
private:
    Result* result_;// 用于线程执行完存该线程任务的结果
};

// 线程池支持模式
enum class PoolMode
{
    MODE_FIXED, // 线程固定数量模式
    MODE_CACHED, // 线程数量可动态增长模式
};

//enum PoolMode2 如果枚举名不同，枚举值名相同，直接使用下面两个值不知道用的是PoolMode1还是2
//{            // c++ 新标准 改为 enum class xxx,加类名域即可区分
//    MODE_FIXED, // 线程固定数量模式
//    MODE_CACHED, // 线程数量可动态增长模式
//};

// 线程类型
class Thread
{
public:
    using ThreadFunc=std::function<void(int)>;//定义一个返回void的函数对象
    //线程构造
    Thread(ThreadFunc func);//接收一个函数
    //线程析构
    ~Thread();
    //启动线程
    void start();
    // 获取线程id
    int getId() const;
private:
    ThreadFunc func_;
    static int generateId_;
    int threadId_; // 保存线程id,用于在线程函数回收自己时，搞清自己在线程vector容器的位置
};

/*
example:
ThreadPool pool;
pool.start(4);

class MyTask : public Task
 {
   public:
     void run(){//自己要执行的线程代码}
 };
                       # 把指针和分配的内存放一起，防止不能释放
 pool.submitTask(std::make_shared<MyTask>());
*/

// 线程池类型
class ThreadPool
{
public:
    // 线程池 构造和析构
    ThreadPool();
    ~ThreadPool();

    // 设置线程池的工作模式
    void setMode(PoolMode mode);

    // 设置task任务队列上线阈值
    void setTaskQueMaxThreshHold(int threshhold);

    // 设置线程池cached模式下线程阈值
    void setThreadSizeThreshHold(int threshhold);

    // 给线程池提交任务
    Result submitTask(std::shared_ptr<Task> sp);

    // 开启线程池
    void start(int initThreadSize = std::thread::hardware_concurrency());//默认构造同核心数量的线程数

    // 禁止拷贝构造和赋值构造
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&)=delete;
private:
    // 定义线程函数
    void threadFunc(int threadid);//传入线程号参数

    // 检查pool的运行状态
    bool checkRunningState() const;

private:
    //std::vector<Thread*> threads_; //线程列表 使用智能指针析构如下
    //std::vector<std::unique_ptr<Thread>> threads_; //线程列表 改成map如下 线程号+线程
    std::unordered_map<int, std::unique_ptr<Thread>> threads_;
    size_t initThreadSize_; //初始的线程数量
    int threadSizeThreshHold_; // 线程数量上限阈值
    std::atomic_int curThreadSize_; // 记录当前线程池里面线程的总数量
    std::atomic_int idleThreadSize_; // 记录空闲线程的数量 用以判断：任务多线程固定不够，加；任务少线程多了，减

    std::queue<std::shared_ptr<Task>> taskQue_;//任务队列 用智能指针的共享拉长其生命周期//run()完再自动释放
    std::atomic_int taskSize_; // 由于多线程接任务-1，用户加任务+1,要确保线程任务数字安全
    int taskQueMaxThreshHold_; // 任务队列数量上限阈值

    std::mutex taskQueMtx_;//保证任务队列的线程安全
    // 条件变量
    std::condition_variable notFull_;//用户条件变量 不满可加任务
    std::condition_variable notEmpty_;//线程列表条件变量 不空可执行线程
    std::condition_variable exitCond_;// 等到线程资源全部回收 用以沟通多线程和主线程 多线程全结束主线程再结束

    PoolMode poolMode_;// 当前线程池的工作模式
    std::atomic_bool isPoolRunning_; // 表示当前线程池的启动状态 保证set各种池属性在启动前

};

#endif //THREADPOOL_THREADPOOL_H
