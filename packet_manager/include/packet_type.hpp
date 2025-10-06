

namespace vsa {
  
  class PacketType {
  public:
    enum Value {
      UserInfo,
      MessageInfo,
      MessageChat,
      FileDataHeader,
      FileDataChunk,
      FileDataEnd,
      FileUploadList,
      FileDownloadRequest,
      FileDeleteRequest,
      UpdateWriteRate
    };
    
  private: 
    Value m_value;

  public:
    PacketType(Value value);
    
    int asInt() const; 
    static PacketType fromInt(int type);
    bool operator==(const Value& other) const;
  };
 
}
