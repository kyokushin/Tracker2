#ifndef DelayTracker_h
#define DelayTracker_h

#include <assert.h>
#include <cmath>
#include <vector>
#include "klt.h"
#include <iostream>

#define PREDICT_ANGLE_USE_BEFORE_TRACK

namespace ys {
	class Point {
		public:
			int x,y;
			friend std::ostream& operator<<(std::ostream &os, const Point &p){
				os<<"[" << p.x << "," << p.y << "]";
				return os;
			}
	};

	class Square {
		public:
			Square(){};
			Square( const Square &src ){
				for( int i=0; i<4; i++ ){
					p[i] = src[i];
				}
			}
			Point &operator[]( int i ){
				assert( 0<=i && i<4 );
				return p[i];
			}
			const Point &operator[]( int i ) const {
				assert( 0<=i && i<4 );
				return p[i];
			}
			bool include( const Point &src ) const {
				return include( src.x, src.y );
			}
			bool include( int x, int y ) const {
				double total_angle = 0.0;
				for( int i=0; i<3; ++i ){
					int p1x = p[i].x - x;
					int p1y = p[i].y - y;
					int p2x = p[i+1].x - x;
					int p2y = p[i+1].y - y;

					double len = sqrt(( p1x * p1x + p1y * p1y )*( p2x * p2x + p2y * p2y ));
					//cos
					double x = ( p1x * p2x + p1y * p2y ) / len;
					//sin
					double y = ( p1x * p2y - p1y * p2x ) / len;
					total_angle += std::atan2( y, x);
				}
				{
					int p1x = p[3].x - x;
					int p1y = p[3].y - y;
					int p2x = p[0].x - x;
					int p2y = p[0].y - y;

					double len = sqrt(( p1x * p1x + p1y * p1y )*( p2x * p2x + p2y * p2y ));
					//cos
					double x = ( p1x * p2x + p1y * p2y ) / len;
					//sin
					double y = ( p1x * p2y - p1y * p2x ) / len;
					total_angle += std::atan2( y, x);
				}

				std::cout<< "total angle" << total_angle <<std::flush;
				if( 2*M_PI - 0.01 <= total_angle
						&& total_angle <= 2*M_PI + 0.01 )
					return true;

				return false;
			}
			friend std::ostream& operator<<(std::ostream &os, const Square &sq){
				for( int i=0; i<3; i++ )
					os<< sq[i] << ",";
				os<< sq[3];
				return os;
			}
		private:
			Point p[4];
	};

	class Size {
		public:
			Size(){}
			Size( int width, int height )
				:width(width),height(height){}
			int width;
			int height;
	};

	class DelayTracker {

		public:
			DelayTracker();
			~DelayTracker();

			//メモリの確保
			void allocate();
			//メモリを開放し、初期状態に。
			void release();

			//初期設定
			void init( const unsigned char *data, const Size &size );
			//状態をクリア。メモリの開放は行わない。
			void clear();

			//トラッキングを1回行う
			bool track( const unsigned char *data );
			//オブジェクトの位置を指定
			void setPosition( const Square &sq );

			//現在の状態
			int getState();
			//オブジェクトの位置を四角形で返す
			Square &getPosition();
			//初期状態からの移動量、スケール変化、回転を求める
			int predictMoveScaleAngle();
			//移動量、スケール変化、回転から現在位置を求める。
			void predictPosition( Square &position );

			enum State {
				NOT_ALLOCATE = -2,
				NOT_INIT = -1,
				REST = 0,
				RECORD = 1,
				TRACK = 2,
				STOPED = 3
			};

			KLT_TrackingContext _klt_context;
			KLT_FeatureList _klt_cur_feature_list;
			KLT_FeatureList _klt_first_feature_list;
#ifdef PREDICT_ANGLE_USE_BEFORE_TRACK
			KLT_FeatureList _klt_before_feature_list;
#endif
			//オブジェクト上にある特徴点かどうかのフラグ
			std::vector<int> _in_object_flags;
			State _state;

			//オブジェクトの初期位置
			Square _initial_position;
			//オブジェクトの現在位置
			Square _current_position;
			//オブジェクトの初期状態からの移動量など
			Point _move;
			double _scale, _angle;

			unsigned char *_before_image;
			Size _image_size;

			//KLTTrackerのパラメータ
			//特徴点数
			int _feature_num;

		private:
			//コピー禁止
			DelayTracker(const DelayTracker &src){}
			DelayTracker &operator=( const DelayTracker &src ){}
	};
};

#endif
