#ifndef __READFILE_H__
#define __READFILE_H__

#include <istream>
#include <memory>
#include <cmath>

#include "synth.h"

namespace synth {

class VCO: public Module {
  class Runner: public synth::Runner {
    size_t _sample_rate, _buffer_size;
    std::unique_ptr<double[]> _saw;
    std::unique_ptr<double[]> _sine;
    double _saw_lastvalue, _sine_phase;

  public:
    Runner(size_t sample_rate, size_t buffer_size):
      _sample_rate(sample_rate),
      _buffer_size(buffer_size),
      _saw(std::make_unique<double[]>(_buffer_size)),
      _sine(std::make_unique<double[]>(_buffer_size))
    {}

    ~Runner() {
    }

    virtual void run() {
      for(size_t i=0; i<_buffer_size; ++i) {
        _saw[i] = _saw_lastvalue;
        _saw_lastvalue += 2.0 * (440.0 / _sample_rate);
        if(_saw_lastvalue >= 1.0) _saw_lastvalue -= 2;

        _sine[i] = sin(_sine_phase);
        _sine_phase += 2.0* 440.0 * M_PI / _sample_rate;
        if(_sine_phase >= 2.0 * M_PI) _sine_phase -= 2.0 * M_PI;
      }

    };

    virtual double * output(size_t n) {
      switch(n) {
        case SAW_OUTPUT:
          return _saw.get();
        case SINE_OUTPUT:
          return _sine.get();
        default:
          return 0;
      }
    }

    virtual void set_input(size_t, double *) {}
  };

  static constexpr size_t SAW_OUTPUT = 0;
  static constexpr size_t SINE_OUTPUT = 1;

public:
  VCO() {}
  virtual std::unique_ptr<synth::Runner> init(size_t sample_rate, size_t buffer_size) const  {
    return std::move(std::make_unique<VCO::Runner>(sample_rate, buffer_size));
  }

  virtual int input_id(const std::string &) const { return -1; }
  virtual int output_id(const std::string &input) const {
    if (input == "saw") return SAW_OUTPUT;
    else if (input == "sine") return SINE_OUTPUT;
    else return -1;
  }
};


void read_file(std::istream &, Patch &p) {
  p.add_module("vco1", std::make_unique<VCO>());
  p.connect("vco1", "saw", "master", "right");
  p.connect("vco1", "sine", "master", "left");
}

}

#endif// __READFILE_H__
