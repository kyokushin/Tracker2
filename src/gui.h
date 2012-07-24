#ifndef gui_h
#define gui_h

namespace ys{

	struct MouseParam {
		MouseParam(){
			state = STATE_NONE;
		}

		int x;
		int y;
		int event;
		int flags;
		int state;

		static const int STATE_NONE;
		static const int STATE_DOWN;
		static const int STATE_UP;
		static const int STATE_DRAG;
	};

	void mouseCallback( int event, int x, int y, int flags, void *param );
	void initMouseParam2Rect( const ys::MouseParam &mparam, cv::Rect &rect );
	void updateMouseParam2Rect( const ys::MouseParam &mparam, cv::Rect &rect );
	void mouseParam2Rect( const ys::MouseParam &mparam, cv::Rect &rect );
}
#endif
