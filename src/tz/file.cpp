struct mapped_file_t
{
  void *data ;
  size_t length ;
  bool unmap ;
  mapped_file_t(const std::string &path) ;
  mapped_file_t(void *data_pointer, size_t len) ;
 ~mapped_file_t() ;

  uint8_t unsigned_byte(size_t offset) ;
  uint16_t unsigned_short(size_t offset) ;
  uint32_t unsigned_word(size_t offset) ;
  uint64_t unsigned_long(size_t offset) ;
  int8_t signed_byte(size_t offset) ;
  int16_t signed_short(size_t offset) ;
  int32_t signed_word(size_t offset) ;
  int64_t signed_long(size_t offset) ;

  void throw_exception(const char *fmt, ...) __attribute__((format(printf, 2, 3))) ;
}
