#ifndef TASK_QUEUE_H_
#define TASK_QUEUE_H_

#include <list>

namespace MSThread{

template<typename TaskT>
class TaskQueue{
public:
    TaskQueue(){}
    ~TaskQueue(){}

private:
    TaskQueue( const TaskQueue &other);
    TaskQueue& operator=( const TaskQueue &other );

public:
    void push( const TaskT &task )
    {
        m_tasks.push_back( task );
    }

    void push_front( const TaskT &task )
    {
        m_tasks.push_front( task );
    }
    
    bool pop( TaskT &task )
    {
        if( !m_tasks.empty() ){
            task = m_tasks.front();
            m_tasks.pop_front();
            return true;
        }

        return false;
    }

    bool empty()
    {
        return m_tasks.empty();
    }

    int size()
    {
        return m_tasks.size();
    }

private:
    std::list<TaskT> m_tasks;
};

}

#endif
