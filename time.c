int getTimer();
int startTimer();


//-----------------------------TIMER-----------------------------

int startTimer(){
    return clock();
}

int getTimer(){
	return sender_timer-(startTime-clock());
}