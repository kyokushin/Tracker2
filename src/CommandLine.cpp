#include <iostream>
#include <boost/program_options.hpp>
#include "CommandLine.h"
#include "usegui.h"

using namespace boost::program_options;
using namespace std;

int ys::commandLine( int argc, char **argv, ys::Params &params ){

	options_description option_input_source("input source options");
	options_description option_other("other options");
	//options_description option_tracker("tracker's options");
	options_description option_position("start position options for tracker(must specify)");
	
	option_input_source.add_options()
#ifdef USE_GUI
		("source,s", value<string>(&params.source_name)->default_value("camera"), 
		 "input source. source=<camera,video,images>.\n when select \"images\" or \"video\", must specify \"filename\" option. ")
#else
		("source,s", value<string>(&params.source_name)->default_value("video"), 
		 "input source. source=<camera,video,images>.\n when select \"images\" or \"video\", must specify \"filename\" option. ")
#endif
		("filename,f", value<string>(&params.filename), 
		 "set file name when select source option as \"video\" or \"images\".")
		;

	option_other.add_options()
		("help,h","show help")
		;

	/*
	option_tracker.add_options()
		("nfeature,n", value<int>(&params.n_feature)->default_value(100), "extract features num")
		;
		*/
	
	option_position.add_options()
		("sx", value<int>(&params.sx), "start x of rect")
		("sy", value<int>(&params.sy), "start y of rect")
		("ex", value<int>(&params.ex), "end x of rect")
		("ey", value<int>(&params.ey), "end y of rect")
		;

	option_input_source
		.add(option_other)
		//.add(option_tracker)
		.add(option_position)
		;

	variables_map values;

	try {
		store(parse_command_line(argc, argv, option_input_source), values);

		notify(values);

		if( values.count("help") 
#ifndef USE_GUI
				|| !values.count("sx") || !values.count("sy") 
				|| !values.count("ex") || !values.count("ey")
#endif
				){

#ifdef USE_GUI
			cerr<< "***** GUI mode *****" <<endl;
#else
			cerr<< "***** CUI mode *****" <<endl;
#endif
			cerr<< option_input_source <<endl;
			cerr<< "when select source as \"video\" or \"images\", "
				<< "you must specify \"filename\" option."
				<< endl
				<< "\"video\" :  specify video file name."
				<< endl
				<< "\"images\" : specify directory name(include image file )"
				<< " or file name(1 line 1 file name)."
				<<endl;
			return 0;
		}
		if( params.source_name == "images" 
			   	&& params.filename == "" ){
			cerr<< "empty \"filename\" options." 
				<<endl
				<< "must specify \"filename\" option" 
				<<endl;
			return 0;
		}
		if( params.source_name == "video"
				&& params.filename == "" ){
			cerr<< "empty \"filename\" option." <<endl
				<< "must specify vidname when select source as \"video\"." <<endl;
			return 0;
		}
		if( params.source_name == "camera"
				&& params.filename == "" ){
			params.filename = "0";
		}
	}
	catch( exception &e ){
		cerr<< "unknown option exception : " << e.what() <<endl;
	}
	
	return 1;
}
