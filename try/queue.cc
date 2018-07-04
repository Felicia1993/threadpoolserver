#include <iostream>
#include <algorithm>
#include <queue>
#include <deque>
#include <stdint.h>

using namespace std;

typedef pair<int, int> P;

struct cmp{
	bool operator()(const P p1, const P p2){
		return p1.second > p2.second;
	}
};

int main(){
	cout<<UINTPTR_MAX<<endl;
	priority_queue<P, deque<P>,cmp> q;
	q.push(P(1,3));
	q.push(P(3,4));
	q.push(P(2,15));
	P p;
	while(!q.empty()){
		p = q.top();
		q.pop();
		cout<<p.first<<" "<<p.second<<endl;
	}
}
