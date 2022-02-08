#ifndef SYNTH_H
#define SYNTH_H


#include <iostream>
#include <tuple>
#include <map>
#include <memory>
#include <algorithm>
#include <functional>
#include <set>
#include <vector>

#include <cassert>

namespace synth {

extern const char note_off;
struct Note {
  char key; // 0-127: keys
};

class Runner {
protected:
  Runner() {}
public:
  Runner(const Runner &rhs) = delete;
  virtual ~Runner() {};
  virtual void run() = 0;
  virtual double * output(size_t n) = 0;
  virtual void set_input(size_t n, double *output) = 0;
};


class Module {
protected:
  Module() {}
public:

  Module(const Module &rhs) = delete;
  virtual ~Module() {}
  virtual std::unique_ptr<Runner> init(size_t sample_rate, size_t buffer_size) const = 0;
  virtual int input_id(const std::string &input) const = 0;
  virtual int output_id(const std::string &input) const = 0;
};


class Patch;

class Player {
  friend class Patch;

  size_t _sample_rate;
  size_t _buffer_size;
  std::vector<std::unique_ptr<Runner>> _runners;
  std::vector<double *> _left;
  std::vector<double *> _right;


public:
  Player() {}
  Player(const Player& s) = delete;
  Player(Player && s);

  int run(double *output);
  void swap(Player &rhs) {
    std::swap(_sample_rate, rhs._sample_rate);
    std::swap(_buffer_size, rhs._buffer_size);
    std::swap(_runners, rhs._runners);
    std::swap(_left, rhs._left);
    std::swap(_right, rhs._right);
  }

};


class Patch {
public:
  Patch() { }

  Patch(const Patch& s) = delete;

  void clear() {
    _module_names.clear();
    _modules.clear();
    _input_map.clear();
    _master_left.clear();
    _master_right.clear();
  }

  void add_module(const std::string &name, std::unique_ptr<Module> && module) {
    _module_names[name] = _modules.size();
    _modules.push_back(std::move(module));
    _input_map.push_back(std::map<int, port>());
  }

  void connect(
    const std::string &source_module_name,
    const std::string &source_output_name,
    const std::string &sink_module_name,
    const std::string &sink_input_name
  );


  void init_player(Player &player, size_t sample_rate, size_t buffer_size) const;

private:
  typedef std::pair<int, int> port;

  std::map<std::string, size_t> _module_names;
  std::vector<std::unique_ptr<Module>> _modules;
  std::vector<std::map<int, port>> _input_map;
  std::set<port> _master_left;
  std::set<port> _master_right;
};


} // namespace synth

#endif//SYNTH_H
