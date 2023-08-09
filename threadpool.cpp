#include "threadpool.h"
#include <functional> //函数对象头文件
#include <thread>
#include <iostream>


/////////////////// 线程池方法的实现
const int TASK_MAX_THRESHHOLD = INT32_MAX;//最大任务数
const int THREAD_MAX_THRESHHOLD= 1024;//最大线程执行数
const int THREAD_MAX_IDLE_TIME = 10;// 线程最大处于空闲的时间

//线程池构造
ThreadPool::ThreadPool()
    :initThreadSize_(0)
    , taskSize_(0)
    , idleThreadSize_(0)
    , curThreadSize_(0)
    , taskQueMaxThreshHold_(TASK_MAX_THRESHHOLD)
    , threadSizeThreshHold_(THREAD_MAX_THRESHHOLD)
    , poolMode_(PoolMode::MODE_FIXED)
    , isPoolRunning_(false)
{
    // std::cout<<taskQue_.size()<<std::endl;
}

//线程池析构
ThreadPool::~ThreadPool()
{   // pool对象到}后执行该析构函数
    isPoolRunning_ = false;
    //notEmpty_.notify_all();//把所有等任务的线程唤醒，开始抢锁-》发现线程池要结束的信息，开始自行析构各种线程
    // 等待线程池里面所有的线程返回 有两种状态：阻塞 & 正在执行任务中
    std::unique_lock<std::mutex> lock(taskQueMtx_);
    notEmpty_.notify_all();//把所有等任务的线程唤醒，开始抢锁-》发现线程池要结束的信息，开始自行析构各种线程
    exitCond_.wait(lock,[&]()->bool {return threads_.size() == 0;});//等其他线程都清空（睡着+等唤醒）

}

// 设置线程池的工作模式
void ThreadPool::setMode(PoolMode mode)
{
    if(checkRunningState())
        return;
    poolMode_=mode;
}


// 设置task任务队列上线阈值
void ThreadPool::setTaskQueMaxThreshHold(int threshhold)
{
    if(checkRunningState())
        return;
    taskQueMaxThreshHold_ = threshhold;
}

void ThreadPool::setThreadSizeThreshHold(int threshhold)
{
    if(checkRunningState())
        return;
    if(poolMode_ == PoolMode::MODE_CACHED)//该模式下才可设置
    {
        threadSizeThreshHold_ = threshhold;
    }
}

// 外部给线程池提交任务  基类为Task的派生任务对象
Result ThreadPool::submitTask(std::shared_ptr<Task> sp)
{
    // 获取锁
    std::unique_lock<std::mutex> lock(taskQueMtx_);
    // 线程的通信 等待任务队列有空余
    //    while(taskQue_.size() == taskQueMaxThreshHold_)
    //    {
    //        notFull_.wait(lock);
    //    }
    // 线程通信等待任务队列有空余   wait(等到条件满足为止)  wait_for(等到时间段完没满足告知结果错误)  wait_until(等到某个时间点告知结果错误)
    // 用户提交任务，你不能用wait让客户老等着，最长不能阻塞超过1s, 否则判断提交任务失败，返回
//    std::cout<<taskQue_.size()<<std::endl;
    if(!notFull_.wait_for(lock,std::chrono::seconds(1),
                         [&]()->bool{return taskQue_.size() < (size_t)taskQueMaxThreshHold_;}))//条件成功，继续执行，否则阻塞返锁)
    {
        //表示notFull_等待1s钟，条件依然没有满足
        std::cerr<<"task queue is full, submit task fail."<<std::endl;
        // return task->getResult(); // Task 里有Result，返回result，不行， 线程执行完task后，task对象被析构掉了
        return Result(sp,false); // 14 改 17 就行？c++17前，这里返回result应该是值赋值给result返回，
    }

    // 如果有空余，把任务放入任务队列中
    taskQue_.emplace(sp);
    taskSize_++;//记录任务数
    // 因为新放了任务，任务队列肯定不空了，在notEmpty_上进行通知，让线程执行任务
    notEmpty_.notify_all();

    // cached模式 任务处理比较紧急 场景：小而快的任务 需要根据任务数量和空闲线程的数量，判断是否需要创建新的线程出来？（空闲线程<任务）
    if(poolMode_ == PoolMode::MODE_CACHED // 判断是cached模式
      && taskSize_ > idleThreadSize_      // 判断任务队列里任务数 > 线程空闲数，空闲线程不够
      && curThreadSize_< threadSizeThreshHold_) // 判断目前运行线程数 < 设置的线程阈值
    {
        std::cout<<" >>> create new thread..."<<std::endl;
        // 创建新线程对象
//        auto ptr=std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc,this));
//        threads_.emplace_back(std::move(ptr));//unique_ptr指针只能指一个该对象，这里通过move转移到形参上接着指
        auto ptr=std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc,this,std::placeholders::_1));
        int threadId = ptr->getId();
        threads_.emplace(threadId, std::move(ptr));
        // 启动线程
        threads_[threadId]->start();
        // 修改线程个数相关的变量
        curThreadSize_++;
        idleThreadSize_++;
    }

    // 返回任务的Result对象
    return Result(sp);
}

