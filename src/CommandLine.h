#ifndef CommandLine_h
#define CommandLine_h

#include<string>

namespace ys{
	struct Params {
		//映像入力ソースの名前
		//camera,video,imagesのうちの１つ
		std::string source_name;

		//入力ソースへのオプション
		//camera：デバイス番号を指定（デフォルトで0になっている）
		//video：動画ファイル名を指定
		//images:1行１画像ファイルがかかれたファイル名
		std::string filename;

		int sx,sy,ex,ey;//初期の矩形位置
	};

	int commandLine( int argc, char **argv, Params &params );
}
#endif
