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
	typedef priority_queue<P, deque<P>,cmp> TimeList;
	priority_queue<P, deque<P>,cmp> q;
	q.push(P(1,3));
	q.push(P(3,4));
	q.push(P(2,15));
	P p = P(1,3);

	while(!q.empty()){
		p = q.top();
		q.pop();
		cout<<p.first<<" "<<p.second<<endl;
	}
	/*P sentry = std::make_pair(2,15);
	TimeList::iterator it = q.lower_bound(sentry);
	if(it == q.end()){
		cout<<"something wrong!\n";
	}*/
}
