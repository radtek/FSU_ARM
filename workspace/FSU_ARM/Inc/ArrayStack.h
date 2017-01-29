/*
 * ArrayStack.h
 *
 *  Created on: 2016-7-3
 *      Author: lcz
 */

#ifndef ARRAYSTACK_H_
#define ARRAYSTACK_H_

template<class T, int SIZE>
class ArrayStack {
public:
	ArrayStack() {	top = -1;	}//缺省构造函数,构造一个空堆栈
	~ArrayStack() {}; //析构函数
	void setEmpty(); //置空堆栈
	bool isEmpty(); //判断堆栈是否为空
	bool push(T element); //入栈
	bool pop(T& element); //出栈
private:
	T Buffer[SIZE];
	int top;
};
template <class T, int SIZE>
void ArrayStack<T, SIZE>:: setEmpty ()
{
	top= -1; //将栈顶指针赋 -1,并不实际清除数组元素
}
template<class T, int SIZE>
bool ArrayStack<T, SIZE>::isEmpty() {
	return (top == -1);
}
template<class T, int SIZE>
bool ArrayStack<T, SIZE>::push(T element) {
	top++;
	if (top > SIZE - 1) {
		top--;
		return false; //堆栈已满,不能执行入栈操作
	}
	Buffer[top] = element;
	return true;
}
template<class T, int SIZE>
bool ArrayStack<T, SIZE>::pop(T& element) {
	if (isEmpty())
		return false;
	element = Buffer[top];
	top--;
	return true;
}

#endif /* ARRAYSTACK_H_ */
