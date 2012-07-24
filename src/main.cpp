#include <cv.h>
#include <highgui.h>

#include <iostream>
#include <sstream>

#include "ImageSequence.h"
#include "CommandLine.h"
#include "usegui.h"
#include "DelayTracker.h"

#ifdef USE_GUI
#include "gui.h"
#endif

#define DEBUG

using namespace std;
using namespace cv;

class Processor : public ys::ImageProcessorInterface {
		public:
			Processor():red(cv::Scalar(0,0,255)),yellow(cv::Scalar(0,255,255)){
				tracker.allocate();
			}
			virtual ~Processor(){
			}
			virtual void init( const cv::Mat &image ){
				gray.create( image.size(), CV_8UC1 );
				cv::cvtColor( image, gray, CV_RGB2GRAY );

				ys::Size size(image.cols, image.rows);
				tracker.init( (unsigned char*)gray.data, size );
			}
			virtual void init( const cv::Mat &image, const cv::Rect &rect ){
#ifdef DEBUG
				cout<< "start init()..." <<flush;
#endif
				gray.create( image.size(), CV_8UC1 );
				cv::cvtColor( image, gray, CV_RGB2GRAY );

				ys::Size size(image.cols, image.rows);
				tracker.init( (unsigned char*)gray.data, size );

				initial_position[0].x = rect.x;
				initial_position[0].y = rect.y;
				initial_position[1].x = rect.x + rect.width;
				initial_position[1].y = rect.y;
				initial_position[2].x = rect.x + rect.width;
				initial_position[2].y = rect.y + rect.height;
				initial_position[3].x = rect.x;
				initial_position[3].y = rect.y + rect.height;
				//cout<< "call setPosition" <<endl;
				//tracker.setPosition( initial_position );
#ifdef DEBUG
				cout<< "finish" <<endl;
#endif
			}

			virtual void stop(){

				tracker.clear();
			}

			//処理内容を記述
			//引数より与えられた画像データは書き換え不可
			//処理結果を画像に書き加える時は、drawImageを使うこと
			virtual void processImage( const cv::Mat &image, const int count ){
				int64 tick = cv::getTickCount();
				cv::cvtColor( image, gray, CV_RGB2GRAY );

				tracker.track( (unsigned char*)gray.data );
				time = (double)( cv::getTickCount() - tick )/cv::getTickFrequency();
			}

			//処理結果を画像に書き加えるためのメソッド
			virtual void drawImage( cv::Mat &image){
#ifdef DEBUG
				cout<< "start draw" <<flush;
#endif

				KLT_Feature *list = tracker._klt_cur_feature_list->feature;
				//KLT_Feature *list = tracker._klt_first_feature_list->feature;
				for( int i=0; i<tracker._feature_num; i++ ){
					KLT_Feature f = list[i];
					if( f->val == KLT_TRACKED )
						circle( image, Point( f->x, f->y ), 2, red );
					else if( f->val > 0 )
						circle( image, Point( f->x, f->y ), 2, yellow );
				}

				if( tracker._state == ys::DelayTracker::TRACK ){
					ys::Square sq = tracker.getPosition();
					Point cv_square[1][4];
					for( int i=0; i<4; i++ ){
						Point &p = cv_square[0][i];
						ys::Point &s = sq[i];
						p.x = s.x;
						p.y = s.y;
#ifdef DEBUG
						cout<< "[" << i << "]" << s.x << "," << s.y <<endl;
#endif
					}
					const Point* points[1] = { cv_square[1] };
					const int points_num[1] = {4};
					cv::polylines( image, points, points_num, 1, true, red, 3 );
				}


				stringstream sstr;
				sstr<< fixed <<time <<flush;

				cv::putText( image, sstr.str(), cv::Point( 0,  image.rows), FONT_HERSHEY_SIMPLEX, 3, red, 3 );

#ifdef DEBUG
				cout<< "...finish" <<endl;
#endif
				
			}
			//処理を終了する条件
			virtual bool stopCriteria(){
				if( tracker._state == ys::DelayTracker::STOPED ){
					tracker.clear();
					return true;
				}
				return false;
			}
			virtual std::string getClassName(){
				return "Processor";
			}
			virtual void keySwitch( int key ){
				if( (char)key == 'p' )
					tracker.setPosition( initial_position );
			}

		private:
			ys::DelayTracker tracker;
			Scalar red, yellow;
			Mat gray;
			ys::Square initial_position;

			double time;
};

int main(int argc, char **argv){

	Processor process;

	ys::ImageSequence seq;
	seq.setCaptureType( (string)"camera" );

	seq.setImageProcessor( &process );

	seq.runGuiMode();

}
