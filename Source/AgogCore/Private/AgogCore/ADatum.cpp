// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ADatum class - Objects that need to be persistent (saved and loaded), or converted to
// to a contiguous memory block form should have conversion methods to and from ADatum.
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp> // Always include AgogCore first (as some builds require a designated precompiled header)
#include <AgogCore/ADatum.hpp>
#ifdef A_INL_IN_CPP
  #include <AgogCore/ADatum.inl>
#endif
#include <AgogCore/AMath.hpp>

//---------------------------------------------------------------------------------------
// Author(s):  Conan Reis
void ADatum::Reference::decrement()
  {
  if (--m_references == 0u)
    {
    if (m_term == ATerm_short)
      {
      free_buffer(m_buffer_p);
      }

    delete this;
    }
  }


//=======================================================================================
// ADatum Class Data
//=======================================================================================

uint32_t ADatum::ms_bytes4 = ADatum::bytes4_init();


//=======================================================================================
// ADatum Method Definitions
//=======================================================================================
    
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Default Constructor
// 
// Returns:  itself
// Params: 
//   data_length: length in bytes of the data part of the data_p.
//   
// See:        Other ADatum constructors, ref_count_increment()
// Modifiers:  explicit
// Author(s):  Conan Reis
ADatum::ADatum(
  uint32_t data_length // = 0u
  )
  {
  m_dref_p = new ("ADatum::Reference")Reference(data_length);
  }

