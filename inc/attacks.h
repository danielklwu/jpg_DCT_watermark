#ifndef ATTACKS_H
#define ATTACKS_H

#include "image.h"

// Attack functions
MyImage* attack_noise(MyImage* img, int noise_level);
MyImage* attack_quality(MyImage* img, int quality);

#endif /* ATTACKS_H */
