/*
  Configuration files
*/

#ifndef SWSENS_CFG_FILE_H
#define SWSENS_CFG_FILE_H

#include <FS.h>

uint8_t check_leader(File &f, char *example);

char *read_cfg_string(File &f, uint8_t max_len);
bool write_cfg_string(File &f, const char *str);

bool read_cfg_uint16(File &f, uint16_t &res);
bool write_cfg_uint16(File &f, uint16_t res);

bool read_cfg_int16(File &f, int16_t &res);
bool write_cfg_int16(File &f, int16_t res);

bool read_cfg_bytes(File &f, uint8_t *res, uint8_t len);
bool write_cfg_bytes(File &f, uint8_t *res, uint8_t len);

#endif SWSENS_CFG_FILE_H
