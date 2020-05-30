
#include "cfg_file.h"
#include <FS.h>

/*
  Check file header and file version

  f                      file to read
  example                reference to string with 2 bytes of the file
                         signature
  returns 0 on error, otherwise - version
*/
uint8_t check_leader(File &f, char *example)
{
  DEBUG_PRINT("Check leader record"); DEBUG_PRINTLN(example);
  char lead_record[3];
  if (f.readBytes(lead_record, 3) != 3)
    return 0;
  if (lead_record[0] != example[0] ||
      lead_record[1] != example[1])
    return 0;
  return lead_record[2];
}

/*
  Reads string from configuration file

  f                      file to read
  max_len                maximum possible length of the string

  returns allocated string or 0 in case of error
*/
char *read_cfg_string(File &f, uint8_t max_len)
{
  char *str;
  uint8_t len;
  if (f.readBytes((char*)&len, 1) != 1 || len > max_len)
    return NULL;
  if ((str= (char*)malloc(len+1)) == NULL)
    return NULL;
  if (f.readBytes(str, len) != len)
  {
    free(str); str= NULL;
    return NULL;
  }
  str[len]= '\0';
  return str;
}

bool write_cfg_string(File &f, const char *str)
{
  DEBUG_PRINTLN("write_cfg_string");
  uint8_t len= strlen(str);
  DEBUG_PRINTLN(len);
  if (f.write((const byte*)&len, 1)!= 1)
    return false;
  DEBUG_PRINTLN("len written");
  if (f.write((const byte*)str, len)!= len)
    return false;
  DEBUG_PRINTLN("write OK");
  return true;
}

bool read_cfg_uint16(File &f, uint16_t &res)
{
  // we do not care about arhitecture byte order
  if ((f.readBytes((char*)&res, 2) == 2))
    return true;
  res= 0;
  return false;
}

bool write_cfg_uint16(File &f, uint16_t res)
{
  // we do not care about arhitecture byte order
  return (f.write((const byte*)&res, 2) == 2);
}

bool read_cfg_int16(File &f, int16_t &res)
{
  // we do not care about arhitecture byte order
  if ((f.readBytes((char*)&res, 2) == 2))
    return true;
  res= 0;
  return false;
}

bool write_cfg_int16(File &f, int16_t res)
{
  // we do not care about arhitecture byte order
  return (f.write((const byte*)&res, 2) == 2);
}

bool read_cfg_bytes(File &f, uint8_t *res, uint8_t len)
{
  if ((f.readBytes((char*)res, len) == len))
    return true;
  res= 0;
  return false;
}

bool write_cfg_bytes(File &f, uint8_t *res, uint8_t len)
{
  return (f.write((const byte*)res, len) == len);
}
