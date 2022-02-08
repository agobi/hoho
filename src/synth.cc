#include "synth.h"

#include <cmath>
#include <cassert>

#include <algorithm>
#include <memory>
#include <queue>


namespace synth {


int Player::run(double (*buffer)) {
  std::for_each(_runners.begin(), _runners.end(), [](std::unique_ptr<Runner> &r) {r->run();});

  auto b = buffer;
  for(size_t i = 0; i < _buffer_size; ++i) {
    *b = 0;
    for(auto j = _left.begin(); j != _left.end(); ++j) *b += (*j)[i];
    *(++b) = 0;
    for(auto j = _right.begin(); j != _right.end(); ++j) *b += (*j)[i];
    ++b;
  }

  return 0;
}

void Patch::connect(
  const std::string &source_module_name,
  const std::string &source_output_name,
  const std::string &sink_module_name,
  const std::string &sink_input_name
) {

  auto source_module = _module_names.find(source_module_name);
  assert(source_module != _module_names.end());
  auto source_port_id = _modules[source_module->second] -> output_id(source_output_name);
  assert(source_port_id >= 0);

  port source = std::make_pair(source_module->second, source_port_id);

  if (sink_module_name == "master") {
    if(sink_input_name == "left") {
      _master_left.insert(source);
    } else if (sink_input_name == "right") {
      _master_right.insert(source);
    } else {
      assert(false);
    }
  } else {
    auto sink_module = _module_names.find(sink_module_name);
    assert(sink_module != _module_names.end());
    auto sink_port_id = _modules[sink_module->second]->input_id(sink_input_name);
    assert(sink_port_id >= 0);

    auto r = _input_map[sink_module->second].insert(std::make_pair(sink_port_id, source));
    assert(r.second);
  }
}

void Patch::init_player(Player &old_p, size_t sample_rate, size_t buffer_size) const {
  // Sort the input
  std::vector<std::vector<int>> graph(_modules.size());
  std::vector<int> in_degree(_modules.size());
  std::vector<int> map(_modules.size());

  Player p;
  p._sample_rate = sample_rate;
  p._buffer_size = buffer_size;

  // create modules in topological order
  for(size_t i=0; i<_modules.size(); ++i) {
    std::set<int> sources;

    for(auto &c : _input_map[i]) {
      sources.insert(c.second.first);
    }

    for(auto &c : sources) {
      ++in_degree[i];
      graph[c].push_back(i);
      std::cout<<c<<" -> "<<i<<std::endl;
    }
  }

  std::queue<int> q;
  for(size_t i=0; i<_modules.size(); ++i) {
    if(in_degree[i] == 0) q.push(i);
  }

  size_t count = 0;
  while(!q.empty()) {
    int node = q.front();
    q.pop();

    // create module
    map[node] = p._runners.size();
    auto current = _modules[node]->init(sample_rate, buffer_size);
    for (auto &c : _input_map[node]) {
      current->set_input(c.first, p._runners[map[c.second.first]]->output(c.second.second));
    }
    p._runners.push_back(std::move(current));

    for(auto i : graph[node]) {
      std::cout<<i<<std::endl;
      if(--in_degree[i] == 0)
        q.push(i);
    }

    ++count;
  }

  for(auto &c : _master_left) {
    p._left.push_back(p._runners[map[c.first]]->output(c.second));
  }

  for(auto &c : _master_right) {
    p._right.push_back(p._runners[map[c.first]]->output(c.second));
  }

  p.swap(old_p);

  assert(count == _modules.size());
}

}