// 开启线程池
void ThreadPool::start(int initThreadSize)
{
    // 设置线程池的运行状态
    isPoolRunning_ = true;

    // 记录初始线程个数
    initThreadSize_ = initThreadSize;
    curThreadSize_ = initThreadSize;//当前有多少线程运行

    // 创建线程对象
    for(int i=0;i<initThreadSize_;i++)
    {
        // 创建thread线程对象的时候，把线程函数给到thread线程对象
        //threads_.emplace_back(new Thread(std::bind(&ThreadPool::threadFunc,this)));
        //std::bind函数的作用是将一个可调用对象（如函数、成员函数、函数对象等）与一组参数绑定在一起，
        // 返回一个新的函数对象。这个新的函数对象可以延迟执行，直到后续调用时再进行实际执行。
        // 对上面代码的替代
        auto ptr=std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc,this,std::placeholders::_1));
        int threadId = ptr->getId();
        threads_.emplace(threadId, std::move(ptr));
        //threads_.emplace_back();//unique_ptr指针只能指一个该对象，这里通过move转移到形参上接着指
    }
    //std::cout<<taskQue_.size()<<std::endl;
    // 启动所有线程 std::vector<Thread*> threads_;
    for(int i=0;i<initThreadSize_;i++)
    {
        threads_[i]->start();
        idleThreadSize_++; // 每开启一个线程，一开始都是空闲线程，空闲线程数+1
    }
}

// 定义线程函数
void ThreadPool::threadFunc(int threadid)
{
    auto lastTime = std::chrono::high_resolution_clock().now();//上一次本线程执行完的时间

    // 等所有任务必须执行完成，线程池才可以回收所有线程资源
    //while(isPoolRunning_) // 每个线程函数都在不停的要任务来做
    for(;;)
    {
        std::shared_ptr<Task> task;//多态的父类
        {
            //先获取锁
            std::unique_lock<std::mutex> lock(taskQueMtx_);
            std::cout<<"tid"<<std::this_thread::get_id()
            <<"尝试获取任务..."<<std::endl;

            // cached模式下，有可能已经创建了很多的线程，但是空闲时间超过60s,应该把多余的线程结束回收掉
            // 超过initThreadSize_数量的线程要进行回收
            // 当前时间 - 上一次线程执行的时间 > 60s
            // 锁+双重判断
            //while(isPoolRunning_ && taskQue_.size()==0) 为了让任务可以执行完，isPoolRunning_条件删除
            while(taskQue_.size()==0)// 任务队列没任务，看看是否自己多余了
            {
                if(!isPoolRunning_)//执行完任务，没任务了，进到这里面析构线程
                {
                    threads_.erase(threadid); // 清空线程vector中的对象
                    std::cout<<"threadid:"<<std::this_thread::get_id()<<" exit!"
                             << std::endl;
                    std::cout<<threads_.size()<<std::endl;
                    exitCond_.notify_all();//唤醒主线程pool的析构wait,看是否全走
                    return;
                }

                if(poolMode_ == PoolMode::MODE_CACHED)//cached情况
                {// 每一秒钟返回一次， 怎么区分：超时返回？还是有任务待执行返回
                        // 条件变量，超时返回了
                        if(std::cv_status::timeout ==
                           notEmpty_.wait_for(lock,std::chrono::seconds(1)))
                        { // 超过1s没有拿到任务，被已经有的线程拿了，说明本线程可能多余了，看加上本线程是否超过起始线程，如果多说明本线程闲
                            auto now = std::chrono::high_resolution_clock().now();
                            auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
                            if(dur.count()>=THREAD_MAX_IDLE_TIME
                               && curThreadSize_ > initThreadSize_)//闲了60s且不再起始线程数中
                            {
                                // 开始回收当前线程
                                // 记录线程数量的相关变量的值修改
                                // 把线程对象从线程列表容器中删除 没办法 threadFunc 中找到对应vector的哪一个线程位置-》改用map,创建线程号为key方便查找
                                // threadid => thread对象 => 删除
                                threads_.erase(threadid); // 清空线程vector中的对象
                                curThreadSize_--;//现有线程-1
                                idleThreadSize_--;//空闲线程-1

                                std::cout<<"threadid:"<<std::this_thread::get_id()<<" exit!"
                                         << std::endl;
                                return;
                            }
                        }
                }
                else//fix情况
                {
                    //等待notEmpty条件
                    notEmpty_.wait(lock);//线程队列等不到就一直等任务
                }
                // 线程池要结束，回收线程资源
//                if(!isPoolRunning_)//看唤醒两种情况的是不是要结束主线程的析构
//                {
//                    // 回收当前线程
//                    threads_.erase(threadid); // 清空线程vector中的对象
//                    std::cout<<"threadid:"<<std::this_thread::get_id()<<" exit!"
//                             << std::endl;
//                    std::cout<<threads_.size()<<"*"<<std::endl;
//                    exitCond_.notify_all();//唤醒主线程pool的析构wait,看是否全走完
//                    return;
//                }
            }

            // 有任务的情况下，跳到这里，为了要让任务执行完，跳过isPoolRunning_判断条件
            /*
            // 线程结束，回收资源
            if(!isPoolRunning_)
            {
                break;
            }*/

            idleThreadSize_--;//任务队列有任务，则本线程会处理下面弄到的任务，本线程不再闲，闲数-1
            std::cout<<"tid"<<std::this_thread::get_id()
                     <<"获取任务成功..."<<std::endl;

            // 从任务队列取一个任务出来
            task = taskQue_.front();//子类给父类
            taskQue_.pop();//拿走任务
            taskSize_--;

            // 如果依然有剩余任务，继续通知其它的线程执行任务
            if(taskQue_.size() > 0)
            {
                notEmpty_.notify_all();
            }

            // 取出一个任务，进行通知，通知submitTask可以继续提交生产任务
            notFull_.notify_all();
        }//把锁释放掉，自己执行拿到的任务即可，无需拿着任务队列的锁

        // 当前线程负责执行这个任务
        if(task != nullptr)
        {
            //task->run(); 这只是执行任务，现在把任务结果返回
            task->exec();
        }
        idleThreadSize_++;//本线程处理完取的任务再次闲下来，闲+1
        lastTime = std::chrono::high_resolution_clock().now();// 更新线程执行完任务的时间，本线程开始空闲
    }
    /* 改了以后，为了让其执行完，没有跳出for循环的语句，把下面析构本线程的语句上移到发现任务为0的地方进行析构
    // 析构结束时，在执行的本线程回到循环while发现要析构了，跳出循环到这，开始析构本线程
    threads_.erase(threadid); // 清空线程vector中的对象
    std::cout<<"threadid:"<<std::this_thread::get_id()<<" exit!"
             << std::endl;
    std::cout<<threads_.size()<<std::endl;
    exitCond_.notify_all();//唤醒主线程pool的析构wait,看是否全走
     */
}

