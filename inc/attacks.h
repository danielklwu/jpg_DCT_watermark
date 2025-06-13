#ifndef ATTACKS_H
#define ATTACKS_H

#include "image.h"

// Attack functions
Image* attack_noise(Image* img, int noise_level);
Image* attack_quality(Image* img, int quality);

#endif /* ATTACKS_H */
