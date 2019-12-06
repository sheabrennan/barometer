let barometer = {
  _crt: ffi('void *mgos_barometer_create_i2c(void *, int, int)'),
  _cls: ffi('void mgos_barometer_destroy(void *)'),
  _gc: ffi('int mgos_barometer_return_capabilities(void *)'),
  _gv: ffi('float mgos_barometer_return_spec(void *,int)'),
  _ht: ffi('bool mgos_barometer_has_thermometer(void *)'),
  _hb: ffi('bool mgos_barometer_has_barometer(void *)'),
  _hh: ffi('bool mgos_barometer_has_hygrometer(void *)'),
  _read: ffi('bool mgos_barometer_read(void *)'),
  

  BARO_NONE: 0,
  BARO_MPL115: 1,
  BARO_MPL3115: 2,
  BARO_BME280: 3, // Also BMP280
  BARO_MS5611: 4,

  ADDRESSES: [null, null, 0x60, null, null],

  create: function(i2cRef,type) {
    let obj = Object.create(barometer._proto);
    obj.barometer = barometer._crt(i2cRef, barometer.ADDRESSES[type], type);
  },

  _proto: {
    close: function() {
      return barometer._cls(this.barometer);
    },
    hasTemp: function() {
      return barometer._ht(this.barometer);
    },
    hasBaro: function() {
      return barometer._hb(this.barometer);
    },
    hasHygo: function() {
      return barometer._hh(this.barometer);
    },
    read: function() {
      if (barometer._read(this.barometer)) {
        let cap = barometer._gc(this.barometer);
        let ret = {
          pressure: null,
          temperature: null,
          humidity: null
        };
        //there's obviously a better way to do this
        //but i'm c dumb and just trying to finish
        // one...damn...project...ever
        if (cap & 0x04) {
          ret.humidity = barometer._gv(this.barometer,0x04)
        }
        if (cap & 0x02) {
          ret.temperature = barometer._gv(this.barometer,0x02)
        }
        if( cap & 0x01) {
          ret.pressure = barometer._gv(this.barometer,(0x01))
        }
        
        return ret;

      }
    }
  }
};
