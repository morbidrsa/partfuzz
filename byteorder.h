#ifndef BYTEORDER_H_INCLUDED
#define BYTEORDER_H_INCLUDED

#ifdef __linux__
#  include <endian.h>
#  include <byteswap.h>
#else
#  error unsupported
#endif

#if BYTE_ORDER == LITTLE_ENDIAN
#  define le16_to_cpu(x) (x)
#  define be16_to_cpu(x) bswap_16(x)
#  define le32_to_cpu(x) (x)
#  define le64_to_cpu(x) (x)
#  define be32_to_cpu(x) bswap_32(x)
#  define be64_to_cpu(x) bswap_64(x)
#  define cpu_to_le16(x) (x)
#  define cpu_to_be16(x) bswap_16(x)
#  define cpu_to_le32(x) (x)
#  define cpu_to_be32(x) bswap_32(x)
#  define cpu_to_le64(x) (x)
#  define cpu_to_be64(x) bswap_64(x)
#elif BYTE_ORDER == BIG_ENDIAN
#  define le16_to_cpu(x) bswap_16(x)
#  define be16_to_cpu(x) (x)
#  define le32_to_cpu(x) bswap_32(x)
#  define le64_to_cpu(x) bswap_64(x)
#  define be32_to_cpu(x) (x)
#  define be64_to_cpu(x) (x)
#  define cpu_to_le16(x) bswap_16(x)
#  define cpu_to_be16(x) (x)
#  define cpu_to_le32(x) bswap_32(x)
#  define cpu_to_be32(x) (x)
#  define cpu_to_le64(x) bswap_64(x)
#  define cpu_to_be64(x) (x)
#else
#  error unsupported
#endif

#endif				/* BYTEORDER_H_INCLUDED */
