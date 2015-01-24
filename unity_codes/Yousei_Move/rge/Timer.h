/*
 *  Timer.h
 *  rgeSample
 *
 *  Created by Yuhei Akamine on 09/07/29.
 *  Copyright 2009 Yuhei Akamine. All rights reserved.
 *
 */
#pragma once 

#include <sys/time.h>

namespace rge {
	
//! 経過時間を得る
class Timer
{
public:
	Timer() {
		mPrevTime.tv_sec = 0;
		mPrevTime.tv_usec= 0;
	}
	
	double elapsedTime() { //!<前回呼び出し時からの経過時間（秒）を得る		
		timeval now;
		timeval prev = mPrevTime;
		//	timezone zone;
		gettimeofday(&now, 0);

		if(mPrevTime.tv_usec == 0 && mPrevTime.tv_sec == 0) {
			mPrevTime = now;
			return 0;
		}
		
		double d1 = now.tv_sec * 1000000 + now.tv_usec;
		double d2 = prev.tv_sec * 1000000 + prev.tv_usec;
		mPrevTime = now;
		
		double delta = (d1-d2)/1000000 ;
		return delta == 0.0 ? 0.0000000001 : delta; 		
	}
	
private:
	struct timeval mPrevTime;
};

}