//---------------------------------------------------------------------------------------
// Default Constructor
// 
// Returns:  itself
// 
// Params: 
//   See "Notes" for the possible argument permutations and their interpretations.
//   
//   data_p:
//     pointer to data (excluding the size header) or a pointer to a buffer (which
//     includes space for the size header).  It may point to `nullptr` in which case no
//     data is copied.
//   data_length:
//     length in bytes of the data part of the `data_p`.  If `data_length` is
//     `ALength_in_header`, then `data_p` points to a complete buffer and contains a
//     correct length in its header (unless it is nullptr).
//   size:
//     If not `0` and `data_p` is not `nullptr`, `data_p` is a buffer of size bytes to be
//     taken over (and eventually freed when no longer used) rather than allocating a new
//     buffer and copying the data. The buffer is of the indicated size and contains
//     enough space for both the size header and any data.
//   term:
//     Indicates whether borrowed buffer should be deallocated by this object (ATerm_short)
//     or if it will be managed/owned (ATerm_long) elsewhere.  Only used if `data_p` is
//     not `nullptr` and `size` is not `0` (case #6 or #8 from Notes section).
//     
// Examples: 
//   ```
//   ADatum datum1(data_p, 7);  // allocate buffer (header_size + 7), set buffer_length to header_size + 7, and copy data_p to buffer
//   ADatum datum2(buffer_p, ALength_in_header, ADatum_header_size + 7)  // take buffer_p over and use it directly
//   ```
//
// See:  ADatum(data_length_pair_count, ...), ref_count_increment()
// 
// Notes: 
//   These are the possible permutations of the arguments and their interpretations:
//   ```
//                                  Buffer size
//   # data_p    data_length        or Data      Interpretation
//   1  nullptr   ALength_in_header  0   n/a     allocate(header_size) & buffer_length = header_size (Default constructor)
//   2 !nullptr   ALength_in_header  0   buffer  allocate(buffer_length) & copy buffer
//   3  nullptr  !ALength_in_header  0   n/a     allocate(length + header_size) & buffer_length = header_size + length
//   4 !nullptr  !ALength_in_header  0   data    allocate(length + header_size) & buffer_length = header_size + length & copy data
//   5  nullptr   ALength_in_header !0   n/a     allocate(size) & buffer_length = header_size
//   6 !nullptr   ALength_in_header !0   buffer  take over buffer
//   7  nullptr  !ALength_in_header !0   n/a     allocate(size) & buffer_length = header_size + length
//   8 !nullptr  !ALength_in_header !0   buffer  take over buffer & buffer_length = header_size + length
//   ```
//
// Modifiers:  explicit
// Author(s):  Conan Reis
ADatum::ADatum(
  const void * data_p,
  uint32_t     data_length, // = ALength_in_header
  uint32_t     size,        // = 0u
  eATerm       term         // = ATerm_short
  )
  {
  // The #N comments indicate the flow control for the various argument permutations as
  // specified by the "Notes" section above.

  if (!data_p && (data_length == ALength_in_header))  // #1, #5
    {
    data_length = 0;  // #1=#3, #5=#7
    }

  uint32_t full_length = data_length + ADatum_header_size;

  if (size)  // #6, #7, #8
    {
    m_dref_p = new ("ADatum::Reference")Reference(
      const_cast<void *>(data_p), size, data_p ? term : ATerm_short);

    if (data_length != ALength_in_header)  // #7, #8
      {
      // Set buffer length
	  A_BYTE_STREAM32(m_dref_p->m_buffer_p, &full_length);
      }
    }
  else  // #2, #3, #4
    {
    if (data_length != ALength_in_header)  // #3, #4
      {
      m_dref_p = new ("ADatum::Reference")Reference(
        nullptr, ADatum_header_size + data_length);
      // Set buffer length
	  A_BYTE_STREAM32(m_dref_p->m_buffer_p, &full_length);

      if (data_p)  // #4
        {
        // Copy data
        memcpy(m_dref_p->m_buffer_p + ADatum_header_size, data_p, data_length);
        }
      }
    else  // #2
      {
      m_dref_p = new ("ADatum::Reference")Reference(nullptr, A_BYTE_STREAM_UI32(data_p));
      // Copy buffer
      memcpy(m_dref_p->m_buffer_p, data_p, m_dref_p->m_size);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Variable length (data pointer - data_length) pair constructor.
// *** It does not do any sort of byte swapping ***
// 
// Returns:  itself
// Params: 
//   data_length_pair_count:
//     specifies the number of (data pointer - data_length) pairs that are to follow.
//     Note it is signed and 1xpair not 2xpair.
//   ...:
//     variable length (data pointer - data_length) pairs.  Data length in bytes.
//     
// Examples: 
//   `ADatum(3, data1_p, data1_length, &data2, data2_length, &number, sizeof(uint32_t));`
//   
// See:        ADatum(data_p, data_length, size) 
// Author(s):  Conan Reis
ADatum::ADatum(
  int32_t data_length_pair_count,
  ...  // (data pointer - data_length) pairs
  )
  {
  va_list  arg_array;
  int32_t  pair_pos = 0;
  uint32_t size     = ADatum_header_size;
  
  if (data_length_pair_count)
    {
    va_start(arg_array, data_length_pair_count);  // Initialize variable arguments

    for (; pair_pos < data_length_pair_count; pair_pos++)
      {
      va_arg(arg_array, const void *);  // Skip data pointer
      size += va_arg(arg_array, uint32_t);  // accumulate size
      }

    va_end(arg_array);  // Reset variable arguments
    }

  m_dref_p = new ("ADatum::Reference")Reference(nullptr, size);
  A_BYTE_STREAM32(m_dref_p->m_buffer_p, &size);

  if (data_length_pair_count)
    {
    const void * data_p;
    uint32_t     length;
    uint32_t     offset = ADatum_header_size;

    pair_pos = 0;
    va_start(arg_array, data_length_pair_count);  // Initialize variable arguments

    for (; pair_pos < data_length_pair_count; pair_pos++)
      {
      data_p = va_arg(arg_array, const void *);  // get data pointer
      length = va_arg(arg_array, uint32_t);          // get data length
      memcpy(m_dref_p->m_buffer_p + offset, data_p, length);
      offset += length;
      }

    va_end(arg_array);  // Reset variable arguments
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Accessor Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Sets the buffer after the ADatum has already been constructed.
// 
// Params: 
//   See "Notes" for the possible argument permutations and their interpretations.
//   
//   data_p:
//     pointer to data (excluding the size header) or a pointer to a buffer (which 
//     includes space for the size header).  It may point to `nullptr` in which case no
//     data is copied.
//   data_length:
//     length in bytes of the data part of `data_p`.  If `data_length` is
//     `ALength_in_header`, then `data_p` points to a complete buffer and contains a
//     correct length in its header (unless it is nullptr).
//   size:
//     If `size` is not `0` and `data_p` is not `nullptr`, `data_p` is a buffer of size
//     bytes to be taken over (and eventually freed when no longer used) rather than
//     allocating a new buffer and copying the data.  The buffer is of the indicated size
//     and contains enough space for both the size header and any data.
//     
// Examples: 
//   ```
//   ADatum datum1(data_p, 7);  // allocate buffer (header_size + 7), set buffer_length to header_size + 7, and copy data_p to buffer
//   ADatum datum2(buffer_p, ALength_in_header, ADatum_header_size + 7)  // take buffer_p over and use it directly
//   ```
//   
// Notes: 
//   These are the possible permutations of the arguments and their interpretations:
//   ```
//                              Buffer size
//   # data_p data_length         or Data    Interpretation
//   1  nullptr   ALength_in_header  0   n/a     allocate(header_size) & buffer_length = header_size (Default constructor)
//   2 !nullptr   ALength_in_header  0   buffer  allocate(buffer_length) & copy buffer
//   3  nullptr  !ALength_in_header  0   n/a     allocate(length + header_size) & buffer_length = header_size + length
//   4 !nullptr  !ALength_in_header  0   data    allocate(length + header_size) & buffer_length = header_size + length & copy data
//   5  nullptr   ALength_in_header !0   n/a     allocate(size) & buffer_length = header_size
//   6 !nullptr   ALength_in_header !0   buffer  take over buffer
//   7  nullptr  !ALength_in_header !0   n/a     allocate(size) & buffer_length = header_size + length
//   8 !nullptr  !ALength_in_header !0   buffer  take over buffer & buffer_length = header_size + length
//   ```
//
// See:        ADatum(data_length_pair_count, ...)
// Modifiers:  explicit
// Author(s):  Conan Reis
void ADatum::set_buffer(
  const void * data_p,      // = nullptr
  uint32_t     data_length, // = ALength_in_header
  uint32_t     size         // = 0u
  )
  {
  // The #N comments indicate the flow control for the various argument permutations as
  // specified by the "Notes" section above.

  if (m_dref_p)
    {
    // $Revisit - CReis Should keep it if this is the only ADatum referencing it
    m_dref_p->decrement();
    }

  if (!data_p && (data_length == ALength_in_header))  // #1, #5
    {
    data_length = 0;  // #1=#3, #5=#7
    }

  uint32_t full_length = data_length + ADatum_header_size;

  if (size)  // #6, #7, #8
    {
    m_dref_p = new ("ADatum::Reference")Reference(const_cast<void *>(data_p), size);

    if (data_length != ALength_in_header)  // #7, #8
      {
      // Set buffer length
      A_BYTE_STREAM32(m_dref_p->m_buffer_p, &full_length);
      }
    }

  else  // #2, #3, #4
    {
    if (data_length != ALength_in_header)  // #3, #4
      {
      m_dref_p = new ("ADatum::Reference")Reference(nullptr, ADatum_header_size + data_length);
      // Set buffer length
      A_BYTE_STREAM32(m_dref_p->m_buffer_p, &full_length);

      if (data_p)  // #4
        {
        // Copy data
        memcpy(m_dref_p->m_buffer_p + ADatum_header_size, data_p, data_length);
        }
      }
    else  // #2
      {
      m_dref_p = new ("ADatum::Reference")Reference(nullptr, A_BYTE_STREAM_UI32(data_p));
      // Copy buffer
      memcpy(m_dref_p->m_buffer_p, data_p, m_dref_p->m_size);
      }
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    
//---------------------------------------------------------------------------------------
// Ensures that this ADatum is the only ADatum object referring to its buffer, and then
// sets its data length to zero.
// 
// Author(s):  Conan Reis
void ADatum::empty()
  {
  if (m_dref_p->m_references != 1u)
    {
    Reference * dref_p = m_dref_p;

    m_dref_p = new ("ADatum::Reference")Reference(nullptr, dref_p->m_size);
    dref_p->decrement();
    }

  uint32_t full_length = ADatum_header_size;

  A_BYTE_STREAM32(m_dref_p->m_buffer_p, &full_length);
  }

//---------------------------------------------------------------------------------------
// Ensures that the current buffer has at least the specified `min_data_length` is
// allocated. Since `ensure_size()` is generally a prelude to modifying the data buffer,
// this method also ensures that this is the only object referring to its buffer.
// 
// Params: 
//   min_data_length:
//     minimum required data length in bytes (i.e. does not include the size header).
//   keep_data:
//     if a new buffer is allocated this indicates whether the current data should be
//     preserved (true) or if it is permissible for the data to be lost making the values
//     of the buffer - including the size header - undefined (false).  If `keep_data` is
//     true and `min_data_length` is less than the currently stored data and a new buffer
//     needs to be allocated due to this object sharing a buffer with other Datums then
//     only the `min_data_length` will be allocated and part of the data will be truncated.
//   min_expand:
//     if the size needs to be increased and `min_expand = true` then the new size should
//     be just large enough (i.e. `size = ADatum_header_size + min_data_length`), if
//     `min_expand = false` then allocates more space than is minimally needed (it doubles
//     the current amount until it is large enough) to allow for additional expansion in
//     the future without needing to reallocate.
//   
// Notes: 
//   Leaves data length uninitialized unless `keep_data` is set to true so
//   `set_data_length()` may need to be called.
//     
// Author(s):  Conan Reis
void ADatum::ensure_size(
  uint32_t min_data_length,
  bool     keep_data, // = true
  bool     min_expand // = true
  )
  {
  Reference * dref_p      = m_dref_p;
  uint32_t    size        = dref_p->m_size;
  uint32_t    new_size    = min_data_length + ADatum_header_size;
  uint32_t    data_length = 0u;

  if (keep_data)
    {
    // Includes size header
    data_length = A_BYTE_STREAM_UI32(dref_p->m_buffer_p);
    #ifdef A_BOUNDS_CHECK
      A_VERIFY(new_size >= data_length, "Tried to keep data during resize to smaller size!", AErrId_low_memory, ADatum);
    #endif
    }

  if (dref_p->m_references != 1u)
    {
    if (!min_expand)
      {
      new_size = calc_size(new_size, size);
      }

    m_dref_p = new ("ADatum::Reference")Reference(nullptr, new_size);

    if (keep_data)
      {
      memcpy(m_dref_p->m_buffer_p, dref_p->m_buffer_p, data_length);
      }

    dref_p->decrement();
    }
  else
    {
    if (size < new_size)
      {   
      if (!min_expand)
        {
        new_size = calc_size(new_size, size);
        }

      if (keep_data)
        {
        uint8_t * old_buffer_p = dref_p->m_buffer_p;

        dref_p->m_buffer_p = alloc_buffer(new_size);
        memcpy(dref_p->m_buffer_p, old_buffer_p, data_length);

        if (dref_p->m_term == ATerm_short)
          {
          free_buffer(old_buffer_p);
          }
        }
      else
        {
        // Delete before allocating more
        if (dref_p->m_term == ATerm_short)
          {
          free_buffer(dref_p->m_buffer_p);
          }

        dref_p->m_buffer_p = alloc_buffer(new_size);
        }

      dref_p->m_term = ATerm_short;
      dref_p->m_size = new_size;
      }
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Internal Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Size 4 in stream byte order used for default/empty ADatum objects referencing ms_bytes4.
// 
// Returns:    4 in Agog stream byte order (little endian)
// Modifiers:  static
// Author(s):  Conan Reis
uint32_t ADatum::bytes4_init()
  {
  uint32_t size = 4u;

  return A_BYTE_STREAM_UI32(&size);
  }

