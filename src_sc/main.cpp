#include "top.h"

int sc_main(int argc, char* argv[]) 
{
  cout << "debug: sc_main: pre" << endl;
  top testbench( "top_module" );
  cout << "debug: sc_main: pos" << endl;
  sc_start();
  testbench.dump_cycle_count();
  return 0;
}
