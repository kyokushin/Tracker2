#ifndef ImageSequence_h
#define ImageSequence_h

#include <string>

#include <cv.h>
#include <highgui.h>

namespace ys {

	class AbstractCapture {
		public:
			virtual ~AbstractCapture(){}

			virtual bool get( cv::Mat &image, int channel=0 ) = 0;
			virtual bool next() = 0;
			virtual int currentNum() = 0;

			virtual bool open(const std::string &filename) = 0;

			virtual bool isOpen() = 0;
			virtual void release() = 0;

			virtual std::string getSourceName() = 0;
			virtual std::string getFileName() = 0;

	};

	class VideoCapture : public AbstractCapture {
		public:
			VideoCapture()
				:_counter(0)
			{}
			virtual ~VideoCapture(){}

			virtual bool get( cv::Mat &image, int channel = 0 );
			virtual bool next();
			virtual int currentNum();

			virtual bool open(const std::string &filename );
			virtual bool isOpen();
			virtual void release();

			virtual std::string getSourceName();
			virtual std::string getFileName();

		protected:
			cv::VideoCapture _cap;
			std::string _filename;
			int _counter;
	};

	class CameraCapture : public VideoCapture {
		public:
			virtual bool open( const std::string &filename );
			virtual std::string getSourceName();
	};

	class ImageList : public AbstractCapture {
		public:


			ImageList()
				:_counter(0),_open(false)
			{}
			virtual ~ImageList(){}

			virtual bool get( cv::Mat &image, int channel = 0);
			virtual bool next();
			virtual int currentNum();

			virtual bool open(const std::string &filename);

			virtual bool isOpen();
			virtual void release();

			virtual std::string getSourceName();
			virtual std::string getFileName();

		private:
			std::vector<std::string> _image_list;
			std::string _filename;
			int _counter;
			bool _open;

			int _readDir2List(const std::string &name );

			int _readFile2List( const std::string &filename );
	};


	AbstractCapture* getCapture(std::string &type);

	class Capture {
		public:
			Capture( std::string &type );
			Capture( int type );
			~Capture();
			bool get( cv::Mat &image, int channel = 0);
			bool next();
			int currentNum();

			bool open(const std::string &filename);

			bool isOpen();
			//開いたファイルやカメラを開放する。
			void release();

			std::string getSourceName();
			
		private:
			AbstractCapture *_cap;
			AbstractCapture* _getSource(std::string &type);
	};

	class ImageProcessorInterface {
		public:
			virtual ~ImageProcessorInterface(){};
			virtual void init( const cv::Mat &image, const cv::Rect &rect ) = 0;
			virtual void init( const cv::Mat &image) = 0;
			virtual void stop() = 0;
			//処理内容を記述
			//引数より与えられた画像データは書き換え不可
			//処理結果を画像に書き加える時は、drawImageを使うこと
			virtual void processImage( const cv::Mat &image, const int count ) = 0;

			//処理結果を画像に書き加えるためのメソッド
			virtual void drawImage( cv::Mat &image ) = 0;
			//処理を終了する条件
			virtual bool stopCriteria() = 0;
			virtual std::string getClassName() = 0;
			//GUIモード時のcv::waitKeyからの入力を利用した処理
			//を書くためのメソッド
			//'s'と 0x1b は予約済み
			virtual void keySwitch( int key ) = 0;
	};

	class ImageSequence {
		public:
			ImageSequence();
			void setCaptureType(std::string type);
			void setSize( cv::Size &size );
			void setSize( int width, int height );
			void setInterval( int interval );
			void setWindowName( std::string &name );
			void showProgress( bool show );
			//処理内容を記述したImageProcessorInterfaceをぶち込む。
			//複数追加可。
			//外部で確保したImageProcessorInterfaceは外部で開放すること
			void setImageProcessor( ImageProcessorInterface *processor );
			void useGUI(bool use);

			void run();
			void runGuiMode();

		private:
			std::string _cap_type;
			ImageProcessorInterface* _processor;
			cv::Size _size;
			int _interval;
			bool _show_progress;
			int _count;
			bool _use_gui;
			std::string _window_name;

	};
}
#endif
