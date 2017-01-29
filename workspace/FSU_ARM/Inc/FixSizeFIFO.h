/*
 * FixSizeFIFO.h
 *
 *  Created on: 2016-7-4
 *      Author: lcz
 */

#ifndef FIXSIZEFIFO_H_
#define FIXSIZEFIFO_H_

template <typename T, int SIZE>
class FixSizeFIFO {	// 高频告警
public:
	FixSizeFIFO() : front(-1), rear(-1), count(0) {}
	~FixSizeFIFO() {}
	T & first();
	T & end();
	void addNew(T t);
	bool isFull() const;
	int Count() const;
	void reset();
public:
private:
	bool isEmpty() const;
	int loop(int pos); //to implement circluar queue using circular array.
private:
	int front;
	int rear;
	int count;
	T arDat[SIZE];
};
template <typename T, int N>
inline void FixSizeFIFO<T, N>::reset() {
	front = rear = count = 0;
}
template <typename T, int N>
inline T & FixSizeFIFO<T, N>::first() {
	return arDat[front];
}

template <typename T, int N>
inline T & FixSizeFIFO<T, N>::end() {
	return arDat[rear];
}

template <typename T, int N>
inline bool FixSizeFIFO<T, N>::isFull() const {
	return N == count;
}
template <typename T, int N>
inline int FixSizeFIFO<T, N>::Count() const {
	return count;
}

template <typename T, int N>
inline bool FixSizeFIFO<T, N>::isEmpty() const {
	return 0 == count;
}

template <typename T, int N>
inline int FixSizeFIFO<T, N>::loop(int pos) {
	if (++pos == N)
		pos = 0;
	return pos;
}

template <typename T, int N>
inline void FixSizeFIFO<T, N>::addNew(T data) {
	if (isEmpty()) {
		rear = loop(rear);
		front = loop(front);
		count = 1;
	} else {
		if (!isFull()) {
			++count;
			rear = loop(rear);
		} else { // full
			rear = loop(rear);
			front = loop(front);
		}
	}
	arDat[rear] = data;
}



#endif /* FIXSIZEFIFO_H_ */
