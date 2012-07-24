#include <DelayTracker.h>
#include "median.h"

//#define DEBUG
#ifdef DEBUG
#include <iostream>
#endif

ys::DelayTracker::DelayTracker()
	:_klt_context(NULL)
	,_klt_cur_feature_list(NULL)
	,_klt_first_feature_list(NULL)
	,_in_object_flags(NULL),_state(NOT_ALLOCATE)
	 ,_feature_num(100),_before_image(NULL)
{ } 

ys::DelayTracker::~DelayTracker()
{
	release();
}

void ys::DelayTracker::allocate()
{
	assert( _state == NOT_ALLOCATE );

	_klt_context = KLTCreateTrackingContext();
	_klt_cur_feature_list = KLTCreateFeatureList(_feature_num);
	_klt_first_feature_list = KLTCreateFeatureList(_feature_num);

	_state = NOT_INIT;
}

void ys::DelayTracker::init( const unsigned char *data,const Size &size )
{
	assert( _state == NOT_INIT );

	_klt_context->sequentialMode = TRUE;
	KLTSetVerbosity(0);

	_image_size = size;
	int area = size.width * size.height;
	_before_image = new unsigned char[area];
	for( int i=0; i<area; i++ )
		_before_image[i] = data[i];

	KLTSelectGoodFeatures( _klt_context, (KLT_PixelType*)_before_image, size.width, size.height, _klt_cur_feature_list );

	for( int i=0; i<_feature_num; i++ ){
		*(_klt_first_feature_list->feature[i]) = *(_klt_cur_feature_list->feature[i]);
		
	}

	_state = REST;
}

void ys::DelayTracker::release()
{
	assert( _state != NOT_ALLOCATE );

	if( _klt_context != NULL ){
		KLTFreeTrackingContext( _klt_context );
		_klt_context = NULL;
	}
	if( _klt_cur_feature_list != NULL ){
		KLTFreeFeatureList( _klt_cur_feature_list );
		_klt_cur_feature_list = NULL;
	}
	if( _klt_first_feature_list != NULL ){
		KLTFreeFeatureList( _klt_first_feature_list );
		_klt_first_feature_list = NULL;
	}
	if( _before_image != NULL ){
		delete[] _before_image;
		_before_image = NULL;
	}

	_state = NOT_ALLOCATE;
}

void ys::DelayTracker::clear(){
	_in_object_flags.clear();
	for( int i=0; i<4; i++ ){
		_current_position[i].x = 0;
		_current_position[i].y = 0;
	}

	_state = NOT_INIT;
}

bool ys::DelayTracker::track( const unsigned char *data )
{
	assert( _state == REST || _state == TRACK );
	
	KLTTrackFeatures( _klt_context,
			(KLT_PixelType*)_before_image,
			(KLT_PixelType*)data,
		   	_image_size.width, _image_size.height,
			_klt_cur_feature_list);

	//オブジェクトの領域が指定されたら実行する部分
	if( _state == TRACK ){
		int track_num = 0;
		KLT_Feature *features = _klt_cur_feature_list->feature;

		for( int i=0; i<_in_object_flags.size(); i++ ){
			if( features[_in_object_flags[i]]->val == KLT_TRACKED )
				track_num++;
		}

		if( track_num < 4 ){
			_state = STOPED;
			return false;
		}
		predictMoveScaleAngle();
#ifdef DEBUG
		std::cout<< "move:" << _move.x << "," << _move.y
			<< " scale:" << _scale
			<< " angle:" << _angle
			<<std::endl;
#endif
		return true;
	}
	if( _state == RECORD ){
		int track_num = 0;
		KLT_Feature *features = _klt_cur_feature_list->feature;

		for( int i=0; i<_feature_num; i++ ){
			if( features[i]->val == KLT_TRACKED )
				track_num++;
		}

		if( track_num < 4 ){
			_state = STOPED;
			return false;
		}
		return true;
	}


}

void ys::DelayTracker::setPosition( const Square &sq )
{
	//特徴点抽出されている必要がある。
	if( _state != REST && _state != RECORD ){
		return;
	}
	_initial_position = sq;

	KLT_Feature *list = _klt_first_feature_list->feature;
	for( int i=0; i<_feature_num; i++ ){

		KLT_Feature f = list[i];
#ifdef DEBUG
		std::cout<< "square" << sq <<  "Point:" << f->x << "," << f->y <<std::flush;
#endif
		if( sq.include( f->x, f->y ) ){
			_in_object_flags.push_back(i);
#ifdef DEBUG
			std::cout<< ":in" <<std::endl;
		}
		else {
			std::cout<< ":out" <<std::endl;
#endif
		}

	}

	if( _in_object_flags.empty() ){
#ifdef DEBUG
		std::cout<< "empty flags" <<std::endl;
#endif
		_state = STOPED;
	}
#ifdef DEBUG
	std::cout<< "flags = " << _in_object_flags.size() <<std::endl;
#endif

	_state = TRACK;
}

int ys::DelayTracker::getState()
{
	return _state;
}

ys::Square &ys::DelayTracker::getPosition()
{
	predictPosition( _current_position );

	return _current_position;
}

