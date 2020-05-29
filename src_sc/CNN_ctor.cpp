#include "CNN.h"

void CNN::def_CNN(){
add_conv_layer(24, 3, 3, true, 1, true, 5, "layerC0", Parallel_type::pt_2XY);
add_conv_layer(24, 3, 3, true, 2, true, 8, "layerP1", Parallel_type::pt_2XYm);
add_conv_layer(48, 3, 3, true, 1, true, 8, "layerC2", Parallel_type::none);
add_conv_layer(48, 3, 3, true, 2, true, 9, "layerP3", Parallel_type::pt_2XYS);
add_conv_layer(48, 3, 3, true, 1, true, 9, "layerC4", Parallel_type::none);
add_conv_layer(10, 1, 1, true, 1, true, 6, "layerC5", Parallel_type::none);
add_avgpooling_layer(6, "layer6");
}
