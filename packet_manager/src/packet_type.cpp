#include "packet_type.hpp"

namespace vsa {
  
  PacketType::PacketType(Value value) : m_value(value) {}
    
  int PacketType::asInt() const {
    return m_value;
  }

  PacketType PacketType::fromInt(int type) {
    return PacketType(static_cast<Value>(type));
  }

  bool PacketType::operator==(const Value& other) const {
    return m_value == other;
  }
}