// 检查pool的运行状态
bool ThreadPool::checkRunningState() const
{
    return isPoolRunning_;
}

//////////////// 线程方法的实现
// 线程类静态成员变量在类外初始化
int Thread::generateId_=0;

//线程构造
Thread::Thread(ThreadFunc func)//接收一个函数
    :func_(func)
    , threadId_(generateId_++)//给线程对象赋个编号值以区分
{

}
//线程析构
Thread::~Thread()
{

}

// 启动线程
void Thread::start()
{
    // 创建一个线程来执行一个线程函数
    std::thread t(func_, threadId_);// linux中的 pthread_detach 如下
    t.detach();//c++11 中线程对象出了函数}就自动析构了，所以设置分离函数，让其自己归属内核管理不析构继续执行
}

int Thread::getId() const
{
    return threadId_;
}

///////////// Task方法实现
Task::Task()
    : result_(nullptr)
{

}

Task::~Task()
{

}

void Task::exec() //要解决一个问题，即线程执行结果后，给result类对象，而且task对象析构还要保证result对象在
{
    if(result_!= nullptr)
    {
        result_->setVal(run());//调用那个接收对象指针，来接收run的结果
    }
}

void Task::setResult(Result* res)// 传外面存结果的指针
{
    result_=res;
}

/////////////  Result方法的实现
Result::Result(std::shared_ptr<Task> task, bool isValid)
        :task_(task)
        ,isValid_(isValid)
{
    task_->setResult(this);// 把result对象自己穿进去接结果
}

Any Result::get()// task外的接收结果
{
    if(!isValid_)//如果任务提交失败，线程函数返回值无效，直接返回空
    {
        return "";
    }

    sem_.wait(); // task任务如果没有执行完，这里会阻塞用户的线程
    return std::move(any_);
}

void Result::setVal(Any any)//task结束中介result拿到结果，告知用户可通过本result接收了
{
    // 存储task的返回值
    this->any_ = std::move(any);
    sem_.post(); // 已经获取了任务的返回值，增加信号量资源
}