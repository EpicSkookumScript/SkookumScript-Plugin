// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Checksum declaration header
//   - generates checksums using the CRC (cyclic redundancy check) algorithm.
//   
//
// A nice property of using CRCs to generate a checksum is that since it has the same
// number of internal state bits as is returned it supports successive calls.  Not all
// checksum algorithms have this property - i.e. MD5, SHA, etc. all use an internal state
// that is usually 160-256 bits but they usually output a hash of 128-bits.  This means
// that the 'prev_crc' technique used here would not fully replicate the internal state
// of the previous checksum operation needed to append more data.
//
// Usage:
//
// 'prev_crc' allows you to append text to a previous calculated CRC checksum:
//
//    uint32_t crc_full  = AChecksum::generate_crc32_cstr("Left_Right");
//    uint32_t crc_parts = AChecksum::generate_crc32_cstr("Left_");
//         crc_parts = AChecksum::generate_crc32_cstr("Right", crc_parts);
//    
// 'crc_parts' is now equal to 'crc_full'
//    
// This can also be useful if naming convention that works with this property is used.
// For example, if the CRC checksum is used as a hash and the same root is used by
// different model names - the base hash can be stored and used to generate the hashes of
// derivative names quickly.  [Note however that ASymbol objects are usually preferable
// instead of hashes - especially when used in conjunction with tools.]
//    
//    uint32_t base_hash   = AChecksum::generate_crc32_cstr("ModelName");
//    
//    uint32_t hash_modela = AChecksum::generate_crc32_cstr("_A", base_hash);    Same as "ModelName_A"
//    uint32_t hash_modelb = AChecksum::generate_crc32_cstr("_B", base_hash);    Same as "ModelName_B"
//    uint32_t hash_modelc = AChecksum::generate_crc32_cstr("_C", base_hash);    Same as "ModelName_C"
//
// References:
//
//    http://en.wikipedia.org/wiki/Cyclic_redundancy_check
//    32-Bit - gnu textutils-2.1  src\cksum.c
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
class A_API AChecksum
  {
  public:

  // Class Methods
    
    static uint32_t generate_crc32(const void * data_p, uint32_t data_num_bytes, uint32_t prev_crc = 0);
    static uint32_t generate_crc32(const AString & str, uint32_t prev_crc = 0);
    static uint32_t generate_crc32_cstr(const char * cstr_p, uint32_t length = ALength_calculate, uint32_t prev_crc = 0);
    static uint32_t generate_crc32_uint8(uint8_t data, uint32_t prev_crc = 0)    { return generate_crc32(&data, sizeof(data), prev_crc); }
    static uint32_t generate_crc32_uint16(uint16_t data, uint32_t prev_crc = 0)  { return generate_crc32(&data, sizeof(data), prev_crc); }
    static uint32_t generate_crc32_uint32(uint32_t data, uint32_t prev_crc = 0)  { return generate_crc32(&data, sizeof(data), prev_crc); }

    static uint32_t generate_crc32_upper(const AString & str, uint32_t prev_crc = 0);
    static uint32_t generate_crc32_cstr_upper(const char * cstr_p, uint32_t length = ALength_calculate, uint32_t prev_crc = 0);

    static uint64_t generate_crc64(const void * data_p, uint32_t data_num_bytes, uint64_t prev_crc = 0);
    static uint64_t generate_crc64(const AString & str, uint64_t prev_crc = 0);
    static uint64_t generate_crc64_cstr(const char * cstr_p, uint32_t length = ALength_calculate, uint64_t prev_crc = 0);

  };
