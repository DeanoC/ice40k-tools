#include <cstdio>
#include "docopt.h"

#include <iostream>
#include <fstream>
#include <array>
#include <iterator>
#include <iomanip>
#include <cmath>
static const char USAGE[] =
R"(iCE40K Tools.
	Generic tool for pre-processing stuff for the iCE40K system

	Usage:
	  ice40ktool mkrom <infile> <outfile> 
	  ice40ktool (-h | --help)
	  ice40ktool --version

	Options:
	  -h --help     Show this screen.
	  --version     Show version.
)";

static const char VERSION[] = "ice40ktool 0.0.0";

int main( int argc, char** argv )
{
	std::map<std::string, docopt::value> args
		= docopt::docopt(USAGE,
						 { argv + 1, argv + argc },
						 true,               // show help if requested
						 VERSION );  // version string

	for(auto const& arg : args) {
		std::cout << arg.first << " = " << arg.second << std::endl;
	}
	if( args["mkrom"].isBool() && args["mkrom"].asBool() == true ) {
		if( args["<infile>"].isString() == true && args["<outfile>"].isString() == true)
		{
			std::ifstream infile( args["<infile>"].asString(), std::ios_base::binary );
			std::vector<char> binData;
			while( infile )
			{
				std::array<char,1024> binSector;
				infile.read( &binSector[0], 1024 );
				auto bsend = binSector.begin();
				std::advance( bsend, infile.gcount() );
				binData.insert( binData.end(), binSector.begin(), bsend );
			}

			const auto outname = args["<outfile>"].asString();
			const auto modname = outname.substr( 0, outname.find_last_of(".") );

			std::ofstream outfile( args["<outfile>"].asString(), std::ios_base::trunc );
			const auto numRoms = (binData.size()+(512-1))/512;
			outfile << "module " <<  modname << "(\n";
			outfile << "\tinput clk,\n";
			outfile << "\tinput [addr_width-1:0] addr,\n";
			outfile << "\toutput [7:0] dout);\n";
			outfile << "\tparameter addr_width = " << (int)std::log2(numRoms)+8 << ";\n";
			outfile << "\treg [7:0] dout;\n";
			for( auto i = 0;i < numRoms; ++i ) {
				outfile << "\treg [7:0] dout_rom" << i << ";\n";
				outfile << "SB_ROM512x8 #(\n";
				outfile << std::hex << std::internal << std::uppercase;
				for( auto k = 0; k < 16; ++k ) {
					outfile << ".INIT_" << k << "(256'h";
					for( auto j = 0; j < 16/4;++j) 
					{ 
						uint32_t data = ((unsigned int)binData[(i*512)+(j*4)+(k*16)+0] & 0xFF) << 24 |
										((unsigned int)binData[(i*512)+(j*4)+(k*16)+1] & 0xFF) << 16 |
										((unsigned int)binData[(i*512)+(j*4)+(k*16)+2] & 0xFF) << 8 |
										((unsigned int)binData[(i*512)+(j*4)+(k*16)+3] & 0xFF) << 0;
						outfile << std::uppercase << std::setfill('0') << std::setw(8) << data; 
					}
					outfile << ")";
					if( k != 16-1 ) { outfile << ",\n"; }
				}
				outfile << std::dec;

				outfile << "\n) _rom" << i << " (\n";
				outfile << ".RDATA(dout_rom" << i << "),\n";
				outfile << ".RADDR(addr[8:0]),\n";
				outfile << ".RCLK(clk), .RCLKE(1'b1), .RE(1'b1)\n"; 
				outfile << ");\n";
			}

			outfile << "always @(posedge clk) begin\n";
			outfile << "\tdout <= dout_rom0;\n";// TODO Mux
			outfile << "end\n";
			outfile << "endmodule\n";
		} else
		{
			std::cout << "Error: Usage should be ice40ktool mkrom <in file> <out file>\n";
		}
	} else
	{
		std::cout << "Unknown tool command\n";
	}
}