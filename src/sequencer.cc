
class Sequence: public Module {
  size_t _channels;
  std::vector<std::vector<Note>> _patterns;
  std::vector<int> _sequence;
public:

  Sequence(size_t channels):
    _channels(channels)
  {  }

  Runner init(size_t sample_rate, size_t buffer_size);
};


