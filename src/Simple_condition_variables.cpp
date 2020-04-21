//============================================================================
// Name        : Simple_condition_variables.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Must have c++-11 or later support for the threads, mutexes and
// 	             condition variables
//============================================================================

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;

mutex m;
int balance = 0;
bool deposit_made = false;
bool transactionsFinished = false;

//void withdraw(int amount){
//	//the bad idea, a busy wait
//	lock_guard<mutex> lck(m);	//DEADLOCK maybe move this after following? unproteceted bool then
//	while (!deposit_made){}		//waiting for deposit made here, PEGS 1 CORE at 100% (see htop)
//								//while holding the mutex
//
//	balance-=amount;
//	if (balance <0)
//		cout<<"You are overdrawn"<<endl;
//	else
//		cout<<"no worries"<<endl;
//}
//
//void deposit(int amount){
//	//deposit is delayed!
//	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
//
//	lock_guard<mutex> lck(m);	//waiting for mutex
//	balance +=amount;
//	deposit_made=true;			//never get here
//}

condition_variable cv;
void withdraw(int amount){
	//just like a lock_guard
	//PLUS you can manually unlock it!
	unique_lock<mutex> lck(m);

	//MUST be a loop to handle
	//spurious wakeups
	while(!deposit_made)
		cv.wait(lck);	//if here, release lock
						//and then sleep until
						//awakened
	balance-=amount;
	if (balance <0)
		cout<<"You are overdrawn"<<endl;
	else{
		transactionsFinished = true;
		cv.notify_all();
		cout<<"withdraw made"<<endl;
	}
}

void deposit(int amount){
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	{
		lock_guard<mutex> lck(m);
		balance +=amount;

		//must change condition while locked!
		deposit_made=true;
	}

	//must release lock before notify
	//notify_all wakes ALL threads
	//waiting on this cv.  One will
	//aquire the mutex, check condition
	//in while and move forward
	//the others go back to sleep
	cv.notify_all();
}
void checkAmount(){
	unique_lock<mutex> lck(m);
	while(!transactionsFinished){
		cv.wait(lck);
	}
	cout<<"Your Amount Is: "<<balance<<endl;
}

int main() {
	//how do you ensure you deposit before withdraw?
	thread t4(checkAmount);
	thread t3(withdraw,30);
	thread t1(deposit,100);
	thread t2(withdraw,50);


	t4.join();
	t3.join();
	t1.join();
	t2.join();

	return 0;
}
