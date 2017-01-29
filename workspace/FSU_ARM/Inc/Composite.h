/*
 * Composite.h
 *
 *  Created on: 2016-5-23
 *      Author: lcz
 */

#ifndef _COMPOSITE_H_
#define _COMPOSITE_H_

#include <vector>

using namespace std;

class Component
{
public:
    virtual void Operation()=0;

    virtual void Add(Component*);
    //virtual void Remove(Component*);
    virtual Component* GetChild(int index);
    virtual ~Component();
protected:
    Component();
};

class Semaphore:public Component
{
public:
    virtual void Operation();
    Semaphore();
    ~Semaphore();
};

class SemaBlock:public Component
{
public:
	SemaBlock();
    ~SemaBlock();
    //实现所有接口
    void Operation();
    void Add(Component*);
//    void Remove(Component*);
    Component* GetChild(int index);
private:
    //这里采用vector来保存子组件
    vector<Component*> vec;
};
#endif
