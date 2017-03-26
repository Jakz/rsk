#include "base/common.h"

namespace iso9660
{
  using uint8 = uint8_t;
  using sint8 = int8_t;
  using uint16_lsb = uint16_t;
  using uint16_msb = uint16_t;
  using sint16_lsb = int16_t;
  using sint16_msb = int16_t;
  using uint32_lsb = uint32_t;
  using uint32_msb = uint32_t;
  using sint32_lsb = int32_t;
  using sint32_msb = int32_t;
  using padding = uint8;
  using file_flags = uint8;
  
  enum volume_descriptor_type : uint8
  {
    TYPE_BOOT_RECORD = 0,
    TYPE_PRIMARY = 1,
    TYPE_SUPPLEMENTARY = 2,
    TYPE_PARTITION = 3,
    TYPE_TERMINATOR = 0xFF
  };
  
  struct uint16_lsb_msb
  {
    uint16_lsb lsb;
    uint16_msb msb;
  };
  
  struct sint16_lsb_msb
  {
    sint16_lsb lsb;
    sint16_msb msb;
  };
  
  struct uint32_lsb_msb
  {
    uint32_lsb lsb;
    uint32_msb msb;
  };
  
  struct sint32_lsb_msb
  {
    sint32_lsb lsb;
    sint32_msb msb;
  };
  
  using strD = uint8;
  using strA = uint8;
  
  constexpr size_t sector_size = 2048;

  
  struct date
  {
    strD year[4];
    strD month[2];
    strD day[2];
    strD hour[2];
    strD minute[2];
    strD second[2];
    strD hundreth[2];
    uint8 gmt;
  } __attribute__((packed));
  
  struct recording_date
  {
    uint8 years;
    uint8 month;
    uint8 day;
    uint8 hour;
    uint8 minute;
    uint8 second;
    uint8 gmt;
  } __attribute__((packed));
  
  struct directory_entry
  {
    uint8 length;
    uint8 extented_attribute_length;
    uint32_lsb_msb extent_location;
    uint32_lsb_msb extent_length;
    recording_date date;
    file_flags flags;
    uint8 interleved_unit_size;
    uint8 interleaved_gap_size;
    uint16_lsb_msb sequence_number;
    uint8 identifier_length;
  } __attribute__((packed));
  
  struct volume_descriptor_header
  {
    volume_descriptor_type type;
    strA identifier[5];
    uint8 version;
  } __attribute__((packed));

  struct descriptor_boot_record
  {
    strA system_ident[32];
    strA identifier[32];
    uint8 padding[1977];
  } __attribute__((packed));
  
  struct descriptor_primary_volume
  {
    padding unused1;
    strA system_ident[32];
    strD volume_ident[32];
    padding unused2[8];
    uint32_lsb_msb volume_space_size;
    padding unused3[32];
    uint16_lsb_msb volume_set_size;
    uint16_lsb_msb volume_seq_number;
    uint16_lsb_msb logical_sector_size;
    uint32_lsb_msb path_table_size;
    uint32_lsb location_lpath_table;
    uint32_lsb location_optional_lpath_table;
    uint32_msb location_mpath_table;
    uint32_msb location_optional_mpath_table;
    directory_entry root;
    padding unused4;
    strD volume_set_ident[128];
    strA publisher_ident[128];
    strA preparer_ident[128];
    strA application_identifier[128];
    strD copyright_file[38];
    strD abstract_file[36];
    strD biblio_file[37];
    date time_creation;
    date time_modification;
    date time_expiration;
    date time_effective;
    uint8 file_structure_version;
    padding unused5;
    uint8 custom_data[512];
    uint8 reserved_data[653];
    
  } __attribute__((packed));
  
  struct descriptor_terminator
  {
    padding unused[sector_size - sizeof(volume_descriptor_header)];
  };
  
  
  struct volume_descriptor
  {
    volume_descriptor_header header;
    union
    {
      descriptor_primary_volume primary_volume;
      descriptor_terminator terminator;
    };
  } __attribute__((packed));
  
  
  
}

using namespace iso9660;
void test()
{
  static_assert(sizeof(volume_descriptor_header) == 7, "");
  static_assert(sizeof(recording_date) == 7, "");
  static_assert(sizeof(directory_entry) == 33, "");

  
  static_assert(sizeof(volume_descriptor) == sector_size, "");
  static_assert(sizeof(descriptor_boot_record) == sector_size - sizeof(volume_descriptor_header), "");
  static_assert(sizeof(descriptor_primary_volume) == sector_size - sizeof(volume_descriptor_header), "");
  
  
  FILE* in = fopen("/Users/jack/Desktop/bomberman.iso", "rb");
  
  fseek(in, sector_size*0x10, SEEK_SET);
  
  volume_descriptor header;
  fread(&header, sizeof(volume_descriptor), 1, in);
  
  
  
  fclose(in);
}