int ys::DelayTracker::predictMoveScaleAngle()
{

	assert( _klt_cur_feature_list->nFeatures
			== _klt_first_feature_list->nFeatures );

	int n = _in_object_flags.size();

	float *scales = new float[n*(n-1)/2];
	float *angles = new float[n];
	int *ofx = new int[n];
	int *ofy = new int[n];

	int count_s = 0;
	int count_a = 0;
	int count_m = 0;

	KLT_Feature *first_feature = _klt_first_feature_list->feature;
	KLT_Feature *cur_feature = _klt_cur_feature_list->feature;

	for(int i=0; i<n; i++){
		int index_i = _in_object_flags[i];
#ifdef DEBUG
		std::cout<< "track index=" << index_i <<std::flush;
#endif 
		KLT_Feature f0i = first_feature[index_i];
		KLT_Feature f2i = cur_feature[index_i];

		if( f0i->val < KLT_TRACKED
				|| f2i->val != KLT_TRACKED)
			continue;//トラックできたscaleだけ分かれば良い

		ofx[count_m] = f2i->x - f0i->x;
		ofy[count_m] = f2i->y - f0i->y;
		count_m++;


		for( int j=0; j<i; j++ ){
			int index_j = _in_object_flags[j];
			KLT_Feature f0j = first_feature[index_j];
			KLT_Feature f2j = cur_feature[index_j];

			if( f0j->val < KLT_TRACKED
					|| f2j->val != KLT_TRACKED)
				continue;//トラックできたscaleだけ分かれば良い

			/******************
			 * スケールの計算 *
			 ******************/

			//2点間の距離を計算
			int sub0_x = f0i->x - f0j->x;
			int sub0_y = f0i->y - f0j->y;
			float distance0 = sqrt(sub0_x * sub0_x + sub0_y * sub0_y);

			int sub2_x = f2i->x - f2j->x;
			int sub2_y = f2i->y - f2j->y;
			float distance2 = sqrt(sub2_x * sub2_x + sub2_y * sub2_y);

			//スケールを計算
			scales[ count_s ] = distance2 / distance0;
			count_s++;

		}

	}

	_scale = median<float>( scales, count_s );
	_move.x = median<int>( ofx, count_m );
	_move.y = median<int>( ofy, count_m );

	/**************
	 * 角度の計算 *
	 **************/
	//領域の中心を計算
	//角度の計算に使う
	int center_x = 0;
	int center_y = 0;

	for( int i=0; i<4; i++ ){
		center_x += _initial_position[i].x;
		center_y += _initial_position[i].y;
	}
	center_x = center_x/4;
	center_y = center_y/4;

	for(int i=0; i<n; i++){
		int index_i = _in_object_flags[i];
#ifdef DEBUG
		std::cout<< "track index=" << index_i <<std::flush;
#endif 
		KLT_Feature f0i = first_feature[index_i];
		KLT_Feature f2i = cur_feature[index_i];

		float sub0_x = f0i->x - center_x;
		float sub0_y = f0i->y - center_y;
		float sub2_x = f2i->x - center_x - _move.x;
		float sub2_y = f2i->y - center_y - _move.y;

		double len = sqrt(
				(sub0_x*sub0_x + sub0_y*sub0_y)
				*(sub2_x*sub2_x + sub2_y*sub2_y)
				);
		//cos
		double x = ( sub0_x * sub2_x + sub0_y * sub2_y ) / len;
		//sin
		double y = ( sub0_x * sub2_y - sub0_y * sub2_x ) / len;
		angles[count_a] = std::atan2( y, x );
		count_a++;
	}
	_angle = median<float>( angles, count_a );
//#ifdef DEBUG
	std::cout<< "scale:" << _scale
		<< ",angle:" << _angle
		<< ",move" << _move <<std::endl;
//#endif
#ifdef DEBUG
	std::cout<< "***scale array[" << count_s << "]" <<std::flush;
	for( int i=0; i<count_s; i++ )
		std::cout<< "[" << i <<  "]" << scales[i] <<std::flush;
	std::cout<< "***angle array[" << count_a << "]" <<std::flush;
	for( int i=0; i<count_a; i++ )
		std::cout<< "[" << i <<  "]" << angles[i] <<std::flush;
	std::cout<< "***move array["  << count_m << "]" <<std::flush;
	for( int i=0; i<count_m; i++ )
		std::cout<< "[" << i <<  "]" << ofx[i] << "," << ofy[i] <<":" <<std::flush;
	
#endif

	/*/
	std::partial_sort(scales, scales + count_s/2, scales + count_s);
	*scale = scales[(count_s-1)/2];
	std::partial_sort(ofx, ofx + count_m/2, ofx + count_m);
	moved->x = ofx[(count_m-1)/2];
	std::partial_sort(ofy, ofy + count_m/2, ofy + count_m);
	moved->y = ofy[(count_m-1)/2];
	*/

	delete[] scales;
	delete[] angles;
	delete[] ofx;
	delete[] ofy;

	return count_m;
}

void ys::DelayTracker::predictPosition( Square &position )
{
	int center_x = 0;
	int center_y = 0;

	for( int i=0; i<4; i++ ){
		center_x += _initial_position[i].x;
		center_y += _initial_position[i].y;
	}
	center_x = center_x/4;
	center_y = center_y/4;

	for( int i=0; i<4; i++ ){
		int c_x = _initial_position[i].x - center_x;
		int c_y = _initial_position[i].y - center_y;

		Point &p = position[i];
		p.x = _scale * (c_x * cos(_angle) - c_y * sin(_angle)) + center_x + _move.x;
		p.y = _scale * (c_x * sin(_angle) + c_y * cos(_angle)) + center_y + _move.y;
	}

}

