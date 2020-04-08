# Mongoose OS driver for AS7262

This is a driver for AMS [AS7262](docs/AS7262_DS000486_2-00.pdf).

## Sensor details

The AS7262 is a cost-effective multi-spectral sensor-on-chip
solution designed to address spectral ID applications. This
highly integrated device delivers 6-channel multi-spectral
sensing in the visible wavelengths from approximately 430nm
to 670nm with full-width half-max (FWHM) of 40nm. An
integrated LED driver with programmable current is provided
for electronic shutter applications.

The AS7262 integrates Gaussian filters into standard CMOS
silicon via nano-optic deposited interference filter technology
and is packaged in an LGA package that provides a built in
aperture to control the light entering the sensor array.

## Example application

```
#include <math.h>
#include "mgos.h"
#include "mgos_config.h"
#include "mgos_as7262.h"

static void timer(void *user_data) {
  struct mgos_as7262 *d = (struct mgos_as7262 *)user_data;
  float v=NAN, b=NAN, g=NAN, y=NAN, o=NAN, r=NAN;
  if (!d) return;

  mgos_as7262_get_channel(d, CHAN_VIOLET, &v);
  mgos_as7262_get_channel(d, CHAN_BLUE, &b);
  mgos_as7262_get_channel(d, CHAN_GREEN, &g);
  mgos_as7262_get_channel(d, CHAN_YELLOW, &y);
  mgos_as7262_get_channel(d, CHAN_ORANGE, &o);
  mgos_as7262_get_channel(d, CHAN_RED, &r);
  LOG(LL_INFO, ("V=%f B=%f G=%f Y=%f O=%f R=%f", v, b, g, y, o, r));
}


enum mgos_app_init_result mgos_app_init(void) {
  struct mgos_as7262 *d;
  if (!(d = mgos_as7262_create(mgos_i2c_get_global(), NULL))) {
    LOG(LL_ERROR, ("Could not create AS7262"));
    return MGOS_APP_INIT_ERROR;
  }

  mgos_set_timer(1000, MGOS_TIMER_REPEAT, timer, d);
  return MGOS_APP_INIT_SUCCESS;
}
```
