#include <highgui.h>
#include "gui.h"

using namespace cv;

const int ys::MouseParam::STATE_NONE = -1;
const int ys::MouseParam::STATE_DOWN = 0;
const int ys::MouseParam::STATE_DRAG = 1;
const int ys::MouseParam::STATE_UP = 2;


void ys::mouseCallback( int event, int x, int y, int flags, void *param ){
	ys::MouseParam *mparam = (ys::MouseParam*)param;
	mparam->x = x;
	mparam->y = y;
	mparam->event = event;
	mparam->flags = flags;

//	cout<< "event:" << event << " flag:" <<flags <<endl;

	if( event == EVENT_LBUTTONDOWN ){
//		cout<< "left button down" <<endl;
		mparam->state = ys::MouseParam::STATE_DOWN;
	}
	else if( event == EVENT_MOUSEMOVE &&
			( mparam->state == ys::MouseParam::STATE_DOWN ||
			mparam->state == ys::MouseParam::STATE_DRAG)){
//		cout<< "left button drag" <<endl;
		mparam->state = ys::MouseParam::STATE_DRAG;
	}
	else if( event == EVENT_LBUTTONUP &&
		  flags & EVENT_FLAG_LBUTTON ){
//		cout<< "left button up" <<endl;
		mparam->state = ys::MouseParam::STATE_UP;
	}
	else {
//		cout<< "state none" <<endl;
		mparam->state = ys::MouseParam::STATE_NONE;
	}
}

void ys::initMouseParam2Rect( const ys::MouseParam &mparam, cv::Rect &rect ){
	rect.x = mparam.x;
	rect.y = mparam.y;
	rect.width = 1;
	rect.height = 1;
}
void ys::updateMouseParam2Rect( const ys::MouseParam &mparam, cv::Rect &rect ){

	int tmpx = rect.x;
	int tmpy = rect.y;
	rect.x = min( tmpx, mparam.x );
	rect.y = min( tmpy, mparam.y );
	rect.width = max(mparam.x, tmpx) - rect.x;
	rect.height = max(mparam.y, tmpy) - rect.y;
}

void ys::mouseParam2Rect( const ys::MouseParam &mparam, cv::Rect &rect ){
	if( mparam.state == ys::MouseParam::STATE_DOWN ){
		initMouseParam2Rect( mparam, rect );
	}
	else if( mparam.state == ys::MouseParam::STATE_DRAG
		  || mparam.state == ys::MouseParam::STATE_UP ) {
		updateMouseParam2Rect( mparam, rect );
	}
}
